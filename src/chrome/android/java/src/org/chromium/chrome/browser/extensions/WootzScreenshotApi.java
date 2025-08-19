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
@JNINamespace("chrome::android")
public class WootzScreenshotApi {
    private static final String TAG = "WootzScreenshotApi";
    private static final int MAX_VIEWPORT_DIMENSION = 1000; // Reduced to prevent large base64
    private static final int JPEG_QUALITY = 80; // Reduced quality to prevent large base64

    /**
     * Called from C++ to trigger screenshot capture.
     * @param nativePtr Pointer to the C++ function object
     */
    @CalledByNative
    public static void captureScreenshot(long nativePtr) {
        Log.i(TAG, "captureScreenshot called with nativePtr: " + nativePtr);
        
        try {
            Activity activity = ApplicationStatus.getLastTrackedFocusedActivity();
            if (activity == null) {
                Log.e(TAG, "No focused activity found");
                onScreenshotError(nativePtr, "No focused activity found");
                return;
            }
            
            // Check if activity is still valid before proceeding
            if (activity.isFinishing() || activity.isDestroyed()) {
                Log.e(TAG, "Activity not valid - finishing: " + activity.isFinishing() + ", destroyed: " + activity.isDestroyed());
                onScreenshotError(nativePtr, "Activity not valid");
                return;
            }
            
            Log.i(TAG, "Got activity: " + activity.getClass().getSimpleName());
            
            // Use ScreenshotTask with COMPOSITOR mode to get the actual web content
            Log.i(TAG, "Using ScreenshotTask with COMPOSITOR mode for web content");
            
            // Create a ScreenshotTask with COMPOSITOR mode to get the actual web content
            ScreenshotTask screenshotTask = new ScreenshotTask(activity, 1); // COMPOSITOR mode = 1
            
            // Capture the screenshot with a timeout to avoid hanging
            screenshotTask.capture(new Runnable() {
                @Override
                public void run() {
                    try {
                        Log.i(TAG, "ScreenshotTask callback executed");
                        
                        if (!screenshotTask.isReady()) {
                            Log.e(TAG, "ScreenshotTask not ready");
                            onScreenshotError(nativePtr, "ScreenshotTask not ready");
                            return;
                        }
                        
                        Bitmap bitmap = screenshotTask.getScreenshot();
                        if (bitmap != null) {
                            Log.i(TAG, "ScreenshotTask captured bitmap: " + bitmap.getWidth() + "x" + bitmap.getHeight());
                            
                            // Log some pixel data to verify it's not black
                            int[] pixels = new int[Math.min(bitmap.getWidth(), 10) * Math.min(bitmap.getHeight(), 10)];
                            bitmap.getPixels(pixels, 0, Math.min(bitmap.getWidth(), 10), 0, 0, 
                                           Math.min(bitmap.getWidth(), 10), Math.min(bitmap.getHeight(), 10));
                            Log.i(TAG, "First few pixels: " + java.util.Arrays.toString(java.util.Arrays.copyOf(pixels, Math.min(pixels.length, 5))));
                            
                            // Scale down if too large to reduce memory usage
                            Bitmap scaledBitmap = scaleBitmapIfNeeded(bitmap);
                            if (scaledBitmap != bitmap) {
                                // Do not recycle the original; ScreenshotTask may still reference it
                                // bitmap.recycle(); // Removed to avoid use-after-free
                                bitmap = scaledBitmap;
                                Log.i(TAG, "Scaled ScreenshotTask bitmap to: " + bitmap.getWidth() + "x" + bitmap.getHeight());
                            }
                            
                            // Convert to base64 with compression
                            String base64Data = convertBitmapToBase64Compressed(bitmap);
                            Log.i(TAG, "ScreenshotTask converted to base64, length: " + base64Data.length());

                            // Send base64 data directly through JNI
                            Log.i(TAG, "Sending base64 data directly through JNI");
                            onScreenshotComplete(nativePtr, base64Data);
                            return;
                        } else {
                            Log.e(TAG, "ScreenshotTask returned null bitmap");
                            onScreenshotError(nativePtr, "ScreenshotTask returned null bitmap");
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "Error in ScreenshotTask callback", e);
                        onScreenshotError(nativePtr, "Error in ScreenshotTask: " + e.getMessage());
                    }
                }
            });
            
        } catch (Exception e) {
            Log.e(TAG, "Error creating ScreenshotTask", e);
            onScreenshotError(nativePtr, "Error creating ScreenshotTask: " + e.getMessage());
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
        
        Log.i(TAG, "Scaling bitmap from " + width + "x" + height + " to " + newWidth + "x" + newHeight);
        
        return Bitmap.createScaledBitmap(original, newWidth, newHeight, true);
    }

    /**
     * Converts a Bitmap to base64-encoded JPEG string with compression.
     * @param bitmap The bitmap to convert
     * @return Base64-encoded JPEG string with data URL prefix
     */
    private static String convertBitmapToBase64Compressed(Bitmap bitmap) {
        Log.i(TAG, "Starting bitmap to base64 conversion with compression");
        
        try {
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
            
            // Use JPEG compression instead of PNG for smaller size
            boolean compressed = bitmap.compress(Bitmap.CompressFormat.JPEG, JPEG_QUALITY, byteArrayOutputStream);
            
            if (!compressed) {
                Log.e(TAG, "Failed to compress bitmap");
                throw new RuntimeException("Failed to compress bitmap");
            }
            
            byte[] byteArray = byteArrayOutputStream.toByteArray();
            Log.i(TAG, "Compressed bitmap size: " + byteArray.length + " bytes");
            
            String base64 = Base64.encodeToString(byteArray, Base64.NO_WRAP);
            Log.i(TAG, "Base64 encoding completed, length: " + base64.length());
            
            return "data:image/jpeg;base64," + base64;
        } catch (Exception e) {
            Log.e(TAG, "Error in convertBitmapToBase64Compressed", e);
            throw e;
        }
    }

    private static native void onScreenshotComplete(long nativePtr, String base64Data);
    private static native void onScreenshotError(long nativePtr, String error);
}
