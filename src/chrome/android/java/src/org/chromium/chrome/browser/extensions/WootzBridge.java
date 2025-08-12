package org.chromium.chrome.browser.extensions;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.base.ContextUtils;
import org.chromium.chrome.browser.ActivityUtils;
import android.content.Context;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.text.util.Linkify;
import android.widget.TextView;
import android.content.Intent;
import android.os.Build;
import android.util.Log;
import org.jni_zero.NativeMethods;
import org.chromium.content_public.browser.WebContents;


public class WootzBridge {
    public static void onDropdownButtonClicked(String selectedFeature, String extensionId, String extensionName) {        
        WootzBridgeJni.get().onDropdownButtonClicked(selectedFeature, extensionId, extensionName);
    }

    @NativeMethods
    interface Natives {
        void onDropdownButtonClicked(String selectedFeature, String extensionId, String extensionName);
    }
}
