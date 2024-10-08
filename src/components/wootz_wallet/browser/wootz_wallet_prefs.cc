/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/wootz_wallet_prefs.h"

#include <string>
#include <utility>
#include <vector>

#include "base/values.h"
#include "components/wootz_wallet/browser/wootz_wallet_constants.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/json_rpc_service.h"
#include "components/wootz_wallet/browser/keyring_service.h"
#include "components/wootz_wallet/browser/keyring_service_migrations.h"
#include "components/wootz_wallet/browser/pref_names.h"
#include "components/wootz_wallet/browser/tx_state_manager.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/pref_names.h"
#include "components/p3a_utils/feature_usage.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/sync_preferences/pref_service_syncable.h"

namespace wootz_wallet {

namespace {

constexpr int kDefaultWalletAutoLockMinutes = 10;

// Deprecated 12/2023.
constexpr char kWootzWalletUserAssetEthContractAddressMigrated[] =
    "wootz.wallet.user.asset.eth_contract_address_migrated";
// Deprecated 12/2023.
constexpr char kWootzWalletUserAssetsAddPreloadingNetworksMigrated[] =
    "wootz.wallet.user.assets.add_preloading_networks_migrated_3";
// Deprecated 12/2023.
constexpr char kWootzWalletUserAssetsAddIsNFTMigrated[] =
    "wootz.wallet.user.assets.add_is_nft_migrated";
// Deprecated 12/2023.
constexpr char kWootzWalletEthereumTransactionsCoinTypeMigrated[] =
    "wootz.wallet.ethereum_transactions.coin_type_migrated";
// Deprecated 12/2023.
constexpr char kWootzWalletDeprecateEthereumTestNetworksMigrated[] =
    "wootz.wallet.deprecated_ethereum_test_networks_migrated";
// Deprecated 12/2023.
constexpr char kWootzWalletUserAssetsAddIsSpamMigrated[] =
    "wootz.wallet.user.assets.add_is_spam_migrated";
// Deprecated 12/2023.
constexpr char kWootzWalletUserAssetsAddIsERC1155Migrated[] =
    "wootz.wallet.user.assets.add_is_erc1155_migrated";
// Deprecated 06/2024.
constexpr char kWootzWalletTransactionsChainIdMigrated[] =
    "wootz.wallet.transactions.chain_id_migrated";

// Deprecated 07/2024
constexpr char kPinnedNFTAssetsMigrated[] = "wootz.wallet.user_pin_data";
// Deprecated 07/2024
constexpr char kAutoPinEnabledMigrated[] = "wootz.wallet.auto_pin_enabled";

base::Value::Dict GetDefaultSelectedNetworks() {
  base::Value::Dict selected_networks;
  selected_networks.Set(kEthereumPrefKey, mojom::kMainnetChainId);
  selected_networks.Set(kSolanaPrefKey, mojom::kSolanaMainnet);
  selected_networks.Set(kFilecoinPrefKey, mojom::kFilecoinMainnet);
  selected_networks.Set(kBitcoinPrefKey, mojom::kBitcoinMainnet);
  selected_networks.Set(kZCashPrefKey, mojom::kZCashMainnet);

  return selected_networks;
}

base::Value::Dict GetDefaultSelectedNetworksPerOrigin() {
  base::Value::Dict selected_networks;
  selected_networks.Set(kEthereumPrefKey, base::Value::Dict());
  selected_networks.Set(kSolanaPrefKey, base::Value::Dict());
  selected_networks.Set(kFilecoinPrefKey, base::Value::Dict());
  selected_networks.Set(kBitcoinPrefKey, base::Value::Dict());
  selected_networks.Set(kZCashPrefKey, base::Value::Dict());

  return selected_networks;
}

base::Value::Dict GetDefaultHiddenNetworks() {
  base::Value::Dict hidden_networks;

  base::Value::List eth_hidden;
  eth_hidden.Append(mojom::kGoerliChainId);
  eth_hidden.Append(mojom::kSepoliaChainId);
  eth_hidden.Append(mojom::kLocalhostChainId);
  eth_hidden.Append(mojom::kFilecoinEthereumTestnetChainId);
  hidden_networks.Set(kEthereumPrefKey, std::move(eth_hidden));

  base::Value::List fil_hidden;
  fil_hidden.Append(mojom::kFilecoinTestnet);
  fil_hidden.Append(mojom::kLocalhostChainId);
  hidden_networks.Set(kFilecoinPrefKey, std::move(fil_hidden));

  base::Value::List sol_hidden;
  sol_hidden.Append(mojom::kSolanaDevnet);
  sol_hidden.Append(mojom::kSolanaTestnet);
  sol_hidden.Append(mojom::kLocalhostChainId);
  hidden_networks.Set(kSolanaPrefKey, std::move(sol_hidden));

  base::Value::List btc_hidden;
  btc_hidden.Append(mojom::kBitcoinTestnet);
  hidden_networks.Set(kBitcoinPrefKey, std::move(btc_hidden));

  base::Value::List zec_hidden;
  zec_hidden.Append(mojom::kZCashTestnet);
  hidden_networks.Set(kZCashPrefKey, std::move(zec_hidden));

  return hidden_networks;
}

void RegisterProfilePrefsDeprecatedMigrationFlags(
    user_prefs::PrefRegistrySyncable* registry) {
  // Deprecated 12/2023
  registry->RegisterBooleanPref(kWootzWalletUserAssetEthContractAddressMigrated,
                                false);
  // Deprecated 12/2023
  registry->RegisterBooleanPref(
      kWootzWalletUserAssetsAddPreloadingNetworksMigrated, false);
  // Deprecated 12/2023
  registry->RegisterBooleanPref(kWootzWalletUserAssetsAddIsNFTMigrated, false);
  // Deprecated 12/2023
  registry->RegisterBooleanPref(
      kWootzWalletEthereumTransactionsCoinTypeMigrated, false);
  // Deprecated 12/2023
  registry->RegisterBooleanPref(
      kWootzWalletDeprecateEthereumTestNetworksMigrated, false);
  // Deprecated 12/2023
  registry->RegisterBooleanPref(kWootzWalletUserAssetsAddIsSpamMigrated, false);
  // Deprecated 12/2023
  registry->RegisterBooleanPref(kWootzWalletUserAssetsAddIsERC1155Migrated,
                                false);
  // Deprecated 06/2024.
  registry->RegisterBooleanPref(kWootzWalletTransactionsChainIdMigrated, false);
}

void RegisterDeprecatedIpfsPrefs(user_prefs::PrefRegistrySyncable* registry) {
  // Deprecated 05/2024
  registry->RegisterDictionaryPref(kPinnedNFTAssetsMigrated);
  // Deprecated 05/2024
  registry->RegisterBooleanPref(kAutoPinEnabledMigrated, false);
}

void ClearDeprecatedProfilePrefsMigrationFlags(PrefService* prefs) {
  // Deprecated 12/2023
  prefs->ClearPref(kWootzWalletUserAssetEthContractAddressMigrated);
  // Deprecated 12/2023
  prefs->ClearPref(kWootzWalletUserAssetsAddPreloadingNetworksMigrated);
  // Deprecated 12/2023
  prefs->ClearPref(kWootzWalletUserAssetsAddIsNFTMigrated);
  // Deprecated 12/2023
  prefs->ClearPref(kWootzWalletEthereumTransactionsCoinTypeMigrated);
  // Deprecated 12/2023
  prefs->ClearPref(kWootzWalletDeprecateEthereumTestNetworksMigrated);
  // Deprecated 12/2023
  prefs->ClearPref(kWootzWalletUserAssetsAddIsSpamMigrated);
  // Deprecated 12/2023
  prefs->ClearPref(kWootzWalletUserAssetsAddIsERC1155Migrated);
  // Deprecated 06/2024.
  prefs->ClearPref(kWootzWalletTransactionsChainIdMigrated);
}

void ClearDeprecatedIpfsPrefs(PrefService* prefs) {
  DCHECK(prefs);
  // Deprecated 05/2024
  prefs->ClearPref(kPinnedNFTAssetsMigrated);
  // Deprecated 05/2024
  prefs->ClearPref(kAutoPinEnabledMigrated);
}

}  // namespace

void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
  registry->RegisterTimePref(kWootzWalletLastUnlockTime, base::Time());
  p3a_utils::RegisterFeatureUsagePrefs(
      registry, kWootzWalletP3AFirstUnlockTime, kWootzWalletP3ALastUnlockTime,
      kWootzWalletP3AUsedSecondDay, nullptr, nullptr);
  registry->RegisterBooleanPref(kWootzWalletP3ANewUserBalanceReported, false);
  registry->RegisterIntegerPref(kWootzWalletP3AOnboardingLastStep, 0);
  registry->RegisterBooleanPref(kWootzWalletP3ANFTGalleryUsed, false);
}

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(prefs::kDisabledByPolicy, false);
  registry->RegisterIntegerPref(
      kDefaultEthereumWallet,
      static_cast<int>(
          wootz_wallet::mojom::DefaultWallet::WootzWalletPreferExtension));
  registry->RegisterIntegerPref(
      kDefaultSolanaWallet,
      static_cast<int>(
          wootz_wallet::mojom::DefaultWallet::WootzWalletPreferExtension));
  registry->RegisterStringPref(kDefaultBaseCurrency, "USD");
  registry->RegisterStringPref(kDefaultBaseCryptocurrency, "BTC");
  registry->RegisterBooleanPref(kShowWalletIconOnToolbar, true);
  registry->RegisterDictionaryPref(kWootzWalletTransactions);
  registry->RegisterDictionaryPref(kWootzWalletP3AActiveWalletDict);
  registry->RegisterDictionaryPref(kWootzWalletKeyrings);
  registry->RegisterBooleanPref(kWootzWalletKeyringEncryptionKeysMigrated,
                                false);
  registry->RegisterDictionaryPref(kWootzWalletCustomNetworks);
  registry->RegisterDictionaryPref(kWootzWalletEip1559CustomChains);
  registry->RegisterDictionaryPref(kWootzWalletHiddenNetworks,
                                   GetDefaultHiddenNetworks());
  registry->RegisterDictionaryPref(kWootzWalletSelectedNetworks,
                                   GetDefaultSelectedNetworks());
  registry->RegisterDictionaryPref(kWootzWalletSelectedNetworksPerOrigin,
                                   GetDefaultSelectedNetworksPerOrigin());
  registry->RegisterListPref(kWootzWalletUserAssetsList,
                             GetDefaultUserAssets());
  registry->RegisterIntegerPref(kWootzWalletAutoLockMinutes,
                                kDefaultWalletAutoLockMinutes);
  registry->RegisterDictionaryPref(kWootzWalletEthAllowancesCache);
  registry->RegisterDictionaryPref(kWootzWalletLastTransactionSentTimeDict);
  registry->RegisterTimePref(kWootzWalletLastDiscoveredAssetsAt, base::Time());

  registry->RegisterBooleanPref(kShouldShowWalletSuggestionBadge, true);
  registry->RegisterBooleanPref(kWootzWalletNftDiscoveryEnabled, false);
  registry->RegisterBooleanPref(kWootzWalletPrivateWindowsEnabled, false);

  registry->RegisterStringPref(kWootzWalletSelectedWalletAccount, "");
  registry->RegisterStringPref(kWootzWalletSelectedEthDappAccount, "");
  registry->RegisterStringPref(kWootzWalletSelectedSolDappAccount, "");

  registry->RegisterIntegerPref(
      kWootzWalletTransactionSimulationOptInStatus,
      static_cast<int>(wootz_wallet::mojom::BlowfishOptInStatus::kUnset));
  registry->RegisterStringPref(kWootzWalletEncryptorSalt, "");
  registry->RegisterDictionaryPref(kWootzWalletMnemonic);
  registry->RegisterBooleanPref(kWootzWalletLegacyEthSeedFormat, false);
  registry->RegisterBooleanPref(kWootzWalletMnemonicBackedUp, false);
}

