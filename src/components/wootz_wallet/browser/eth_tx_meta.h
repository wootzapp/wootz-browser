/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_TX_META_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_TX_META_H_

#include <memory>
#include <utility>

#include "components/wootz_wallet/browser/eth_transaction.h"
#include "components/wootz_wallet/browser/tx_meta.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/wootz_wallet_types.h"

namespace base {
class Value;
}  // namespace base

namespace wootz_wallet {

struct TransactionReceipt;

class EthTxMeta : public TxMeta {
 public:
  EthTxMeta();
  EthTxMeta(const mojom::AccountIdPtr& from,
            std::unique_ptr<EthTransaction> tx);
  ~EthTxMeta() override;
  EthTxMeta(const EthTxMeta&) = delete;
  EthTxMeta operator=(const EthTxMeta&) = delete;
  bool operator==(const EthTxMeta&) const;

  // TxMeta
  base::Value::Dict ToValue() const override;
  mojom::TransactionInfoPtr ToTransactionInfo() const override;
  mojom::CoinType GetCoinType() const override;

  TransactionReceipt tx_receipt() const { return tx_receipt_; }
  EthTransaction* tx() const { return tx_.get(); }
  bool sign_only() const { return sign_only_; }

  void set_tx_receipt(const TransactionReceipt& tx_receipt) {
    tx_receipt_ = tx_receipt;
  }
  void set_tx(std::unique_ptr<EthTransaction> tx) { tx_ = std::move(tx); }
  void set_sign_only(bool sign_only) { sign_only_ = sign_only; }

  bool IsRetriable() const;

 private:
  bool sign_only_ = false;
  TransactionReceipt tx_receipt_;
  std::unique_ptr<EthTransaction> tx_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_TX_META_H_
