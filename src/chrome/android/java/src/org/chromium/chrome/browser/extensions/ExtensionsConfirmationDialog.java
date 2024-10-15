package org.chromium.chrome.browser.extensions;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.base.ContextUtils;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content.browser.webcontents.WebContentsImpl;
import org.chromium.chrome.browser.ActivityUtils;
import android.content.Context;
import androidx.appcompat.app.AlertDialog;
import java.util.ArrayList;

import android.util.Log;

public class ExtensionsConfirmationDialog {
    @CalledByNative
    public static void showInstallDialog(long doneCallback, WebContents webContents) {
        Context context = ActivityUtils.getActivityFromWebContents(webContents);
        new AlertDialog.Builder(context)
            .setTitle("Install Extension")
            .setMessage("Do you want to install this extension?")
            .setPositiveButton("Install", (dialog, which) -> {
                Log.d("exts", "install");
                ExtensionsConfirmationDialogJni.get().onDialogResult(doneCallback, true);
            })
            .setNegativeButton("Cancel", (dialog, which) -> {
                Log.d("exts", "cancel");
                ExtensionsConfirmationDialogJni.get().onDialogResult(doneCallback, false);
            })
            .show();
    }

    @NativeMethods
    interface Natives {
        void onDialogResult(long doneCallback, boolean confirmed);
    }
}