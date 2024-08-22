/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_STRING_UTILS_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_STRING_UTILS_H_

#include <string>
#include <vector>

#include "components/wootz_wallet/common/wootz_wallet_types.h"

namespace wootz_wallet {

// Returns true if a string contains only 0-9 digits
bool IsValidBase10String(const std::string& input);

// Takes a base-10 string as input and converts it to a uint256_t
bool Base10ValueToUint256(const std::string& input, uint256_t* out);
bool Base10ValueToInt256(const std::string& input, int256_t* out);

// Takes a uint256_t as input and converts it to a base-10 string
std::string Uint256ValueToBase10(uint256_t input);

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_STRING_UTILS_H_
