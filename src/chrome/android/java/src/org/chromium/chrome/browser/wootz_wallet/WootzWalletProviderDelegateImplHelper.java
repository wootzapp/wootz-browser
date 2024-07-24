/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.wootz_wallet;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.base.Callback;
import org.chromium.base.Callbacks;
import org.chromium.base.Log;
import org.chromium.wootz_wallet.mojom.CoinType;
import org.chromium.chrome.browser.app.WootzActivity;
import org.chromium.chrome.browser.settings.WootzWalletPreferences;
import org.chromium.content_public.browser.WebContents;

@JNINamespace("wootz_wallet")
public class WootzWalletProviderDelegateImplHelper {
    private static final String TAG = "WootzWalletProvider";

    @CalledByNative
    public static void showPanel() {
        try {
            WootzActivity activity = WootzActivity.getWootzActivity();
            activity.showWalletPanel(false);
        } catch (WootzActivity.WootzActivityNotFoundException e) {
            Log.e(TAG, "showPanel " + e);
        }
    }

    @CalledByNative
    public static void showWalletBackup() {
        try {
            WootzActivity activity = WootzActivity.getWootzActivity();
            activity.openWootzWalletBackup();
        } catch (WootzActivity.WootzActivityNotFoundException e) {
            Log.e(TAG, "showWalletBackup", e);
        }
    }

    @CalledByNative
    public static void showWalletOnboarding() {
        try {
            WootzActivity activity = WootzActivity.getWootzActivity();
            activity.showWalletOnboarding();
        } catch (WootzActivity.WootzActivityNotFoundException e) {
            Log.e(TAG, "showWalletOnboarding " + e);
        }
    }

    @CalledByNative
    public static void walletInteractionDetected(WebContents webContents) {
        try {
            WootzActivity activity = WootzActivity.getWootzActivity();
            activity.walletInteractionDetected(webContents);
        } catch (WootzActivity.WootzActivityNotFoundException e) {
            Log.e(TAG, "walletInteractionDetected " + e);
        }
    }

    @CalledByNative
    public static boolean isWeb3NotificationAllowed() {
        return WootzWalletPreferences.getPrefWeb3NotificationsEnabled();
    }

    @CalledByNative
    public static void ShowAccountCreation(@CoinType.EnumType int coinType) {
        try {
            WootzActivity activity = WootzActivity.getWootzActivity();
            activity.showAccountCreation(coinType);
        } catch (WootzActivity.WootzActivityNotFoundException e) {
            Log.e(TAG, "ShowAccountCreation " + e);
        }
    }

    public static void IsSolanaConnected(
            WebContents webContents, String account, Callbacks.Callback1<Boolean> callback) {
        Callback<Boolean> callbackWrapper = result -> {
            callback.call(result);
        };
        WootzWalletProviderDelegateImplHelperJni.get().IsSolanaConnected(
                webContents, account, callbackWrapper);
    }

    @NativeMethods
    interface Natives {
        void IsSolanaConnected(WebContents webContents, String account, Callback<Boolean> callback);
    }
}
