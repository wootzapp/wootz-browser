// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OPTIMIZATION_GUIDE_CORE_OPTIMIZATION_GUIDE_MODEL_EXECUTOR_H_
#define COMPONENTS_OPTIMIZATION_GUIDE_CORE_OPTIMIZATION_GUIDE_MODEL_EXECUTOR_H_

#include "base/functional/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "base/types/expected.h"
#include "components/optimization_guide/core/model_execution/feature_keys.h"
#include "components/optimization_guide/core/model_execution/optimization_guide_model_execution_error.h"
#include "components/optimization_guide/core/model_quality/model_quality_log_entry.h"
#include "components/optimization_guide/proto/model_execution.pb.h"

namespace optimization_guide {

// The result type of model execution.
using OptimizationGuideModelExecutionResult =
    base::expected<const proto::Any /*response_metadata*/,
                   OptimizationGuideModelExecutionError>;

// A response type used for OptimizationGuideModelExecutor::Session.
struct StreamingResponse {
  // The response proto. This may be incomplete until `is_complete` is true.
  // This will contain the full response up to this point in the stream. Callers
  // should replace any previous streamed response with the new value while
  // `is_complete` is false.
  const proto::Any response;

  // True if streaming has finished.
  bool is_complete = false;
};

struct OptimizationGuideModelStreamingExecutionResult {
  OptimizationGuideModelStreamingExecutionResult();
  explicit OptimizationGuideModelStreamingExecutionResult(
      base::expected<const StreamingResponse,
                     OptimizationGuideModelExecutionError> response,
      bool provided_by_on_device,
      std::unique_ptr<ModelQualityLogEntry> log_entry = nullptr);

  ~OptimizationGuideModelStreamingExecutionResult();
  OptimizationGuideModelStreamingExecutionResult(
      OptimizationGuideModelStreamingExecutionResult&& src);

  base::expected<const StreamingResponse, OptimizationGuideModelExecutionError>
      response;
  // True if the response was computed on-device.
  bool provided_by_on_device = false;
  // The log entry will be null until `StreamingResponse.is_complete` is true.
  std::unique_ptr<ModelQualityLogEntry> log_entry;
};

// The callback for receiving the model execution result and model quality log
// entry.
using OptimizationGuideModelExecutionResultCallback =
    base::OnceCallback<void(OptimizationGuideModelExecutionResult,
                            std::unique_ptr<ModelQualityLogEntry>)>;

// The callback for receiving streamed output from the model. The log entry will
// be null until `StreamingResponse.is_complete` is true.
using OptimizationGuideModelExecutionResultStreamingCallback =
    base::RepeatingCallback<void(
        OptimizationGuideModelStreamingExecutionResult)>;

// Params used to control sampling output tokens for the on-device model.
struct SamplingParams {
  uint32_t top_k = 1;
  float temperature = 0.0f;
};

// Params to control model config per-session.
struct SessionConfigParams {
  std::optional<SamplingParams> sampling_params;

  // Whether to disable server fallback if on-device model is unavailable.
  //
  // This API will change but is done here quickly for simplicity while the
  // capabilities API gets designed. Please ask owners before using this API.
  bool disable_server_fallback = false;
};

// Reasons why the on-device model was not available for use.
//
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class OnDeviceModelEligibilityReason {
  kUnknown = 0,
  // Success.
  kSuccess = 1,
  // The feature flag gating on-device model execution was disabled.
  kFeatureNotEnabled = 2,
  // There was no on-device model available.
  kModelNotAvailable = 3,
  // The on-device model was available but there was not an execution config
  // available for the feature.
  kConfigNotAvailableForFeature = 4,
  // The GPU is blocked.
  kGpuBlocked = 5,
  // The on-device model process crashed too many times for this version.
  kTooManyRecentCrashes = 6,
  // The on-device model took too long too many times for this version.
  kTooManyRecentTimeouts = 7,
  // The on-device safety model was required but not available.
  kSafetyModelNotAvailable = 8,
  // The on-device safety model was available but there was not a safety config
  // available for the feature.
  kSafetyConfigNotAvailableForFeature = 9,
  // The on-device language detection model was required but not available.
  kLanguageDetectionModelNotAvailable = 10,
  // On-device model execution for this feature was not enabled.
  kFeatureExecutionNotEnabled = 11,
  // On-device model adaptation was required but not available.
  kModelAdaptationNotAvailable = 12,

  // This must be kept in sync with
  // OptimizationGuideOnDeviceModelEligibilityReason in optimization/enums.xml.

  // Insert new values before this line.
  kMaxValue = kModelAdaptationNotAvailable,
};

// Interface for model execution.
class OptimizationGuideModelExecutor {
 public:
  virtual ~OptimizationGuideModelExecutor() = default;

  // A model session that will save context for future ExecuteModel() calls.
  class Session {
   public:
    virtual ~Session() = default;

    // Adds context to this session. This will be saved for future Execute()
    // calls. Calling multiple times will replace previous calls to
    // AddContext(). Calling this while a ExecuteModel() call is still streaming
    // a response will cancel the ongoing ExecuteModel() call by calling its
    // `callback` with the kCancelled error.
    virtual void AddContext(
        const google::protobuf::MessageLite& request_metadata) = 0;

    // Execute the model with `request_metadata` and streams the result to
    // `callback`. The execute call will include context from the last
    // AddContext() call. Data provided to the last AddContext() call does not
    // need to be provided here. Calling this while another ExecuteModel() call
    // is still streaming a response will cancel the previous call by calling
    // `callback` with the kCancelled error.
    virtual void ExecuteModel(
        const google::protobuf::MessageLite& request_metadata,
        OptimizationGuideModelExecutionResultStreamingCallback callback) = 0;
  };

  // Whether an on-device session can be created for `feature`. An optional
  // `debug_reason` parameter can be provided for more detailed reasons for why
  // an on-device session could not be created.
  virtual bool CanCreateOnDeviceSession(
      ModelBasedCapabilityKey feature,
      raw_ptr<OnDeviceModelEligibilityReason> debug_reason) = 0;

  // Starts a session which allows streaming input and output from the model.
  // May return nullptr if model execution is not supported. This session should
  // not outlive OptimizationGuideModelExecutor.
  virtual std::unique_ptr<Session> StartSession(
      ModelBasedCapabilityKey feature,
      const std::optional<SessionConfigParams>& config_params) = 0;

  // Executes the model for `feature` with `request_metadata` and invokes the
  // `callback` with the result.
  virtual void ExecuteModel(
      ModelBasedCapabilityKey feature,
      const google::protobuf::MessageLite& request_metadata,
      OptimizationGuideModelExecutionResultCallback callback) = 0;
};

}  // namespace optimization_guide

#endif  // COMPONENTS_OPTIMIZATION_GUIDE_CORE_OPTIMIZATION_GUIDE_MODEL_EXECUTOR_H_
