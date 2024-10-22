package org.chromium.chrome.browser.extensions;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

import org.chromium.base.ContextUtils;
import org.chromium.chrome.browser.ActivityUtils;
import android.content.Context;
import android.app.AlertDialog;
import org.chromium.content_public.browser.WebContents;

@JNINamespace("extensions")
public class WootzBridge {

    @CalledByNative
    private static void showDialog(WebContents webContents) {
        Context context = ActivityUtils.getActivityFromWebContents(webContents);
        new AlertDialog.Builder(context)
            .setTitle("Wootz Dialog")
            .setMessage("This is a dialog from the Wootz API!")
            .setPositiveButton("OK", null)
            .show();
    }
}
