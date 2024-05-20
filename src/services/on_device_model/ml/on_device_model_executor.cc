// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/on_device_model/ml/on_device_model_executor.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/check.h"
#include "base/compiler_specific.h"
#include "base/containers/unique_ptr_adapters.h"
#include "base/logging.h"
#include "base/memory/raw_ref.h"
#include "base/memory/scoped_refptr.h"
#include "base/metrics/field_trial_params.h"
#include "base/metrics/histogram_functions.h"
#include "base/numerics/safe_conversions.h"
#include "base/task/thread_pool.h"
#include "base/timer/elapsed_timer.h"
#include "components/optimization_guide/core/optimization_guide_features.h"
#include "services/on_device_model/ml/chrome_ml.h"
#include "services/on_device_model/ml/language_detector.h"
#include "services/on_device_model/public/mojom/on_device_model.mojom.h"
#include "services/on_device_model/public/mojom/on_device_model_service.mojom.h"

#if BUILDFLAG(IS_MAC)
#include "base/apple/foundation_util.h"
#endif

using on_device_model::mojom::LoadModelResult;

namespace ml {
namespace {

constexpr uint32_t kReserveTokensForSafety = 2;

const base::FeatureParam<int> kMaxTopK{
    &optimization_guide::features::kOptimizationGuideOnDeviceModel,
    "on_device_model_max_topk", 128};

const base::FeatureParam<bool> kPreferTextureWeights{
    &optimization_guide::features::kOptimizationGuideOnDeviceModel,
    "on_device_model_prefer_texture_weights", true};

const base::FeatureParam<bool> kEnableHostMappedPointer{
    &optimization_guide::features::kOptimizationGuideOnDeviceModel,
    "on_device_model_enable_host_mapped_pointer", true};

const base::FeatureParam<bool> kUseLowPower{
    &optimization_guide::features::kOptimizationGuideOnDeviceModel,
    "on_device_model_use_low_power", false};

const base::FeatureParam<bool> kAllowFp16{
    &optimization_guide::features::kOptimizationGuideOnDeviceModel,
    "on_device_model_allow_fp16", true};

// Helper to bind object methods as weak task-posting callback functions.
template <typename R, typename C, typename... Args>
std::function<R(Args...)> CreateWeakCallbackFn(R (C::*method)(Args...),
                                               C* that) {
  return [weak_ptr = that->AsWeakPtr(), method,
          task_runner =
              base::SequencedTaskRunner::GetCurrentDefault()](Args&&... args) {
    task_runner->PostTask(
        FROM_HERE,
        base::BindOnce(method, weak_ptr, std::forward<Args>(args)...));
  };
}

int CalculateTokensPerSecond(int num_tokens, base::TimeDelta duration) {
  if (duration.InMicroseconds() <= 0) {
    return 0;
  }
  return (num_tokens / static_cast<float>(duration.InMicroseconds())) *
         base::Time::kMicrosecondsPerSecond;
}

float GetTemperature(std::optional<float> temperature) {
  return std::max(0.0f, temperature.value_or(0.0f));
}

uint32_t GetTopK(std::optional<uint32_t> top_k) {
  return std::min(static_cast<uint32_t>(kMaxTopK.Get()),
                  std::max(1u, top_k.value_or(1)));
}

// Handles sending and canceling responses.
class Responder : public base::SupportsWeakPtr<Responder> {
 public:
  explicit Responder(
      mojo::PendingRemote<on_device_model::mojom::StreamingResponder> responder,
      scoped_refptr<LanguageDetector> language_detector,
      base::OnceClosure on_complete)
      : responder_(std::move(responder)),
        language_detector_(std::move(language_detector)),
        on_complete_(std::move(on_complete)) {
    responder_.set_disconnect_handler(
        base::BindOnce(&Responder::Cancel, base::Unretained(this)));
  }
  ~Responder() { Cancel(); }

  ChromeMLCancelFn* GetCancelFn() { return &cancel_; }

