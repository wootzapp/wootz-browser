/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/common/features.h"

#include "base/feature_list.h"
#include "components/wootz_wallet/common/buildflags.h"
#include "build/build_config.h"

namespace wootz_wallet::features {

BASE_FEATURE(kNativeWootzWalletFeature,
             "NativeWootzWallet",
             base::FEATURE_ENABLED_BY_DEFAULT);

BASE_FEATURE(kWootzWalletBitcoinFeature,
             "WootzWalletBitcoin",
             base::FEATURE_ENABLED_BY_DEFAULT
);
const base::FeatureParam<int> kBitcoinRpcThrottle{&kWootzWalletBitcoinFeature,
                                                  "rpc_throttle", 1};
const base::FeatureParam<bool> kBitcoinTestnetDiscovery{
    &kWootzWalletBitcoinFeature, "testnet_discovery", false};

BASE_FEATURE(kWootzWalletBitcoinImportFeature,
             "WootzWalletBitcoinImport",
             base::FEATURE_ENABLED_BY_DEFAULT);

BASE_FEATURE(kWootzWalletZCashFeature,
             "WootzWalletZCash",
#if BUILDFLAG(ENABLE_ZCASH_BY_DEFAULT)
             base::FEATURE_ENABLED_BY_DEFAULT
#else
             base::FEATURE_DISABLED_BY_DEFAULT
#endif
);

const base::FeatureParam<bool> kZCashShieldedTransactionsEnabled{
    &kWootzWalletZCashFeature, "zcash_shielded_transactions_enabled", false};

BASE_FEATURE(kWootzWalletAnkrBalancesFeature,
             "WootzWalletAnkrBalances",
             base::FEATURE_DISABLED_BY_DEFAULT);

BASE_FEATURE(kWootzWalletTransactionSimulationsFeature,
             "WootzWalletTransactionSimulations",
             base::FEATURE_DISABLED_BY_DEFAULT);
}  // namespace wootz_wallet::features
