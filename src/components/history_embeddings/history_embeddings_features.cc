// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/history_embeddings/history_embeddings_features.h"

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"
#include "build/build_config.h"

namespace history_embeddings {

BASE_FEATURE(kHistoryEmbeddings,
             "HistoryEmbeddings",
             base::FEATURE_DISABLED_BY_DEFAULT);

const base::FeatureParam<int> kPassageExtractionDelay(&kHistoryEmbeddings,
                                                      "PassageExtractionDelay",
                                                      0);

const base::FeatureParam<int> kPassageExtractionMaxWordsPerAggregatePassage(
    &kHistoryEmbeddings,
    "PassageExtractionMaxWordsPerAggregatePassage",
    200);

const base::FeatureParam<int> kSearchResultItemCount(&kHistoryEmbeddings,
                                                     "SearchResultItemCount",
                                                     3);

const base::FeatureParam<bool> kAtKeywordAcceleration(&kHistoryEmbeddings,
                                                      "AtKeywordAcceleration",
                                                      false);

const base::FeatureParam<double> kContentVisibilityThreshold(
    &kHistoryEmbeddings,
    "ContentVisibilityThreshold",
    0.5);

const base::FeatureParam<bool> kUseMlEmbedder(&kHistoryEmbeddings,
                                              "UseMlEmbedder",
                                              true);

const base::FeatureParam<bool> kOmniboxUnscoped(&kHistoryEmbeddings,
                                                "OmniboxUnscoped",
                                                false);

const base::FeatureParam<int> kScheduledEmbeddingsMin(&kHistoryEmbeddings,
                                                      "ScheduledEmbeddingsMin",
                                                      1);
const base::FeatureParam<int> kScheduledEmbeddingsMax(&kHistoryEmbeddings,
                                                      "ScheduledEmbeddingsMax",
                                                      1);

const base::FeatureParam<bool> kSendQualityLog(&kHistoryEmbeddings,
                                               "SendQualityLog",
                                               false);

const base::FeatureParam<int> kEmbedderNumThreads(&kHistoryEmbeddings,
                                                  "EmbeddingsNumThreads",
                                                  4);

}  // namespace history_embeddings
