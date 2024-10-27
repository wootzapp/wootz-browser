// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_

#include <set>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/extensions/extension_icon_manager.h"
#include "chrome/common/extensions/api/omnibox.h"
#include "components/search_engines/template_url_service.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/common/extension_id.h"
#include "ui/base/window_open_disposition.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

class Profile;

namespace content {
class BrowserContext;
class WebContents;
}

namespace extensions {
class WootzInfoFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.info", WOOTZ_INFO)

  WootzInfoFunction() = default;

  WootzInfoFunction(
      const WootzInfoFunction&) = delete;
  WootzInfoFunction& operator=(
      const WootzInfoFunction&) = delete;

 protected:
  ~WootzInfoFunction() override {}

  ResponseAction Run() override;
};


class WootzCreateWalletFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.createWallet", WOOTZ_CREATE_WALLET)
 protected:
  ~WootzCreateWalletFunction() override {}
  ResponseAction Run() override;
 private:
  void OnWalletCreated(const std::optional<std::string>& recovery_phrase);
};

class WootzUnlockWalletFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.unlockWallet", WOOTZ_UNLOCK_WALLET)
 protected:
  ~WootzUnlockWalletFunction() override {}
  ResponseAction Run() override;
 private:
  void OnUnlocked(bool success);
};

class WootzLockWalletFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.lockWallet", WOOTZ_LOCK_WALLET)
 protected:
  ~WootzLockWalletFunction() override {}
  ResponseAction Run() override;
};

class WootzIsLockedFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.isLocked", WOOTZ_IS_LOCKED)
 protected:
  ~WootzIsLockedFunction() override {}
  ResponseAction Run() override;
 private:
  void OnIsLocked(bool is_locked);
};

class WootzGetAllAccountsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.getAllAccounts", WOOTZ_GET_ALL_ACCOUNTS)
 protected:
  ~WootzGetAllAccountsFunction() override {}
  ResponseAction Run() override;

 private:
  void OnGetAllAccounts(wootz_wallet::mojom::AllAccountsInfoPtr all_accounts_info);
  
  base::WeakPtrFactory<WootzGetAllAccountsFunction> weak_factory_{this};
};

class WootzSignMessageFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.signMessage", WOOTZ_SIGN_MESSAGE)
  
  static void NotifyExtensionOfPendingRequest(content::BrowserContext* context);

 private:
  ~WootzSignMessageFunction() override {}
  ResponseAction Run() override;
  static void OnGetPendingRequests(
      content::BrowserContext* context,
      std::vector<wootz_wallet::mojom::SignMessageRequestPtr> requests);
};
}

#endif  // CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_
