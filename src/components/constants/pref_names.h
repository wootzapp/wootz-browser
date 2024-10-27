/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_CONSTANTS_PREF_NAMES_H_
#define COMPONENTS_CONSTANTS_PREF_NAMES_H_

#include "build/build_config.h"

inline constexpr char kWootzAutofillPrivateWindows[] =
    "wootz.autofill_private_windows";
inline constexpr char kManagedWootzShieldsDisabledForUrls[] =
    "wootz.managed_shields_disabled";
inline constexpr char kManagedWootzShieldsEnabledForUrls[] =
    "wootz.managed_shields_enabled";
inline constexpr char kAdsBlocked[] = "wootz.stats.ads_blocked";
// We no longer update this pref, but we keep it around for now because it's
// added to kAdsBlocked when being displayed.
inline constexpr char kTrackersBlocked[] = "wootz.stats.trackers_blocked";
inline constexpr char kJavascriptBlocked[] = "wootz.stats.javascript_blocked";
inline constexpr char kHttpsUpgrades[] = "wootz.stats.https_upgrades";
inline constexpr char kFingerprintingBlocked[] =
    "wootz.stats.fingerprinting_blocked";
inline constexpr char kLastCheckYMD[] = "wootz.stats.last_check_ymd";
inline constexpr char kLastCheckWOY[] = "wootz.stats.last_check_woy";
inline constexpr char kLastCheckMonth[] = "wootz.stats.last_check_month";
inline constexpr char kFirstCheckMade[] = "wootz.stats.first_check_made";
// Set to true if the user met the threshold requirements and successfully
// sent a ping to the stats-updater server.
inline constexpr char kThresholdCheckMade[] =
    "wootz.stats.threshold_check_made";
// Anonymous usage pings enabled
inline constexpr char kStatsReportingEnabled[] =
    "wootz.stats.reporting_enabled";
// Serialized query for to send to the stats-updater server. Needs to be saved
// in the case that the user sends the standard usage ping, stops the browser,
// meets the threshold requirements, and then starts the browser before the
// threshold ping was sent.
inline constexpr char kThresholdQuery[] = "wootz.stats.threshold_query";
inline constexpr char kWeekOfInstallation[] =
    "wootz.stats.week_of_installation";
inline constexpr char kWidevineEnabled[] = "wootz.widevine_opted_in";
inline constexpr char kAskEnableWidvine[] = "wootz.ask_widevine_install";
inline constexpr char kShowBookmarksButton[] = "wootz.show_bookmarks_button";
inline constexpr char kShowSidePanelButton[] = "wootz.show_side_panel_button";
inline constexpr char kLocationBarIsWide[] = "wootz.location_bar_is_wide";
inline constexpr char kReferralDownloadID[] = "wootz.referral.download_id";
inline constexpr char kReferralTimestamp[] = "wootz.referral.timestamp";
inline constexpr char kReferralAttemptTimestamp[] =
    "wootz.referral.referral_attempt_timestamp";
inline constexpr char kReferralAttemptCount[] =
    "wootz.referral.referral_attempt_count";
inline constexpr char kReferralAndroidFirstRunTimestamp[] =
    "wootz.referral_android_first_run_timestamp";
inline constexpr char kNoScriptControlType[] = "wootz.no_script_default";
inline constexpr char kShieldsAdvancedViewEnabled[] =
    "wootz.shields.advanced_view_enabled";
inline constexpr char kShieldsStatsBadgeVisible[] =
    "wootz.shields.stats_badge_visible";
inline constexpr char kAdControlType[] = "wootz.ad_default";
inline constexpr char kGoogleLoginControlType[] = "wootz.google_login_default";
inline constexpr char kWebTorrentEnabled[] = "wootz.webtorrent_enabled";
// Deprecated
inline constexpr char kHangoutsEnabled[] = "wootz.hangouts_enabled";
inline constexpr char kNewTabPageShowClock[] = "wootz.new_tab_page.show_clock";
inline constexpr char kNewTabPageClockFormat[] =
    "wootz.new_tab_page.clock_format";
inline constexpr char kNewTabPageShowStats[] = "wootz.new_tab_page.show_stats";
inline constexpr char kNewTabPageShowRewards[] =
    "wootz.new_tab_page.show_rewards";
inline constexpr char kNewTabPageShowWootzTalk[] =
    "wootz.new_tab_page.show_together";
inline constexpr char kNewTabPageHideAllWidgets[] =
    "wootz.new_tab_page.hide_all_widgets";
inline constexpr char kNewTabPageShowsOptions[] =
    "wootz.new_tab_page.shows_options";
inline constexpr char kWootzNewsIntroDismissed[] =
    "wootz.today.intro_dismissed";
inline constexpr char kAlwaysShowBookmarkBarOnNTP[] =
    "wootz.always_show_bookmark_bar_on_ntp";
