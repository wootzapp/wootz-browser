/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_SERVICE_DELEGATE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_SERVICE_DELEGATE_H_

#include <memory>
#include <optional>
#include <string>

#include "base/functional/callback.h"
#include "base/observer_list_types.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/wootz_wallet_types.h"
#include "url/origin.h"

namespace content {
class BrowserContext;
}

namespace wootz_wallet {

class WootzWalletServiceDelegate {
 public:
  using IsExternalWalletInstalledCallback = base::OnceCallback<void(bool)>;
  using IsExternalWalletInitializedCallback = base::OnceCallback<void(bool)>;
  using GetImportInfoCallback =
      base::OnceCallback<void(bool, ImportInfo, ImportError)>;
  using GetWebSitesWithPermissionCallback =
      mojom::WootzWalletService::GetWebSitesWithPermissionCallback;
  using ResetWebSitePermissionCallback =
      mojom::WootzWalletService::ResetWebSitePermissionCallback;

  WootzWalletServiceDelegate() = default;
  WootzWalletServiceDelegate(const WootzWalletServiceDelegate&) = delete;
  WootzWalletServiceDelegate& operator=(const WootzWalletServiceDelegate&) =
      delete;
  virtual ~WootzWalletServiceDelegate() = default;

  class Observer : public base::CheckedObserver {
   public:
    virtual void OnActiveOriginChanged(
        const mojom::OriginInfoPtr& origin_info) {}
  };
  virtual void AddObserver(Observer* observer) {}
  virtual void RemoveObserver(Observer* observer) {}

  virtual void IsExternalWalletInstalled(mojom::ExternalWalletType,
                                         IsExternalWalletInstalledCallback);
  virtual void IsExternalWalletInitialized(mojom::ExternalWalletType,
                                           IsExternalWalletInitializedCallback);
  virtual void GetImportInfoFromExternalWallet(mojom::ExternalWalletType type,
                                               const std::string& password,
                                               GetImportInfoCallback callback);
  virtual bool AddPermission(mojom::CoinType coin,
                             const url::Origin& origin,
                             const std::string& account);
  virtual bool HasPermission(mojom::CoinType coin,
                             const url::Origin& origin,
                             const std::string& account);
  virtual bool ResetPermission(mojom::CoinType coin,
                               const url::Origin& origin,
                               const std::string& account);
  virtual bool IsPermissionDenied(mojom::CoinType coin,
                                  const url::Origin& origin);
  virtual void ResetAllPermissions() {}
  virtual void GetWebSitesWithPermission(
      mojom::CoinType coin,
      GetWebSitesWithPermissionCallback callback);
  virtual void ResetWebSitePermission(mojom::CoinType coin,
                                      const std::string& formed_website,
                                      ResetWebSitePermissionCallback callback);

  virtual std::optional<url::Origin> GetActiveOrigin();

  virtual void ClearWalletUIStoragePartition();

  virtual base::FilePath GetWalletBaseDirectory() = 0;

  virtual bool IsPrivateWindow() = 0;

  static std::unique_ptr<WootzWalletServiceDelegate> Create(
      content::BrowserContext* browser_context);
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_SERVICE_DELEGATE_H_
