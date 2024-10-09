/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_L10N_COMMON_LOCALIZATION_UTIL_H_
#define COMPONENTS_L10N_COMMON_LOCALIZATION_UTIL_H_

#include <string>

namespace wootz_l10n {

// Return localized resource as a UTF-16 string for the given |resource_id|.
std::u16string GetLocalizedResourceUTF16String(int resource_id);

}  // namespace wootz_l10n

#endif  // COMPONENTS_L10N_COMMON_LOCALIZATION_UTIL_H_
