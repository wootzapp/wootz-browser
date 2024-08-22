/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "components/wootz_component_updater/browser/wootz_component.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/task/sequenced_task_runner.h"

namespace wootz_component_updater {

WootzComponent::WootzComponent(Delegate* delegate)
    : delegate_(delegate),
      weak_factory_(this) {}

WootzComponent::~WootzComponent() = default;

void WootzComponent::Register(const std::string& component_name,
                              const std::string& component_id,
                              const std::string& component_base64_public_key) {
  VLOG(2) << "register component: " << component_id;
  component_name_ = component_name;
  component_id_ = component_id;
  component_base64_public_key_ = component_base64_public_key;

  auto registered_callback =
      base::BindOnce(&WootzComponent::OnComponentRegistered,
                     delegate_,
                     component_id);
  auto ready_callback =
      base::BindRepeating(&WootzComponent::OnComponentReadyInternal,
                          weak_factory_.GetWeakPtr(),
                          component_id);

  delegate_->Register(component_name_,
                      component_base64_public_key_,
                      std::move(registered_callback),
                      ready_callback);
}

bool WootzComponent::Unregister() {
  VLOG(2) << "unregister component: " << component_id_;
  return delegate_->Unregister(component_id_);
}

scoped_refptr<base::SequencedTaskRunner> WootzComponent::GetTaskRunner() {
  return delegate_->GetTaskRunner();
}

void WootzComponent::AddObserver(ComponentObserver* observer) {
  DCHECK(delegate_);
  delegate_->AddObserver(observer);
}

void WootzComponent::RemoveObserver(ComponentObserver* observer) {
  DCHECK(delegate_);
  delegate_->RemoveObserver(observer);
}

void WootzComponent::OnComponentReadyInternal(
    const std::string& component_id,
    const base::FilePath& install_dir,
    const std::string& manifest) {
  VLOG(2) << "component ready: " << manifest;
  OnComponentReady(component_id, install_dir, manifest);
}

void WootzComponent::OnComponentReady(
    const std::string& component_id,
    const base::FilePath& install_dir,
    const std::string& manifest) {}

// static
void WootzComponent::OnComponentRegistered(
    Delegate* delegate,
    const std::string& component_id) {
  VLOG(2) << "component registered: " << component_id;
  delegate->OnDemandUpdate(component_id);
}

WootzComponent::Delegate* WootzComponent::delegate() {
  return delegate_;
}

}  // namespace wootz_component_updater
