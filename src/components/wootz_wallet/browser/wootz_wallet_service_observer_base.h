/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_SERVICE_OBSERVER_BASE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_SERVICE_OBSERVER_BASE_H_

#include <string>
#include <vector>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

class WootzWalletServiceObserverBase
    : public mojom::WootzWalletServiceObserver {
  void OnActiveOriginChanged(mojom::OriginInfoPtr origin_info) override {}
  void OnDefaultEthereumWalletChanged(mojom::DefaultWallet wallet) override {}
  void OnDefaultSolanaWalletChanged(mojom::DefaultWallet wallet) override {}
  void OnDefaultBaseCurrencyChanged(const std::string& currency) override {}
  void OnDefaultBaseCryptocurrencyChanged(
      const std::string& cryptocurrency) override {}
  void OnNetworkListChanged() override {}
  void OnDiscoverAssetsStarted() override {}
  void OnDiscoverAssetsCompleted(
      std::vector<mojom::BlockchainTokenPtr> discovered_assets) override {}
  void OnResetWallet() override {}
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_SERVICE_OBSERVER_BASE_H_
