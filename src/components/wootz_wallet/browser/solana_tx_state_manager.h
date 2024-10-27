/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_SOLANA_TX_STATE_MANAGER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_SOLANA_TX_STATE_MANAGER_H_

#include <memory>
#include <string>

#include "components/wootz_wallet/browser/tx_state_manager.h"

class PrefService;

namespace base {
class Value;
}  // namespace base

namespace wootz_wallet {

class TxMeta;
class TxStorageDelegate;
class SolanaTxMeta;

class SolanaTxStateManager : public TxStateManager {
 public:
  SolanaTxStateManager(PrefService* prefs,
                       TxStorageDelegate* delegate,
                       AccountResolverDelegate* account_resolver_delegate);
  ~SolanaTxStateManager() override;
  SolanaTxStateManager(const SolanaTxStateManager&) = delete;
  SolanaTxStateManager operator=(const SolanaTxStateManager&) = delete;

  std::unique_ptr<SolanaTxMeta> GetSolanaTx(const std::string& id);
  std::unique_ptr<SolanaTxMeta> ValueToSolanaTxMeta(
      const base::Value::Dict& value);

 private:
  mojom::CoinType GetCoinType() const override;

  std::unique_ptr<TxMeta> ValueToTxMeta(
      const base::Value::Dict& value) override;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_SOLANA_TX_STATE_MANAGER_H_
