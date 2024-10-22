package org.chromium.chrome.browser.extensions;

import android.app.AlertDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import org.chromium.content_public.browser.WebContents;

public class BrowsingDataConsentDialog {
    public static void show(Context context, WebContents webContents, long nativeCallbackPtr) {
        // new AlertDialog.Builder(context)
        //     .setTitle("Browsing Data Collection")
        //     .setMessage("Do you consent to the collection of your browsing data?")
        //     .setPositiveButton("Yes", (dialog, which) -> {
        //         SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        //         prefs.edit().putBoolean("browsing_data_consent", true).apply();
        //         WootzBridgeJni.get().onConsentDialogResult(nativeCallbackPtr, true);
        //     })
        //     .setNegativeButton("No", (dialog, which) -> {
        //         WootzBridgeJni.get().onConsentDialogResult(nativeCallbackPtr, false);
        //     })
        //     .show();
    }

    public interface ConsentCallback {
        void onConsent(boolean consented);
    }
}