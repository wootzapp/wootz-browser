/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/wootz_wallet_service_delegate.h"

#include <optional>
#include <utility>
#include <vector>

#include "base/notreached.h"

namespace wootz_wallet {

void WootzWalletServiceDelegate::IsExternalWalletInstalled(
    mojom::ExternalWalletType type,
    IsExternalWalletInstalledCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(false);
}

void WootzWalletServiceDelegate::IsExternalWalletInitialized(
    mojom::ExternalWalletType type,
    IsExternalWalletInitializedCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(false);
}

void WootzWalletServiceDelegate::GetImportInfoFromExternalWallet(
    mojom::ExternalWalletType type,
    const std::string& password,
    GetImportInfoCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(false, ImportInfo(), ImportError::kInternalError);
}

bool WootzWalletServiceDelegate::AddPermission(mojom::CoinType coin,
                                               const url::Origin& origin,
                                               const std::string& account) {
  NOTIMPLEMENTED();
  return false;
}

bool WootzWalletServiceDelegate::HasPermission(mojom::CoinType coin,
                                               const url::Origin& origin,
                                               const std::string& account) {
  NOTIMPLEMENTED();
  return false;
}

bool WootzWalletServiceDelegate::ResetPermission(mojom::CoinType coin,
                                                 const url::Origin& origin,
                                                 const std::string& account) {
  NOTIMPLEMENTED();
  return false;
}

bool WootzWalletServiceDelegate::IsPermissionDenied(mojom::CoinType coin,
                                                    const url::Origin& origin) {
  NOTIMPLEMENTED();
  return false;
}

std::optional<url::Origin> WootzWalletServiceDelegate::GetActiveOrigin() {
  NOTIMPLEMENTED();
  return std::nullopt;
}

void WootzWalletServiceDelegate::ClearWalletUIStoragePartition() {}

void WootzWalletServiceDelegate::GetWebSitesWithPermission(
    mojom::CoinType coin,
    GetWebSitesWithPermissionCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(std::vector<std::string>());
}

void WootzWalletServiceDelegate::ResetWebSitePermission(
    mojom::CoinType coin,
    const std::string& formed_website,
    ResetWebSitePermissionCallback callback) {
  NOTIMPLEMENTED();
  std::move(callback).Run(false);
}

}  // namespace wootz_wallet
