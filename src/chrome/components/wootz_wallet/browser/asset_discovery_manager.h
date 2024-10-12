/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef WOOTZ_COMPONENTS_WOOTZ_WALLET_BROWSER_ASSET_DISCOVERY_MANAGER_H_
#define WOOTZ_COMPONENTS_WOOTZ_WALLET_BROWSER_ASSET_DISCOVERY_MANAGER_H_

#include <map>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chrome/components/api_request_helper/api_request_helper.h"
#include "chrome/components/wootz_wallet/browser/asset_discovery_task.h"
#include "chrome/components/wootz_wallet/browser/keyring_service_observer_base.h"
#include "chrome/components/wootz_wallet/browser/simple_hash_client.h"
#include "chrome/components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "mojo/public/cpp/bindings/receiver.h"

class PrefService;

namespace wootz_wallet {

class WootzWalletService;
class JsonRpcService;
class SimpleHashClient;
class KeyringService;
class NetworkManager;

class AssetDiscoveryManager : public KeyringServiceObserverBase {
 public:
  using APIRequestHelper = api_request_helper::APIRequestHelper;
  using APIRequestResult = api_request_helper::APIRequestResult;
  AssetDiscoveryManager(
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
      WootzWalletService* wallet_service,
      JsonRpcService* json_rpc_service,
      KeyringService* keyring_service,
      SimpleHashClient* simple_hash_client,
      PrefService* prefs);

  AssetDiscoveryManager(const AssetDiscoveryManager&) = delete;
  AssetDiscoveryManager& operator=(AssetDiscoveryManager&) = delete;
  ~AssetDiscoveryManager() override;

  // KeyringServiceObserverBase:
  void AccountsAdded(
      std::vector<mojom::AccountInfoPtr> added_accounts) override;

  // Called by frontend via WootzWalletService and when new accounts are added
  // via the KeyringServiceObserver implementation
  void DiscoverAssetsOnAllSupportedChains(
      const std::map<mojom::CoinType, std::vector<std::string>>&
          account_addresses,
      bool bypass_rate_limit);

  void SetQueueForTesting(
      std::queue<std::unique_ptr<AssetDiscoveryTask>> queue) {
    queue_ = std::move(queue);
  }

  size_t GetQueueSizeForTesting() { return queue_.size(); }

 private:
  friend class AssetDiscoveryManagerUnitTest;
  FRIEND_TEST_ALL_PREFIXES(AssetDiscoveryManagerUnitTest,
                           GetFungibleSupportedChains);
  FRIEND_TEST_ALL_PREFIXES(AssetDiscoveryManagerUnitTest,
                           GetNonFungibleSupportedChains);

  const std::map<mojom::CoinType, std::vector<std::string>>&
  GetFungibleSupportedChains();
  const std::map<mojom::CoinType, std::vector<std::string>>
  GetNonFungibleSupportedChains();

  void AddTask(const std::map<mojom::CoinType, std::vector<std::string>>&
                   account_addresses);
  void FinishTask();

  std::unique_ptr<APIRequestHelper> api_request_helper_;
  std::queue<std::unique_ptr<AssetDiscoveryTask>> queue_;
  raw_ptr<WootzWalletService> wallet_service_;
  raw_ptr<NetworkManager> network_manager_;
  raw_ptr<JsonRpcService> json_rpc_service_;
  raw_ptr<KeyringService> keyring_service_;
  raw_ptr<SimpleHashClient> simple_hash_client_;
  raw_ptr<PrefService> prefs_;
  mojo::Receiver<wootz_wallet::mojom::KeyringServiceObserver>
      keyring_service_observer_receiver_{this};
  base::WeakPtrFactory<AssetDiscoveryManager> weak_ptr_factory_;
};

}  // namespace wootz_wallet

#endif  // WOOTZ_COMPONENTS_WOOTZ_WALLET_BROWSER_ASSET_DISCOVERY_MANAGER_H_
