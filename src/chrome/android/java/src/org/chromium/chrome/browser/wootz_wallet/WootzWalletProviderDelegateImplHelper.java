/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
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
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.settings.WootzWalletPreferences;
import org.chromium.content_public.browser.WebContents;

import android.app.AlertDialog;
import android.content.Context;
import org.chromium.chrome.browser.extensions.ExtensionInfo;
import org.chromium.chrome.browser.extensions.Extensions;
import org.chromium.chrome.browser.extensions.OpenExtensionsById;

import java.util.ArrayList;
import android.app.Dialog;
import android.graphics.drawable.ColorDrawable;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.Window;
import android.widget.TextView;
import android.view.WindowManager;
import android.content.res.Configuration;
import android.graphics.Color;
import android.view.View;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.util.ConfigurationUtils;
import org.chromium.chrome.browser.wootz_wallet.util.AndroidUtils;
import android.graphics.drawable.GradientDrawable;

@JNINamespace("wootz_wallet")
public class WootzWalletProviderDelegateImplHelper {
    private static final String TAG = "WootzWalletProvider";

    @CalledByNative
    public static void showPanel() {
        try {
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            // activity.showWalletPanel(false);
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
            Log.e(TAG, "showPanel " + e);
        }
    }

    @CalledByNative
    public static void showUnlockWalletAlert() {
        try {
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            
            final String walletExtensionId = Extensions.getExtensionsInfo().stream()
                    .filter(ext -> ext.getName().equals("Wootz Wallet"))
                    .map(ExtensionInfo::getId)
                    .findFirst()
                    .orElse(null);

            if (walletExtensionId != null) {
                Dialog dialog = new Dialog(activity, R.style.WootzWalletDAppNotificationDialogBottom);
                dialog.setContentView(R.layout.dapps_dialog);
                
                // Get root view for click listener
                View rootView = dialog.findViewById(R.id.dapp_dialog_root);
                rootView.setOnClickListener(v -> {
                    OpenExtensionsById.openExtensionByIdNative(walletExtensionId);
                    dialog.dismiss();
                });
                
                // Configure dialog window
                Window window = dialog.getWindow();
                if (window != null) {
                    WindowManager.LayoutParams params = window.getAttributes();
                    params.width = dpToPx(activity, 320);
                    params.height = dpToPx(activity, 250);
                    params.gravity = Gravity.BOTTOM;
                    window.setAttributes(params);
                }
                
                dialog.setCancelable(true);
                dialog.show();
                
                // Auto dismiss after 30 seconds
                new Handler(Looper.getMainLooper()).postDelayed(() -> {
                    if (dialog != null && dialog.isShowing()) {
                        dialog.dismiss();
                    }
                }, 30000);
                
            } else {
                Log.e(TAG, "Failed to find Wootz Wallet extension");
            }
            
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
            Log.e(TAG, "ShowUnlockWalletAlert " + e);
        }
    }

    private static int dpToPx(Context context, int dp) {
        return (int) (dp * context.getResources().getDisplayMetrics().density);
    }

    @CalledByNative
    public static void showWalletBackup() {
        try {
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            activity.openWootzWalletBackup();
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
            Log.e(TAG, "showWalletBackup", e);
        }
    }

    @CalledByNative
    public static void showWalletOnboarding() {
        try {
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            
            Dialog dialog = new Dialog(activity, R.style.WootzWalletDAppNotificationDialogBottom);
            dialog.setContentView(R.layout.dapps_dialog);
            
            TextView textView = dialog.findViewById(R.id.tv_dapp_desc);
            if (textView != null) {
                textView.setText("Wootzapp Wallet is not installed. Please install it to continue.");
            }

            // Get root view for click listener
            View rootView = dialog.findViewById(R.id.dapp_dialog_root);
            rootView.setOnClickListener(v -> {
                // Check if Wootz Wallet extension is installed
                final String walletExtensionId = Extensions.getExtensionsInfo().stream()
                        .filter(ext -> ext.getName().equals("Wootz Wallet"))
                        .map(ExtensionInfo::getId)
                        .findFirst()
                        .orElse(null);
                
                if (walletExtensionId != null) {
                    // If extension is installed, open it directly
                    OpenExtensionsById.openExtensionByIdNative(walletExtensionId);
                } else {
                    // If extension is not installed, navigate to flow-store in a new tab
                    if (activity.getActivityTab() != null) {
                        // Create a new tab instead of using the current one
                        activity.getTabCreator(false).createNewTab(
                            new org.chromium.content_public.browser.LoadUrlParams("wootzapp://flow-store"),
                            org.chromium.chrome.browser.tab.TabLaunchType.FROM_CHROME_UI,
                            activity.getActivityTab());
                    } else {
                        // Fallback to intent if no active tab
                        android.content.Intent intent = new android.content.Intent(android.content.Intent.ACTION_VIEW);
                        intent.setData(android.net.Uri.parse("wootzapp://flow-store"));
                        activity.startActivity(intent);
                    }
                }
                dialog.dismiss();
            });
            
            // Configure dialog window
            Window window = dialog.getWindow();
            if (window != null) {
                WindowManager.LayoutParams params = window.getAttributes();
                params.width = dpToPx(activity, 320);
                params.height = dpToPx(activity, 250);
                params.gravity = Gravity.BOTTOM;
                window.setAttributes(params);
            }
            
            dialog.setCancelable(true);
            dialog.show();
            
            // Auto dismiss after 30 seconds
            new Handler(Looper.getMainLooper()).postDelayed(() -> {
                if (dialog != null && dialog.isShowing()) {
                    dialog.dismiss();
                }
            }, 30000);
            
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
            Log.e(TAG, "showWalletOnboarding " + e);
        }
    }

    @CalledByNative
    public static void walletInteractionDetected(WebContents webContents) {
        try {
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            activity.walletInteractionDetected(webContents);
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
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
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            activity.showAccountCreation(coinType);
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
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
