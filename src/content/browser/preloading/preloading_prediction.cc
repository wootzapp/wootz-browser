// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/preloading/preloading_prediction.h"

#include "base/metrics/histogram_functions.h"
#include "base/strings/strcat.h"
#include "content/public/browser/page.h"
#include "services/metrics/public/cpp/metrics_utils.h"
#include "services/metrics/public/cpp/ukm_builders.h"
#include "services/metrics/public/cpp/ukm_recorder.h"

namespace content {

PreloadingPrediction::PreloadingPrediction(
    PreloadingPredictor predictor,
    PreloadingConfidence confidence,
    ukm::SourceId triggered_primary_page_source_id,
    PreloadingURLMatchCallback url_match_predicate)
    : predictor_type_(predictor),
      triggered_primary_page_source_id_(triggered_primary_page_source_id),
      url_match_predicate_(std::move(url_match_predicate)),
      confidence_(confidence) {}

PreloadingPrediction::~PreloadingPrediction() = default;
PreloadingPrediction::PreloadingPrediction(PreloadingPrediction&&) = default;
PreloadingPrediction& PreloadingPrediction::operator=(PreloadingPrediction&&) =
    default;

void PreloadingPrediction::RecordPreloadingPredictionUKMs(
    ukm::SourceId navigated_page_source_id) {
  ukm::UkmRecorder* ukm_recorder = ukm::UkmRecorder::Get();

  // Don't log when the source id is invalid.
  if (navigated_page_source_id != ukm::kInvalidSourceId) {
    ukm::builders::Preloading_Prediction builder(navigated_page_source_id);
    builder.SetPreloadingPredictor(predictor_type_.ukm_value())
        .SetConfidence(static_cast<int>(confidence_))
        .SetAccuratePrediction(is_accurate_prediction_);
    if (time_to_next_navigation_) {
      builder.SetTimeToNextNavigation(ukm::GetExponentialBucketMinForCounts1000(
          time_to_next_navigation_->InMilliseconds()));
    }
    builder.Record(ukm_recorder);
  }

  if (triggered_primary_page_source_id_ != ukm::kInvalidSourceId) {
    ukm::builders::Preloading_Prediction_PreviousPrimaryPage builder(
        triggered_primary_page_source_id_);
    builder.SetPreloadingPredictor(predictor_type_.ukm_value())
        .SetConfidence(static_cast<int>(confidence_))
        .SetAccuratePrediction(is_accurate_prediction_);
    if (time_to_next_navigation_) {
      builder.SetTimeToNextNavigation(ukm::GetExponentialBucketMinForCounts1000(
          time_to_next_navigation_->InMilliseconds()));
    }
    builder.Record(ukm_recorder);
  }
}

void PreloadingPrediction::SetIsAccuratePrediction(const GURL& navigated_url) {
  DCHECK(url_match_predicate_);

  // `PreloadingAttemptImpl::SetIsAccurateTriggering` is called during
  // `WCO::DidStartNavigation`.
  if (!time_to_next_navigation_) {
    time_to_next_navigation_ = elapsed_timer_.Elapsed();
  }

  // Use the predicate to match the URLs as the matching logic varies for each
  // predictor.
  is_accurate_prediction_ |= url_match_predicate_.Run(navigated_url);
}

ExperimentalPreloadingPrediction::ExperimentalPreloadingPrediction(
    std::string_view name,
    PreloadingURLMatchCallback url_match_predicate,
    float score,
    float min_score,
    float max_score,
    size_t buckets)
    : name_(name),
      buckets_(buckets),
      normalized_score_((score - min_score) / (max_score - min_score)),
      url_match_predicate_(std::move(url_match_predicate)) {
  CHECK_GT(max_score, min_score);
  CHECK_LT(buckets, 101u);
}

void ExperimentalPreloadingPrediction::SetIsAccuratePrediction(
    const GURL& navigated_url) {
  is_accurate_prediction_ = url_match_predicate_.Run(navigated_url);
}

ExperimentalPreloadingPrediction::~ExperimentalPreloadingPrediction() = default;
ExperimentalPreloadingPrediction::ExperimentalPreloadingPrediction(
    ExperimentalPreloadingPrediction&&) = default;
ExperimentalPreloadingPrediction& ExperimentalPreloadingPrediction::operator=(
    ExperimentalPreloadingPrediction&&) = default;

void ExperimentalPreloadingPrediction::RecordToUMA() const {
  const auto uma_experimental_prediction =
      base::StrCat({"Preloading.Experimental.", PredictorName(), ".",
                    IsAccuratePrediction() ? "Positive" : "Negative"});
  base::UmaHistogramExactLinear(uma_experimental_prediction,
                                normalized_score_ * buckets_, buckets_ + 1);
}

}  // namespace content
