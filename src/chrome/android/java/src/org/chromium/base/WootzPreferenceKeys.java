/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.base;

public final class WootzPreferenceKeys {
    public static final String WOOTZ_BOTTOM_TOOLBAR_ENABLED_KEY =
            "wootz_bottom_toolbar_enabled_key";
    public static final String WOOTZ_BOTTOM_TOOLBAR_SET_KEY = "wootz_bottom_toolbar_enabled";
    public static final String WOOTZ_IS_MENU_FROM_BOTTOM = "wootz_is_menu_from_bottom";
    public static final String WOOTZ_USE_CUSTOM_TABS = "use_custom_tabs";
    public static final String WOOTZ_APP_OPEN_COUNT = "wootz_app_open_count";
    public static final String WOOTZ_ROLE_MANAGER_DIALOG_COUNT = "wootz_role_manager_dialog_count";
    public static final String WOOTZ_IS_DEFAULT = "wootz_is_default";
    public static final String WOOTZ_WAS_DEFAULT_ASK_COUNT = "wootz_was_default_ask_count";
    public static final String WOOTZ_SET_DEFAULT_BOTTOM_SHEET_COUNT =
            "wootz_set_default_bottom_sheet_count";
    public static final String WOOTZ_DEFAULT_DONT_ASK = "wootz_default_dont_ask";
    public static final String WOOTZ_UPDATE_EXTRA_PARAM =
            "org.chromium.chrome.browser.upgrade.UPDATE_NOTIFICATION_NEW";
    public static final String WOOTZ_NOTIFICATION_PREF_NAME =
            "org.chromium.chrome.browser.upgrade.NotificationUpdateTimeStampPreferences_New";
    public static final String WOOTZ_MILLISECONDS_NAME =
            "org.chromium.chrome.browser.upgrade.Milliseconds_New";
    public static final String WOOTZ_DOWNLOADS_AUTOMATICALLY_OPEN_WHEN_POSSIBLE =
            "org.chromium.chrome.browser.downloads.Automatically_Open_When_Possible";
    public static final String WOOTZ_DOWNLOADS_DOWNLOAD_PROGRESS_NOTIFICATION_BUBBLE =
            "org.chromium.chrome.browser.downloads.Download_Progress_Notification_Bubble";
    public static final String WOOTZ_DOUBLE_RESTART =
            "org.chromium.chrome.browser.Wootz_Double_Restart";
    public static final String WOOTZ_TAB_GROUPS_ENABLED =
            "org.chromium.chrome.browser.Wootz_Tab_Groups_Enabled";
    public static final String WOOTZ_DISABLE_SHARING_HUB =
            "org.chromium.chrome.browser.Wootz_Disable_Sharing_Hub";

    public static final String WOOTZ_USE_BIOMETRICS_FOR_WALLET =
            "org.chromium.chrome.browser.Wootz_Use_Biometrics_For_Wallet";
    public static final String WOOTZ_BIOMETRICS_FOR_WALLET_IV =
            "org.chromium.chrome.browser.Wootz_Biometrics_For_Wallet_Iv";
    public static final String WOOTZ_BIOMETRICS_FOR_WALLET_ENCRYPTED =
            "org.chromium.chrome.browser.Wootz_Biometrics_For_Wallet_Encrypted";
    public static final String WOOTZ_AD_FREE_CALLOUT_DIALOG = "wootz_ad_free_callout_dialog";
    public static final String WOOTZ_OPENED_YOUTUBE = "wootz_opened_youtube";
    public static final String WOOTZ_BACKGROUND_VIDEO_PLAYBACK_CONVERTED_TO_FEATURE =
            "wootz_background_video_playback_converted_to_feature";
    public static final String WOOTZ_DEFERRED_DEEPLINK_PLAYLIST =
            "wootz_deferred_deeplink_playlist";
    public static final String WOOTZ_DEFERRED_DEEPLINK_VPN = "wootz_deferred_deeplink_vpn";
    public static final String WOOTZ_CLOSE_TABS_ON_EXIT = "close_tabs_on_exit";
    public static final String WOOTZ_CLEAR_ON_EXIT = "clear_on_exit";
    public static final String WOOTZ_QUICK_ACTION_SEARCH_AND_BOOKMARK_WIDGET_TILES =
            "org.chromium.chrome.browser.widget.quickactionsearchandbookmark.QuickActionSearchAndBookmarkWidgetProvider.TILES";
    public static final String ENABLE_MULTI_WINDOWS = "enable_multi_windows";
    public static final String ENABLE_MULTI_WINDOWS_UPGRADE = "enable_multi_windows_upgrade";

    // These are dynamic keys
    public static final String WOOTZ_RECYCLERVIEW_POSITION = "recyclerview_visible_position_";
    public static final String WOOTZ_RECYCLERVIEW_OFFSET_POSITION = "recyclerview_offset_position_";

    public static final String WOOTZ_IN_APP_UPDATE_TIMING = "in_app_update_timing";

    public static final String DAY_ZERO_EXPT_FLAG = "day_zero_expt_flag";

    /*
     * Checks if preference key is used in Wootz.
     * It's no op currently. We might reconsider
     * using it in the future for keys sanitation
     */
    public static boolean isWootzKeyInUse(String key) {
        return true;
    }
}