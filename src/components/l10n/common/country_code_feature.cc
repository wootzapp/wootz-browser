/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/l10n/common/country_code_feature.h"

namespace wootz_l10n {

BASE_FEATURE(kFetchResourcesByCountryCodeFeature,
             "FetchResourcesByCountryCode",
             base::FEATURE_ENABLED_BY_DEFAULT);

}  // namespace wootz_l10n
