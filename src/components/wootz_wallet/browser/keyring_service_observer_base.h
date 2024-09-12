/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_KEYRING_SERVICE_OBSERVER_BASE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_KEYRING_SERVICE_OBSERVER_BASE_H_

#include <string>
#include <vector>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

class KeyringServiceObserverBase : public mojom::KeyringServiceObserver {
  void WalletCreated() override {}
  void WalletRestored() override {}
  void WalletReset() override {}
  void Locked() override {}
  void Unlocked() override {}
  void BackedUp() override {}
  void AccountsChanged() override {}
  void AccountsAdded(std::vector<mojom::AccountInfoPtr> accounts) override {}
  void AutoLockMinutesChanged() override {}
  void SelectedWalletAccountChanged(mojom::AccountInfoPtr account) override {}
  void SelectedDappAccountChanged(mojom::CoinType coin,
                                  mojom::AccountInfoPtr account) override {}
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_KEYRING_SERVICE_OBSERVER_BASE_H_
