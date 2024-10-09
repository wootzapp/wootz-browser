/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_STATS_BROWSER_WOOTZ_STATS_UPDATER_OBSERVER_H_
#define COMPONENTS_WOOTZ_STATS_BROWSER_WOOTZ_STATS_UPDATER_OBSERVER_H_

#include "base/observer_list_types.h"

namespace wootz_stats {

class WootzStatsUpdaterObserver : public base::CheckedObserver {
 public:
  ~WootzStatsUpdaterObserver() override {}

  virtual void OnStatsPingFired() {}
};

}  // namespace wootz_stats
#endif  // COMPONENTS_WOOTZ_STATS_BROWSER_WOOTZ_STATS_UPDATER_OBSERVER_H_
