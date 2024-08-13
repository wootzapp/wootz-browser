/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_REWARDS_REWARDS_UTIL_H_
#define CHROME_BROWSER_WOOTZ_REWARDS_REWARDS_UTIL_H_

#include "chrome/components/wootz_rewards/common/rewards_util.h"

class Profile;

namespace wootz_rewards {

bool IsSupportedForProfile(
    Profile* profile,
    IsSupportedOptions options = IsSupportedOptions::kNone);

}  // namespace wootz_rewards

#endif  // CHROME_BROWSER_WOOTZ_REWARDS_REWARDS_UTIL_H_