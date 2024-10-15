/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/wootz_wallet_service_delegate_base.h"

#include "components/wootz_wallet/browser/wootz_wallet_constants.h"
#include "components/wootz_wallet/browser/permission_utils.h"
#include "components/permissions/contexts/wootz_wallet_permission_context.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_context.h"

namespace wootz_wallet {

WootzWalletServiceDelegateBase::WootzWalletServiceDelegateBase(
    content::BrowserContext* context)
    : context_(context) {
  wallet_base_directory_ = context->GetPath().AppendASCII(kWalletBaseDirectory);
  is_private_window_ =
      Profile::FromBrowserContext(context)->IsIncognitoProfile();
}

WootzWalletServiceDelegateBase::~WootzWalletServiceDelegateBase() = default;

bool WootzWalletServiceDelegateBase::HasPermission(mojom::CoinType coin,
                                                   const url::Origin& origin,
                                                   const std::string& account) {
  bool has_permission = false;
  auto type = CoinTypeToPermissionType(coin);
  if (!type) {
    return false;
  }

  bool success = permissions::WootzWalletPermissionContext::HasPermission(
      *type, context_, origin, account, &has_permission);
  return success && has_permission;
}

bool WootzWalletServiceDelegateBase::ResetPermission(
    mojom::CoinType coin,
    const url::Origin& origin,
    const std::string& account) {
  auto type = CoinTypeToPermissionType(coin);
  if (!type) {
    return false;
  }

  return permissions::WootzWalletPermissionContext::ResetPermission(
      *type, context_, origin, account);
}

bool WootzWalletServiceDelegateBase::IsPermissionDenied(
    mojom::CoinType coin,
    const url::Origin& origin) {
  auto type = CoinTypeToPermissionType(coin);
  if (!type) {
    return false;
  }

  return permissions::WootzWalletPermissionContext::IsPermissionDenied(
      *type, context_, origin);
}

void WootzWalletServiceDelegateBase::ResetAllPermissions() {
  permissions::WootzWalletPermissionContext::ResetAllPermissions(context_);
}

base::FilePath WootzWalletServiceDelegateBase::GetWalletBaseDirectory() {
  return wallet_base_directory_;
}

bool WootzWalletServiceDelegateBase::IsPrivateWindow() {
  return is_private_window_;
}

}  // namespace wootz_wallet
