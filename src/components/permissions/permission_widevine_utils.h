/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_PERMISSIONS_PERMISSION_WIDEVINE_UTILS_H_
#define COMPONENTS_PERMISSIONS_PERMISSION_WIDEVINE_UTILS_H_

#include <vector>

#include "base/memory/raw_ptr.h"

namespace permissions {
class PermissionRequest;

bool HasWidevinePermissionRequest(
    const std::vector<
        raw_ptr<permissions::PermissionRequest, VectorExperimental>>& requests);

}  // namespace permissions

#endif  // COMPONENTS_PERMISSIONS_PERMISSION_WIDEVINE_UTILS_H_
