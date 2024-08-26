/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_component_updater/browser/mock_on_demand_updater.h"

#include "components/wootz_component_updater/browser/wootz_on_demand_updater.h"

namespace wootz_component_updater {

MockOnDemandUpdater::MockOnDemandUpdater() {
  prev_on_demand_updater_ =
      WootzOnDemandUpdater::GetInstance()->RegisterOnDemandUpdater(this);
}

MockOnDemandUpdater::~MockOnDemandUpdater() {
  WootzOnDemandUpdater::GetInstance()->RegisterOnDemandUpdater(
      prev_on_demand_updater_);
}

}  // namespace wootz_component_updater