inline constexpr char kWootzDarkMode[] = "wootz.dark_mode";
inline constexpr char kWootzShieldsSettingsVersion[] =
    "wootz.shields_settings_version";
inline constexpr char kDefaultBrowserPromptEnabled[] =
    "wootz.default_browser_prompt_enabled";

inline constexpr char kWebDiscoveryEnabled[] = "wootz.web_discovery_enabled";
inline constexpr char kWebDiscoveryCTAState[] = "wootz.web_discovery.cta_state";
inline constexpr char kDontAskEnableWebDiscovery[] =
    "wootz.dont_ask_enable_web_discovery";
inline constexpr char kWootzSearchVisitCount[] =
    "wootz.wootz_search_visit_count";

inline constexpr char kWootzGCMChannelStatus[] = "wootz.gcm.channel_status";
inline constexpr char kImportDialogExtensions[] = "import_dialog_extensions";
inline constexpr char kImportDialogPayments[] = "import_dialog_payments";
inline constexpr char kMRUCyclingEnabled[] = "wootz.mru_cycling_enabled";
inline constexpr char kTabsSearchShow[] = "wootz.tabs_search_show";
inline constexpr char kTabMuteIndicatorNotClickable[] =
    "wootz.tabs.mute_indicator_not_clickable";
inline constexpr char kDontAskForCrashReporting[] =
    "wootz.dont_ask_for_crash_reporting";

// Cast extension requires a browser restart once the setting is toggled.
// kEnableMediaRouterOnRestart is used as a proxy to identify the current
// state of the switch and prefs::kEnableMediaRouter is updated to
// kEnableMediaRouterOnRestart on restart.
inline constexpr char kEnableMediaRouterOnRestart[] =
    "wootz.enable_media_router_on_restart";

#if BUILDFLAG(IS_ANDROID)
inline constexpr char kDesktopModeEnabled[] = "wootz.desktop_mode_enabled";
inline constexpr char kPlayYTVideoInBrowserEnabled[] =
    "wootz.play_yt_video_in_browser_enabled";
inline constexpr char kBackgroundVideoPlaybackEnabled[] =
    "wootz.background_video_playback";
inline constexpr char kSafetynetCheckFailed[] = "safetynetcheck.failed";
inline constexpr char kSafetynetStatus[] = "safetynet.status";
#endif

#if !BUILDFLAG(IS_ANDROID)
inline constexpr char kEnableWindowClosingConfirm[] =
    "wootz.enable_window_closing_confirm";
inline constexpr char kEnableClosingLastTab[] = "wootz.enable_closing_last_tab";
inline constexpr char kShowFullscreenReminder[] =
    "wootz.show_fullscreen_reminder";
#endif

inline constexpr char kDefaultBrowserLaunchingCount[] =
    "wootz.default_browser.launching_count";

// deprecated
inline constexpr char kNewTabPageShowTopSites[] =
    "wootz.new_tab_page.show_top_sites";
inline constexpr char kOtherBookmarksMigrated[] =
    "wootz.other_bookmarks_migrated";

// Obsolete widget removal prefs
#if !BUILDFLAG(IS_IOS) && !BUILDFLAG(IS_ANDROID)
inline constexpr char kFTXAccessToken[] = "wootz.ftx.access_token";
inline constexpr char kFTXOauthHost[] = "wootz.ftx.oauth_host";
inline constexpr char kFTXNewTabPageShowFTX[] = "ftx.new_tab_page.show_ftx";
inline constexpr char kCryptoDotComNewTabPageShowCryptoDotCom[] =
    "crypto_dot_com.new_tab_page.show_crypto_dot_com";
inline constexpr char kCryptoDotComHasBoughtCrypto[] =
    "crypto_dot_com.new_tab_page.has_bought_crypto";
inline constexpr char kCryptoDotComHasInteracted[] =
    "crypto_dot_com.new_tab_page.has_interacted";
inline constexpr char kGeminiAccessToken[] = "wootz.gemini.access_token";
inline constexpr char kGeminiRefreshToken[] = "wootz.gemini.refresh_token";
inline constexpr char kNewTabPageShowGemini[] =
    "wootz.new_tab_page.show_gemini";
#endif

#if !BUILDFLAG(IS_IOS)
inline constexpr char kBinanceAccessToken[] = "wootz.binance.access_token";
inline constexpr char kBinanceRefreshToken[] = "wootz.binance.refresh_token";
inline constexpr char kNewTabPageShowBinance[] =
    "wootz.new_tab_page.show_binance";
inline constexpr char kWootzSuggestedSiteSuggestionsEnabled[] =
    "wootz.wootz_suggested_site_suggestions_enabled";
#endif

#endif  // COMPONENTS_CONSTANTS_PREF_NAMES_H_
