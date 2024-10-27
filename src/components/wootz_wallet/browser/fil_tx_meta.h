/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_FIL_TX_META_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_FIL_TX_META_H_

#include <memory>
#include <utility>

#include "components/wootz_wallet/browser/fil_transaction.h"
#include "components/wootz_wallet/browser/tx_meta.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace base {
class Value;
}  // namespace base

namespace wootz_wallet {

class FilTransaction;

class FilTxMeta : public TxMeta {
 public:
  FilTxMeta();
  FilTxMeta(const mojom::AccountIdPtr& from,
            std::unique_ptr<FilTransaction> tx);
  FilTxMeta(const FilTxMeta&) = delete;
  FilTxMeta operator=(const FilTxMeta&) = delete;
  bool operator==(const FilTxMeta&) const;
  ~FilTxMeta() override;

  // TxMeta
  base::Value::Dict ToValue() const override;
  mojom::TransactionInfoPtr ToTransactionInfo() const override;
  mojom::CoinType GetCoinType() const override;

  FilTransaction* tx() const { return tx_.get(); }
  void set_tx(std::unique_ptr<FilTransaction> tx) { tx_ = std::move(tx); }

 private:
  std::unique_ptr<FilTransaction> tx_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_FIL_TX_META_H_
