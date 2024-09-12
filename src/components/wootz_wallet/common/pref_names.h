/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_PREF_NAMES_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_PREF_NAMES_H_

namespace wootz_wallet {
namespace prefs {

// Used to enable/disable Wootz Wallet via a policy.
inline constexpr char kDisabledByPolicy[] = "wootz.wallet.disabled_by_policy";

}  // namespace prefs
}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_PREF_NAMES_H_
