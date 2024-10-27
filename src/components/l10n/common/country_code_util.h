/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_L10N_COMMON_COUNTRY_CODE_UTIL_H_
#define COMPONENTS_L10N_COMMON_COUNTRY_CODE_UTIL_H_

#include <string>

class PrefService;

namespace wootz_l10n {

// Return the country code from local state pref or default ISO country code.
std::string GetCountryCode(const PrefService* local_state);

}  // namespace wootz_l10n

#endif  // COMPONENTS_L10N_COMMON_COUNTRY_CODE_UTIL_H_
