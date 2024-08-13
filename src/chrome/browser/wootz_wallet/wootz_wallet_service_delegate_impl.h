/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef WOOTZ_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_IMPL_H_
#define WOOTZ_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_IMPL_H_

#include <memory>
#include <optional>
#include <string>

#include "base/containers/flat_map.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_delegate_base.h"
#include "chrome/browser/wootz_wallet/external_wallets_importer.h"
#include "chrome/components/wootz_wallet/browser/wootz_wallet_service_delegate.h"
#include "chrome/components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "chrome/browser/ui/browser_tab_strip_tracker.h"
#include "chrome/browser/ui/browser_tab_strip_tracker_delegate.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"

namespace content {
class BrowserContext;
class WebContents;
}  // namespace content

namespace wootz_wallet {

class WootzWalletServiceDelegateImpl : public WootzWalletServiceDelegateBase,
                                       public TabStripModelObserver,
                                       public BrowserTabStripTrackerDelegate {
 public:
  explicit WootzWalletServiceDelegateImpl(content::BrowserContext* context);
  WootzWalletServiceDelegateImpl(const WootzWalletServiceDelegateImpl&) =
      delete;
  WootzWalletServiceDelegateImpl& operator=(
      const WootzWalletServiceDelegateImpl&) = delete;
  ~WootzWalletServiceDelegateImpl() override;

  static void SetActiveWebContentsForTesting(
      content::WebContents* web_contents);

  void IsExternalWalletInstalled(mojom::ExternalWalletType,
                                 IsExternalWalletInstalledCallback) override;
  void IsExternalWalletInitialized(
      mojom::ExternalWalletType,
      IsExternalWalletInitializedCallback) override;
  void GetImportInfoFromExternalWallet(mojom::ExternalWalletType type,
                                       const std::string& password,
                                       GetImportInfoCallback callback) override;

  std::optional<url::Origin> GetActiveOrigin() override;

  void ClearWalletUIStoragePartition() override;

  void AddObserver(WootzWalletServiceDelegate::Observer* observer) override;
  void RemoveObserver(WootzWalletServiceDelegate::Observer* observer) override;

  // TabStripModelObserver:
  void OnTabStripModelChanged(
      TabStripModel* tab_strip_model,
      const TabStripModelChange& change,
      const TabStripSelectionChange& selection) override;
  void TabChangedAt(content::WebContents* contents,
                    int index,
                    TabChangeType change_type) override;

  // BrowserTabStripTrackerDelegate:
  bool ShouldTrackBrowser(Browser* browser) override;

 private:
  friend class WootzWalletServiceDelegateImplUnitTest;

  void ContinueIsExternalWalletInitialized(mojom::ExternalWalletType type,
                                           IsExternalWalletInitializedCallback,
                                           bool init_success);
  void ContinueGetImportInfoFromExternalWallet(mojom::ExternalWalletType type,
                                               const std::string& password,
                                               GetImportInfoCallback callback,
                                               bool init_success);

  std::optional<url::Origin> GetActiveOriginInternal();
  void FireActiveOriginChanged();

  base::flat_map<mojom::ExternalWalletType,
                 std::unique_ptr<ExternalWalletsImporter>>
      importers_;

  BrowserTabStripTracker browser_tab_strip_tracker_;
  base::ObserverList<WootzWalletServiceDelegate::Observer> observer_list_;

  base::WeakPtrFactory<WootzWalletServiceDelegateImpl> weak_ptr_factory_;
};

}  // namespace wootz_wallet

#endif  // WOOTZ_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_DELEGATE_IMPL_H_
