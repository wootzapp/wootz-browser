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
import org.chromium.content_public.browser.WebContents;
// import org.chromium.chrome.browser.extensions.BrowsingHistoryService;

public class WootzBridge {

    @CalledByNative
    private static void showDialog(WebContents webContents) {
        final Context context = ActivityUtils.getActivityFromWebContents(webContents);
        
        SpannableString message = new SpannableString(
            "This extension requests permission to access and track your browsing history, " +
            "open tabs, and other browser activities. This data will be used to provide " +
            "personalized services and improve your browsing experience.\n\n" +
            "For more information, please visit our privacy policy at " +
            "https://example.com/privacy-policy\n\n" +
            "Do you consent to allow this extension to access this information?"
        );
        Linkify.addLinks(message, Linkify.WEB_URLS);

        AlertDialog dialog = new AlertDialog.Builder(context)
            .setTitle("Permission Request")
            .setMessage(message)
            .setPositiveButton("Allow", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    // Start the background service
                    Intent serviceIntent = new Intent(context, BrowsingHistoryService.class);
                    context.startService(serviceIntent);
                    
                    onConsentResult(true);
                }
            })
            .setNegativeButton("Deny", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    onConsentResult(false);
                }
            })
            .create();

        dialog.show();

        // Make the message clickable
        ((TextView)dialog.findViewById(android.R.id.message)).setMovementMethod(LinkMovementMethod.getInstance());
    }

    // @CalledByNative
    // private static void stopActivitr

    public static void onConsentResult(boolean consented) {
        if (consented) {
            Intent serviceIntent = new Intent(ContextUtils.getApplicationContext(), BrowsingHistoryService.class);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                ContextUtils.getApplicationContext().startForegroundService(serviceIntent);
            } else {
                ContextUtils.getApplicationContext().startService(serviceIntent);
            }
        }
        WootzBridgeJni.get().onConsentResult(consented);
    }

    @NativeMethods
    interface Natives {
        void onConsentResult(boolean consented);
    }
}
