/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_SIMULATION_REQUEST_HELPER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_SIMULATION_REQUEST_HELPER_H_

#include <optional>
#include <string>
#include <utility>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

namespace evm {

std::optional<std::pair<std::string, std::string>> EncodeScanTransactionParams(
    const mojom::TransactionInfoPtr& tx_info);

}  // namespace evm

namespace solana {

std::optional<std::pair<std::string, std::string>> EncodeScanTransactionParams(
    const mojom::SolanaTransactionRequestUnionPtr& request);

void PopulateRecentBlockhash(mojom::SolanaTransactionRequestUnion& request,
                             const std::string& recent_blockhash);

std::optional<bool> HasEmptyRecentBlockhash(
    const mojom::SolanaTransactionRequestUnionPtr& request);

}  // namespace solana

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_SIMULATION_REQUEST_HELPER_H_
