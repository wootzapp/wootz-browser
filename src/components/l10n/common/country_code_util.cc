/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/l10n/common/country_code_util.h"

#include "components/l10n/common/country_code_feature.h"
#include "components/l10n/common/locale_util.h"
#include "components/l10n/common/prefs.h"
#include "components/prefs/pref_service.h"

namespace wootz_l10n {

std::string GetCountryCode(const PrefService* local_state) {
  if (base::FeatureList::IsEnabled(kFetchResourcesByCountryCodeFeature)) {
    return local_state->GetString(wootz_l10n::prefs::kCountryCode);
  }
  return GetDefaultISOCountryCodeString();
}

}  // namespace wootz_l10n
