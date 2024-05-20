// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.safety_hub;

import org.jni_zero.JniType;
import org.jni_zero.NativeMethods;

import org.chromium.chrome.browser.profiles.Profile;

public class UnusedSitePermissionsBridge {
    @NativeMethods
    interface Natives {
        @JniType("std::vector<PermissionsData>")
        PermissionsData[] getRevokedPermissions(@JniType("Profile*") Profile profile);

        void regrantPermissions(
                @JniType("Profile*") Profile profile,
                @JniType("std::string") String primaryPattern);

        void undoRegrantPermissions(
                @JniType("Profile*") Profile profile,
                @JniType("PermissionsData") PermissionsData permissionsData);

        void clearRevokedPermissionsReviewList(@JniType("Profile*") Profile profile);

        void restoreRevokedPermissionsReviewList(
                @JniType("Profile*") Profile profile,
                @JniType("std::vector<PermissionsData>") PermissionsData[] permissionsDataList);
    }
}
