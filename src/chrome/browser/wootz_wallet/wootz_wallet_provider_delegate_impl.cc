/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/wootz_wallet_provider_delegate_impl.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_provider_delegate_impl_helper.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_tab_helper.h"
#include "components/wootz_wallet/browser/wootz_wallet_constants.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/permission_utils.h"
#include "components/permissions/contexts/wootz_wallet_permission_context.h"
#include "components/content_settings/core/common/content_settings.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/visibility.h"
#include "content/public/browser/web_contents.h"

namespace wootz_wallet {

namespace {

bool IsAccountInAllowedList(const std::vector<std::string>& allowed_accounts,
                            const std::string& account) {
  for (const auto& allowed_account : allowed_accounts) {
    if (base::CompareCaseInsensitiveASCII(account, allowed_account) == 0) {
      return true;
    }
  }
  return false;
}

void OnRequestPermissions(
    const std::vector<std::string>& accounts,
    WootzWalletProviderDelegate::RequestPermissionsCallback callback,
    const std::vector<blink::mojom::PermissionStatus>& responses) {
  DCHECK(responses.empty() || responses.size() == accounts.size());

  std::vector<std::string> granted_accounts;
  for (size_t i = 0; i < responses.size(); i++) {
    if (responses[i] == blink::mojom::PermissionStatus::GRANTED) {
      granted_accounts.push_back(accounts[i]);
    }
  }
  // The responses array will be empty if operation failed.
  if (responses.empty()) {
    std::move(callback).Run(mojom::RequestPermissionsError::kInternal,
                            std::nullopt);
  } else {
    std::move(callback).Run(mojom::RequestPermissionsError::kNone,
                            granted_accounts);
  }
}

}  // namespace

WootzWalletProviderDelegateImpl::WootzWalletProviderDelegateImpl(
    content::WebContents* web_contents,
    content::RenderFrameHost* const render_frame_host)
    : WebContentsObserver(web_contents),
      web_contents_(web_contents),
      host_id_(render_frame_host->GetGlobalId()),
      weak_ptr_factory_(this) {}

WootzWalletProviderDelegateImpl::~WootzWalletProviderDelegateImpl() = default;

url::Origin WootzWalletProviderDelegateImpl::GetOrigin() const {
  auto* rfh = content::RenderFrameHost::FromID(host_id_);
  return rfh ? rfh->GetLastCommittedOrigin() : url::Origin();
}

bool WootzWalletProviderDelegateImpl::IsTabVisible() {
  return web_contents_
             ? web_contents_->GetVisibility() == content::Visibility::VISIBLE
             : false;
}

void WootzWalletProviderDelegateImpl::ShowPanel() {
  ::wootz_wallet::ShowPanel(web_contents_);
}

void WootzWalletProviderDelegateImpl::ShowWalletBackup() {
  ::wootz_wallet::ShowWalletBackup();
}

void WootzWalletProviderDelegateImpl::WalletInteractionDetected() {
  ::wootz_wallet::WalletInteractionDetected(web_contents_);
}

void WootzWalletProviderDelegateImpl::ShowWalletOnboarding() {
  ::wootz_wallet::ShowWalletOnboarding(web_contents_);
}

void WootzWalletProviderDelegateImpl::ShowAccountCreation(
    mojom::CoinType type) {
  ::wootz_wallet::ShowAccountCreation(web_contents_, type);
}

std::optional<std::vector<std::string>>
WootzWalletProviderDelegateImpl::GetAllowedAccounts(
    mojom::CoinType type,
    const std::vector<std::string>& accounts) {
  if (IsPermissionDenied(type)) {
    return std::vector<std::string>();
  }

  auto permission = CoinTypeToPermissionType(type);
  if (!permission) {
    return std::nullopt;
  }

  return permissions::WootzWalletPermissionContext::GetAllowedAccounts(
      *permission, content::RenderFrameHost::FromID(host_id_), accounts);
}

void WootzWalletProviderDelegateImpl::RequestPermissions(
    mojom::CoinType type,
    const std::vector<std::string>& accounts,
    RequestPermissionsCallback callback) {
  if (!IsWeb3NotificationAllowed()) {
    std::move(callback).Run(mojom::RequestPermissionsError::kNone,
                            std::vector<std::string>());
    return;
  }
  auto request_type = CoinTypeToPermissionRequestType(type);
  auto permission = CoinTypeToPermissionType(type);
  if (!request_type || !permission) {
    std::move(callback).Run(mojom::RequestPermissionsError::kInternal,
                            std::nullopt);
    return;
  }
  // Check if there's already a permission request in progress
  auto* rfh = content::RenderFrameHost::FromID(host_id_);
  if (rfh && permissions::WootzWalletPermissionContext::HasRequestsInProgress(
                 rfh, *request_type)) {
    std::move(callback).Run(mojom::RequestPermissionsError::kRequestInProgress,
                            std::nullopt);
    return;
  }

  permissions::WootzWalletPermissionContext::RequestPermissions(
      *permission, content::RenderFrameHost::FromID(host_id_), accounts,
      base::BindOnce(&OnRequestPermissions, accounts, std::move(callback)));
}

bool WootzWalletProviderDelegateImpl::IsAccountAllowed(
    mojom::CoinType type,
    const std::string& account) {
  auto permission = CoinTypeToPermissionType(type);
  if (!permission) {
    return false;
  }

  const auto allowed_accounts =
      permissions::WootzWalletPermissionContext::GetAllowedAccounts(
          *permission, content::RenderFrameHost::FromID(host_id_), {account});

  return allowed_accounts && IsAccountInAllowedList(*allowed_accounts, account);
}

bool WootzWalletProviderDelegateImpl::IsPermissionDenied(mojom::CoinType type) {
  auto permission = CoinTypeToPermissionType(type);
  if (!permission) {
    return false;
  }

  auto* rfh = content::RenderFrameHost::FromID(host_id_);
  if (!rfh) {
    return false;
  }

  auto* web_contents = content::WebContents::FromRenderFrameHost(rfh);

  return permissions::WootzWalletPermissionContext::IsPermissionDenied(
      *permission, web_contents->GetBrowserContext(),
      url::Origin::Create(rfh->GetLastCommittedURL()));
}

void WootzWalletProviderDelegateImpl::AddSolanaConnectedAccount(
    const std::string& account) {
  if (!web_contents_) {
    return;
  }
  auto* tab_helper =
      wootz_wallet::WootzWalletTabHelper::FromWebContents(web_contents_);
  if (tab_helper) {
    tab_helper->AddSolanaConnectedAccount(host_id_, account);
  }
}

void WootzWalletProviderDelegateImpl::RemoveSolanaConnectedAccount(
    const std::string& account) {
  if (!web_contents_) {
    return;
  }
  auto* tab_helper =
      wootz_wallet::WootzWalletTabHelper::FromWebContents(web_contents_);
  if (tab_helper) {
    tab_helper->RemoveSolanaConnectedAccount(host_id_, account);
  }
}

bool WootzWalletProviderDelegateImpl::IsSolanaAccountConnected(
    const std::string& account) {
  if (!web_contents_) {
    return false;
  }
  auto* tab_helper =
      wootz_wallet::WootzWalletTabHelper::FromWebContents(web_contents_);
  if (!tab_helper) {
    return false;
  }

  return tab_helper->IsSolanaAccountConnected(host_id_, account);
}

void WootzWalletProviderDelegateImpl::WebContentsDestroyed() {
  web_contents_ = nullptr;
}

// This is for dapp inside iframe navigates away.
void WootzWalletProviderDelegateImpl::RenderFrameHostChanged(
    content::RenderFrameHost* old_host,
    content::RenderFrameHost* new_host) {
  if (!old_host || old_host != content::RenderFrameHost::FromID(host_id_)) {
    return;
  }
  auto* tab_helper =
      wootz_wallet::WootzWalletTabHelper::FromWebContents(web_contents_);
  if (tab_helper) {
    tab_helper->ClearSolanaConnectedAccounts(host_id_);
  }
}

void WootzWalletProviderDelegateImpl::PrimaryPageChanged(content::Page& page) {
  if (!web_contents_) {
    return;
  }
  auto* tab_helper =
      wootz_wallet::WootzWalletTabHelper::FromWebContents(web_contents_);
  if (tab_helper) {
    tab_helper->ClearSolanaConnectedAccounts(host_id_);
  }
}

}  // namespace wootz_wallet
