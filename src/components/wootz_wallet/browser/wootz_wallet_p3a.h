/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_P3A_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_P3A_H_

#include <optional>

#include "base/memory/raw_ptr.h"
#include "components/wootz_wallet/browser/keyring_service_observer_base.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/prefs/pref_change_registrar.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"

class PrefService;

namespace wootz_wallet {

inline constexpr char kKeyringCreatedHistogramName[] =
    "Wootz.Wallet.KeyringCreated";
inline constexpr char kOnboardingConversionHistogramName[] =
    "Wootz.Wallet.OnboardingConversion.3";
inline constexpr char kNewUserBalanceHistogramName[] =
    "Wootz.Wallet.NewUserBalance";
inline constexpr char kEthProviderHistogramName[] =
    "Wootz.Wallet.EthProvider.4";
inline constexpr char kSolProviderHistogramName[] =
    "Wootz.Wallet.SolProvider.2";
inline constexpr char kEthTransactionSentHistogramName[] =
    "Wootz.Wallet.EthTransactionSent";
inline constexpr char kSolTransactionSentHistogramName[] =
    "Wootz.Wallet.SolTransactionSent";
inline constexpr char kFilTransactionSentHistogramName[] =
    "Wootz.Wallet.FilTransactionSent";
inline constexpr char kBtcTransactionSentHistogramName[] =
    "Wootz.Wallet.BtcTransactionSent";
inline constexpr char kZecTransactionSentHistogramName[] =
    "Wootz.Wallet.ZecTransactionSent";
inline constexpr char kEthActiveAccountHistogramName[] =
    "Wootz.Wallet.ActiveEthAccounts";
inline constexpr char kSolActiveAccountHistogramName[] =
    "Wootz.Wallet.ActiveSolAccounts";
inline constexpr char kFilActiveAccountHistogramName[] =
    "Wootz.Wallet.ActiveFilAccounts";
inline constexpr char kBtcActiveAccountHistogramName[] =
    "Wootz.Wallet.ActiveBtcAccounts";
inline constexpr char kZecActiveAccountHistogramName[] =
    "Wootz.Wallet.ActiveZecAccounts";
inline constexpr char kWootzWalletDailyHistogramName[] =
    "Wootz.Wallet.UsageDaily";
inline constexpr char kWootzWalletWeeklyHistogramName[] =
    "Wootz.Wallet.UsageWeekly";
inline constexpr char kWootzWalletMonthlyHistogramName[] =
    "Wootz.Wallet.UsageMonthly";
inline constexpr char kWootzWalletNewUserReturningHistogramName[] =
    "Wootz.Wallet.NewUserReturning";
inline constexpr char kWootzWalletLastUsageTimeHistogramName[] =
    "Wootz.Wallet.LastUsageTime";
inline constexpr char kWootzWalletNFTCountHistogramName[] =
    "Wootz.Wallet.NFTCount";
inline constexpr char kWootzWalletNFTNewUserHistogramName[] =
    "Wootz.Wallet.NFTNewUser";
inline constexpr char kWootzWalletNFTDiscoveryEnabledHistogramName[] =
    "Wootz.Wallet.NFTDiscoveryEnabled";

class WootzWalletService;
class KeyringService;
class TxService;

enum class JSProviderAnswer {
  kNoWallet = 0,
  kWalletDisabled = 1,
  kNativeNotOverridden = 2,
  kNativeOverridingDisallowed = 3,
  kThirdPartyNotOverriding = 4,
  kThirdPartyOverriding = 5,
  kMaxValue = kThirdPartyOverriding
};

// Reports WootzWallet related P3A data
class WootzWalletP3A : public KeyringServiceObserverBase,
                       public mojom::WootzWalletP3A,
                       public mojom::TxServiceObserver {
 public:
  WootzWalletP3A(WootzWalletService* wallet_service,
                 KeyringService* keyring_service,
                 TxService* tx_service,
                 PrefService* profile_prefs,
                 PrefService* local_state);

  // For testing
  WootzWalletP3A();

  ~WootzWalletP3A() override;
  WootzWalletP3A(const WootzWalletP3A&) = delete;
  WootzWalletP3A& operator=(WootzWalletP3A&) = delete;

  void Bind(mojo::PendingReceiver<mojom::WootzWalletP3A> receiver);

  void AddObservers();

  void ReportUsage(bool unlocked);
  void ReportJSProvider(mojom::JSProviderType provider_type,
                        mojom::CoinType coin_type,
                        bool allow_provider_overwrite) override;
  void ReportOnboardingAction(mojom::OnboardingAction action) override;
  void RecordActiveWalletCount(int count, mojom::CoinType coin_type) override;
  void RecordNFTGalleryView(int nft_count) override;

  // KeyringServiceObserverBase:
  void WalletCreated() override;

  // mojom::TxServiceObserver:
  void OnNewUnapprovedTx(mojom::TransactionInfoPtr tx_info) override {}
  void OnUnapprovedTxUpdated(mojom::TransactionInfoPtr tx_info) override {}
  void OnTransactionStatusChanged(mojom::TransactionInfoPtr tx_info) override;
  void OnTxServiceReset() override {}

 private:
  FRIEND_TEST_ALL_PREFIXES(WootzWalletP3AUnitTest, ReportTransactionSent);
  friend class WootzWalletP3AUnitTest;

  void MigrateUsageProfilePrefsToLocalState();
  void OnUpdateTimerFired();
  void WriteUsageStatsToHistogram();
  void RecordInitialWootzWalletP3AState();
  std::optional<mojom::OnboardingAction> GetLastOnboardingAction();
  void RecordOnboardingHistogram();
  void MaybeRecordNewUserBalance();
  void ReportNftDiscoverySetting();
  void ReportTransactionSent(mojom::CoinType coin, bool new_send);
  raw_ptr<WootzWalletService> wallet_service_;
  raw_ptr<KeyringService> keyring_service_;
  raw_ptr<TxService> tx_service_;
  raw_ptr<PrefService> profile_prefs_;
  raw_ptr<PrefService> local_state_;

  mojo::Receiver<mojom::KeyringServiceObserver>
      keyring_service_observer_receiver_{this};

  mojo::Receiver<mojom::TxServiceObserver> tx_service_observer_receiver_{this};

  base::OneShotTimer onboarding_report_timer_;

  mojo::ReceiverSet<mojom::WootzWalletP3A> receivers_;
  base::RepeatingTimer update_timer_;
  PrefChangeRegistrar local_state_change_registrar_;
  PrefChangeRegistrar profile_pref_change_registrar_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_P3A_H_
