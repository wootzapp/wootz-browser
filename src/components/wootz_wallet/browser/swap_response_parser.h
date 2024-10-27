/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_SWAP_RESPONSE_PARSER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_SWAP_RESPONSE_PARSER_H_

#include <optional>
#include <string>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

namespace zeroex {

mojom::ZeroExQuotePtr ParseQuoteResponse(const base::Value& json_value,
                                         bool expect_transaction_data);
mojom::ZeroExErrorPtr ParseErrorResponse(const base::Value& json_value);

}  // namespace zeroex

namespace jupiter {

mojom::JupiterQuotePtr ParseQuoteResponse(const base::Value& json_value);
std::optional<std::string> ParseTransactionResponse(
    const base::Value& json_value);
mojom::JupiterErrorPtr ParseErrorResponse(const base::Value& json_value);

}  // namespace jupiter

namespace lifi {

mojom::LiFiTransactionUnionPtr ParseTransactionResponse(
    const base::Value& json_value);
mojom::LiFiQuotePtr ParseQuoteResponse(const base::Value& json_value);
mojom::LiFiErrorPtr ParseErrorResponse(const base::Value& json_value);

}  // namespace lifi

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_SWAP_RESPONSE_PARSER_H_
