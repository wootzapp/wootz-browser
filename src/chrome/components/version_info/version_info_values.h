/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_COMPONENTS_VERSION_INFO_VERSION_INFO_VALUES_H_
#define CHROME_COMPONENTS_VERSION_INFO_VERSION_INFO_VALUES_H_

namespace version_info {
namespace constants {
inline constexpr char kWootzBrowserVersion[] = "@BRAVE_BROWSER_VERSION@";
inline constexpr char kWootzChromiumVersion[] = "@BRAVE_CHROMIUM_VERSION@";
inline constexpr char kWootzVersionNumberForDisplay[] = "@BRAVE_BROWSER_VERSION@ Chromium: @BRAVE_CHROMIUM_VERSION@";
}  // namespace constants
}  // namespace version_info

#endif  // BRAVE_COMPONENTS_VERSION_INFO_VERSION_INFO_VALUES_H_
