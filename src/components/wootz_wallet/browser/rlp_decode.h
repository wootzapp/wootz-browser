/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_RLP_DECODE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_RLP_DECODE_H_

#include <string>

#include "base/values.h"

namespace wootz_wallet {

// Recursive Length Prefix (RLP) decoding of arbitrarily nested arrays of data
// Input string should be a hex string but without the 0x prefix
bool RLPDecode(const std::string& s, base::Value* output);

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_RLP_DECODE_H_
