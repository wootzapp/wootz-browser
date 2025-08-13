// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_

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
#include "components/automation_agent/content/browser/automation_controller.h"

#include "base/functional/callback.h"

class Profile;

namespace content {
class BrowserContext;
class WebContents;
}

namespace extensions {

content::WebContents* WebContentsIdToJavaWebContents(int webContentsId);

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
  void OnDropdownButtonClicked(const std::string& selectedFeature, const std::string& extensionId, const std::string& extensionName);

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

class WootzHelloWorldFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.helloWorld", WOOTZ_HELLOWORLD)

  WootzHelloWorldFunction() = default;

  WootzHelloWorldFunction(const WootzHelloWorldFunction&) = delete;
  WootzHelloWorldFunction& operator=(const WootzHelloWorldFunction&) = delete;

 protected:
  ~WootzHelloWorldFunction() override {}

  ResponseAction Run() override;
};

class WootzShowDialogFunction : public ExtensionFunction {
    DECLARE_EXTENSION_FUNCTION("wootz.showDialog", WOOTZ_SHOWDIALOG)
    WootzShowDialogFunction() = default;

    WootzShowDialogFunction(const WootzShowDialogFunction&) = delete;
    WootzShowDialogFunction& operator=(const WootzShowDialogFunction&) = delete;

  protected:
    ~WootzShowDialogFunction() override {}

    ResponseAction Run() override;
};

class WootzLogFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.log", WOOTZ_LOG)
  WootzLogFunction() = default;

  WootzLogFunction(
      const WootzLogFunction&) = delete;
  WootzLogFunction& operator=(
      const WootzLogFunction&) = delete;

 protected:
  ~WootzLogFunction() override {}

  ResponseAction Run() override;
};

class WootzSetSelectedChainsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.setSelectedChains", WOOTZ_SELECT_CHAIN)
  WootzSetSelectedChainsFunction() = default;

 protected:
  ~WootzSetSelectedChainsFunction() override = default;
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

class WootzIsWalletCreatedFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.isWalletCreated", WOOTZ_IS_WALLET_CREATED)
 protected:
  ~WootzIsWalletCreatedFunction() override {}
  ResponseAction Run() override;
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

class WootzSignTransactionFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.signTransaction", WOOTZ_SIGN_TRANSACTION)
  WootzSignTransactionFunction() = default;
  WootzSignTransactionFunction(const WootzSignTransactionFunction&) = delete;
  WootzSignTransactionFunction& operator=(const WootzSignTransactionFunction&) = delete;

 protected:
  ~WootzSignTransactionFunction() override {}
  ResponseAction Run() override;

 private:
  void OnTransactionSigned(bool success,
                          wootz_wallet::mojom::ProviderErrorUnionPtr error,
                          const std::string& error_message);

  void OnTransactionRejected(bool success);
  
}; 

class WootzSignSolanaTransactionFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.signSolanaTransaction", WOOTZ_SIGN_SOLANA_TRANSACTION)
  
  static void NotifyExtensionOfPendingRequest(content::BrowserContext* context);

 protected:
  ~WootzSignSolanaTransactionFunction() override {}
  ResponseAction Run() override;
 
 private:
  static void OnGetPendingRequests(
    content::BrowserContext* context,
    std::vector<wootz_wallet::mojom::SignTransactionRequestPtr> requests);
};

// background service api
class WootzSetJobFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.setJob", WOOTZ_SETJOB)
  WootzSetJobFunction() = default;
  WootzSetJobFunction(const WootzSetJobFunction&) = delete;
  WootzSetJobFunction& operator=(const WootzSetJobFunction&) = delete;

 protected:
  ~WootzSetJobFunction() override = default;
  ResponseAction Run() override;
};

class WootzRemoveJobFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.removeJob", WOOTZ_REMOVEJOB)
  WootzRemoveJobFunction() = default;
  WootzRemoveJobFunction(const WootzRemoveJobFunction&) = delete;
  WootzRemoveJobFunction& operator=(const WootzRemoveJobFunction&) = delete;

 protected:
  ~WootzRemoveJobFunction() override = default;
  ResponseAction Run() override;
};

class WootzGetJobsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.getJobs", WOOTZ_GETJOBS)
  WootzGetJobsFunction() = default;
  WootzGetJobsFunction(const WootzGetJobsFunction&) = delete;
  WootzGetJobsFunction& operator=(const WootzGetJobsFunction&) = delete;

 protected:
  ~WootzGetJobsFunction() override = default;
  ResponseAction Run() override;
};

class WootzListJobsFunction : public ExtensionFunction {
public:
 DECLARE_EXTENSION_FUNCTION("wootz.listJobs", WOOTZ_LISTJOBS)

 WootzListJobsFunction() = default;

 WootzListJobsFunction(const WootzListJobsFunction&) = delete;
 WootzListJobsFunction& operator=(const WootzListJobsFunction&) = delete;

protected:
 ~WootzListJobsFunction() override {}

