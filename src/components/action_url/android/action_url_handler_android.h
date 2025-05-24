// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_ANDROID_ACTION_URL_HANDLER_ANDROID_H_
#define COMPONENTS_ACTION_URL_ANDROID_ACTION_URL_HANDLER_ANDROID_H_

#include <memory>
#include <string>

#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/action_url/core/anchor_data.h"
#include "content/public/browser/render_frame_host.h"
#include "third_party/blink/public/common/tokens/tokens.h"
#include "url/gurl.h"

using base::android::JavaRef;
using base::android::ScopedJavaGlobalRef;
using base::android::ScopedJavaLocalRef;
using blink::LocalFrameToken;

namespace content {
class RenderFrameHost;
class WebContents;
}  // namespace content

namespace action_url {

using base::android::JavaParamRef;
using ActionUrlResponseCallback = base::OnceCallback<void(std::string, std::string)>;

class ContentActionUrlDriver;
class ContentActionUrlDriverFactory;

// Helper class responsible for bridging the C++ and Java.
class ActionUrlHandlerAndroid {
 public:
  explicit ActionUrlHandlerAndroid(AnchorData anchor_data);
  ~ActionUrlHandlerAndroid();

  void GetActionUrlForBlinkUrl(ActionUrlResponseCallback callback);

 private:
  ContentActionUrlDriver* GetDriver();
  // A reference to the Java counterpart of this class.
  ScopedJavaGlobalRef<jobject> java_obj_;

  AnchorData anchor_data_;
};

}  // namespace action_url

#endif  // COMPONENTS_ACTION_URL_ANDROID_ACTION_URL_HANDLER_ANDROID_H_