void RegisterLocalStatePrefsForMigration(PrefRegistrySimple* registry) {
  // Added 04/2023
  registry->RegisterTimePref(kWootzWalletP3ALastReportTimeDeprecated,
                             base::Time());
  registry->RegisterTimePref(kWootzWalletP3AFirstReportTimeDeprecated,
                             base::Time());
  registry->RegisterListPref(kWootzWalletP3AWeeklyStorageDeprecated);
}

void RegisterProfilePrefsForMigration(
    user_prefs::PrefRegistrySyncable* registry) {
  RegisterProfilePrefsDeprecatedMigrationFlags(registry);
  RegisterDeprecatedIpfsPrefs(registry);

  // Added 04/2023
  p3a_utils::RegisterFeatureUsagePrefs(
      registry, kWootzWalletP3AFirstUnlockTime, kWootzWalletP3ALastUnlockTime,
      kWootzWalletP3AUsedSecondDay, nullptr, nullptr);
  registry->RegisterTimePref(kWootzWalletLastUnlockTime, base::Time());
  registry->RegisterTimePref(kWootzWalletP3ALastReportTimeDeprecated,
                             base::Time());
  registry->RegisterTimePref(kWootzWalletP3AFirstReportTimeDeprecated,
                             base::Time());
  registry->RegisterListPref(kWootzWalletP3AWeeklyStorageDeprecated);

  // Added 03/2023
  registry->RegisterIntegerPref(kWootzWalletDefaultHiddenNetworksVersion, 0);

  // Added 06/2023
  registry->RegisterIntegerPref(
      kWootzWalletSelectedCoinDeprecated,
      static_cast<int>(wootz_wallet::mojom::CoinType::ETH));

  // Added 07/2023
  registry->RegisterBooleanPref(kWootzWalletTransactionsFromPrefsToDBMigrated,
                                false);

  // Added 08/2023
  registry->RegisterBooleanPref(kWootzWalletCustomNetworksFantomMainnetMigrated,
                                false);

  // Added 01/2024
  registry->RegisterDictionaryPref(kWootzWalletUserAssetsDeprecated);
  // Added 01/2024
  registry->RegisterBooleanPref(kWootzWalletTransactionsDBFormatMigrated,
                                false);
  // Added 06/2024
  registry->RegisterBooleanPref(kWootzWalletEip1559ForCustomNetworksMigrated,
                                false);
  // Added 06/2024
  registry->RegisterBooleanPref(kSupportEip1559OnLocalhostChainDeprecated,
                                false);
  // Added 06/2024
  registry->RegisterBooleanPref(kWootzWalletIsCompressedNftMigrated, false);
}

