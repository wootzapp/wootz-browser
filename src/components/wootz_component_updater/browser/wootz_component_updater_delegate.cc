/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_component_updater/browser/wootz_component_updater_delegate.h"

#include <memory>
#include <utility>

#include "base/task/sequenced_task_runner.h"
#include "base/task/thread_pool.h"
#include "components/wootz_component_updater/browser/wootz_component_installer.h"
#include "components/wootz_component_updater/browser/wootz_on_demand_updater.h"
#include "components/component_updater/component_updater_service.h"
#include "components/prefs/pref_service.h"

using wootz_component_updater::WootzComponent;
using wootz_component_updater::WootzOnDemandUpdater;
using component_updater::ComponentUpdateService;

namespace wootz {

WootzComponentUpdaterDelegate::WootzComponentUpdaterDelegate(
    ComponentUpdateService* component_updater,
    PrefService* local_state,
    const std::string& locale)
    : component_updater_(
          raw_ref<ComponentUpdateService>::from_ptr(component_updater)),
      local_state_(raw_ref<PrefService>::from_ptr(local_state)),
      locale_(locale),
      task_runner_(base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(), base::TaskPriority::USER_BLOCKING,
           base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN})) {}

WootzComponentUpdaterDelegate::~WootzComponentUpdaterDelegate() = default;

void WootzComponentUpdaterDelegate::Register(
    const std::string& component_name,
    const std::string& component_base64_public_key,
    base::OnceClosure registered_callback,
    WootzComponent::ReadyCallback ready_callback) {
  wootz::RegisterComponent(base::to_address(component_updater_), component_name,
                           component_base64_public_key,
                           std::move(registered_callback),
                           std::move(ready_callback));
}

bool WootzComponentUpdaterDelegate::Unregister(
    const std::string& component_id) {
  return component_updater_->UnregisterComponent(component_id);
}

void WootzComponentUpdaterDelegate::OnDemandUpdate(
    const std::string& component_id) {
  WootzOnDemandUpdater::GetInstance()->OnDemandUpdate(component_id);
}

void WootzComponentUpdaterDelegate::AddObserver(ComponentObserver* observer) {
  component_updater_->AddObserver(observer);
}

void WootzComponentUpdaterDelegate::RemoveObserver(
    ComponentObserver* observer) {
  component_updater_->RemoveObserver(observer);
}

scoped_refptr<base::SequencedTaskRunner>
WootzComponentUpdaterDelegate::GetTaskRunner() {
  return task_runner_;
}

const std::string& WootzComponentUpdaterDelegate::locale() const {
  return locale_;
}

PrefService* WootzComponentUpdaterDelegate::local_state() {
  return base::to_address(local_state_);
}

}  // namespace wootz
