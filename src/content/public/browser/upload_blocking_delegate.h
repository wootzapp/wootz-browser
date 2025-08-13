// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_UPLOAD_BLOCKING_DELEGATE_H_
#define CONTENT_PUBLIC_BROWSER_UPLOAD_BLOCKING_DELEGATE_H_

#include <string>

namespace content {

class WebContents;

// Delegate interface for showing upload blocking notifications.
// The embedder (e.g., chrome/) should implement this interface to provide
// platform-specific UI for upload blocking notifications.
class UploadBlockingDelegate {
 public:
  virtual ~UploadBlockingDelegate() = default;

  // Called when an upload is blocked and a notification should be shown.
  // |web_contents| - The WebContents where the upload was blocked
  // |message| - The message to display to the user
  // |url| - The URL where the upload was blocked
  virtual void ShowUploadBlockedNotification(WebContents* web_contents,
                                           const std::string& message,
                                           const std::string& url) = 0;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_UPLOAD_BLOCKING_DELEGATE_H_
