/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_PROVIDER_DELEGATE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_PROVIDER_DELEGATE_H_

#include <optional>
#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom-forward.h"
#include "url/origin.h"

namespace wootz_wallet {

class WootzWalletProviderDelegate {
 public:
  using RequestPermissionsCallback = base::OnceCallback<void(
      mojom::RequestPermissionsError error,
      const std::optional<std::vector<std::string>>& allowed_accounts)>;

  WootzWalletProviderDelegate() = default;
  WootzWalletProviderDelegate(const WootzWalletProviderDelegate&) = delete;
  WootzWalletProviderDelegate& operator=(const WootzWalletProviderDelegate&) =
      delete;
  virtual ~WootzWalletProviderDelegate() = default;

  virtual bool IsTabVisible() = 0;
  virtual void ShowPanel() = 0;
  virtual void ShowWalletBackup() = 0;
  virtual void WalletInteractionDetected() = 0;
  virtual void ShowWalletOnboarding() = 0;
  virtual void ShowAccountCreation(mojom::CoinType type) = 0;
  virtual url::Origin GetOrigin() const = 0;
  virtual void RequestPermissions(mojom::CoinType type,
                                  const std::vector<std::string>& accounts,
                                  RequestPermissionsCallback) = 0;
  virtual bool IsAccountAllowed(mojom::CoinType type,
                                const std::string& account) = 0;
  virtual std::optional<std::vector<std::string>> GetAllowedAccounts(
      mojom::CoinType type,
      const std::vector<std::string>& accounts) = 0;
  virtual bool IsPermissionDenied(mojom::CoinType type) = 0;
  virtual void AddSolanaConnectedAccount(const std::string& account) {}
  virtual void RemoveSolanaConnectedAccount(const std::string& account) {}
  virtual bool IsSolanaAccountConnected(const std::string& account);
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_PROVIDER_DELEGATE_H_