  ChromeMLExecutionOutputFn CreateOutputFn() {
    return [weak_ptr = AsWeakPtr(),
            task_runner = base::SequencedTaskRunner::GetCurrentDefault()](
               const ChromeMLExecutionOutput* output) {
      std::optional<std::string> text;
      std::optional<std::vector<float>> class_scores;
      switch (output->status) {
        case ChromeMLExecutionStatus::kInProgress:
          CHECK(output->text);
          text.emplace(output->text);
          break;
        case ChromeMLExecutionStatus::kComplete:
          DCHECK(!output->text);
          break;
      }

      if (output->ts_scores) {
        class_scores.emplace(output->ts_scores,
                             output->ts_scores + output->num_ts_scores);
      }

      task_runner->PostTask(
          FROM_HERE, base::BindOnce(&Responder::OnOutput, weak_ptr,
                                    std::move(text), std::move(class_scores)));
    };
  }

 private:
  void OnOutput(std::optional<std::string> text,
                std::optional<std::vector<float>> class_scores) {
    if (text) {
      num_tokens_++;
      output_so_far_ += *text;
      if (first_token_time_ == base::TimeTicks()) {
        first_token_time_ = base::TimeTicks::Now();
      }

      auto chunk = on_device_model::mojom::ResponseChunk::New();
      chunk->text = *text;
      chunk->safety_info = CreateSafetyInfo(output_so_far_, class_scores);
      responder_->OnResponse(std::move(chunk));
    } else {
      base::UmaHistogramCounts10000("OnDeviceModel.TokenCount.Output",
                                    num_tokens_);
      if (num_tokens_ > 1) {
        // Time starts at the first token to avoid counting input processing
        // time, so calculate using num_tokens_ - 1.
        base::UmaHistogramCounts1000(
            "OnDeviceModel.TokensPerSecond.Output",
            CalculateTokensPerSecond(
                num_tokens_ - 1, base::TimeTicks::Now() - first_token_time_));
      }

      auto summary = on_device_model::mojom::ResponseSummary::New();
      summary->safety_info = CreateSafetyInfo(output_so_far_, class_scores);
      responder_->OnComplete(std::move(summary));
      if (!on_complete_.is_null()) {
        std::move(on_complete_).Run();
      }
    }
  }

  on_device_model::mojom::SafetyInfoPtr CreateSafetyInfo(
      std::string_view text,
      std::optional<std::vector<float>>& class_scores) {
    if (!class_scores) {
      return nullptr;
    }

    auto safety_info = on_device_model::mojom::SafetyInfo::New();
    safety_info->class_scores = std::move(*class_scores);
    if (language_detector_) {
      safety_info->language = language_detector_->DetectLanguage(text);
    }
    return safety_info;
  }

  void Cancel() {
    if (cancel_) {
      cancel_();
    }
    if (!on_complete_.is_null()) {
      std::move(on_complete_).Run();
    }
  }

  base::TimeTicks first_token_time_;
  int num_tokens_ = 0;
  std::string output_so_far_;
  mojo::Remote<on_device_model::mojom::StreamingResponder> responder_;
  const scoped_refptr<LanguageDetector> language_detector_;
  ChromeMLCancelFn cancel_;
  base::OnceClosure on_complete_;
};

// Handles calling the ContextClient on completion and canceling the context
// request.
class ContextHolder : public base::SupportsWeakPtr<ContextHolder> {
 public:
  explicit ContextHolder(
      mojo::PendingRemote<on_device_model::mojom::ContextClient> client,
      base::OnceCallback<void(ContextHolder*)> on_disconnect,
      base::OnceClosure on_complete)
      : client_(std::move(client)),
        on_disconnect_(std::move(on_disconnect)),
        on_complete_(std::move(on_complete)) {
    if (client_) {
      client_.set_disconnect_handler(
          base::BindOnce(&ContextHolder::OnDisconnect, base::Unretained(this)));
    }
  }
  ~ContextHolder() {
    if (cancel_) {
      cancel_();
    }
    if (!on_complete_.is_null()) {
      std::move(on_complete_).Run();
    }
  }

  ChromeMLCancelFn* GetCancelFn() { return &cancel_; }

  ChromeMLContextSavedFn CreateContextSavedFn() {
    return CreateWeakCallbackFn(&ContextHolder::OnComplete, this);
  }

