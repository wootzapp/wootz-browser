/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_L10N_BROWSER_DEFAULT_LOCALE_UTIL_H_
#define COMPONENTS_L10N_BROWSER_DEFAULT_LOCALE_UTIL_H_

#include <optional>
#include <string>

#include "absl/types/optional.h"

namespace wootz_l10n {

// Returns the current default locale of the device. When the locale should
// match the application locale or an eligible string pack for localization use
// the canonicalized l10n_util::GetApplicationLocale.
std::optional<std::string> MaybeGetDefaultLocaleString();

}  // namespace wootz_l10n

#endif  // COMPONENTS_L10N_BROWSER_DEFAULT_LOCALE_UTIL_H_
