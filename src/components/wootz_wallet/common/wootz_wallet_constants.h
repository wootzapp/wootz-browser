/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_WOOTZ_WALLET_CONSTANTS_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_WOOTZ_WALLET_CONSTANTS_H_

namespace wootz_wallet {

constexpr int kSolanaKeypairSize = 64;
constexpr int kSolanaSignatureSize = 64;
constexpr int kSolanaPrikeySize = 32;
constexpr int kSolanaPubkeySize = 32;
constexpr int kSolanaHashSize = 32;
// 1232 = 1280(IPv6 minimum MTU) - 40(size of the IPv6 header) - 8(size of the
// fragment header)
constexpr size_t kSolanaMaxTxSize = 1232;

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_WOOTZ_WALLET_CONSTANTS_H_
