// Copyright 2024 The WootzApp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Added By DevJangid
package org.chromium.chrome.browser.extensions;

import org.chromium.chrome.browser.app.ChromeActivity;


import android.content.Context;
import androidx.appcompat.app.AlertDialog;
import java.util.ArrayList;
import android.util.Log;

public class OpenExtensionsById {
    private static final String TAG ="OpenExtensionsById";
    public static void openExtensionById(String extensionId){
        Log.d(TAG," Extension id by "+extensionId);

        try {
            Log.d(TAG, "JANGID: CALLING openExtensionById");
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            activity.getRootUiCoordinatorForTesting().getAppMenuCoordinatorForTesting()
                    .openExtensionById(extensionId);
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
            Log.e(TAG, "JANGID: openExtensionById " + e);
        }
    }

    public static void closeExtensionBottomSheet(){
        try {
            Log.d(TAG, "JANGID: CALLING closeExtensionBottomSheet");
            ChromeActivity activity = ChromeActivity.getChromeActivity();
            activity.getRootUiCoordinatorForTesting().getAppMenuCoordinatorForTesting()
                    .closeExtensionBottomSheet();
        } catch (ChromeActivity.ChromeActivityNotFoundException e) {
            Log.e(TAG, "JANGID: closeExtensionBottomSheet " + e);
        }
    }

}