/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/components/version_info/version_info.h"

#include "chrome/components/version_info/version_info_values.h"

namespace version_info {

std::string GetWootzVersionWithoutChromiumMajorVersion() {
  return constants::kWootzBrowserVersion;
}

std::string GetWootzChromiumVersionNumber() {
  return constants::kWootzChromiumVersion;
}

}  // namespace version_info
