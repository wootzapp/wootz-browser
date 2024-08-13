/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_COMPONENTS_VERSION_INFO_VERSION_INFO_H_
#define CHROME_COMPONENTS_VERSION_INFO_VERSION_INFO_H_

#include <string>

#include "chrome/components/version_info/version_info_values.h"

namespace version_info {

std::string GetWootzVersionWithoutChromiumMajorVersion();

constexpr std::string GetWootzVersionNumberForDisplay() {
  return constants::kWootzVersionNumberForDisplay;
}

std::string GetWootzChromiumVersionNumber();

}  // namespace version_info

#endif  // BRAVE_COMPONENTS_VERSION_INFO_VERSION_INFO_H_
