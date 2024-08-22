/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/wootz_wallet_service_delegate_impl_android.h"

#include <optional>
#include <utility>
#include <vector>

#include "base/ranges/algorithm.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/permission_utils.h"
#include "components/permissions/contexts/wootz_wallet_permission_context.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

namespace wootz_wallet {

namespace {
content::WebContents* GetActiveWebContents(content::BrowserContext* context) {
  auto tab_models = TabModelList::models();
  auto iter = base::ranges::find_if(
      tab_models, [](const auto& model) { return model->IsActiveModel(); });
  if (iter == tab_models.end()) {
    return nullptr;
  }

  auto* active_contents = (*iter)->GetActiveWebContents();
  if (!active_contents) {
    return nullptr;
  }
  DCHECK_EQ(active_contents->GetBrowserContext(), context);
  return active_contents;
}
}  // namespace

WootzWalletServiceDelegateImpl::WootzWalletServiceDelegateImpl(
    content::BrowserContext* context)
    : WootzWalletServiceDelegateBase(context), weak_ptr_factory_(this) {}

WootzWalletServiceDelegateImpl::~WootzWalletServiceDelegateImpl() = default;

bool WootzWalletServiceDelegateImpl::AddPermission(mojom::CoinType coin,
                                                   const url::Origin& origin,
                                                   const std::string& account) {
  auto type = CoinTypeToPermissionType(coin);
  if (!type) {
    return false;
  }

  return permissions::WootzWalletPermissionContext::AddPermission(
      *type, context_, origin, account);
}

void WootzWalletServiceDelegateImpl::GetWebSitesWithPermission(
    mojom::CoinType coin,
    GetWebSitesWithPermissionCallback callback) {
  std::vector<std::string> result;
  auto type = CoinTypeToPermissionType(coin);
  if (!type) {
    std::move(callback).Run(result);
    return;
  }
  std::move(callback).Run(
      permissions::WootzWalletPermissionContext::GetWebSitesWithPermission(
          *type, context_));
}

void WootzWalletServiceDelegateImpl::ResetWebSitePermission(
    mojom::CoinType coin,
    const std::string& formed_website,
    ResetWebSitePermissionCallback callback) {
  auto type = CoinTypeToPermissionType(coin);
  if (!type) {
    std::move(callback).Run(false);
    return;
  }
  std::move(callback).Run(
      permissions::WootzWalletPermissionContext::ResetWebSitePermission(
          *type, context_, formed_website));
}

std::optional<url::Origin> WootzWalletServiceDelegateImpl::GetActiveOrigin() {
  content::WebContents* contents = GetActiveWebContents(context_);
  auto origin = contents
                    ? contents->GetPrimaryMainFrame()->GetLastCommittedOrigin()
                    : std::optional<url::Origin>();
  return origin;
}

}  // namespace wootz_wallet
