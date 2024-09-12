/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_BASE_H_
#define CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_BASE_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "components/wootz_wallet/browser/wootz_wallet_service_delegate.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace content {
class BrowserContext;
}

namespace url {
class Origin;
}

namespace wootz_wallet {

// Shared WootzWalletServiceDelegate implementation between Desktop and Android.
class WootzWalletServiceDelegateBase : public WootzWalletServiceDelegate {
 public:
  explicit WootzWalletServiceDelegateBase(content::BrowserContext* context);
  WootzWalletServiceDelegateBase(const WootzWalletServiceDelegateBase&) =
      delete;
  WootzWalletServiceDelegateBase& operator=(
      const WootzWalletServiceDelegateBase&) = delete;
  ~WootzWalletServiceDelegateBase() override;

  bool HasPermission(mojom::CoinType coin,
                     const url::Origin& origin,
                     const std::string& account) override;
  bool ResetPermission(mojom::CoinType coin,
                       const url::Origin& origin,
                       const std::string& account) override;
  bool IsPermissionDenied(mojom::CoinType coin,
                          const url::Origin& origin) override;
  void ResetAllPermissions() override;

  base::FilePath GetWalletBaseDirectory() override;
  bool IsPrivateWindow() override;

 protected:
  base::FilePath wallet_base_directory_;
  bool is_private_window_ = false;
  raw_ptr<content::BrowserContext> context_ = nullptr;
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_BASE_H_
