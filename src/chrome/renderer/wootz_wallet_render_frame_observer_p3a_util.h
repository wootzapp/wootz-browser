/* Copyright (c) 2022 The Dark Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_RENDERER_WOOTZ_WALLET_RENDER_FRAME_OBSERVER_P3A_UTIL_H_
#define CHROME_RENDERER_WOOTZ_WALLET_RENDER_FRAME_OBSERVER_P3A_UTIL_H_

#include "chrome/common/wootz_renderer_configuration.mojom.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "content/public/renderer/render_frame.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace v8 {
class Context;
template <class T>
class Local;
class Isolate;
}  // namespace v8

namespace wootz_wallet {

class WootzWalletRenderFrameObserverP3AUtil {
 public:
  WootzWalletRenderFrameObserverP3AUtil();
  ~WootzWalletRenderFrameObserverP3AUtil();

  void ReportJSProviders(content::RenderFrame* render_frame,
                         const wootz::mojom::DynamicParams& dynamic_params);

 private:
  bool EnsureConnected(content::RenderFrame* render_frame);

  void ReportJSProvider(v8::Isolate* isolate,
                        v8::Local<v8::Context>& context,
                        mojom::CoinType coin_type,
                        const char* provider_object_key,
                        bool allow_provider_overwrite);

  mojo::Remote<wootz_wallet::mojom::WootzWalletP3A> wootz_wallet_p3a_;
};

}  // namespace wootz_wallet

#endif  // CHROME_RENDERER_WOOTZ_WALLET_RENDER_FRAME_OBSERVER_P3A_UTIL_H_