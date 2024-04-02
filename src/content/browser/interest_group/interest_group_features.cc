// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/interest_group/interest_group_features.h"

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"

namespace features {

// Please keep features in alphabetical order.

// Enable parsing ad auction response headers for an iframe navigation request.
BASE_FEATURE(kEnableIFrameAdAuctionHeaders,
             "EnableIFrameAdAuctionHeaders",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enable updating the executionMode to "frozen-context" when updating a user's
// interests groups.
BASE_FEATURE(kEnableUpdatingExecutionModeToFrozenContext,
             "EnableUpdatingExecutionModeToFrozenContext",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enable updating userBiddingSignals when updating a user's interests groups.
BASE_FEATURE(kEnableUpdatingUserBiddingSignals,
             "EnableUpdatingUserBiddingSignals",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enable write ahead logging for interest group storage.
BASE_FEATURE(kFledgeEnableWALForInterestGroupStorage,
             "FledgeEnableWALForInterestGroupStorage",
             base::FEATURE_DISABLED_BY_DEFAULT);

BASE_FEATURE(kFledgeFacilitatedTestingSignalsHeaders,
             "FledgeFacilitatedTestingSignalsHeaders",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enable prefetching B&A keys on the first joinAdInterestGroup call.
BASE_FEATURE(kFledgePrefetchBandAKeys,
             "FledgePrefetchBandAKeys",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enable storing a retrieving B&A keys for the interest group
// database.
BASE_FEATURE(kFledgeStoreBandAKeysInDB,
             "FledgeStoreBandAKeysInDB",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enable detecting inconsistency in the `PageImpl` used in the auction. Abort
// the auction when detected.
BASE_FEATURE(kDetectInconsistentPageImpl,
             "DetectInconsistentPageImpl",
             base::FEATURE_ENABLED_BY_DEFAULT);

}  // namespace features
