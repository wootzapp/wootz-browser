/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_GAS_UTILS_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_GAS_UTILS_H_

#include <optional>
#include <string>
#include <vector>

#include "components/wootz_wallet/common/wootz_wallet_types.h"

namespace wootz_wallet {

namespace eth {

std::optional<uint256_t> ScaleBaseFeePerGas(const std::string& value);

bool GetSuggested1559Fees(const std::vector<std::string>& base_fee_per_gas,
                          const std::vector<double>& gas_used_ratio,
                          const std::string& oldest_block,
                          const std::vector<std::vector<std::string>>& reward,
                          uint256_t* low_priority_fee,
                          uint256_t* avg_priority_fee,
                          uint256_t* high_priority_fee,
                          uint256_t* suggested_base_fee_per_gas);

}  // namespace eth

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_ETH_GAS_UTILS_H_
