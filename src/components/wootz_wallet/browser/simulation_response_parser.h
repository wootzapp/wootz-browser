/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_SIMULATION_RESPONSE_PARSER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_SIMULATION_RESPONSE_PARSER_H_

#include <optional>
#include <string>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

namespace evm {

mojom::EVMSimulationResponsePtr ParseSimulationResponse(
    const base::Value& json_value,
    const std::string& user_account);

}  // namespace evm

namespace solana {

mojom::SolanaSimulationResponsePtr ParseSimulationResponse(
    const base::Value& json_value,
    const std::string& user_account);

}  // namespace solana

std::optional<std::string> ParseSimulationErrorResponse(
    const base::Value& json_value);

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_SIMULATION_RESPONSE_PARSER_H_
