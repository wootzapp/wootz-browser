/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_L10N_COMMON_DEFAULT_LOCALE_H_
#define COMPONENTS_L10N_COMMON_DEFAULT_LOCALE_H_

#include <string>

namespace wootz_l10n {

const std::string& DefaultLocaleString();
std::string& MutableDefaultLocaleStringForTesting();

}  // namespace wootz_l10n

#endif  // COMPONENTS_L10N_COMMON_DEFAULT_LOCALE_H_
