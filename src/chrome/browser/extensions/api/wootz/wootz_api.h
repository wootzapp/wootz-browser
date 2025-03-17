// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_WOOTZAPP_WOOTZAPP_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_WOOTZAPP_WOOTZAPP_API_H_

#include <set>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/extensions/extension_icon_manager.h"
#include "chrome/common/extensions/api/omnibox.h"
#include "components/search_engines/template_url_service.h"
#include "components/wootz_wallet/browser/tx_state_manager.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom-forward.h"
#include "content/public/browser/file_select_listener.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/common/extension_id.h"
#include "ui/base/window_open_disposition.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/browser/tx_service.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "ui/shell_dialogs/select_file_dialog.h"
#include "ui/shell_dialogs/select_file_dialog_android.h"
#include "ui/shell_dialogs/selected_file_info.h"
#include "third_party/blink/public/mojom/choosers/file_chooser.mojom.h"
#include "mojo/public/cpp/bindings/receiver.h"

#include "base/functional/callback.h"

class Profile;

namespace content {
class BrowserContext;
class WebContents;
}

namespace extensions {

class WootzAPI : public BrowserContextKeyedAPI,
                 public wootz_wallet::mojom::TxServiceObserver {
 public:
  explicit WootzAPI(content::BrowserContext* context);
  ~WootzAPI() override;

  // BrowserContextKeyedAPI implementation
  static BrowserContextKeyedAPIFactory<WootzAPI>* GetFactoryInstance();
  // TxServiceObserver implementation
  void OnNewUnapprovedTx(wootz_wallet::mojom::TransactionInfoPtr tx_info) override;
  void OnUnapprovedTxUpdated(wootz_wallet::mojom::TransactionInfoPtr tx_info) override;
  void OnTransactionStatusChanged(wootz_wallet::mojom::TransactionInfoPtr tx_info) override;
  void OnTxServiceReset() override;

 private:
  friend class BrowserContextKeyedAPIFactory<WootzAPI>;

  void StartObserving();
  void StopObserving();
  void DispatchEvent(events::HistogramValue histogram_value,
                    const std::string& event_name,
                    base::Value::List args);

  raw_ptr<content::BrowserContext> browser_context_;
  mojo::Receiver<wootz_wallet::mojom::TxServiceObserver> observer_receiver_;

  static const char* service_name() { return "WootzAPI"; }
  static const bool kServiceIsNULLWhileTesting = true;
};

class WootzappInfoFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.info", WOOTZAPP_INFO)

  WootzappInfoFunction() = default;

  WootzappInfoFunction(
      const WootzappInfoFunction&) = delete;
  WootzappInfoFunction& operator=(
      const WootzappInfoFunction&) = delete;

 protected:
  ~WootzappInfoFunction() override {}

  ResponseAction Run() override;
};

class WootzappHelloWorldFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.helloWorld", WOOTZAPP_HELLOWORLD)

  WootzappHelloWorldFunction() = default;

  WootzappHelloWorldFunction(const WootzappHelloWorldFunction&) = delete;
  WootzappHelloWorldFunction& operator=(const WootzappHelloWorldFunction&) = delete;

 protected:
  ~WootzappHelloWorldFunction() override {}

  ResponseAction Run() override;
};

class WootzappShowDialogFunction : public ExtensionFunction {
    DECLARE_EXTENSION_FUNCTION("wootzapp.showDialog", WOOTZAPP_SHOWDIALOG)
    WootzappShowDialogFunction() = default;

    WootzappShowDialogFunction(const WootzappShowDialogFunction&) = delete;
    WootzappShowDialogFunction& operator=(const WootzappShowDialogFunction&) = delete;

  protected:
    ~WootzappShowDialogFunction() override {}

    ResponseAction Run() override;
};

class WootzappLogFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.log", WOOTZAPP_LOG)
  WootzappLogFunction() = default;

  WootzappLogFunction(
      const WootzappLogFunction&) = delete;
  WootzappLogFunction& operator=(
      const WootzappLogFunction&) = delete;

 protected:
  ~WootzappLogFunction() override {}

  ResponseAction Run() override;
};

class WootzappSetSelectedChainsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.setSelectedChains", WOOTZAPP_SELECT_CHAIN)
  WootzappSetSelectedChainsFunction() = default;

 protected:
  ~WootzappSetSelectedChainsFunction() override = default;
  ResponseAction Run() override;
};

class WootzappCreateWalletFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.createWallet", WOOTZAPP_CREATE_WALLET)
 protected:
  ~WootzappCreateWalletFunction() override {}
  ResponseAction Run() override;
 private:
  void OnWalletCreated(const std::optional<std::string>& recovery_phrase);
};

class WootzappIsWalletCreatedFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.isWalletCreated", WOOTZAPP_IS_WALLET_CREATED)
 protected:
  ~WootzappIsWalletCreatedFunction() override {}
  ResponseAction Run() override;
};

class WootzappUnlockWalletFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.unlockWallet", WOOTZAPP_UNLOCK_WALLET)
 protected:
  ~WootzappUnlockWalletFunction() override {}
  ResponseAction Run() override;
 private:
  void OnUnlocked(bool success);
};

class WootzappLockWalletFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.lockWallet", WOOTZAPP_LOCK_WALLET)
 protected:
  ~WootzappLockWalletFunction() override {}
  ResponseAction Run() override;
};

class WootzappIsLockedFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.isLocked", WOOTZAPP_IS_LOCKED)
 protected:
  ~WootzappIsLockedFunction() override {}
  ResponseAction Run() override;
 private:
  void OnIsLocked(bool is_locked);
};

class WootzappGetAllAccountsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.getAllAccounts", WOOTZAPP_GET_ALL_ACCOUNTS)
 protected:
  ~WootzappGetAllAccountsFunction() override {}
  ResponseAction Run() override;

 private:
  void OnGetAllAccounts(wootz_wallet::mojom::AllAccountsInfoPtr all_accounts_info);
  
  base::WeakPtrFactory<WootzappGetAllAccountsFunction> weak_factory_{this};
};

class WootzappSignMessageFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.signMessage", WOOTZAPP_SIGN_MESSAGE)
  
  static void NotifyExtensionOfPendingRequest(content::BrowserContext* context);

 private:
  ~WootzappSignMessageFunction() override {}
  ResponseAction Run() override;
  static void OnGetPendingRequests(
      content::BrowserContext* context,
      std::vector<wootz_wallet::mojom::SignMessageRequestPtr> requests);
};

class WootzappSignTransactionFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.signTransaction", WOOTZAPP_SIGN_TRANSACTION)
  WootzappSignTransactionFunction() = default;
  WootzappSignTransactionFunction(const WootzappSignTransactionFunction&) = delete;
  WootzappSignTransactionFunction& operator=(const WootzappSignTransactionFunction&) = delete;

 protected:
  ~WootzappSignTransactionFunction() override {}
  ResponseAction Run() override;

 private:
  void OnTransactionSigned(bool success,
                          wootz_wallet::mojom::ProviderErrorUnionPtr error,
                          const std::string& error_message);

  void OnTransactionRejected(bool success);
  
}; 

class WootzappSignSolanaTransactionFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.signSolanaTransaction", WOOTZAPP_SIGN_SOLANA_TRANSACTION)
  
  static void NotifyExtensionOfPendingRequest(content::BrowserContext* context);

 protected:
  ~WootzappSignSolanaTransactionFunction() override {}
  ResponseAction Run() override;
 
 private:
  static void OnGetPendingRequests(
    content::BrowserContext* context,
    std::vector<wootz_wallet::mojom::SignTransactionRequestPtr> requests);
};

// background service api
class WootzappSetJobFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.setJob", WOOTZAPP_SETJOB)
  WootzappSetJobFunction() = default;
  WootzappSetJobFunction(const WootzappSetJobFunction&) = delete;
  WootzappSetJobFunction& operator=(const WootzappSetJobFunction&) = delete;

 protected:
  ~WootzappSetJobFunction() override = default;
  ResponseAction Run() override;
};

class WootzappRemoveJobFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.removeJob", WOOTZAPP_REMOVEJOB)
  WootzappRemoveJobFunction() = default;
  WootzappRemoveJobFunction(const WootzappRemoveJobFunction&) = delete;
  WootzappRemoveJobFunction& operator=(const WootzappRemoveJobFunction&) = delete;

 protected:
  ~WootzappRemoveJobFunction() override = default;
  ResponseAction Run() override;
};

class WootzappGetJobsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.getJobs", WOOTZAPP_GETJOBS)
  WootzappGetJobsFunction() = default;
  WootzappGetJobsFunction(const WootzappGetJobsFunction&) = delete;
  WootzappGetJobsFunction& operator=(const WootzappGetJobsFunction&) = delete;

 protected:
  ~WootzappGetJobsFunction() override = default;
  ResponseAction Run() override;
};

class WootzappListJobsFunction : public ExtensionFunction {
public:
 DECLARE_EXTENSION_FUNCTION("wootzapp.listJobs", WOOTZAPP_LISTJOBS)

 WootzappListJobsFunction() = default;

 WootzappListJobsFunction(const WootzappListJobsFunction&) = delete;
 WootzappListJobsFunction& operator=(const WootzappListJobsFunction&) = delete;

protected:
 ~WootzappListJobsFunction() override {}

 ResponseAction Run() override;
};

class WootzappCleanJobsFunction : public ExtensionFunction {
public:
 DECLARE_EXTENSION_FUNCTION("wootzapp.cleanJobs", WOOTZAPP_CLEANJOBS)

 WootzappCleanJobsFunction() = default;

 WootzappCleanJobsFunction(const WootzappCleanJobsFunction&) = delete;
 WootzappCleanJobsFunction& operator=(const WootzappCleanJobsFunction&) = delete;

protected:
 ~WootzappCleanJobsFunction() override {}

 ResponseAction Run() override;
};

class WootzappGetBrowserInfoFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootzapp.getBrowserInfo", WOOTZAPP_GET_BROWSER_INFO)

  WootzappGetBrowserInfoFunction() = default;

  WootzappGetBrowserInfoFunction(const WootzappGetBrowserInfoFunction&) = delete;
  WootzappGetBrowserInfoFunction& operator=(const WootzappGetBrowserInfoFunction&) = delete;

 protected:
  ~WootzappGetBrowserInfoFunction() override {}

  ResponseAction Run() override;
};

}  // namespace extensions
#endif  // CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_
