/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_COMPONENT_UPDATER_DELEGATE_H_
#define COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_COMPONENT_UPDATER_DELEGATE_H_

#include <string>

#include "base/functional/callback.h"
#include "base/memory/raw_ref.h"
#include "components/wootz_component_updater/browser/wootz_component.h"

using wootz_component_updater::WootzComponent;

namespace base {
class SequencedTaskRunner;
}

namespace component_updater {
class ComponentUpdateService;
}

class PrefService;

namespace wootz {

class WootzComponentUpdaterDelegate : public WootzComponent::Delegate {
 public:
  WootzComponentUpdaterDelegate(
      component_updater::ComponentUpdateService* updater,
      PrefService* local_state,
      const std::string& locale);
  WootzComponentUpdaterDelegate(const WootzComponentUpdaterDelegate&) = delete;
  WootzComponentUpdaterDelegate& operator=(
      const WootzComponentUpdaterDelegate&) = delete;
  ~WootzComponentUpdaterDelegate() override;

  using ComponentObserver = update_client::UpdateClient::Observer;
  // wootz_component_updater::WootzComponent::Delegate implementation
  void Register(const std::string& component_name,
                const std::string& component_base64_public_key,
                base::OnceClosure registered_callback,
                WootzComponent::ReadyCallback ready_callback) override;
  bool Unregister(const std::string& component_id) override;
  void OnDemandUpdate(const std::string& component_id) override;

  void AddObserver(ComponentObserver* observer) override;
  void RemoveObserver(ComponentObserver* observer) override;

  scoped_refptr<base::SequencedTaskRunner> GetTaskRunner() override;

  const std::string& locale() const override;
  PrefService* local_state() override;

 private:
  const raw_ref<component_updater::ComponentUpdateService> component_updater_;
  const raw_ref<PrefService> local_state_;
  std::string locale_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
};

}  // namespace wootz

#endif  // COMPONENTS_WOOTZ_COMPONENT_UPDATER_BROWSER_WOOTZ_COMPONENT_UPDATER_DELEGATE_H_
