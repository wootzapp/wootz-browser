/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_rewards/rewards_util.h"

#include "chrome/components/wootz_rewards/common/rewards_util.h"
#include "chrome/browser/profiles/profile.h"

namespace wootz_rewards {

bool IsSupportedForProfile(Profile* profile, IsSupportedOptions options) {
  DCHECK(profile);
  return profile->IsRegularProfile() &&
         IsSupported(profile->GetPrefs(), options);
}

}  // namespace wootz_rewards