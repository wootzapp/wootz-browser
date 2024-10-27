/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <memory>

#include "components/wootz_wallet/browser/wootz_wallet_service_delegate.h"
#include "build/build_config.h"
#include "content/public/browser/browser_context.h"

#if BUILDFLAG(IS_ANDROID)
#include "chrome/browser/wootz_wallet/wootz_wallet_service_delegate_impl_android.h"
#else
#include "chrome/browser/wootz_wallet/wootz_wallet_service_delegate_impl.h"
#endif

namespace wootz_wallet {

// static
std::unique_ptr<WootzWalletServiceDelegate> WootzWalletServiceDelegate::Create(
    content::BrowserContext* browser_context) {
  return std::make_unique<WootzWalletServiceDelegateImpl>(browser_context);
}

}  // namespace wootz_wallet
