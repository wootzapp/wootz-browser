/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_CONSTANTS_WEBUI_URL_CONSTANTS_H_
#define COMPONENTS_CONSTANTS_WEBUI_URL_CONSTANTS_H_

#include "build/build_config.h"

inline constexpr char kAdblockHost[] = "adblock";
inline constexpr char kAdblockInternalsHost[] = "adblock-internals";
inline constexpr char kAdblockJS[] = "wootz_adblock.js";
inline constexpr char kSkusInternalsHost[] = "skus-internals";
inline constexpr char kWebcompatReporterHost[] = "webcompat";
inline constexpr char kRewardsPageHost[] = "rewards";
inline constexpr char kRewardsPageURL[] = "chrome://rewards/";
inline constexpr char kRewardsPageTopHost[] = "rewards.top-chrome";
inline constexpr char kRewardsPageTopURL[] = "chrome://rewards.top-chrome";
inline constexpr char kRewardsInternalsHost[] = "rewards-internals";
inline constexpr char kWelcomeHost[] = "welcome";
inline constexpr char kWelcomeJS[] = "wootz_welcome.js";
inline constexpr char kWootzRewardsPanelURL[] =
    "chrome://rewards-panel.top-chrome";
inline constexpr char kWootzRewardsPanelHost[] = "rewards-panel.top-chrome";
inline constexpr char kWootzTipPanelURL[] = "chrome://tip-panel.top-chrome";
inline constexpr char kWootzTipPanelHost[] = "tip-panel.top-chrome";
inline constexpr char kWootzNewTabJS[] = "wootz_new_tab.js";
inline constexpr char kWootzNewsInternalsHost[] = "news-internals";
inline constexpr char kWootzUIAdblockURL[] = "chrome://adblock/";
inline constexpr char kWootzUIWebcompatReporterURL[] = "chrome://webcompat/";
inline constexpr char kWootzUIWalletURL[] = "wootzapp://wallet/";
inline constexpr char kWootzUIWalletOnboardingURL[] =
    "wootzapp://wallet/crypto/onboarding";
inline constexpr char kWootzUIWalletAccountCreationURL[] =
    "wootzapp://wallet/crypto/accounts/add-account/create/";
inline constexpr char kWootzUIWalletPanelURL[] =
    "wootzapp://wallet-panel.top-chrome/";
inline constexpr char kWalletPanelHost[] = "wallet-panel.top-chrome";
inline constexpr char kVPNPanelURL[] =
    "chrome-untrusted://vpn-panel.top-chrome/";
inline constexpr char kVPNPanelHost[] = "vpn-panel.top-chrome";
inline constexpr char kWootzUIWalletPageURL[] = "wootzapp://wallet/";
inline constexpr char kWalletPageHost[] = "wallet";
inline constexpr char kExtensionSettingsURL[] = "wootzapp://settings/extensions";
inline constexpr char kWalletSettingsURL[] = "wootzapp://settings/wallet";
inline constexpr char kWootzSyncPath[] = "wootzSync";
inline constexpr char kWootzSyncSetupPath[] = "wootzSync/setup";
inline constexpr char kTorInternalsHost[] = "tor-internals";
inline constexpr char kUntrustedLedgerHost[] = "ledger-bridge";
inline constexpr char kUntrustedLedgerURL[] =
    "chrome-untrusted://ledger-bridge/";
inline constexpr char kUntrustedNftHost[] = "nft-display";
inline constexpr char kUntrustedNftURL[] = "chrome-untrusted://nft-display/";
inline constexpr char kUntrustedLineChartHost[] = "line-chart-display";
inline constexpr char kUntrustedLineChartURL[] =
    "chrome-untrusted://line-chart-display/";
inline constexpr char kUntrustedMarketHost[] = "market-display";
inline constexpr char kUntrustedMarketURL[] =
    "chrome-untrusted://market-display/";
inline constexpr char kUntrustedTrezorHost[] = "trezor-bridge";
inline constexpr char kUntrustedTrezorURL[] =
    "chrome-untrusted://trezor-bridge/";
// inline constexpr char kShieldsPanelURL[] = "chrome://wootz-shields.top-chrome";
// inline constexpr char kShieldsPanelHost[] = "wootz-shields.top-chrome";
inline constexpr char kCookieListOptInHost[] = "cookie-list-opt-in.top-chrome";
inline constexpr char kCookieListOptInURL[] =
    "chrome://cookie-list-opt-in.top-chrome";
// inline constexpr char kFederatedInternalsURL[] = "wootz://federated-internals";
inline constexpr char kFederatedInternalsHost[] = "federated-internals";
inline constexpr char kContentFiltersPath[] = "shields/filters";
inline constexpr char kPlaylistHost[] = "playlist";
inline constexpr char kPlaylistURL[] = "chrome-untrusted://playlist/";
inline constexpr char kPlaylistPlayerHost[] = "playlist-player";
inline constexpr char kPlaylistPlayerURL[] =
    "chrome-untrusted://playlist-player/";
// inline constexpr char kSpeedreaderPanelURL[] =
//     "chrome://chrome-speedreader.top-chrome";
// inline constexpr char kSpeedreaderPanelHost[] = "wootz-speedreader.top-chrome";
inline constexpr char kShortcutsURL[] = "chrome://settings/system/shortcuts";
inline constexpr char kChatUIURL[] = "chrome-untrusted://chat/";
inline constexpr char kChatUIHost[] = "chat";

inline constexpr char kRewriterUIURL[] = "chrome://rewriter/";
inline constexpr char kRewriterUIHost[] = "rewriter";

inline constexpr char16_t kTransactionSimulationLearnMoreURL[] =
    u"https://github.com/wootz/wootz-browser/wiki/Transaction-Simulation";

#endif  // COMPONENTS_CONSTANTS_WEBUI_URL_CONSTANTS_H_
