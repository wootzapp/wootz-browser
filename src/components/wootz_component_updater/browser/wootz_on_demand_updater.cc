/* Copyright (c) 2020 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/wootz_component_updater/browser/wootz_on_demand_updater.h"

#include <string>
#include <utility>

#include "base/check_is_test.h"
#include "base/functional/callback.h"  // IWYU pragma: keep
#include "base/no_destructor.h"

namespace wootz_component_updater {

WootzOnDemandUpdater* WootzOnDemandUpdater::GetInstance() {
  static base::NoDestructor<WootzOnDemandUpdater> instance;
  return instance.get();
}

WootzOnDemandUpdater::WootzOnDemandUpdater() = default;

WootzOnDemandUpdater::~WootzOnDemandUpdater() = default;

component_updater::OnDemandUpdater*
WootzOnDemandUpdater::RegisterOnDemandUpdater(
    component_updater::OnDemandUpdater* on_demand_updater) {
  if (!on_demand_updater) {
    CHECK_IS_TEST();
  }
  return std::exchange(on_demand_updater_, on_demand_updater);
}

void WootzOnDemandUpdater::OnDemandUpdate(const std::string& id) {
  OnDemandUpdate(id, component_updater::OnDemandUpdater::Priority::FOREGROUND,
                 component_updater::Callback());
}

void WootzOnDemandUpdater::OnDemandUpdate(
    const std::string& id,
    component_updater::OnDemandUpdater::Priority priority,
    component_updater::Callback callback) {
  CHECK(on_demand_updater_);
  on_demand_updater_->OnDemandUpdate(id, priority, std::move(callback));
}

void WootzOnDemandUpdater::OnDemandUpdate(
    const std::vector<std::string>& ids,
    component_updater::OnDemandUpdater::Priority priority,
    component_updater::Callback callback) {
  CHECK(on_demand_updater_);
  on_demand_updater_->OnDemandUpdate(ids, priority, std::move(callback));
}

}  // namespace wootz_component_updater
