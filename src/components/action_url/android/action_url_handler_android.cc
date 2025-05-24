// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/android/action_url_handler_android.h"

#include "base/android/jni_string.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "components/action_url/android/jni_headers/ActionUrlHandler_jni.h"
#include "components/action_url/content/browser/content_action_url_driver.h"
#include "components/action_url/content/browser/content_action_url_driver_factory.h"
#include "content/public/browser/global_routing_id.h"
#include "url/android/gurl_android.h"

using base::android::AttachCurrentThread;
using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaParamRef;
using base::android::JavaRef;
using base::android::ScopedJavaGlobalRef;
using base::android::ScopedJavaLocalRef;
using blink::LocalFrameToken;

namespace action_url {

ActionUrlHandlerAndroid::ActionUrlHandlerAndroid(AnchorData anchor_data) {
  JNIEnv* env = AttachCurrentThread();
  java_obj_ =
      Java_ActionUrlHandler_Constructor(env, reinterpret_cast<int64_t>(this));
  anchor_data_ = anchor_data;
}

ActionUrlHandlerAndroid::~ActionUrlHandlerAndroid() {
  Java_ActionUrlHandler_clearNativePtr(AttachCurrentThread(), java_obj_);
}

void ActionUrlHandlerAndroid::GetActionUrlForBlinkUrl(
    ActionUrlResponseCallback callback) {
  LOG(INFO) << "Unfurling ::" << __func__;
  JNIEnv* env = base::android::AttachCurrentThread();
  auto* callback_ptr = new ActionUrlResponseCallback(std::move(callback));

  Java_ActionUrlHandler_getActionUrlForBlinkUrl(
      env, java_obj_, url::GURLAndroid::FromNativeGURL(env, anchor_data_.url),
      reinterpret_cast<jlong>(callback_ptr));
}

ContentActionUrlDriver* ActionUrlHandlerAndroid::GetDriver() {
  auto* rfh = content::RenderFrameHost::FromFrameToken(
      content::GlobalRenderFrameHostToken(anchor_data_.process_id,
                                          anchor_data_.host_frame));
  ContentActionUrlDriverFactory* factory =
      ContentActionUrlDriverFactory::FromWebContents(
          content::WebContents::FromRenderFrameHost(rfh));
  return factory ? factory->GetDriverForFrame(rfh) : nullptr;
}

static void JNI_ActionUrlHandler_ActionUrlFetched(
    JNIEnv* env,
    const JavaParamRef<jstring>& j_action_url,
    const JavaParamRef<jstring>& j_tag,
    jlong callback_ptr) {
  std::string action_url =
      base::android::ConvertJavaStringToUTF8(env, j_action_url);
  std::string tag =
      base::android::ConvertJavaStringToUTF8(env, j_tag);
  auto* callback =
      reinterpret_cast<action_url::ActionUrlResponseCallback*>(callback_ptr);
  std::move(*callback).Run(action_url, tag);
}

}  // namespace action_url
