// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_SWITCHES_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_SWITCHES_H_

namespace wootz_wallet {
namespace switches {

// Allows auto unlocking wallet password with command line.
inline constexpr char kDevWalletPassword[] = "dev-wallet-password";

// Ratios service dev URL
inline constexpr char kAssetRatioDevUrl[] = "asset-ratio-dev-url";
inline constexpr char kMeldAssetRatioDevUrl[] = "meld-asset-ratio-dev-url";

// ZCash rpc mainnet endpoint.
inline constexpr char kZCashMainnetRpcUrl[] = "zcash-mainnet-rpc-url";

// ZCash rpc testnet endpoint.
inline constexpr char kZCashTestnetRpcUrl[] = "zcash-testnet-rpc-url";

}  // namespace switches
}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_SWITCHES_H_
