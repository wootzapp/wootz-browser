// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.privacy_sandbox;

import org.jni_zero.NativeMethods;

/** Bridge, providing access to the native-side Tracking Protection configuration. */
// TODO(crbug.com/1410601): Pass in the profile and remove GetActiveUserProfile in C++.
public class TrackingProtectionBridge {

    public static @NoticeType int getRequiredNotice() {
        return TrackingProtectionBridgeJni.get().getRequiredNotice();
    }

    public static void noticeActionTaken(@NoticeType int noticeType, @NoticeAction int action) {
        TrackingProtectionBridgeJni.get().noticeActionTaken(noticeType, action);
    }

    public static void noticeRequested(@NoticeType int noticeType) {
        TrackingProtectionBridgeJni.get().noticeRequested(noticeType);
    }

    public static void noticeShown(@NoticeType int noticeType) {
        TrackingProtectionBridgeJni.get().noticeShown(noticeType);
    }

    public static boolean isOffboarded() {
        return TrackingProtectionBridgeJni.get().isOffboarded();
    }

    @NativeMethods
    public interface Natives {
        void noticeRequested(int noticeType);

        void noticeShown(int noticeType);

        void noticeActionTaken(int noticeType, int action);

        int getRequiredNotice();

        boolean isOffboarded();
    }
}
