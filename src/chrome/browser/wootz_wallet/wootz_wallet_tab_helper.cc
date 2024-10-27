/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/wootz_wallet_tab_helper.h"

#include <string>
#include <utility>
#include <vector>

#include "components/wootz_wallet/browser/permission_utils.h"
#include "components/constants/webui_url_constants.h"
#include "components/permissions/permission_request.h"
#include "components/permissions/permission_request_manager.h"
#include "components/permissions/request_type.h"
#include "content/public/browser/global_routing_id.h"
#include "content/public/browser/web_contents.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/wootz_wallet/wallet_bubble_manager_delegate.h"
#endif

namespace wootz_wallet {

WootzWalletTabHelper::WootzWalletTabHelper(content::WebContents* web_contents)
    : content::WebContentsUserData<WootzWalletTabHelper>(*web_contents) {}

WootzWalletTabHelper::~WootzWalletTabHelper() {
#if !BUILDFLAG(IS_ANDROID)
  if (IsShowingBubble()) {
    CloseBubble();
  }
#endif  // !BUILDFLAG(IS_ANDROID)
}

void WootzWalletTabHelper::AddSolanaConnectedAccount(
    const content::GlobalRenderFrameHostId& id,
    const std::string& account) {
  base::flat_set<std::string> connection_set;
  if (solana_connected_accounts_.contains(id)) {
    connection_set = solana_connected_accounts_.at(id);
  }
  connection_set.insert(account);
  solana_connected_accounts_[id] = std::move(connection_set);
}

void WootzWalletTabHelper::RemoveSolanaConnectedAccount(
    const content::GlobalRenderFrameHostId& id,
    const std::string& account) {
  if (!solana_connected_accounts_.contains(id)) {
    return;
  }
  auto connection_set = solana_connected_accounts_.at(id);
  connection_set.erase(account);
  solana_connected_accounts_[id] = std::move(connection_set);
}

bool WootzWalletTabHelper::IsSolanaAccountConnected(
    const content::GlobalRenderFrameHostId& id,
    const std::string& account) {
  if (!solana_connected_accounts_.contains(id)) {
    return false;
  }
  auto connection_set = solana_connected_accounts_.at(id);
  return connection_set.contains(account);
}

void WootzWalletTabHelper::ClearSolanaConnectedAccounts(
    const content::GlobalRenderFrameHostId& id) {
  solana_connected_accounts_.erase(id);
}

#if !BUILDFLAG(IS_ANDROID)
void WootzWalletTabHelper::SetCloseOnDeactivate(bool close) {
  if (wallet_bubble_manager_delegate_) {
    wallet_bubble_manager_delegate_->CloseOnDeactivate(close);
  }
  close_on_deactivate_for_testing_ = close;
}

void WootzWalletTabHelper::ShowBubble() {
  if (skip_delegate_for_testing_) {
    is_showing_bubble_for_testing_ = true;
    return;
  }
  auto bubble_url = GetBubbleURL();
  if (!bubble_url.is_valid()) {
    return;
  }
  wallet_bubble_manager_delegate_ =
      WalletBubbleManagerDelegate::Create(&GetWebContents(), bubble_url);
  wallet_bubble_manager_delegate_->ShowBubble();
  if (show_bubble_callback_for_testing_) {
    std::move(show_bubble_callback_for_testing_).Run();
  }
}

void WootzWalletTabHelper::ShowApproveWalletBubble() {
  // If the Wallet page is open, then it will try to open the UI.
  // But the user may have already had the panel UI opened.
  // We want to avoid a hiding / showing of the panel in that case.
  if (IsShowingBubble()) {
    return;
  }
  wallet_bubble_manager_delegate_ = WalletBubbleManagerDelegate::Create(
      &GetWebContents(), GetApproveBubbleURL());
  wallet_bubble_manager_delegate_->ShowBubble();
}

void WootzWalletTabHelper::CloseBubble() {
  if (skip_delegate_for_testing_) {
    is_showing_bubble_for_testing_ = false;
    return;
  }
  if (wallet_bubble_manager_delegate_) {
    wallet_bubble_manager_delegate_->CloseBubble();
  }
}

bool WootzWalletTabHelper::IsShowingBubble() {
  if (skip_delegate_for_testing_) {
    return is_showing_bubble_for_testing_;
  }
  return wallet_bubble_manager_delegate_ &&
         wallet_bubble_manager_delegate_->IsShowingBubble();
}

bool WootzWalletTabHelper::IsBubbleClosedForTesting() {
  return wallet_bubble_manager_delegate_ &&
         wallet_bubble_manager_delegate_->IsBubbleClosedForTesting();
}

GURL WootzWalletTabHelper::GetBubbleURL() {
  auto* manager =
      permissions::PermissionRequestManager::FromWebContents(&GetWebContents());
  if (!manager) {
    return GURL();
  }

  GURL webui_url = GURL(kWootzUIWalletPanelURL);

  // General panel UI if no pending ethereum permission request.
  // Only check the first entry because it will not be grouped with other
  // types.
  if (manager->Requests().empty() ||
      (manager->Requests()[0]->request_type() !=
           permissions::RequestType::kWootzEthereum &&
       manager->Requests()[0]->request_type() !=
           permissions::RequestType::kWootzSolana)) {
    return webui_url;
  }

  // Handle ConnectWithSite (ethereum permission) request.
  std::vector<std::string> accounts;
  url::Origin requesting_origin;
  for (permissions::PermissionRequest* request : manager->Requests()) {
    std::string account;
    if (!wootz_wallet::ParseRequestingOriginFromSubRequest(
            request->request_type(),
            url::Origin::Create(request->requesting_origin()),
            &requesting_origin, &account)) {
      continue;
    }
    accounts.push_back(account);
  }
  DCHECK(!accounts.empty());

  webui_url = wootz_wallet::GetConnectWithSiteWebUIURL(webui_url, accounts,
                                                       requesting_origin);
  DCHECK(webui_url.is_valid());

  return webui_url;
}

content::WebContents* WootzWalletTabHelper::GetBubbleWebContentsForTesting() {
  return wallet_bubble_manager_delegate_->GetWebContentsForTesting();
}

const std::vector<int32_t>& WootzWalletTabHelper::GetPopupIdsForTesting() {
  return wallet_bubble_manager_delegate_->GetPopupIdsForTesting();
}

GURL WootzWalletTabHelper::GetApproveBubbleURL() {
  GURL webui_url = GURL(kWootzUIWalletPanelURL);
  GURL::Replacements replacements;
  const std::string ref = "approveTransaction";
  replacements.SetRefStr(ref);
  return webui_url.ReplaceComponents(replacements);
}

#endif  // !BUILDFLAG(IS_ANDROID)

WEB_CONTENTS_USER_DATA_KEY_IMPL(WootzWalletTabHelper);

}  // namespace wootz_wallet
