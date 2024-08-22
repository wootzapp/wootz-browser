/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_FEATURES_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_FEATURES_H_

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"

namespace wootz_wallet {
namespace features {

BASE_DECLARE_FEATURE(kNativeWootzWalletFeature);
BASE_DECLARE_FEATURE(kWootzWalletBitcoinFeature);
extern const base::FeatureParam<int> kBitcoinRpcThrottle;
extern const base::FeatureParam<bool> kBitcoinTestnetDiscovery;
BASE_DECLARE_FEATURE(kWootzWalletBitcoinImportFeature);
BASE_DECLARE_FEATURE(kWootzWalletZCashFeature);
// Adds shielded operations support for Z Cash
extern const base::FeatureParam<bool> kZCashShieldedTransactionsEnabled;
BASE_DECLARE_FEATURE(kWootzWalletAnkrBalancesFeature);
BASE_DECLARE_FEATURE(kWootzWalletTransactionSimulationsFeature);

}  // namespace features
}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_FEATURES_H_
