/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.base;

/**
 * A list of feature flags exposed to Java.
 */
public abstract class WootzFeatureList {
    public static final String NATIVE_WOOTZ_WALLET = "NativeWootzWallet";
    public static final String USE_DEV_UPDATER_URL = "UseDevUpdaterUrl";
    public static final String FORCE_WEB_CONTENTS_DARK_MODE = "WebContentsForceDark";
    public static final String ENABLE_FORCE_DARK = "enable-force-dark";
    public static final String ENABLE_PARALLEL_DOWNLOADING = "enable-parallel-downloading";
    public static final String WOOTZ_SEARCH_OMNIBOX_BANNER = "WootzSearchOmniboxBanner";
    public static final String WOOTZ_BACKGROUND_VIDEO_PLAYBACK = "WootzBackgroundVideoPlayback";
    public static final String WOOTZ_BACKGROUND_VIDEO_PLAYBACK_INTERNAL =
            "wootz-background-video-playback";
    public static final String WOOTZ_ANDROID_SAFE_BROWSING = "WootzAndroidSafeBrowsing";
    public static final String WOOTZ_VPN_LINK_SUBSCRIPTION_ANDROID_UI =
            "WootzVPNLinkSubscriptionAndroidUI";
    public static final String DEBOUNCE = "WootzDebounce";
    public static final String WOOTZ_GOOGLE_SIGN_IN_PERMISSION = "WootzGoogleSignInPermission";
    public static final String WOOTZ_LOCALHOST_PERMISSION = "WootzLocalhostPermission";
    public static final String WOOTZ_PLAYLIST = "Playlist";
    public static final String WOOTZ_SPEEDREADER = "Speedreader";
    public static final String HTTPS_BY_DEFAULT = "HttpsByDefault";
    public static final String WOOTZ_FORGET_FIRST_PARTY_STORAGE = "WootzForgetFirstPartyStorage";
    public static final String WOOTZ_REQUEST_OTR_TAB = "WootzRequestOTRTab";
    public static final String AI_CHAT = "AIChat";
    public static final String WOOTZ_SHOW_STRICT_FINGERPRINTING_MODE =
            "WootzShowStrictFingerprintingMode";
    public static final String WOOTZ_DAY_ZERO_EXPERIMENT = "WootzDayZeroExperiment";
    public static final String WOOTZ_FALLBACK_DOH_PROVIDER = "WootzFallbackDoHProvider";
}