// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/preloading/prerender/prerender_metrics.h"

#include <cmath>
#include <memory>
#include <optional>

#include "base/check_op.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/metrics_hashes.h"
#include "base/strings/string_util.h"
#include "content/browser/devtools/devtools_instrumentation.h"
#include "content/browser/preloading/preloading_trigger_type_impl.h"
#include "content/browser/preloading/prerender/prerender_final_status.h"
#include "content/browser/preloading/prerender/prerender_host.h"
#include "content/public/browser/preloading_trigger_type.h"
#include "services/metrics/public/cpp/ukm_builders.h"
#include "services/metrics/public/cpp/ukm_recorder.h"

namespace content {

namespace {

// Do not add new value.
// These values are used to persists sparse metrics to logs.
enum HeaderMismatchType : uint32_t {
  kMatch = 0,
  kMissingInPrerendering = 1,
  kMissingInActivation = 2,
  kValueMismatch = 3,
  kMaxValue = kValueMismatch
};

PrerenderCancelledInterface GetCancelledInterfaceType(
    const std::string& interface_name) {
  if (interface_name == "device.mojom.GamepadHapticsManager")
    return PrerenderCancelledInterface::kGamepadHapticsManager;
  else if (interface_name == "device.mojom.GamepadMonitor")
    return PrerenderCancelledInterface::kGamepadMonitor;
  else if (interface_name ==
           "chrome.mojom.TrustedVaultEncryptionKeysExtension") {
    return PrerenderCancelledInterface::kTrustedVaultEncryptionKeys;
  }
  return PrerenderCancelledInterface::kUnknown;
}

int32_t InterfaceNameHasher(const std::string& interface_name) {
  return static_cast<int32_t>(base::HashMetricNameAs32Bits(interface_name));
}

int32_t HeaderMismatchHasher(const std::string& header,
                             HeaderMismatchType mismatch_type) {
  // Throw two bits away to encode the mismatch type.
  // {0---30} bits are the encoded hash number.
  // {31, 32} bits encode the mismatch type.
  static_assert(HeaderMismatchType::kMaxValue == 3u,
                "HeaderMismatchType should use 2 bits at most.");
  return static_cast<int32_t>(base::HashMetricNameAs32Bits(header) << 2 |
                              mismatch_type);
}

std::string GenerateHistogramName(const std::string& histogram_base_name,
                                  PreloadingTriggerType trigger_type,
                                  const std::string& embedder_suffix) {
  switch (trigger_type) {
    case PreloadingTriggerType::kSpeculationRule:
      CHECK(embedder_suffix.empty());
      return std::string(histogram_base_name) + ".SpeculationRule";
    case PreloadingTriggerType::kSpeculationRuleFromIsolatedWorld:
      CHECK(embedder_suffix.empty());
      return std::string(histogram_base_name) +
             ".SpeculationRuleFromIsolatedWorld";
    case PreloadingTriggerType::kSpeculationRuleFromAutoSpeculationRules:
      CHECK(embedder_suffix.empty());
      return std::string(histogram_base_name) +
             ".SpeculationRuleFromAutoSpeculationRules";
    case PreloadingTriggerType::kEmbedder:
      CHECK(!embedder_suffix.empty());
      return std::string(histogram_base_name) + ".Embedder_" + embedder_suffix;
  }
  NOTREACHED_NORETURN();
}

void ReportHeaderMismatch(const std::string& key,
                          HeaderMismatchType mismatch_type,
                          PreloadingTriggerType trigger_type,
                          const std::string& embedder_histogram_suffix) {
  base::UmaHistogramSparse(
      GenerateHistogramName("Prerender.Experimental.ActivationHeadersMismatch",
                            trigger_type, embedder_histogram_suffix),
      HeaderMismatchHasher(base::ToLowerASCII(key), mismatch_type));
}

void ReportAllPrerenderMismatchedHeaders(
    const std::vector<PrerenderMismatchedHeaders>& mismatched_headers,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  for (const auto& mismatched_header : mismatched_headers) {
    if (mismatched_header.initial_value.has_value() &&
        mismatched_header.activation_value.has_value()) {
      ReportHeaderMismatch(mismatched_header.header_name,
                           HeaderMismatchType::kValueMismatch, trigger_type,
                           embedder_histogram_suffix);
    } else if (mismatched_header.initial_value.has_value()) {
      ReportHeaderMismatch(mismatched_header.header_name,
                           HeaderMismatchType::kMissingInActivation,
                           trigger_type, embedder_histogram_suffix);
    } else {
      ReportHeaderMismatch(mismatched_header.header_name,
                           HeaderMismatchType::kMissingInPrerendering,
                           trigger_type, embedder_histogram_suffix);
    }
  }
}

// Called by MojoBinderPolicyApplier. This function records the Mojo interface
// that causes MojoBinderPolicyApplier to cancel prerendering.
void RecordPrerenderCancelledInterface(
    const std::string& interface_name,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  const PrerenderCancelledInterface interface_type =
      GetCancelledInterfaceType(interface_name);
  base::UmaHistogramEnumeration(
      GenerateHistogramName(
          "Prerender.Experimental.PrerenderCancelledInterface", trigger_type,
          embedder_histogram_suffix),
      interface_type);
  if (interface_type == PrerenderCancelledInterface::kUnknown) {
    // These interfaces can be required by embedders, or not set to kCancel
    // expclitly, e.g., channel-associated interfaces. Record these interfaces
    // with the sparse histogram to ensure all of them are tracked.
    base::UmaHistogramSparse(
        GenerateHistogramName(
            "Prerender.Experimental.PrerenderCancelledUnknownInterface",
            trigger_type, embedder_histogram_suffix),
        InterfaceNameHasher(interface_name));
  }
}

void RecordPrerenderFinalStatusUma(
    PrerenderFinalStatus final_status,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  base::UmaHistogramEnumeration(
      GenerateHistogramName("Prerender.Experimental.PrerenderHostFinalStatus",
                            trigger_type, embedder_histogram_suffix),
      final_status);
}

void RecordDidFailLoadErrorType(int32_t error_code,
                                PreloadingTriggerType trigger_type,
                                const std::string& embedder_histogram_suffix) {
  base::UmaHistogramSparse(
      GenerateHistogramName(
          "Prerender.Experimental.PrerenderLoadingFailureError", trigger_type,
          embedder_histogram_suffix),
      std::abs(error_code));
}

}  // namespace

// static
PrerenderCancellationReason
PrerenderCancellationReason::BuildForDisallowActivationState(
    uint64_t disallow_activation_reason) {
  return PrerenderCancellationReason(
      PrerenderFinalStatus::kInactivePageRestriction,
      disallow_activation_reason);
}

// static
PrerenderCancellationReason
PrerenderCancellationReason::BuildForMojoBinderPolicy(
    const std::string& interface_name) {
  return PrerenderCancellationReason(PrerenderFinalStatus::kMojoBinderPolicy,
                                     interface_name);
}

const std::vector<PrerenderMismatchedHeaders>*
PrerenderCancellationReason::GetPrerenderMismatchedHeaders() const {
  return absl::get_if<std::vector<PrerenderMismatchedHeaders>>(&explanation_);
}

// static
PrerenderCancellationReason PrerenderCancellationReason::
    CreateCandidateReasonForActivationParameterMismatch() {
  return PrerenderCancellationReason(
      PrerenderFinalStatus::kActivationNavigationParameterMismatch);
}

void PrerenderCancellationReason::SetPrerenderMismatchedHeaders(
    std::unique_ptr<std::vector<PrerenderMismatchedHeaders>>
        mismatched_headers) {
  explanation_ = std::move(*mismatched_headers);
}

//  static
PrerenderCancellationReason PrerenderCancellationReason::BuildForLoadingError(
    int32_t error_code) {
  return PrerenderCancellationReason(PrerenderFinalStatus::kDidFailLoad,
                                     error_code);
}

PrerenderCancellationReason::PrerenderCancellationReason(
    PrerenderFinalStatus final_status)
    : PrerenderCancellationReason(final_status, DetailedReasonVariant()) {}

PrerenderCancellationReason::PrerenderCancellationReason(
    PrerenderCancellationReason&& reason) = default;

PrerenderCancellationReason::~PrerenderCancellationReason() = default;

PrerenderCancellationReason::PrerenderCancellationReason(
    PrerenderFinalStatus final_status,
    DetailedReasonVariant explanation)
    : final_status_(final_status), explanation_(std::move(explanation)) {}

void PrerenderCancellationReason::ReportMetrics(
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) const {
  switch (final_status_) {
    case PrerenderFinalStatus::kInactivePageRestriction:
      CHECK(absl::holds_alternative<uint64_t>(explanation_));
      base::UmaHistogramSparse(
          GenerateHistogramName("Prerender.CanceledForInactivePageRestriction."
                                "DisallowActivationReason",
                                trigger_type, embedder_histogram_suffix),

          absl::get<uint64_t>(explanation_));
      break;
    case PrerenderFinalStatus::kMojoBinderPolicy:
      CHECK(absl::holds_alternative<std::string>(explanation_));
      RecordPrerenderCancelledInterface(absl::get<std::string>(explanation_),
                                        trigger_type,
                                        embedder_histogram_suffix);
      break;
    case PrerenderFinalStatus::kDidFailLoad:
      CHECK(absl::holds_alternative<int32_t>(explanation_));
      RecordDidFailLoadErrorType(absl::get<int32_t>(explanation_), trigger_type,
                                 embedder_histogram_suffix);
      break;
    case PrerenderFinalStatus::kActivationNavigationParameterMismatch:
      CHECK(absl::holds_alternative<std::vector<PrerenderMismatchedHeaders>>(
                explanation_) ||
            absl::holds_alternative<absl::monostate>(explanation_));
      if (auto* mismatched_headers =
              absl::get_if<std::vector<PrerenderMismatchedHeaders>>(
                  &explanation_)) {
        ReportAllPrerenderMismatchedHeaders(*mismatched_headers, trigger_type,
                                            embedder_histogram_suffix);
      }
      break;
    default:
      CHECK(absl::holds_alternative<absl::monostate>(explanation_));
      // Other types need not to report.
      break;
  }
}

std::optional<std::string>
PrerenderCancellationReason::DisallowedMojoInterface() const {
  switch (final_status_) {
    case PrerenderFinalStatus::kMojoBinderPolicy:
      return absl::get<std::string>(explanation_);
    default:
      return std::nullopt;
  }
}

PrerenderMismatchedHeaders::PrerenderMismatchedHeaders(
    const std::string& header_name,
    const std::optional<std::string> initial_value,
    const std::optional<std::string> activation_value)
    : header_name(header_name),
      initial_value(initial_value),
      activation_value(activation_value) {}

PrerenderMismatchedHeaders::~PrerenderMismatchedHeaders() = default;

PrerenderMismatchedHeaders::PrerenderMismatchedHeaders(
    PrerenderMismatchedHeaders&& other) = default;

void RecordPrerenderTriggered(ukm::SourceId ukm_id) {
  ukm::builders::PrerenderPageLoad(ukm_id).SetTriggeredPrerender(true).Record(
      ukm::UkmRecorder::Get());
}

void RecordPrerenderActivationTime(
    base::TimeDelta delta,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  base::UmaHistogramTimes(
      GenerateHistogramName("Navigation.TimeToActivatePrerender", trigger_type,
                            embedder_histogram_suffix),
      delta);
}

void RecordFailedPrerenderFinalStatus(
    const PrerenderCancellationReason& cancellation_reason,
    const PrerenderAttributes& attributes) {
  CHECK_NE(cancellation_reason.final_status(),
           PrerenderFinalStatus::kActivated);
  RecordPrerenderFinalStatusUma(cancellation_reason.final_status(),
                                attributes.trigger_type,
                                attributes.embedder_histogram_suffix);

  if (attributes.initiator_ukm_id != ukm::kInvalidSourceId) {
    // `initiator_ukm_id` must be valid for the speculation rules.
    CHECK(IsSpeculationRuleType(attributes.trigger_type));
    ukm::builders::PrerenderPageLoad(attributes.initiator_ukm_id)
        .SetFinalStatus(static_cast<int>(cancellation_reason.final_status()))
        .Record(ukm::UkmRecorder::Get());
  }
}

void ReportSuccessActivation(const PrerenderAttributes& attributes,
                             ukm::SourceId prerendered_ukm_id) {
  RecordPrerenderFinalStatusUma(PrerenderFinalStatus::kActivated,
                                attributes.trigger_type,
                                attributes.embedder_histogram_suffix);
  if (attributes.initiator_ukm_id != ukm::kInvalidSourceId) {
    // `initiator_ukm_id` must be valid only for the speculation rules.
    CHECK(IsSpeculationRuleType(attributes.trigger_type));
    ukm::builders::PrerenderPageLoad(attributes.initiator_ukm_id)
        .SetFinalStatus(static_cast<int>(PrerenderFinalStatus::kActivated))
        .Record(ukm::UkmRecorder::Get());
  }

  if (prerendered_ukm_id != ukm::kInvalidSourceId) {
    ukm::builders::PrerenderPageLoad(prerendered_ukm_id)
        .SetFinalStatus(static_cast<int>(PrerenderFinalStatus::kActivated))
        .Record(ukm::UkmRecorder::Get());
  }
}

void RecordPrerenderActivationNavigationParamsMatch(
    PrerenderHost::ActivationNavigationParamsMatch result,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_suffix) {
  base::UmaHistogramEnumeration(
      GenerateHistogramName(
          "Prerender.Experimental.ActivationNavigationParamsMatch",
          trigger_type, embedder_suffix),
      result);
}

void RecordPrerenderRedirectionMismatchType(
    PrerenderCrossOriginRedirectionMismatch mismatch_type,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  CHECK_EQ(trigger_type, PreloadingTriggerType::kEmbedder);
  base::UmaHistogramEnumeration(
      GenerateHistogramName(
          "Prerender.Experimental.PrerenderCrossOriginRedirectionMismatch",
          trigger_type, embedder_histogram_suffix),
      mismatch_type);
}

void RecordPrerenderRedirectionProtocolChange(
    PrerenderCrossOriginRedirectionProtocolChange change_type,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  CHECK_EQ(trigger_type, PreloadingTriggerType::kEmbedder);
  base::UmaHistogramEnumeration(
      GenerateHistogramName(
          "Prerender.Experimental.CrossOriginRedirectionProtocolChange",
          trigger_type, embedder_histogram_suffix),
      change_type);
}

void RecordPrerenderActivationTransition(
    int32_t potential_activation_transition,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  base::UmaHistogramSparse(
      GenerateHistogramName(
          "Prerender.Experimental.ActivationTransitionMismatch", trigger_type,
          embedder_histogram_suffix),
      potential_activation_transition);
}

static_assert(
    static_cast<int>(PrerenderBackNavigationEligibility::kMaxValue) +
        static_cast<int>(
            PreloadingEligibility::kPreloadingEligibilityContentStart2) <
    static_cast<int>(PreloadingEligibility::kPreloadingEligibilityContentEnd2));

PreloadingEligibility ToPreloadingEligibility(
    PrerenderBackNavigationEligibility eligibility) {
  if (eligibility == PrerenderBackNavigationEligibility::kEligible) {
    return PreloadingEligibility::kEligible;
  }

  return static_cast<PreloadingEligibility>(
      static_cast<int>(eligibility) +
      static_cast<int>(
          PreloadingEligibility::kPreloadingEligibilityContentStart2));
}

void RecordPrerenderBackNavigationEligibility(
    PreloadingPredictor predictor,
    PrerenderBackNavigationEligibility eligibility,
    PreloadingAttempt* preloading_attempt) {
  const std::string histogram_name =
      std::string("Preloading.PrerenderBackNavigationEligibility.") +
      std::string(predictor.name());
  base::UmaHistogramEnumeration(histogram_name, eligibility);

  if (preloading_attempt) {
    preloading_attempt->SetEligibility(ToPreloadingEligibility(eligibility));
  }
}

void RecordPrerenderActivationCommitDeferTime(
    base::TimeDelta time_delta,
    PreloadingTriggerType trigger_type,
    const std::string& embedder_histogram_suffix) {
  base::UmaHistogramTimes(
      GenerateHistogramName("Navigation.Prerender.ActivationCommitDeferTime",
                            trigger_type, embedder_histogram_suffix),
      time_delta);
}

void RecordReceivedPrerendersPerPrimaryPageChangedCount(
    int number,
    PreloadingTriggerType trigger_type,
    const std::string& eagerness_category) {
  base::UmaHistogramCounts100(
      GenerateHistogramName("Prerender.Experimental."
                            "ReceivedPrerendersPerPrimaryPageChangedCount2",
                            trigger_type, /*embedder_suffix=*/"") +
          "." + eagerness_category,
      number);
}

}  // namespace content
