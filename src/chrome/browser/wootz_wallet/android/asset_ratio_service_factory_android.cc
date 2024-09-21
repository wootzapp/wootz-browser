/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "base/android/jni_android.h"
#include "chrome/browser/wootz_wallet/asset_ratio_service_factory.h"
#include "chrome/android/chrome_jni_headers/AssetRatioServiceFactory_jni.h"
#include "chrome/browser/profiles/profile.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

namespace chrome {
namespace android {
static jlong JNI_AssetRatioServiceFactory_GetInterfaceToAssetRatioService(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  auto* profile = Profile::FromJavaObject(profile_android);
  auto pending =
      wootz_wallet::AssetRatioServiceFactory::GetInstance()->GetForContext(
          profile);

  return static_cast<jlong>(1);
}

}  // namespace android
}  // namespace chrome
