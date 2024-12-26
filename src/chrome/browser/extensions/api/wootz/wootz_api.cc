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
#include "base/android/shared_preferences/shared_preferences_manager.h"
#include "chrome/browser/preferences/android/chrome_shared_preferences.h"
#include "base/json/json_writer.h"
#include "base/json/json_reader.h"
#include "base/json/values_util.h"
#include "base/time/time.h"

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

ExtensionFunction::ResponseAction WootzLogFunction::Run() {
  if (args().size() < 1) {
    return RespondNow(NoArguments());
  }
  LOG(ERROR) << "CONSOLE.LOG: " << args()[0];
  return RespondNow(NoArguments());
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

// background worker

const char kWootzJobsListKey[] = "Chrome.Wootzapp.Jobs";
const char kWootzJobResultsKey[] = "Chrome.Wootzapp.JobsResult";

ExtensionFunction::ResponseAction WootzSetJobFunction::Run() {
  if (!args()[0].GetIfString())
    return RespondNow(Error("URL must be a string"));
  std::string url = *args()[0].GetIfString();

  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  
  std::string jobs_json = prefs.ReadString(kWootzJobsListKey, "[]");
  LOG(ERROR) << "WOOTZ JOBS: " << jobs_json;
  absl::optional<base::Value> parsed = base::JSONReader::Read(jobs_json);
  base::Value::List* jobs = parsed->GetIfList();
  if (!jobs) {
    jobs = new base::Value::List();
  }

  jobs->Append(url);

  std::string new_jobs_json;
  base::JSONWriter::Write(base::Value(std::move(*jobs)), &new_jobs_json);
  prefs.WriteString(kWootzJobsListKey, new_jobs_json);

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WootzRemoveJobFunction::Run() {
  if (!args()[0].GetIfString())
    return RespondNow(Error("URL must be a string"));
  std::string url = *args()[0].GetIfString();

  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  
  std::string jobs_json = prefs.ReadString(kWootzJobsListKey, "[]");

  LOG(ERROR) << "WOOTZ JOBS: " << jobs_json;

  absl::optional<base::Value> parsed = base::JSONReader::Read(jobs_json);
  base::Value::List* jobs = parsed->GetIfList();
  if (!jobs) return RespondNow(NoArguments());

  for (auto it = jobs->begin(); it != jobs->end(); ) {
    if (it->GetIfString() && *it->GetIfString() == url) {
      it = jobs->erase(it);
    } else {
      ++it;
    }
  }

  std::string new_jobs_json;
  base::JSONWriter::Write(base::Value(std::move(*jobs)), &new_jobs_json);
  prefs.WriteString(kWootzJobsListKey, new_jobs_json);

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WootzGetJobsFunction::Run() {
  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  std::string results_json = prefs.ReadString(kWootzJobResultsKey, "[]");
  
  absl::optional<base::Value> parsed = base::JSONReader::Read(results_json);
  if (!parsed || !parsed->is_list()) {
    // Return empty array rather than error
    base::Value::List empty;
    return RespondNow(WithArguments(base::Value(std::move(empty))));
  }

  return RespondNow(WithArguments(std::move(*parsed)));
}

ExtensionFunction::ResponseAction WootzListJobsFunction::Run() {
  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  std::string jobs_json = prefs.ReadString(kWootzJobsListKey, "[]");
  
  LOG(ERROR) << "WOOTZ JOBS LIST JSON: " << jobs_json;

  absl::optional<base::Value> parsed = base::JSONReader::Read(jobs_json);
  if (!parsed || !parsed->is_list()) {
    base::Value::List empty;
    return RespondNow(WithArguments(base::Value(std::move(empty))));
  }

  return RespondNow(WithArguments(std::move(*parsed)));
}

ExtensionFunction::ResponseAction WootzCleanJobsFunction::Run() {
  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  
  // Clear both jobs and results
  prefs.RemoveKey(kWootzJobsListKey);
  prefs.RemoveKey(kWootzJobResultsKey);

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