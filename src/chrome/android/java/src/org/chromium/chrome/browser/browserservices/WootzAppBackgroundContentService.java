package org.chromium.chrome.browser.browserservices;

import android.content.Context;
import android.util.Log;

import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content_public.browser.NavigationHandle;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.browser.WebContentsObserver;
import org.chromium.chrome.browser.content.WebContentsFactory;
import org.chromium.base.R.id;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.profiles.Profile;
import java.util.Map;
import java.util.HashMap;
import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import androidx.annotation.Nullable;

public class WootzAppBackgroundContentService {
    private static final String TAG = "WootzAppBgContentSvc";

    private static final Map<Integer, WebContents> sWebContentsMap = new HashMap<>();

    /**
     * Creates a background WebContents and loads the given URL.
     * This does NOT attach to any UI.
     */
    @CalledByNative
    public static void createBackgroundWebContents(int webContentsId, String url) {
        destroyBackgroundWebContents(webContentsId);
    
        Profile profile = ProfileManager.getLastUsedRegularProfile();
        if (profile == null) return;
    
        WebContents webContents = WebContentsFactory.createWebContents(profile, true, false);
        if (webContents == null) return;
    
        webContents.getNavigationController().loadUrl(new LoadUrlParams(url));
        sWebContentsMap.put(webContentsId, webContents);
    }

    /**
     * Destroys the background WebContents with the given name if it exists.
     */
    @CalledByNative
    public static void destroyBackgroundWebContents(int webContentsId) {
        WebContents webContents = sWebContentsMap.get(webContentsId);
        if (webContents != null) {
            webContents.destroy();
            sWebContentsMap.remove(webContentsId);
            Log.e(TAG, "Background WebContents destroyed for webContentsId: " + webContentsId);
        }
    }

    /**
     * Destroys all background WebContents.
     */
    @CalledByNative
    public static void destroyAllBackgroundWebContents() {
        for (WebContents wc : sWebContentsMap.values()) {
            wc.destroy();
        }
        sWebContentsMap.clear();
        Log.e(TAG, "All background WebContents destroyed.");
    }

    /**
     * Returns the background WebContents, if you want to interact with it (inject JS, etc).
     */
    @CalledByNative
    public static WebContents getBackgroundWebContents(int webContentsId) {
        return sWebContentsMap.get(webContentsId);
    }

}
