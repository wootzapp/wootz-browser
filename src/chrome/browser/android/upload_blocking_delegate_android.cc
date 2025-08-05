// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/upload_blocking_delegate_android.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/logging.h"
#include "content/public/browser/web_contents.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaLocalRef;

namespace chrome {
namespace android {

UploadBlockingDelegateAndroid::UploadBlockingDelegateAndroid() = default;

UploadBlockingDelegateAndroid::~UploadBlockingDelegateAndroid() = default;

void UploadBlockingDelegateAndroid::ShowUploadBlockedNotification(
    content::WebContents* web_contents,
    const std::string& message,
    const std::string& url) {
  LOG(INFO) << "UploadBlockingDelegateAndroid: Showing notification for URL: "
            << url;
  LOG(INFO) << "UploadBlockingDelegateAndroid: Message: " << message;

  if (!web_contents) {
    LOG(ERROR) << "UploadBlockingDelegateAndroid: No WebContents available";
    return;
  }

  ShowCustomSnackbar(web_contents, message);
}

void UploadBlockingDelegateAndroid::ShowCustomSnackbar(
    content::WebContents* web_contents,
    const std::string& message) {
  LOG(INFO) << "UploadBlockingDelegateAndroid: Creating custom notification";

  // Get the Java WebContents
  JNIEnv* env = AttachCurrentThread();
  if (!env) {
    LOG(ERROR)
        << "UploadBlockingDelegateAndroid: Failed to attach to JNI thread";
    return;
  }

  // For now, just show a simple Android Toast
  // In a real implementation, you would:
  // 1. Get the WindowAndroid from WebContents
  // 2. Get the Activity from WindowAndroid
  // 3. Create and show a custom snackbar using Android UI components

  ScopedJavaLocalRef<jstring> java_message =
      ConvertUTF8ToJavaString(env, message);

  // This would call a JNI method to show the toast/snackbar
  // Java_ChromeUploadBlockingHelper_showNotification(env, java_message);

  LOG(INFO) << "UploadBlockingDelegateAndroid: Notification request completed";
}

}  // namespace android
}  // namespace chrome
