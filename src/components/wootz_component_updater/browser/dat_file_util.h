/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_DAT_FILE_UTIL_H_
#define COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_DAT_FILE_UTIL_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/files/file_path.h"

namespace wootz_component_updater {

using DATFileDataBuffer = std::vector<unsigned char>;

std::string GetDATFileAsString(const base::FilePath& file_path);

DATFileDataBuffer ReadDATFileData(const base::FilePath& dat_file_path);

}  // namespace wootz_component_updater

#endif  // COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_DAT_FILE_UTIL_H_