 private:
  void OnComplete(int tokens_processed) {
    if (tokens_processed > 0) {
      base::UmaHistogramCounts10000("OnDeviceModel.TokenCount.Context",
                                    tokens_processed);
      base::UmaHistogramCounts10000(
          "OnDeviceModel.TokensPerSecond.Context",
          CalculateTokensPerSecond(tokens_processed, timer_.Elapsed()));
    }
    if (client_) {
      client_->OnComplete(tokens_processed);
    }
    if (!on_complete_.is_null()) {
      std::move(on_complete_).Run();
    }
    OnDisconnect();
  }

  void OnDisconnect() {
    if (on_disconnect_) {
      std::move(on_disconnect_).Run(this);
    }
    // this may be deleted.
  }

  base::ElapsedTimer timer_;
  mojo::Remote<on_device_model::mojom::ContextClient> client_;
  base::OnceCallback<void(ContextHolder*)> on_disconnect_;
  ChromeMLCancelFn cancel_;
  base::OnceClosure on_complete_;
};

class SessionImpl : public on_device_model::OnDeviceModel::Session {
 public:
  SessionImpl(const ChromeML& chrome_ml,
              ChromeMLModel model,
              uint32_t max_tokens,
              scoped_refptr<LanguageDetector> language_detector,
              std::optional<uint32_t> adaptation_id)
      : chrome_ml_(chrome_ml),
        model_(model),
        max_tokens_(max_tokens),
        language_detector_(std::move(language_detector)),
        adaptation_id_(adaptation_id) {}
  ~SessionImpl() override = default;

  SessionImpl(const SessionImpl&) = delete;
  SessionImpl& operator=(const SessionImpl&) = delete;

  DISABLE_CFI_DLSYM
  void AddContext(
      on_device_model::mojom::InputOptionsPtr input,
      mojo::PendingRemote<on_device_model::mojom::ContextClient> client,
      base::OnceClosure on_complete) override {
    auto context_holder = std::make_unique<ContextHolder>(
        std::move(client),
        base::BindOnce(&SessionImpl::RemoveContext, base::Unretained(this)),
        std::move(on_complete));
    ChromeMLContextSavedFn context_saved_fn =
        context_holder->CreateContextSavedFn();
    ChromeMLExecuteOptions options{
        .prompt = input->text.c_str(),
        .context_mode = GetContextMode(*input) | ContextMode::kSave,
        .max_tokens =
            std::min(input->max_tokens.value_or(max_tokens_), max_tokens_),
        .token_offset = input->token_offset.value_or(0),
        .context_saved_fn = &context_saved_fn,
        .top_k = GetTopK(input->top_k),
        .temperature = GetTemperature(input->temperature),
    };
    if (adaptation_id_) {
      options.adaptation_id = &adaptation_id_.value();
    }
    chrome_ml_->api().ExecuteModel(model_, &options,
                                   context_holder->GetCancelFn());
    context_holders_.insert(std::move(context_holder));
    // Once we have added context, it should not be cleared.
    clear_context_ = false;
  }

  DISABLE_CFI_DLSYM
  void Execute(
      on_device_model::mojom::InputOptionsPtr input,
      mojo::PendingRemote<on_device_model::mojom::StreamingResponder> response,
      base::OnceClosure on_complete) override {
    responder_ = std::make_unique<Responder>(
        std::move(response), language_detector_, std::move(on_complete));
    ChromeMLExecutionOutputFn output_fn = responder_->CreateOutputFn();
    int32_t ts_interval = -1;
    if (input->safety_interval.has_value()) {
      ts_interval =
          base::saturated_cast<int32_t>(input->safety_interval.value());
    }
    ChromeMLExecuteOptions options{
        .prompt = input->text.c_str(),
        .context_mode = GetContextMode(*input),
        .max_tokens =
            std::min(input->max_tokens.value_or(max_tokens_), max_tokens_),
        .token_offset = input->token_offset.value_or(0),
        .max_output_tokens = input->max_output_tokens.value_or(0),
        .score_ts_interval = ts_interval,
        .execution_output_fn = &output_fn,
        .top_k = GetTopK(input->top_k),
        .temperature = GetTemperature(input->temperature),
    };
    if (adaptation_id_) {
      options.adaptation_id = &adaptation_id_.value();
    }
    chrome_ml_->api().ExecuteModel(model_, &options, responder_->GetCancelFn());
  }