void ClearJsonRpcServiceProfilePrefs(PrefService* prefs) {
  DCHECK(prefs);
  prefs->ClearPref(kWootzWalletCustomNetworks);
  prefs->ClearPref(kWootzWalletHiddenNetworks);
  prefs->ClearPref(kWootzWalletSelectedNetworks);
  prefs->ClearPref(kWootzWalletSelectedNetworksPerOrigin);
  prefs->ClearPref(kWootzWalletEip1559CustomChains);
}

void ClearKeyringServiceProfilePrefs(PrefService* prefs) {
  DCHECK(prefs);
  prefs->ClearPref(kWootzWalletKeyrings);
  prefs->ClearPref(kWootzWalletEncryptorSalt);
  prefs->ClearPref(kWootzWalletMnemonic);
  prefs->ClearPref(kWootzWalletLegacyEthSeedFormat);
  prefs->ClearPref(kWootzWalletMnemonicBackedUp);
  prefs->ClearPref(kWootzWalletAutoLockMinutes);
  prefs->ClearPref(kWootzWalletSelectedWalletAccount);
  prefs->ClearPref(kWootzWalletSelectedEthDappAccount);
  prefs->ClearPref(kWootzWalletSelectedSolDappAccount);
}

void ClearTxServiceProfilePrefs(PrefService* prefs) {
  DCHECK(prefs);
  // Remove this when we remove kWootzWalletTransactions.
  prefs->ClearPref(kWootzWalletTransactions);
}

