/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.preferences;

import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.base.ThreadUtils;
import org.chromium.chrome.browser.profiles.Profile;

@JNINamespace("chrome::android")
public class WootzPrefServiceBridge {
    private WootzPrefServiceBridge() {
    }

    private static WootzPrefServiceBridge sInstance;

    public static WootzPrefServiceBridge getInstance() {
        ThreadUtils.assertOnUiThread();
        if (sInstance == null) {
            sInstance = new WootzPrefServiceBridge();
        }
        return sInstance;
    }

    public void setCookiesBlockType(String type) {
        WootzPrefServiceBridgeJni.get().setCookiesBlockType(type);
    }

    public String getCookiesBlockType() {
        return WootzPrefServiceBridgeJni.get().getCookiesBlockType();
    }

    public void setPlayYTVideoInBrowserEnabled(boolean enabled) {
        WootzPrefServiceBridgeJni.get().setPlayYTVideoInBrowserEnabled(enabled);
    }

    public boolean getPlayYTVideoInBrowserEnabled() {
        return WootzPrefServiceBridgeJni.get().getPlayYTVideoInBrowserEnabled();
    }

    public void setDesktopModeEnabled(boolean enabled) {
        WootzPrefServiceBridgeJni.get().setDesktopModeEnabled(enabled);
    }

    public boolean getDesktopModeEnabled() {
        return WootzPrefServiceBridgeJni.get().getDesktopModeEnabled();
    }

    public void setBackgroundVideoPlaybackEnabled(boolean enabled) {
        WootzPrefServiceBridgeJni.get().setBackgroundVideoPlaybackEnabled(enabled);
    }

    public boolean getBackgroundVideoPlaybackEnabled() {
        return WootzPrefServiceBridgeJni.get().getBackgroundVideoPlaybackEnabled();
    }

    public long getTrackersBlockedCount(Profile profile) {
        return WootzPrefServiceBridgeJni.get().getTrackersBlockedCount(profile);
    }

    public long getDataSaved(Profile profile) {
        return WootzPrefServiceBridgeJni.get().getDataSaved(profile);
    }

    /**
     * @param whether SafetyNet check failed.
     */
    public void setSafetynetCheckFailed(boolean value) {
        WootzPrefServiceBridgeJni.get().setSafetynetCheckFailed(value);
    }

    public boolean getSafetynetCheckFailed() {
        return WootzPrefServiceBridgeJni.get().getSafetynetCheckFailed();
    }

    public void resetPromotionLastFetchStamp() {
        WootzPrefServiceBridgeJni.get().resetPromotionLastFetchStamp();
    }

    public void setOldTrackersBlockedCount(Profile profile, long count) {
        WootzPrefServiceBridgeJni.get().setOldTrackersBlockedCount(profile, count);
    }

    public void setOldHttpsUpgradesCount(Profile profile, long count) {
        WootzPrefServiceBridgeJni.get().setOldHttpsUpgradesCount(profile, count);
    }

    public boolean GetBooleanForContentSetting(int content_type) {
        return WootzPrefServiceBridgeJni.get().getBooleanForContentSetting(content_type);
    }

    public void setWebrtcPolicy(int policy) {
        WootzPrefServiceBridgeJni.get().setWebrtcPolicy(policy);
    }

    public int getWebrtcPolicy() {
        return WootzPrefServiceBridgeJni.get().getWebrtcPolicy();
    }

    @NativeMethods
    interface Natives {
        void setCookiesBlockType(String type);
        String getCookiesBlockType();

        void setPlayYTVideoInBrowserEnabled(boolean enabled);
        boolean getPlayYTVideoInBrowserEnabled();

        void setDesktopModeEnabled(boolean enabled);
        boolean getDesktopModeEnabled();

        void setBackgroundVideoPlaybackEnabled(boolean enabled);
        boolean getBackgroundVideoPlaybackEnabled();

        long getTrackersBlockedCount(Profile profile);
        long getDataSaved(Profile profile);

        // Used to pass total stat from upgrading old tabs based browser
        // to a new core based
        void setOldTrackersBlockedCount(Profile profile, long count);
        void setOldHttpsUpgradesCount(Profile profile, long count);

        void setSafetynetCheckFailed(boolean value);
        boolean getSafetynetCheckFailed();

        void resetPromotionLastFetchStamp();
        boolean getBooleanForContentSetting(int content_type);

        void setWebrtcPolicy(int policy);
        int getWebrtcPolicy();
    }
}