  void ClearContext() override { clear_context_ = true; }

  DISABLE_CFI_DLSYM
  void SizeInTokens(const std::string& text,
                    base::OnceCallback<void(uint32_t)> callback) override {
    if (!chrome_ml_->api().SizeInTokens) {
      std::move(callback).Run(0);
      return;
    }

    auto shared_callback = std::make_shared<base::OnceCallback<void(uint32_t)>>(
        std::move(callback));
    auto fn = [shared_callback = shared_callback,
               task_runner =
                   base::SequencedTaskRunner::GetCurrentDefault()](int result) {
      if (!shared_callback->is_null()) {
        task_runner->PostTask(
            FROM_HERE, base::BindOnce(std::move(*shared_callback), result));
      }
    };
    chrome_ml_->api().SizeInTokens(model_, text, fn);
  }

 private:
  void RemoveContext(ContextHolder* context) {
    std::erase_if(context_holders_, base::MatchesUniquePtr(context));
  }

  int GetContextMode(const on_device_model::mojom::InputOptions& input) {
    int context_mode = ContextMode::kNone;
    if (input.ignore_context) {
      context_mode |= ContextMode::kIgnoreContext;
    }
    if (clear_context_) {
      context_mode |= ContextMode::kReset;
    }
    return context_mode;
  }

