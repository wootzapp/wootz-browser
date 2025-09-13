// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.extensions;

import android.app.Activity;
import android.graphics.Bitmap;
import android.util.Log;
import android.util.Base64;
import android.view.View;
import android.view.ViewGroup;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.base.ApplicationStatus;
import org.chromium.chrome.browser.feedback.ScreenshotTask;

import java.io.ByteArrayOutputStream;

/**
 * Java implementation of screenshot functionality for Wootz extensions.
 * Captures only the visible viewport for efficiency.
 */
public class WootzScreenshotApi {
    private static final String TAG = "WootzScreenshotApi";
    private static final int MAX_VIEWPORT_DIMENSION = 1080; // Reduced to prevent large base64
    private static final int JPEG_QUALITY = 80; // Reduced quality to prevent large base64

    /**
     * Called from C++ to trigger screenshot capture.
     */
    @CalledByNative
    public static void captureScreenshot() {
        try {
            Activity activity = ApplicationStatus.getLastTrackedFocusedActivity();
            if (activity == null) {
                Log.e(TAG, "No focused activity found");
                WootzScreenshotApiJni.get().onScreenshotError("No focused activity found");
                return;
            }

            // Check if activity is still valid before proceeding
            if (activity.isFinishing() || activity.isDestroyed()) {
                Log.e(TAG, "Activity not valid - finishing: " + activity.isFinishing() + ", destroyed: " + activity.isDestroyed());
                WootzScreenshotApiJni.get().onScreenshotError("Activity not valid");
                return;
            }

            // Create a ScreenshotTask with COMPOSITOR mode to get the actual web content
            ScreenshotTask screenshotTask = new ScreenshotTask(activity, 1); // COMPOSITOR mode = 1

            // Capture the screenshot with a timeout to avoid hanging
            screenshotTask.capture(new Runnable() {
                @Override
                public void run() {
                    try {

                        if (!screenshotTask.isReady()) {
                            Log.e(TAG, "ScreenshotTask not ready");
                            WootzScreenshotApiJni.get().onScreenshotError("ScreenshotTask not ready");
                            return;
                        }

                        Bitmap bitmap = screenshotTask.getScreenshot();
                        if (bitmap != null) {
                            // Scale down if too large to reduce memory usage
                            Bitmap scaledBitmap = scaleBitmapIfNeeded(bitmap);
                            if (scaledBitmap != bitmap) {
                                bitmap = scaledBitmap;
                            }

                            // Convert to base64 with compression
                            String base64Data = convertBitmapToBase64Compressed(bitmap);
                            WootzScreenshotApiJni.get().onScreenshotComplete(base64Data);
                            return;
                        } else {
                            Log.e(TAG, "ScreenshotTask returned null bitmap");
                            WootzScreenshotApiJni.get().onScreenshotError("ScreenshotTask returned null bitmap");
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "Error in ScreenshotTask callback", e);
                        WootzScreenshotApiJni.get().onScreenshotError("Error in ScreenshotTask: " + e.getMessage());
                    }
                }
            });

        } catch (Exception e) {
            Log.e(TAG, "Error creating ScreenshotTask", e);
            WootzScreenshotApiJni.get().onScreenshotError("Error creating ScreenshotTask: " + e.getMessage());
        }
    }

    /**
     * Scales down bitmap if it exceeds maximum dimensions.
     */
    private static Bitmap scaleBitmapIfNeeded(Bitmap original) {
        int width = original.getWidth();
        int height = original.getHeight();

        if (width <= MAX_VIEWPORT_DIMENSION && height <= MAX_VIEWPORT_DIMENSION) {
            return original; // No scaling needed
        }

        // Calculate new dimensions maintaining aspect ratio
        float scale = Math.min(
            (float) MAX_VIEWPORT_DIMENSION / width,
            (float) MAX_VIEWPORT_DIMENSION / height
        );

        int newWidth = Math.round(width * scale);
        int newHeight = Math.round(height * scale);

        return Bitmap.createScaledBitmap(original, newWidth, newHeight, true);
    }

    /**
     * Converts a Bitmap to base64-encoded JPEG string with compression.
     * @param bitmap The bitmap to convert
     * @return Base64-encoded JPEG string with data URL prefix
     */
    private static String convertBitmapToBase64Compressed(Bitmap bitmap) {
        try {
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();

            // Use JPEG compression instead of PNG for smaller size
            boolean compressed = bitmap.compress(Bitmap.CompressFormat.JPEG, JPEG_QUALITY, byteArrayOutputStream);

            if (!compressed) {
                Log.e(TAG, "Failed to compress bitmap");
                throw new RuntimeException("Failed to compress bitmap");
            }
            byte[] byteArray = byteArrayOutputStream.toByteArray();

            String base64 = Base64.encodeToString(byteArray, Base64.NO_WRAP);
            
            return "data:image/jpeg;base64," + base64;
        } catch (Exception e) {
            Log.e(TAG, "Error in convertBitmapToBase64Compressed", e);
            throw e;
        }
    }

    @NativeMethods
    interface Natives {
        void onScreenshotComplete(String base64Data);
        void onScreenshotError(String error);
    }
}