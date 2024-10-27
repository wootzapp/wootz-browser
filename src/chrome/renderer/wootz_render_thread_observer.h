/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_RENDERER_WOOTZ_RENDER_THREAD_OBSERVER_H_
#define CHROME_RENDERER_WOOTZ_RENDER_THREAD_OBSERVER_H_

#include "chrome/common/wootz_renderer_configuration.mojom.h"
#include "content/public/renderer/render_thread_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver_set.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"

namespace blink {
class AssociatedInterfaceRegistry;
}

class WootzRenderThreadObserver
    : public content::RenderThreadObserver,
      public wootz::mojom::WootzRendererConfiguration {
 public:
  WootzRenderThreadObserver(const WootzRenderThreadObserver&) = delete;
  WootzRenderThreadObserver& operator=(const WootzRenderThreadObserver&) =
      delete;
  WootzRenderThreadObserver();
  ~WootzRenderThreadObserver() override;

  // Return the dynamic parameters - those that may change while the
  // render process is running.
  static const wootz::mojom::DynamicParams& GetDynamicParams();

 private:
  // content::RenderThreadObserver:
  void RegisterMojoInterfaces(
      blink::AssociatedInterfaceRegistry* associated_interfaces) override;
  void UnregisterMojoInterfaces(
      blink::AssociatedInterfaceRegistry* associated_interfaces) override;

  void SetConfiguration(wootz::mojom::DynamicParamsPtr params) override;

  void OnRendererConfigurationAssociatedRequest(
      mojo::PendingAssociatedReceiver<wootz::mojom::WootzRendererConfiguration>
          receiver);


  mojo::AssociatedReceiverSet<wootz::mojom::WootzRendererConfiguration>
      renderer_configuration_receivers_;
};

#endif  // CHROME_RENDERER_WOOTZ_RENDER_THREAD_OBSERVER_H_