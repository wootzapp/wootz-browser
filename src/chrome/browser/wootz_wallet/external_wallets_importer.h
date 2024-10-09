/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_EXTERNAL_WALLETS_IMPORTER_H_
#define CHROME_BROWSER_WOOTZ_WALLET_EXTERNAL_WALLETS_IMPORTER_H_

#include <memory>
#include <optional>
#include <string>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "base/values.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/wootz_wallet_types.h"

namespace content {
class BrowserContext;
}

namespace extensions {
class Extension;
}

namespace wootz_wallet {

// This is used to import data from Crypto Wallets and MetaMask, its lifetime
// should be bound to the end of each WootzWalletServiceDelegateImpl function
// calls.
class ExternalWalletsImporter {
 public:
  using InitCallback = base::OnceCallback<void(bool)>;
  using GetImportInfoCallback =
      base::OnceCallback<void(bool, ImportInfo, ImportError)>;

  explicit ExternalWalletsImporter(mojom::ExternalWalletType,
                                   content::BrowserContext*);
  ~ExternalWalletsImporter();
  ExternalWalletsImporter(const ExternalWalletsImporter&) = delete;
  ExternalWalletsImporter& operator=(const ExternalWalletsImporter&) = delete;

  // callback will be call when initialization is finished.
  void Initialize(InitCallback);
  bool IsInitialized() const;

  bool IsExternalWalletInstalled() const;
  bool IsExternalWalletInitialized() const;
  void GetImportInfo(const std::string& password, GetImportInfoCallback) const;

  void SetStorageDataForTesting(base::Value::Dict);
  void SetExternalWalletInstalledForTesting(bool installed);

 private:
  const extensions::Extension* GetCryptoWallets() const;
  const extensions::Extension* GetMetaMask() const;
  void OnCryptoWalletsLoaded(InitCallback);
  bool IsCryptoWalletsInstalledInternal() const;

  void GetLocalStorage(const extensions::Extension&, InitCallback);
  void OnGetLocalStorage(InitCallback, base::Value::Dict);

  void GetMnemonic(bool is_legacy_crypto_wallets,
                   GetImportInfoCallback callback,
                   const std::string& password) const;

  bool is_external_wallet_installed_for_testing_ = false;
  mojom::ExternalWalletType type_;
  raw_ptr<content::BrowserContext> context_ = nullptr;
  std::optional<base::Value::Dict> storage_data_;
  scoped_refptr<extensions::Extension> extension_;

  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<ExternalWalletsImporter> weak_ptr_factory_;
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_EXTERNAL_WALLETS_IMPORTER_H_
