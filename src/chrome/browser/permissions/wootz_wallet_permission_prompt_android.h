/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_PERMISSIONS_WOOTZ_WALLET_PERMISSION_PROMPT_ANDROID_H_
#define CHROME_BROWSER_PERMISSIONS_WOOTZ_WALLET_PERMISSION_PROMPT_ANDROID_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "chrome/browser/permissions/wootz_dapp_permission_prompt_dialog_controller_android.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/permissions/permissions_client.h"

namespace content {
class WebContents;
}

namespace permissions {
class PermissionPromptAndroid;
}

class WootzWalletPermissionPrompt
    : public WootzDappPermissionPromptDialogController::Delegate,
      public permissions::PermissionsClient::PermissionMessageDelegate {
 public:
  class Delegate {
   public:
    Delegate();
    Delegate(const base::WeakPtr<permissions::PermissionPromptAndroid>&
                 permission_prompt);
    virtual ~Delegate();
    virtual void Closing();

   private:
    base::WeakPtr<permissions::PermissionPromptAndroid> permission_prompt_;
  };

  WootzWalletPermissionPrompt(content::WebContents* web_contents,
                              std::unique_ptr<Delegate> delegate,
                              wootz_wallet::mojom::CoinType coin_type);
  ~WootzWalletPermissionPrompt() override;

 protected:
  // WootzDappPermissionPromptDialogController::Delegate:
  void OnDialogDismissed() override;
  void ConnectToSite(const std::vector<std::string>& accounts,
                     int permission_lifetime_option) override;
  void CancelConnectToSite() override;

 private:
  std::unique_ptr<WootzDappPermissionPromptDialogController> dialog_controller_;
  raw_ptr<content::WebContents> web_contents_ = nullptr;
  std::unique_ptr<Delegate> delegate_;

  bool has_interacted_with_dialog_ = false;
};

#endif  // CHROME_BROWSER_PERMISSIONS_WOOTZ_WALLET_PERMISSION_PROMPT_ANDROID_H_
