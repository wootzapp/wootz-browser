/* Copyright (c) 2020 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/wootz_component_updater/browser/features.h"

namespace wootz_component_updater {

BASE_FEATURE(kUseDevUpdaterUrl,
             "UseDevUpdaterUrl",
             base::FEATURE_DISABLED_BY_DEFAULT);

}  // namespace wootz_component_updater
