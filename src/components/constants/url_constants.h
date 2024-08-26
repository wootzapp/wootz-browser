/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_CONSTANTS_URL_CONSTANTS_H_
#define COMPONENTS_CONSTANTS_URL_CONSTANTS_H_

inline constexpr char kChromeExtensionScheme[] = "chrome-extension";
inline constexpr char kWootzUIScheme[] = "wootz";
inline constexpr char kMagnetScheme[] = "magnet";
inline constexpr char kWidevineTOS[] = "https://policies.google.com/terms";
inline constexpr char kRewardsUpholdSupport[] =
    "https://uphold.com/en/wootz/support";
inline constexpr char kP3ALearnMoreURL[] = "https://chrome.com/P3A";
inline constexpr char kP3ASettingsLink[] = "chrome://settings/privacy";
inline constexpr char kImportDataHelpURL[] =
    "https://support.wootz.com/hc/en-us/articles/360019782291#safari";
inline constexpr char kCryptoWalletsLearnMoreURL[] =
    "https://support.wootz.com/hc/en-us/articles/360034535452";
inline constexpr char kPermissionPromptLearnMoreUrl[] =
    "https://github.com/wootz/wootz-browser/wiki/Web-API-Permissions";
inline constexpr char kPermissionPromptHardwareAccessPrivacyRisksURL[] =
    "https://github.com/wootz/wootz-browser/wiki/"
    "Privacy-risks-from-allowing-sites-to-access-hardware";
inline constexpr char kSpeedreaderLearnMoreUrl[] =
    "https://support.wootz.com/hc/en-us/articles/"
    "360045031392-What-is-SpeedReader";
inline constexpr char kWebDiscoveryLearnMoreUrl[] =
    "https://chrome.com/privacy/browser/#web-discovery-project";
inline constexpr char kWootzSearchHost[] = "search.wootz.com";
inline constexpr char kWidevineLearnMoreUrl[] =
    "https://support.wootz.com/hc/en-us/articles/"
    "360023851591-How-do-I-view-DRM-protected-content-";
inline constexpr char kDevChannelDeprecationLearnMoreUrl[] =
    "https://support.wootz.com/hc/en-us/articles/"
    "17924707453581-How-do-I-migrate-my-Wootz-Dev-data-to-another-channel-"
    "Nightly-Beta-Release-";

// This is introduced to replace |kDownloadChromeUrl| in
// outdated_upgrade_bubble_view.cc"
// |kDownloadChromeUrl| couldn't be replaced with char array because array
// should be initialized with initialize list or string literal.
// So, this macro is used.
#define kDownloadWootzUrl "https://www.wootz.com/download"

#endif  // COMPONENTS_CONSTANTS_URL_CONSTANTS_H_
