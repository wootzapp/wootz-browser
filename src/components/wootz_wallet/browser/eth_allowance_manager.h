/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_ALLOWANCE_MANAGER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_ALLOWANCE_MANAGER_H_

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/wootz_wallet_types.h"

class PrefService;

namespace wootz_wallet {

class JsonRpcService;
class KeyringService;

class EthAllowanceManager {
 public:
  EthAllowanceManager(JsonRpcService* json_rpc_service,
                      KeyringService* keyring_service,
                      PrefService* prefs);
  EthAllowanceManager(const EthAllowanceManager&) = delete;
  EthAllowanceManager& operator=(const EthAllowanceManager&) = delete;
  EthAllowanceManager(const EthAllowanceManager&&) = delete;
  EthAllowanceManager& operator=(const EthAllowanceManager&&) = delete;

  ~EthAllowanceManager();

  using ResultCallback =
      mojom::WootzWalletService::DiscoverEthAllowancesCallback;

  void DiscoverEthAllowancesOnAllSupportedChains(ResultCallback callback);
  void Reset();

 private:
  friend class EthAllowanceManagerUnitTest;
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest, LoadCachedAllowances);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           CouldNotLoadCachedAllowancesPrefsEmpty);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           CouldNotLoadCachedAllowancesByAddress);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           CouldNotLoadCachedAllowancesIncorrectCacheData);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           BreakAllowanceDiscoveringIfTokenListEmpty);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest, AllowancesLoading);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest, AllowancesRevoked);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           AllowancesIgnorePendingBlocks);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           AllowancesIgnoreWrongTopicsData);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           AllowancesIgnoreWrongAmountData);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest, NoAllowancesLoaded);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest,
                           NoAllowancesLoadedForSkippedNetwork);
  FRIEND_TEST_ALL_PREFIXES(EthAllowanceManagerUnitTest, AllowancesLoadingReset);

  void OnGetAllowances(const int& task_id,
                       const std::vector<Log>& logs,
                       base::Value rawlogs,
                       mojom::ProviderError error,
                       const std::string& error_message);

  void LoadCachedAllowances(
      const std::string& chain_id,
      const std::string& hex_account_address,
      std::map<std::string, mojom::AllowanceInfoPtr>& allowance_map);
  struct EthAllowanceTask {
    explicit EthAllowanceTask(const int& taskid,
                              const std::string& chain_id,
                              const std::string& account_address,
                              const uint256_t& latest_block_number);
    EthAllowanceTask(const EthAllowanceTask&) = delete;
    EthAllowanceTask& operator=(const EthAllowanceTask&) = delete;
    EthAllowanceTask(const EthAllowanceTask&&) = delete;
    EthAllowanceTask& operator=(const EthAllowanceTask&&) = delete;
    ~EthAllowanceTask();

    void SetResults(std::vector<mojom::AllowanceInfoPtr> alwns);
    void MarkComplete();
    int task_id;
    std::vector<mojom::AllowanceInfoPtr> allowances_;
    std::string account_address_;
    wootz_wallet::uint256_t latest_block_number_;
    std::string chain_id_;
    bool is_completed_{false};
  };
  void OnGetCurrentBlock(const std::string& chain_id,
                         base::Value::List contract_addresses,
                         const std::vector<std::string>& account_addresses,
                         uint256_t block_num,
                         mojom::ProviderError error,
                         const std::string& error_message);
  bool IsAllTasksCompleted() const;
  void MaybeMergeAllResultsAndCallBack();
  void OnDiscoverEthAllowancesCompleted(
      const std::vector<mojom::AllowanceInfoPtr>& result);

  int get_block_tasks_{0};
  std::map<int, std::unique_ptr<EthAllowanceTask>> allowance_discovery_tasks_;
  std::vector<ResultCallback> discover_eth_allowance_callbacks_;
  raw_ptr<JsonRpcService> json_rpc_service_;
  raw_ptr<KeyringService> keyring_service_;
  raw_ptr<PrefService> prefs_;

  base::WeakPtrFactory<EthAllowanceManager> weak_ptr_factory_{this};
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_ALLOWANCE_MANAGER_H_
