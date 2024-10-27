/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/renderer/wootz_render_thread_observer.h"

#include <utility>

#include "base/logging.h"
#include "base/no_destructor.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace {

wootz::mojom::DynamicParams* GetDynamicConfigParams() {
  static base::NoDestructor<wootz::mojom::DynamicParams> dynamic_params;
  return dynamic_params.get();
}

}  // namespace

WootzRenderThreadObserver::WootzRenderThreadObserver() = default;

WootzRenderThreadObserver::~WootzRenderThreadObserver() = default;

// static
const wootz::mojom::DynamicParams&
WootzRenderThreadObserver::GetDynamicParams() {
  return *GetDynamicConfigParams();
}

void WootzRenderThreadObserver::RegisterMojoInterfaces(
    blink::AssociatedInterfaceRegistry* associated_interfaces) {
  associated_interfaces->AddInterface<wootz::mojom::WootzRendererConfiguration>(
      base::BindRepeating(
          &WootzRenderThreadObserver::OnRendererConfigurationAssociatedRequest,
          base::Unretained(this)));
}

void WootzRenderThreadObserver::UnregisterMojoInterfaces(
    blink::AssociatedInterfaceRegistry* associated_interfaces) {
  associated_interfaces->RemoveInterface(
      wootz::mojom::WootzRendererConfiguration::Name_);
}

void WootzRenderThreadObserver::OnRendererConfigurationAssociatedRequest(
    mojo::PendingAssociatedReceiver<wootz::mojom::WootzRendererConfiguration>
        receiver) {
  renderer_configuration_receivers_.Add(this, std::move(receiver));
}

void WootzRenderThreadObserver::SetConfiguration(
    wootz::mojom::DynamicParamsPtr params) {
  *GetDynamicConfigParams() = std::move(*params);
}