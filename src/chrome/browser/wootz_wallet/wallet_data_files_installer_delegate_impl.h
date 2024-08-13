/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef WOOTZ_BROWSER_WOOTZ_WALLET_WALLET_DATA_FILES_INSTALLER_DELEGATE_IMPL_H_
#define WOOTZ_BROWSER_WOOTZ_WALLET_WALLET_DATA_FILES_INSTALLER_DELEGATE_IMPL_H_

#include "chrome/components/wootz_wallet/browser/wallet_data_files_installer_delegate.h"

namespace wootz_wallet {

class WalletDataFilesInstallerDelegateImpl
    : public WalletDataFilesInstallerDelegate {
 public:
  WalletDataFilesInstallerDelegateImpl() = default;
  ~WalletDataFilesInstallerDelegateImpl() override = default;
  component_updater::ComponentUpdateService* GetComponentUpdater() override;
};

}  // namespace wootz_wallet

#endif  // WOOTZ_BROWSER_WOOTZ_WALLET_WALLET_DATA_FILES_INSTALLER_DELEGATE_IMPL_H_
