/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/block_tracker.h"

#include "base/containers/contains.h"
#include "components/wootz_wallet/browser/json_rpc_service.h"

namespace wootz_wallet {

BlockTracker::BlockTracker() = default;

BlockTracker::~BlockTracker() = default;

void BlockTracker::Stop(const std::string& chain_id) {
  if (base::Contains(timers_, chain_id)) {
    timers_.erase(chain_id);
  }
}

void BlockTracker::Stop() {
  timers_.clear();
}

bool BlockTracker::IsRunning(const std::string& chain_id) const {
  if (!base::Contains(timers_, chain_id)) {
    return false;
  }
  return timers_.at(chain_id)->IsRunning();
}

}  // namespace wootz_wallet
