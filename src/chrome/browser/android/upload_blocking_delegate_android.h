// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_UPLOAD_BLOCKING_DELEGATE_ANDROID_H_
#define CHROME_BROWSER_ANDROID_UPLOAD_BLOCKING_DELEGATE_ANDROID_H_

#include <string>

#include "content/public/browser/upload_blocking_delegate.h"

namespace content {
class WebContents;
}

namespace chrome {
namespace android {

// Android-specific implementation of UploadBlockingDelegate that shows
// native Android notifications when uploads are blocked.
class UploadBlockingDelegateAndroid : public content::UploadBlockingDelegate {
 public:
  UploadBlockingDelegateAndroid();
  ~UploadBlockingDelegateAndroid() override;

  UploadBlockingDelegateAndroid(const UploadBlockingDelegateAndroid&) = delete;
  UploadBlockingDelegateAndroid& operator=(const UploadBlockingDelegateAndroid&) = delete;

  // content::UploadBlockingDelegate:
  void ShowUploadBlockedNotification(content::WebContents* web_contents,
                                   const std::string& message,
                                   const std::string& url) override;

 private:
  // Shows a custom snackbar-like notification using basic Android components
  void ShowCustomSnackbar(content::WebContents* web_contents, const std::string& message);
};

}  // namespace android
}  // namespace chrome

#endif  // CHROME_BROWSER_ANDROID_UPLOAD_BLOCKING_DELEGATE_ANDROID_H_
