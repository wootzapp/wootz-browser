package org.chromium.chrome.browser.offlinepages;

import android.content.Context;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

import org.chromium.base.ContextUtils;
import org.chromium.base.Log;

import java.io.File;

/**
 * Utility class to provide storage paths for Wootz offline pages.
 */
@JNINamespace("wootz_offline_pages")
public class WootzOfflinePagePathUtils {
    private static final String TAG = "Kartik: WootzOfflinePagePathUtils";
    private static final String OFFLINE_PAGES_DIR = "WootzOfflinePages";

    /**
     * Returns the external storage directory path for offline pages.
     * Uses app-specific external storage: /storage/emulated/0/Android/data/{package}/files/WootzOfflinePages/
     * This doesn't require any permissions and is accessible via file managers and USB.
     * 
     * @return Absolute path to the offline pages directory, or null if unavailable
     */
    @CalledByNative
    public static String getOfflinePageStoragePath() {
        try {
            Context context = ContextUtils.getApplicationContext();
            // Get app-specific external files directory (no permissions required)
            File externalFilesDir = context.getExternalFilesDir(null);
            
            if (externalFilesDir == null) {
                Log.e(TAG, "External storage not available");
                return null;
            }
            
            // Create WootzOfflinePages subdirectory
            File offlinePagesDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
            String path = offlinePagesDir.getAbsolutePath();
            
            Log.i(TAG, "Offline pages storage path: " + path);
            return path;
        } catch (Exception e) {
            Log.e(TAG, "Failed to get offline pages storage path: " + e.getMessage());
            return null;
        }
    }
}

