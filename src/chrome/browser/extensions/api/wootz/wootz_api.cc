// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/wootz/wootz_api.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "base/android/build_info.h"
#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/functional/bind.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"
// #include "chrome/android/chrome_jni_headers/WootzBridge_jni.h"
#include "chrome/browser/profiles/profile.h"
#include "components/search_engines/template_url_service.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/common/extension_id.h"
#include "ui/gfx/image/image.h"

namespace extensions {

ExtensionFunction::ResponseAction WootzInfoFunction::Run() {
  const base::android::BuildInfo* build_info =
      base::android::BuildInfo::GetInstance();

  // Create a dictionary to store the relevant build information
  base::Value::Dict build_info_dict;
  build_info_dict.Set("device", build_info->device());
  build_info_dict.Set("manufacturer", build_info->manufacturer());
  build_info_dict.Set("model", build_info->model());
  build_info_dict.Set("brand", build_info->brand());
  build_info_dict.Set("sdk_int", build_info->sdk_int());
  build_info_dict.Set("android_build_id", build_info->android_build_id());
  build_info_dict.Set("android_build_fp", build_info->android_build_fp());
  build_info_dict.Set("gms_version_code", build_info->gms_version_code());
  build_info_dict.Set("host_package_name", build_info->host_package_name());
  build_info_dict.Set("package_name", build_info->package_name());
  build_info_dict.Set("package_version_code",
                      build_info->package_version_code());
  build_info_dict.Set("package_version_name",
                      build_info->package_version_name());
  build_info_dict.Set("abi_name", build_info->abi_name());
  build_info_dict.Set("is_tv", build_info->is_tv());
  build_info_dict.Set("is_automotive", build_info->is_automotive());
  build_info_dict.Set("is_foldable", build_info->is_foldable());

  std::string json_string;
  base::JSONWriter::Write(build_info_dict, &json_string);

  return RespondNow(WithArguments(json_string));
}

ExtensionFunction::ResponseAction WootzHelloWorldFunction::Run() {
  base::Value::Dict result;
  result.Set("message", "Hello, World!");

  std::string json_string;
  base::JSONWriter::Write(result, &json_string);

  return RespondNow(WithArguments(json_string));
}

[[maybe_unused]]
ExtensionFunction::ResponseAction WootzShowDialogFunction::Run() {
#if 0
    JNIEnv* env = base::android::AttachCurrentThread();
    
    content::WebContents* web_contents = GetSenderWebContents();
    if (!web_contents) {
        return RespondNow(Error("Unable to get WebContents"));
    }

    Java_WootzBridge_showDialog(env, web_contents->GetJavaWebContents());
#endif
    return RespondNow(NoArguments());
}

// ExtensionFunction::ResponseAction
// WootzShowConsentDialogAndMaybeStartServiceFunction::Run() {
//   JNIEnv* env = base::android::AttachCurrentThread();

//     content::WebContents* web_contents = GetSenderWebContents();
//     if (!web_contents) {
//         return RespondNow(Error("Unable to get WebContents"));
//     }

//     // First, check if we already have consent
//     if (!Java_WootzBridge_hasUserConsent(env)) {
//         // If not, show the consent dialog
//         // auto* callback_ptr = new base::OnceCallback<void(bool)>(
//         //     base::BindOnce(&WootzShowConsentDialogAndMaybeStartServiceFunction::OnConsentResult,
//         //                    this));

//         Java_WootzBridge_showConsentDialog(env, reinterpret_cast<jlong>(this),
//                                            web_contents->GetJavaWebContents());
//         return RespondLater();
//     } else {
//         // If we already have consent, start the service directly
//         Java_WootzBridge_startBrowsingDataService(env);
//         return RespondNow(NoArguments());
//     }
// //   base::Value::Dict result;
// //   result.Set("message", "Consent dialog shown and service started");

// //   std::string json_string;
// //   base::JSONWriter::Write(result, &json_string);

// //   return RespondNow(WithArguments(json_string));
// }
// void WootzShowConsentDialogAndMaybeStartServiceFunction::OnConsentDialogResult(JNIEnv* env, jboolean consented) {
//     if (consented) {
//         Java_WootzBridge_startBrowsingDataService(env);
//     }
//     Respond(NoArguments());
// }

}  // namespace extensions

// extern "C" JNIEXPORT void JNICALL
// Java_org_chromium_chrome_browser_extensions_WootzBridge_nativeOnConsentDialogResult(
//     JNIEnv* env,
//     jclass clazz,
//     jlong native_ptr,
//     jboolean consented) {
//   auto* function = reinterpret_cast<extensions::WootzShowConsentDialogAndMaybeStartServiceFunction*>(native_ptr);
//   function->OnConsentDialogResult(env, consented);
// }