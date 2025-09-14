// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.incognito;

import android.view.Window;
import android.view.WindowManager;

import androidx.annotation.NonNull;

import org.chromium.base.supplier.Supplier;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.customtabs.ContentPrivacyPreferencesBridge;

/**
 * An abstract base class to provide common functionalities related to allowing/blocking snapshot
 * for Incognito tabs across {@link ChromeTabbedActivity} and {@link CustomTabActivity}.
 */
public abstract class IncognitoSnapshotController {
    private final @NonNull Window mWindow;
    private final @NonNull Supplier<Boolean> mIsShowingIncognitoSupplier;

    /**
     * @param window The {@link Window} on which the snapshot capability needs to be controlled.
     * @param isShowingIncognitoSupplier {@link Supplier<Boolean>} which indicates whether we are
     *     showing Incognito or not currently.
     */
    protected IncognitoSnapshotController(
            @NonNull Window window, @NonNull Supplier<Boolean> isShowingIncognitoSupplier) {
        mWindow = window;
        mIsShowingIncognitoSupplier = isShowingIncognitoSupplier;
    }

    /** Sets the attributes flags to secure if there is an incognito tab visible. */
    protected void updateIncognitoTabSnapshotState() {
        // Safety check to prevent native crashes
        if (mWindow == null) return;
        
        try {
            WindowManager.LayoutParams attributes = mWindow.getAttributes();
            if (attributes == null) return;
            
            boolean currentSecureState =
                    (attributes.flags & WindowManager.LayoutParams.FLAG_SECURE)
                            == WindowManager.LayoutParams.FLAG_SECURE;

            // Block screenshots when:
            // 1. Content privacy is enabled via SAML, OR
            // 2. User is in incognito mode (and incognito screenshots are disabled)
            boolean contentPrivacyEnabled = ContentPrivacyPreferencesBridge.isContentPrivacyEnabled();
            boolean isIncognitoMode = mIsShowingIncognitoSupplier.get();
            
            boolean expectedSecureState = contentPrivacyEnabled || isIncognitoMode;
            
            if (currentSecureState == expectedSecureState) return;

            if (expectedSecureState) {
                mWindow.addFlags(WindowManager.LayoutParams.FLAG_SECURE);
            } else {
                mWindow.clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
            }
        } catch (Exception e) {
            // Log error but don't crash
            android.util.Log.e("IncognitoSnapshotController", "Error setting FLAG_SECURE", e);
        }
    }
}