void ClearWootzWalletServicePrefs(PrefService* prefs) {
  DCHECK(prefs);
  prefs->ClearPref(kWootzWalletUserAssetsList);
  prefs->ClearPref(kDefaultBaseCurrency);
  prefs->ClearPref(kDefaultBaseCryptocurrency);
  prefs->ClearPref(kWootzWalletEthAllowancesCache);
}

void MigrateObsoleteProfilePrefs(PrefService* prefs) {
  ClearDeprecatedProfilePrefsMigrationFlags(prefs);

  // Added 03/2023 to add filecoin evm support.
  WootzWalletService::MigrateHiddenNetworks(prefs);

  // Added 08/2023 to add Fantom as a custom network if selected for the default
  // or custom origins.
  WootzWalletService::MigrateFantomMainnetAsCustomNetwork(prefs);

  // Added 07/2023
  MigrateDerivedAccountIndex(prefs);

  // Added 01/2024 migrate assets pref to plain list.
  WootzWalletService::MigrateAssetsPrefToList(prefs);

  // Added 06/2024 to migrate Eip1559 flag to a separate pref.
  WootzWalletService::MigrateEip1559ForCustomNetworks(prefs);

  // Added 05/2024
  ClearDeprecatedIpfsPrefs(prefs);
}

}  // namespace wootz_wallet
