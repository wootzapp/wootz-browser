/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_ENCODING_UTILS_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_ENCODING_UTILS_H_

#include <string>
#include <vector>

#include "base/containers/span.h"

namespace wootz_wallet {

// A bridge function to call DecodeBase58 in bitcoin-core.
// It will return false if length of decoded byte array does not match len
// param.
bool Base58Decode(const std::string& str,
                  std::vector<uint8_t>* ret,
                  int len,
                  bool strict = true);
// A bridge function to call EncodeBase58 in bitcoin-core.
std::string Base58Encode(const std::vector<uint8_t>& bytes);
std::string Base58Encode(base::span<const uint8_t> bytes);
std::string Base58EncodeWithCheck(const std::vector<uint8_t>& bytes);

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_ENCODING_UTILS_H_
