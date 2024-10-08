/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_L10N_COMMON_PREFS_H_
#define COMPONENTS_L10N_COMMON_PREFS_H_

class PrefRegistrySimple;

namespace wootz_l10n {

namespace prefs {

// The country code used to register component resources.
inline constexpr char kCountryCode[] = "wootz.country_code";

}  // namespace prefs

void RegisterL10nLocalStatePrefs(PrefRegistrySimple* registry);

}  // namespace wootz_l10n

#endif  // COMPONENTS_L10N_COMMON_PREFS_H_
