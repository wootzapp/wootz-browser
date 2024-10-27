/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_PERMISSIONS_WOOTZ_DAPP_PERMISSION_PROMPT_DIALOG_CONTROLLER_ANDROID_H_
#define CHROME_BROWSER_PERMISSIONS_WOOTZ_DAPP_PERMISSION_PROMPT_DIALOG_CONTROLLER_ANDROID_H_

#include <jni.h>
#include <string>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "base/memory/raw_ptr.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace content {
class WebContents;
}

class WootzDappPermissionPromptDialogController {
 public:
  class Delegate {
   public:
    virtual void OnDialogDismissed() = 0;
    virtual void ConnectToSite(const std::vector<std::string>& accounts,
                               int permission_lifetime_option) = 0;
    virtual void CancelConnectToSite() = 0;
  };

  // Both the `delegate` and `web_contents` should outlive `this`.
  WootzDappPermissionPromptDialogController(
      Delegate* delegate,
      content::WebContents* web_contents_,
      wootz_wallet::mojom::CoinType coin_type_);
  ~WootzDappPermissionPromptDialogController();

  void ShowDialog();
  void DismissDialog();

  void OnPrimaryButtonClicked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobjectArray>& accounts,
      int permission_lifetime_option);
  void OnNegativeButtonClicked(JNIEnv* env);
  void OnDialogDismissed(JNIEnv* env);

 private:
  base::android::ScopedJavaGlobalRef<jobject> GetOrCreateJavaObject();

  raw_ptr<Delegate> delegate_ = nullptr;
  raw_ptr<content::WebContents> web_contents_ = nullptr;
  wootz_wallet::mojom::CoinType coin_type_;

  // The corresponding java object.
  base::android::ScopedJavaGlobalRef<jobject> java_object_;
};

#endif  // CHROME_BROWSER_PERMISSIONS_WOOTZ_DAPP_PERMISSION_PROMPT_DIALOG_CONTROLLER_ANDROID_H_
