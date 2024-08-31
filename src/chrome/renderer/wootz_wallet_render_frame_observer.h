/* Copyright (c) 2021 The Dark Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_RENDERER_WOOTZ_WALLET_RENDER_FRAME_OBSERVER_H_
#define CHROME_RENDERER_WOOTZ_WALLET_RENDER_FRAME_OBSERVER_H_

#include <optional>

#include "chrome/common/wootz_renderer_configuration.mojom.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/renderer/js_ethereum_provider.h"
#include "components/wootz_wallet/renderer/js_solana_provider.h"
#include "chrome/renderer/wootz_wallet_render_frame_observer_p3a_util.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "third_party/blink/public/web/web_navigation_type.h"
#include "url/gurl.h"
#include "v8/include/v8.h"

namespace wootz_wallet {

class WootzWalletRenderFrameObserver : public content::RenderFrameObserver {
 public:
  using GetDynamicParamsCallback =
      base::RepeatingCallback<const wootz::mojom::DynamicParams&()>;

  explicit WootzWalletRenderFrameObserver(
      content::RenderFrame* render_frame,
      GetDynamicParamsCallback get_dynamic_params_callback);
  ~WootzWalletRenderFrameObserver() override;

  // RenderFrameObserver implementation.
  void DidStartNavigation(
      const GURL& url,
      std::optional<blink::WebNavigationType> navigation_type) override;
  void DidClearWindowObject() override;

  void DidFinishLoad() override;

 private:
  // RenderFrameObserver implementation.
  void OnDestruct() override;

  bool IsPageValid();
  bool CanCreateProvider();

  GURL url_;
  GetDynamicParamsCallback get_dynamic_params_callback_;

  WootzWalletRenderFrameObserverP3AUtil p3a_util_;
};

}  // namespace wootz_wallet

#endif  // CHROME_RENDERER_WOOTZ_WALLET_RENDER_FRAME_OBSERVER_H_