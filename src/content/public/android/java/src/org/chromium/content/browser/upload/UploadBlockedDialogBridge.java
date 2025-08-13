// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser.upload;

import android.app.Activity;

import androidx.annotation.NonNull;

import org.jni_zero.CalledByNative;
import org.jni_zero.NativeMethods;

import org.chromium.content_public.browser.WebContents;
import org.chromium.ui.base.WindowAndroid;

/**
 * Glues upload blocked dialog notification and handles the communication to native backend.
 */
public class UploadBlockedDialogBridge {
    private long mNativeUploadBlockedDialogBridge;
    private UploadBlockedDialog mDialog;

    /**
     * Constructor, taking a pointer to the native instance.
     * @param nativeUploadBlockedDialogBridge Pointer to the native object.
     */
    public UploadBlockedDialogBridge(long nativeUploadBlockedDialogBridge) {
        mNativeUploadBlockedDialogBridge = nativeUploadBlockedDialogBridge;
        mDialog = new UploadBlockedDialog();
    }

    @CalledByNative
    public static UploadBlockedDialogBridge create(long nativeDialog) {
        return new UploadBlockedDialogBridge(nativeDialog);
    }

    /**
     * Static method to show upload blocked snackbar
     * @param webContents The WebContents where upload was blocked
     * @param message The message to display
     */
    @CalledByNative
    public static void ShowDialog(WebContents webContents, String message) {
        if (webContents == null) {
            return;
        }

        WindowAndroid windowAndroid = webContents.getTopLevelNativeWindow();
        if (windowAndroid == null) {
            return;
        }

        Activity activity = windowAndroid.getActivity().get();
        if (activity == null) {
            return;
        }

        // Create and show the snackbar dialog
        UploadBlockedDialog dialog = new UploadBlockedDialog();
        dialog.show(activity, message, (result) -> {
            // Snackbar dismissed
        });
    }

    /**
     * Called to show a snackbar for blocked upload.
     * @param windowAndroid Window to show the snackbar.
     * @param url URL where upload was blocked.
     */
    @CalledByNative
    public void showDialog(WindowAndroid windowAndroid, String url) {
        Activity activity = windowAndroid.getActivity().get();
        if (activity == null) {
            return;
        }

        String message = "Upload blocked on " + extractDomain(url) + " by your organization";
        
        mDialog.show(activity, message, (result) -> {
        });
    }

    private String extractDomain(String url) {
        try {
            java.net.URI uri = new java.net.URI(url);
            String domain = uri.getHost();
            return domain != null ? domain : "this site";
        } catch (Exception e) {
            return "this site";
        }
    }

    @CalledByNative
    private void destroy() {
        mNativeUploadBlockedDialogBridge = 0;
    }



}
