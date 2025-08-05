// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * UPLOAD BLOCKING DELEGATE PATTERN IMPLEMENTATION
 * 
 * This implementation follows the proper Chromium architecture pattern for 
 * showing UI from content/ module:
 * 
 * 1. DELEGATE INTERFACE (content/public/browser/upload_blocking_delegate.h):
 *    - Defines the interface that embedders must implement
 *    - Pure virtual methods for showing upload blocked notifications
 *    - Platform-agnostic interface
 * 
 * 2. SERVICE LAYER (content/browser/upload/upload_blocking_service.*):
 *    - Business logic for upload blocking
 *    - Holds reference to delegate (set by embedder)
 *    - Calls delegate methods when notification needed
 * 
 * 3. EMBEDDER IMPLEMENTATION (chrome/browser/android/upload_blocking_delegate_android.*):
 *    - Android-specific implementation of the delegate interface
 *    - Uses Android UI components (Toast, custom snackbar, etc.)
 *    - Can access Android-specific APIs like WindowAndroid, Activity
 * 
 * 4. INTEGRATION (content/browser/web_contents/web_contents_impl.cc):
 *    - Calls UploadBlockingService when upload is blocked
 *    - Service delegates to embedder implementation
 *    - No direct Android dependencies in content/
 * 
 * 5. SETUP (chrome/browser/android/chrome_main_delegate_android.cc or similar):
 *    - Embedder creates and sets the delegate instance
 *    - UploadBlockingService::GetInstance()->SetDelegate(delegate)
 * 
 * BENEFITS:
 * - Clean separation of concerns
 * - Content/ doesn't depend on Android UI
 * - Embedders can provide custom UI implementations
 * - Testable (can mock the delegate)
 * - Follows Chromium's layered architecture
 * 
 * ANDROID SNACKBAR IMPLEMENTATION:
 * The Android embedder implementation can:
 * 1. Get WindowAndroid from WebContents
 * 2. Get Activity from WindowAndroid
 * 3. Create custom snackbar using basic Android components:
 *    - LinearLayout with dark background
 *    - TextView for message
 *    - Button for action ("OK")
 *    - Position at bottom of screen
 *    - Auto-dismiss after timeout
 * 
 * This avoids Material Design dependency conflicts while providing
 * the same user experience as native Android snackbars.
 */

#ifndef CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKING_ARCHITECTURE_DOC_H_
#define CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKING_ARCHITECTURE_DOC_H_

// This is a documentation header - no actual code

#endif  // CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKING_ARCHITECTURE_DOC_H_
