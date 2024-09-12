/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_MAX_SEND_SOLVER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_MAX_SEND_SOLVER_H_

#include <string>
#include <vector>

#include "base/types/expected.h"
#include "components/wootz_wallet/browser/bitcoin/bitcoin_transaction.h"

namespace wootz_wallet {

// Coin selection algorithm which spends all available utxos. No change output
// is created. Receiver gets everything except fee.
class BitcoinMaxSendSolver {
 public:
  BitcoinMaxSendSolver(
      BitcoinTransaction base_transaction,
      double fee_rate,
      const std::vector<BitcoinTransaction::TxInputGroup>& input_groups);
  ~BitcoinMaxSendSolver();

  base::expected<BitcoinTransaction, std::string> Solve();

 private:
  BitcoinTransaction base_transaction_;
  double fee_rate_ = 0;
  std::vector<BitcoinTransaction::TxInputGroup> input_groups_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_MAX_SEND_SOLVER_H_
