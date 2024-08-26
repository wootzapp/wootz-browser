/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_IMPL_ANDROID_H_
#define CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_IMPL_ANDROID_H_

#include <memory>
#include <optional>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_delegate_base.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace content {
class BrowserContext;
}

namespace wootz_wallet {

class ExternalWalletsImporter;

class WootzWalletServiceDelegateImpl : public WootzWalletServiceDelegateBase {
 public:
  explicit WootzWalletServiceDelegateImpl(content::BrowserContext* context);
  WootzWalletServiceDelegateImpl(const WootzWalletServiceDelegateImpl&) =
      delete;
  WootzWalletServiceDelegateImpl& operator=(
      const WootzWalletServiceDelegateImpl&) = delete;
  ~WootzWalletServiceDelegateImpl() override;

  bool AddPermission(mojom::CoinType coin,
                     const url::Origin& origin,
                     const std::string& account) override;
  void GetWebSitesWithPermission(
      mojom::CoinType coin,
      GetWebSitesWithPermissionCallback callback) override;
  void ResetWebSitePermission(mojom::CoinType coin,
                              const std::string& formed_website,
                              ResetWebSitePermissionCallback callback) override;
  std::optional<url::Origin> GetActiveOrigin() override;

 private:
  base::ObserverList<WootzWalletServiceDelegate::Observer> observer_list_;

  base::WeakPtrFactory<WootzWalletServiceDelegateImpl> weak_ptr_factory_;
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_IMPL_ANDROID_H_
