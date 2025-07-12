// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.customtabs;

import org.jni_zero.NativeMethods;

/**
 * Bridge class to access content privacy preferences from C++ to Java.
 * This allows Java code to check the SAML-controlled content privacy setting.
 */
public class ContentPrivacyPreferencesBridge {
    
    /**
     * Get the current content privacy enabled state from native preferences.
     * @return true if content privacy is enabled (screenshots should be blocked), false otherwise
     */
    public static boolean isContentPrivacyEnabled() {
        return ContentPrivacyPreferencesBridgeJni.get().isContentPrivacyEnabled();
    }
    
    @NativeMethods
    interface Natives {
        /**
         * Returns whether content privacy is enabled from SAML preferences.
         * @return true if content privacy is enabled, false otherwise (default: false)
         */
        boolean isContentPrivacyEnabled();
    }
} 