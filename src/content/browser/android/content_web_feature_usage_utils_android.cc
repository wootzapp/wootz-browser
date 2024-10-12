// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "content/browser/web_contents/web_contents_android.h"
#include "content/public/android/content_jni_headers/ContentWebFeatureUsageUtils_jni.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_client.h"

namespace content {

static void JNI_ContentWebFeatureUsageUtils_LogWebFeatureForCurrentPage(
    JNIEnv* env,
    const jni_zero::JavaParamRef<jobject>& jweb_contents,
    jint web_feature) {
  WebContents* web_contents = WebContents::FromJavaWebContents(jweb_contents);
  GetContentClient()->browser()->LogWebFeatureForCurrentPage(
      web_contents->GetPrimaryMainFrame(),
      static_cast<blink::mojom::WebFeature>(web_feature));
}

}  // namespace content