  bool clear_context_ = true;
  const raw_ref<const ChromeML> chrome_ml_;
  ChromeMLModel model_;
  const uint32_t max_tokens_;
  const scoped_refptr<LanguageDetector> language_detector_;
  std::unique_ptr<Responder> responder_;
  std::set<std::unique_ptr<ContextHolder>> context_holders_;
  std::optional<uint32_t> adaptation_id_;
};

}  // namespace

OnDeviceModelExecutor::OnDeviceModelExecutor(
    base::PassKey<OnDeviceModelExecutor>,
    const ChromeML& chrome_ml)
    : chrome_ml_(chrome_ml),
      task_runner_(base::SequencedTaskRunner::GetCurrentDefault()) {}

DISABLE_CFI_DLSYM
OnDeviceModelExecutor::~OnDeviceModelExecutor() {
  if (model_ != 0) {
    chrome_ml_->api().DestroyModel(model_);
  }
}

// static
base::expected<std::unique_ptr<OnDeviceModelExecutor>, LoadModelResult>
OnDeviceModelExecutor::CreateWithResult(
    const ChromeML& chrome_ml,
    on_device_model::mojom::LoadModelParamsPtr params) {
  auto executor = std::make_unique<OnDeviceModelExecutor>(
      base::PassKey<OnDeviceModelExecutor>(), chrome_ml);
  auto load_model_result = executor->Init(std::move(params));
  if (load_model_result == LoadModelResult::kSuccess) {
    return base::ok<std::unique_ptr<OnDeviceModelExecutor>>(
        std::move(executor));
  }
  return base::unexpected(load_model_result);
}

std::unique_ptr<on_device_model::OnDeviceModel::Session>
OnDeviceModelExecutor::CreateSession(std::optional<uint32_t> adaptation_id) {
  return std::make_unique<SessionImpl>(*chrome_ml_, model_,
                                       max_tokens_ - kReserveTokensForSafety,
                                       language_detector_, adaptation_id);
}

on_device_model::mojom::LanguageDetectionResultPtr
OnDeviceModelExecutor::DetectLanguage(const std::string& text) {
  if (!language_detector_) {
    return nullptr;
  }
  return language_detector_->DetectLanguage(text);
}

DISABLE_CFI_DLSYM
on_device_model::mojom::SafetyInfoPtr OnDeviceModelExecutor::ClassifyTextSafety(
    const std::string& text) {
  if (!chrome_ml_->api().ClassifyTextSafety) {
    return nullptr;
  }

  // First query the API to see how much storage we need for class scores.
  size_t num_scores = 0;
  if (chrome_ml_->api().ClassifyTextSafety(model_, text.c_str(), nullptr,
                                           &num_scores) !=
      ChromeMLSafetyResult::kInsufficientStorage) {
    return nullptr;
  }

  auto safety_info = on_device_model::mojom::SafetyInfo::New();
  safety_info->class_scores.resize(num_scores);
  const auto result = chrome_ml_->api().ClassifyTextSafety(
      model_, text.c_str(), safety_info->class_scores.data(), &num_scores);
  if (result != ChromeMLSafetyResult::kOk) {
    return nullptr;
  }
  CHECK_EQ(num_scores, safety_info->class_scores.size());
  if (language_detector_) {
    safety_info->language = language_detector_->DetectLanguage(text);
  }
  return safety_info;
}

DISABLE_CFI_DLSYM
base::expected<uint32_t, LoadModelResult> OnDeviceModelExecutor::LoadAdaptation(
    on_device_model::mojom::LoadAdaptationParamsPtr params) {
  if (!chrome_ml_->api().CreateAdaptation) {
    return base::unexpected(LoadModelResult::kFailedToLoadLibrary);
  }

  on_device_model::AdaptationAssets assets = std::move(params->assets);

  uint32_t id;
  ChromeMLModelData data = {
      .weights_file = assets.weights.TakePlatformFile(),
  };
  ChromeMLAdaptationDescriptor descriptor = {
      .model_data = &data,
  };
  if (!chrome_ml_->api().CreateAdaptation(model_, &descriptor, id)) {
    return base::unexpected(LoadModelResult::kFailedToLoadLibrary);
  }
  return base::ok(id);
}

DISABLE_CFI_DLSYM
LoadModelResult OnDeviceModelExecutor::Init(
    on_device_model::mojom::LoadModelParamsPtr params) {
  if (chrome_ml_->IsGpuBlocked()) {
    return LoadModelResult::kGpuBlocked;
  }
  on_device_model::ModelAssets assets = std::move(params->assets);

  if (assets.ts_data.IsValid()) {
    if (!ts_data_.Initialize(std::move(assets.ts_data)) ||
        !assets.ts_sp_model.IsValid() ||
        !ts_sp_model_.Initialize(std::move(assets.ts_sp_model))) {
      LOG(ERROR) << "Invalid TS model data supplied";
      return LoadModelResult::kFailedToLoadLibrary;
    }
  }

  if (assets.language_detection_model.IsValid()) {
    language_detector_ =
        LanguageDetector::Create(std::move(assets.language_detection_model));
    if (!language_detector_) {
      LOG(ERROR) << "Failed to initialize language detection";
      return LoadModelResult::kFailedToLoadLibrary;
    }
  }

  max_tokens_ = std::max(params->max_tokens, kReserveTokensForSafety);

  ChromeMLModelData data = {
      .weights_file = assets.weights.TakePlatformFile(),
  };
  ChromeMLModelDescriptor descriptor = {
      .model_data = &data,
      .max_tokens = max_tokens_,
      .temperature = 0.0f,
      .top_k = kMaxTopK.Get(),
      .ts_dimension = params->ts_dimension.value_or(0),
      .adaptation_ranks = params->adaptation_ranks.data(),
      .adaptation_ranks_size = params->adaptation_ranks.size(),
      .prefer_texture_weights = kPreferTextureWeights.Get(),
      .enable_host_mapped_pointer = kEnableHostMappedPointer.Get(),
      .use_low_power = kUseLowPower.Get(),
      .allow_fp16 = kAllowFp16.Get(),
  };
  if (ts_data_.IsValid()) {
    CHECK(ts_sp_model_.IsValid());
    descriptor.ts_data = ts_data_.data();
    descriptor.ts_size = ts_data_.length();
    descriptor.ts_spm_data = ts_sp_model_.data();
    descriptor.ts_spm_size = ts_sp_model_.length();
  };
  model_ = chrome_ml_->api().CreateModel(&descriptor,
                                         reinterpret_cast<uintptr_t>(this),
                                         OnDeviceModelExecutor::Schedule);
  return (model_ != 0) ? LoadModelResult::kSuccess
                       : LoadModelResult::kFailedToLoadLibrary;
}

// static
void OnDeviceModelExecutor::Schedule(uintptr_t context,
                                     std::function<void()>* fn) {
  base::ThreadPool::PostTask(
      FROM_HERE, {base::TaskPriority::USER_BLOCKING, base::MayBlock()},
      base::BindOnce([](std::function<void()> fn) { fn(); }, std::move(*fn)));
}

}  // namespace ml
