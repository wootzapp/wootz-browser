package org.chromium.chrome.browser.firstrun;

import org.chromium.base.Log;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.components.embedder_support.util.UrlConstants;
import org.chromium.chrome.browser.ChromeActivity;
import org.chromium.chrome.browser.init.FirstRunStatus;

public class ExtensionInstaller {
    private static final String TAG = "ExtInstaller";

    public static void installExtensionIfFirstRun(ChromeActivity activity) {
        // Check if this is first run
        if (FirstRunStatus.getFirstRunFlowComplete(activity)) {
            Log.i(TAG, "Not first run - skipping extension installation");
            return;
        }

        Log.i(TAG, "First run detected - starting extension installation");
        
        // Get the extension URL
        String extensionUrl = "https://raw.githubusercontent.com/wootzapp/ext-store/main/Artifact/Artifact.crx";
        
        // Call into C++ to handle the actual download and installation
        nativeInstallExtension(Profile.getLastUsedRegularProfile(), extensionUrl);
    }

    private static native void nativeInstallExtension(Profile profile, String url);
}
