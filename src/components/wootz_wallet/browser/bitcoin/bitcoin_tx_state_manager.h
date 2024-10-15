/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_TX_STATE_MANAGER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_TX_STATE_MANAGER_H_

#include <memory>
#include <string>

#include "components/wootz_wallet/browser/tx_state_manager.h"

class PrefService;

namespace base {
class Value;
}  // namespace base

namespace wootz_wallet {

class TxMeta;
class BitcoinTxMeta;
class TxStorageDelegate;

class BitcoinTxStateManager : public TxStateManager {
 public:
  BitcoinTxStateManager(PrefService* prefs,
                        TxStorageDelegate* delegate,
                        AccountResolverDelegate* account_resolver_delegate);
  ~BitcoinTxStateManager() override;
  BitcoinTxStateManager(const BitcoinTxStateManager&) = delete;
  BitcoinTxStateManager operator=(const BitcoinTxStateManager&) = delete;

  std::unique_ptr<BitcoinTxMeta> GetBitcoinTx(const std::string& id);
  std::unique_ptr<BitcoinTxMeta> ValueToBitcoinTxMeta(
      const base::Value::Dict& value);

 private:
  mojom::CoinType GetCoinType() const override;

  std::unique_ptr<TxMeta> ValueToTxMeta(
      const base::Value::Dict& value) override;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_TX_STATE_MANAGER_H_
