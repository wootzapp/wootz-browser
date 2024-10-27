/* Copyright (c) 2020 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_PREF_NAMES_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_PREF_NAMES_H_

inline constexpr char kShouldShowWalletSuggestionBadge[] =
    "wootz.wallet.should_show_wallet_suggestion_badge";
inline constexpr char kDefaultEthereumWallet[] = "wootz.wallet.default_wallet2";
inline constexpr char kDefaultSolanaWallet[] =
    "wootz.wallet.default_solana_wallet";
inline constexpr char kDefaultBaseCurrency[] =
    "wootz.wallet.default_base_currency";
inline constexpr char kDefaultBaseCryptocurrency[] =
    "wootz.wallet.default_base_cryptocurrency";
inline constexpr char kWootzWalletTransactions[] = "wootz.wallet.transactions";
inline constexpr char kShowWalletIconOnToolbar[] =
    "wootz.wallet.show_wallet_icon_on_toolbar";
inline constexpr char kWootzWalletLastUnlockTime[] =
    "wootz.wallet.wallet_last_unlock_time_v2";
inline constexpr char kWootzWalletPingReportedUnlockTime[] =
    "wootz.wallet.wallet_report_unlock_time_ping";
inline constexpr char kWootzWalletP3ALastReportTimeDeprecated[] =
    "wootz.wallet.wallet_p3a_last_report_time";
inline constexpr char kWootzWalletP3AFirstReportTimeDeprecated[] =
    "wootz.wallet.wallet_p3a_first_report_time";
inline constexpr char kWootzWalletP3ANFTGalleryUsed[] =
    "wootz.wallet.wallet_p3a_nft_gallery_used";
inline constexpr char kWootzWalletP3ANewUserBalanceReported[] =
    "wootz.wallet.p3a_new_user_balance_reported";
inline constexpr char kWootzWalletP3AWeeklyStorageDeprecated[] =
    "wootz.wallet.wallet_p3a_weekly_storage";
inline constexpr char kWootzWalletP3AActiveWalletDict[] =
    "wootz.wallet.wallet_p3a_active_wallets";
inline constexpr char kWootzWalletCustomNetworks[] =
    "wootz.wallet.custom_networks";
inline constexpr char kWootzWalletEip1559CustomChains[] =
    "wootz.wallet.eip1559_chains";
inline constexpr char kWootzWalletHiddenNetworks[] =
    "wootz.wallet.hidden_networks";
inline constexpr char kWootzWalletSelectedNetworks[] =
    "wootz.wallet.selected_networks";
inline constexpr char kWootzWalletSelectedNetworksPerOrigin[] =
    "wootz.wallet.selected_networks_origin";
inline constexpr char kWootzWalletSelectedWalletAccount[] =
    "wootz.wallet.selected_wallet_account";
inline constexpr char kWootzWalletSelectedEthDappAccount[] =
    "wootz.wallet.selected_eth_dapp_account";
inline constexpr char kWootzWalletSelectedSolDappAccount[] =
    "wootz.wallet.selected_sol_dapp_account";
inline constexpr char kWootzWalletKeyrings[] = "wootz.wallet.keyrings";
inline constexpr char kWootzWalletUserAssetsList[] =
    "wootz.wallet.wallet_user_assets_list";
inline constexpr char kWootzWalletEthAllowancesCache[] =
    "wootz.wallet.eth_allowances_cache";
// Added 03/2023 to add networks hidden by default
inline constexpr char kWootzWalletDefaultHiddenNetworksVersion[] =
    "wootz.wallet.user.assets.default_hidden_networks_version";
inline constexpr char kWootzWalletAutoLockMinutes[] =
    "wootz.wallet.auto_lock_minutes";
inline constexpr char kWootzWalletP3AFirstUnlockTime[] =
    "wootz.wallet.p3a_first_unlock_time";
inline constexpr char kWootzWalletP3ALastUnlockTime[] =
    "wootz.wallet.p3a_last_unlock_time";
inline constexpr char kWootzWalletP3AUsedSecondDay[] =
    "wootz.wallet.p3a_used_second_day";
inline constexpr char kWootzWalletP3AOnboardingLastStep[] =
    "wootz.wallet.p3a_last_onboarding_step";
inline constexpr char kWootzWalletKeyringEncryptionKeysMigrated[] =
    "wootz.wallet.keyring_encryption_keys_migrated";
inline constexpr char kWootzWalletLastTransactionSentTimeDict[] =
    "wootz.wallet.last_transaction_sent_time_dict";
inline constexpr char kWootzWalletNftDiscoveryEnabled[] =
    "wootz.wallet.nft_discovery_enabled";
inline constexpr char kWootzWalletLastDiscoveredAssetsAt[] =
    "wootz.wallet.last_discovered_assets_at";
inline constexpr char kWootzWalletPrivateWindowsEnabled[] =
    "wootz.wallet.private_windows_enabled";
inline constexpr char kWootzWalletTransactionSimulationOptInStatus[] =
    "wootz.wallet.transaction_simulation_opt_in_status";
inline constexpr char kWootzWalletEncryptorSalt[] =
    "wootz.wallet.encryptor_salt";
inline constexpr char kWootzWalletMnemonic[] =
    "wootz.wallet.encrypted_mnemonic";
inline constexpr char kWootzWalletLegacyEthSeedFormat[] =
    "wootz.wallet.legacy_eth_seed_format";
inline constexpr char kWootzWalletMnemonicBackedUp[] =
    "wootz.wallet.mnemonic_backed_up";

// Added 07/2023 to migrate transactions from prefs to DB.
inline constexpr char kWootzWalletTransactionsFromPrefsToDBMigrated[] =
    "wootz.wallet.transactions.from_prefs_to_db_migrated";
// Added 08/2023 to migrate Fantom mainnet, previously a preloaded network,
// to a custom network.
inline constexpr char kWootzWalletCustomNetworksFantomMainnetMigrated[] =
    "wootz.wallet.custom_networks.fantom_mainnet_migrated";
// Added 01/2024 to migrate transactions DB to one-level dict.
inline constexpr char kWootzWalletTransactionsDBFormatMigrated[] =
    "wootz.wallet.transactions_db_format_migrated";
// Added 06/2024 to migrate Eip1559 flag to a separate pref.
inline constexpr char kWootzWalletEip1559ForCustomNetworksMigrated[] =
    "wootz.wallet.eip1559_chains_migrated";
// 06/2024 to mark existing compressed Solana NFTs as compressed.
inline constexpr char kWootzWalletIsCompressedNftMigrated[] =
    "wootz.wallet.is_compressed_nft_migrated";

// 06/2023
inline constexpr char kWootzWalletSelectedCoinDeprecated[] =
    "wootz.wallet.selected_coin";
// 01/2024
inline constexpr char kWootzWalletUserAssetsDeprecated[] =
    "wootz.wallet.wallet_user_assets";
// 06/2024 migrated to kWootzWalletEip1559CustomChains.
inline constexpr char kSupportEip1559OnLocalhostChainDeprecated[] =
    "wootz.wallet.support_eip1559_on_localhost_chain";

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_PREF_NAMES_H_
