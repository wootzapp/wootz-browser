// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKED_DIALOG_BRIDGE_H_
#define CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKED_DIALOG_BRIDGE_H_

#include <string>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

namespace ui {
class WindowAndroid;
}

namespace content {

class WebContents;

// Class for showing dialogs to notify user that upload has been blocked.
class UploadBlockedDialogBridge {
 public:
  UploadBlockedDialogBridge();
  UploadBlockedDialogBridge(const UploadBlockedDialogBridge&) = delete;
  UploadBlockedDialogBridge& operator=(const UploadBlockedDialogBridge&) =
      delete;

  ~UploadBlockedDialogBridge();

  // Called to create and show a dialog for a blocked upload.
  // void Show(const std::string& url, ui::WindowAndroid* window_android);

  // Static method to show dialog through JNI
  static void ShowDialog(WebContents* web_contents, const std::string& message);

  // Called from Java via JNI when dialog is dismissed.
  // void OnDialogDismissed(JNIEnv* env,
  //                       const base::android::JavaParamRef<jobject>& jcaller);

 private:
  // The corresponding java object.
  base::android::ScopedJavaGlobalRef<jobject> java_object_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKED_DIALOG_BRIDGE_H_
