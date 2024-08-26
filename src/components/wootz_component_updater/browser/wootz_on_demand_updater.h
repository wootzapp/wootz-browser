/* Copyright (c) 2020 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_ON_DEMAND_UPDATER_H_
#define COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_ON_DEMAND_UPDATER_H_

#include <string>
#include <vector>

#include "components/component_updater/component_updater_service.h"

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

namespace wootz_component_updater {

class WootzOnDemandUpdater {
 public:
  static WootzOnDemandUpdater* GetInstance();

  WootzOnDemandUpdater(const WootzOnDemandUpdater&) = delete;
  WootzOnDemandUpdater& operator=(const WootzOnDemandUpdater&) = delete;

  component_updater::OnDemandUpdater* RegisterOnDemandUpdater(
      component_updater::OnDemandUpdater* on_demand_updater);

  void OnDemandUpdate(const std::string& id);

  void OnDemandUpdate(const std::string& id,
                      component_updater::OnDemandUpdater::Priority priority,
                      component_updater::Callback callback);
  void OnDemandUpdate(const std::vector<std::string>& ids,
                      component_updater::OnDemandUpdater::Priority priority,
                      component_updater::Callback callback);

 private:
  friend base::NoDestructor<WootzOnDemandUpdater>;
  WootzOnDemandUpdater();
  ~WootzOnDemandUpdater();

  raw_ptr<component_updater::OnDemandUpdater> on_demand_updater_ = nullptr;
};

}  // namespace wootz_component_updater

#endif  // COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_ON_DEMAND_UPDATER_H_