 ResponseAction Run() override;
};

class WootzCleanJobsFunction : public ExtensionFunction {
public:
 DECLARE_EXTENSION_FUNCTION("wootz.cleanJobs", WOOTZ_CLEANJOBS)

 WootzCleanJobsFunction() = default;

 WootzCleanJobsFunction(const WootzCleanJobsFunction&) = delete;
 WootzCleanJobsFunction& operator=(const WootzCleanJobsFunction&) = delete;

protected:
 ~WootzCleanJobsFunction() override {}

 ResponseAction Run() override;
};


class WootzGetBrowserInfoFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.getBrowserInfo", WOOTZ_GET_BROWSER_INFO)

  WootzGetBrowserInfoFunction() = default;

  WootzGetBrowserInfoFunction(const WootzGetBrowserInfoFunction&) = delete;
  WootzGetBrowserInfoFunction& operator=(const WootzGetBrowserInfoFunction&) = delete;

 protected:
  ~WootzGetBrowserInfoFunction() override {}

  ResponseAction Run() override;
};

class WootzGenerateZKProofFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.generateZKProof", WOOTZ_GENERATE_ZK_PROOF)

  WootzGenerateZKProofFunction() = default;
  WootzGenerateZKProofFunction(const WootzGenerateZKProofFunction&) = delete;
  WootzGenerateZKProofFunction& operator=(const WootzGenerateZKProofFunction&) = delete;

 protected:
  ~WootzGenerateZKProofFunction() override {}
  ResponseAction Run() override;
  
};

class WootzSetBlinksEnabledFunction : public ExtensionFunction {
 public:
  WootzSetBlinksEnabledFunction() = default;
  DECLARE_EXTENSION_FUNCTION("wootz.setBlinksEnabled", WOOTZ_SETBLINKSENABLED)
 protected:
  ~WootzSetBlinksEnabledFunction() override = default;
  ResponseAction Run() override;
};


class WootzReplaceAdFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.replaceAd", WOOTZ_REPLACE_AD)
  WootzReplaceAdFunction() = default;
  WootzReplaceAdFunction(const WootzReplaceAdFunction&) = delete;
  WootzReplaceAdFunction& operator=(const WootzReplaceAdFunction&) = delete;

 protected:
  ~WootzReplaceAdFunction() override {}
  ResponseAction Run() override;
};

class WootzGetPageStateFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.getPageState", WOOTZ_GET_PAGE_STATE)
  ResponseAction Run() override;

 private:
  void OnGetPageStateComplete(bool success, const std::string& state);
};

class WootzPerformActionFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.performAction", WOOTZ_PERFORM_ACTION)
  ResponseAction Run() override;

 private:
  void OnActionComplete(bool success);
};


class WootzSubmitSamlResponseFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.submitSamlResponse", WOOTZ_SUBMIT_SAML_RESPONSE)
 protected:
  ~WootzSubmitSamlResponseFunction() override {}
  ResponseAction Run() override;
};

class WootzCreateBackgroundWebContentsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.createBackgroundWebContents", WOOTZ_CREATE_BACKGROUND_WEBCONTENTS)
  WootzCreateBackgroundWebContentsFunction() = default;
 protected:
  ~WootzCreateBackgroundWebContentsFunction() override = default;
  ResponseAction Run() override;
};

class WootzDestroyBackgroundWebContentsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.destroyBackgroundWebContents", WOOTZ_DESTROY_BACKGROUND_WEBCONTENTS)
  WootzDestroyBackgroundWebContentsFunction() = default;
 protected:
  ~WootzDestroyBackgroundWebContentsFunction() override = default;
  ResponseAction Run() override;
};

class WootzMaskSensitiveElementsFunction : public ExtensionFunction {
 public:
  WootzMaskSensitiveElementsFunction();
  DECLARE_EXTENSION_FUNCTION("wootz.maskSensitiveElements", WOOTZ_MASK_SENSITIVE_ELEMENTS)
  
 protected:
  ~WootzMaskSensitiveElementsFunction() override;
  ResponseAction Run() override;

 private:
  void SendSelectorsToRenderer(const std::vector<std::string>& selectors, int tab_id = -1);
  void OnMaskingComplete(int masked_count);
  
  // WeakPtr factory for safe async operations (must be last member)
  base::WeakPtrFactory<WootzMaskSensitiveElementsFunction> weak_factory_{this};
 };

class WootzChangeWootzAppSearchConfigurationFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.changeWootzAppSearchConfiguration", WOOTZ_CHANGE_WOOTZAPP_SEARCH_CONFIGURATION)
  WootzChangeWootzAppSearchConfigurationFunction() = default;

 protected:
  ~WootzChangeWootzAppSearchConfigurationFunction() override = default;
  ResponseAction Run() override;
};

}  // namespace extensions
#endif  // CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_
