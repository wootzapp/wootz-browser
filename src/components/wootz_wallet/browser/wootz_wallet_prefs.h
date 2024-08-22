/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_PREFS_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_PREFS_H_

class PrefService;
class PrefRegistrySimple;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace wootz_wallet {

void RegisterLocalStatePrefs(PrefRegistrySimple* registry);
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);
void RegisterLocalStatePrefsForMigration(PrefRegistrySimple* registry);
void RegisterProfilePrefsForMigration(
    user_prefs::PrefRegistrySyncable* registry);
void ClearJsonRpcServiceProfilePrefs(PrefService* prefs);
void ClearKeyringServiceProfilePrefs(PrefService* prefs);
void ClearTxServiceProfilePrefs(PrefService* prefs);
void ClearWootzWalletServicePrefs(PrefService* prefs);
void MigrateObsoleteProfilePrefs(PrefService* prefs);

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_PREFS_H_
