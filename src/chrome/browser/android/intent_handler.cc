// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_string.h"
#include "base/strings/string_util.h"
#include "chrome/android/chrome_jni_headers/IntentHandler_jni.h"
#include "services/network/public/cpp/cors/cors.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "chromecast/common/cors_exempt_headers.h"
#include "net/http/http_request_headers.h"
#include "url/gurl.h"
#include "base/logging.h"
#include "chrome/browser/ui/webui/startup_crx_install/startup_crx_install_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"

using base::android::JavaParamRef;

namespace chrome {
namespace android {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterStringPref("utm_source", "");
  LOG(INFO) << "Registered utm_source preference in intent_handler";
}
jboolean JNI_IntentHandler_IsCorsSafelistedHeader(
    JNIEnv* env,
    const JavaParamRef<jstring>& j_header_name,
    const JavaParamRef<jstring>& j_header_value) {
  std::string header_name(
      base::android::ConvertJavaStringToUTF8(env, j_header_name));
  std::string header_value(
      base::android::ConvertJavaStringToUTF8(env, j_header_value));

  return network::cors::IsCorsSafelistedHeader(header_name, header_value);
}

void JNI_IntentHandler_StoreUtmSource(JNIEnv* env, const JavaParamRef<jstring>& j_utm_source) {
  std::string utm_source(
      base::android::ConvertJavaStringToUTF8(env, j_utm_source));
  
  // Log the UTM source for debugging
  LOG(INFO) << "Storing UTM source in intent_handler: " << utm_source;
  
  // Get the last used profile
  Profile* profile = ProfileManager::GetLastUsedProfile();
  if (!profile) {
    LOG(ERROR) << "Failed to get profile for storing UTM source";
    return;
  }
  
  // Store the UTM source in user preferences
  PrefService* prefs = profile->GetPrefs();
  if (prefs) {
    // Store in both preference keys for compatibility
    prefs->SetString("utm_source", utm_source);
    
    // Also store in the namespaced preference that startup_crx_install uses
    // Include the startup_crx_install header
    prefs->SetString(startup_crx_install::kUtmSourcePref, utm_source);
    
    LOG(INFO) << "Successfully stored UTM source in both preferences";
  } else {
    LOG(ERROR) << "Failed to get preferences for storing UTM source";
  }
}

static jstring JNI_IntentHandler_GetUtmSource(JNIEnv* env) {
  Profile* profile = ProfileManager::GetLastUsedProfile();
  if (!profile) {
    LOG(ERROR) << "Failed to get profile in GetUtmSource";
    return base::android::ConvertUTF8ToJavaString(env, "").Release();
  }
  PrefService* prefs = profile->GetPrefs();
  if (!prefs) {
    LOG(ERROR) << "Failed to get prefs in GetUtmSource";
    return base::android::ConvertUTF8ToJavaString(env, "").Release();
  }

  std::string utm_source;
  if (prefs->FindPreference(startup_crx_install::kUtmSourcePref)) {
    utm_source = prefs->GetString(startup_crx_install::kUtmSourcePref);
  } else if (prefs->FindPreference("utm_source")) {
    // Fall back to the non-namespaced preference
    utm_source = prefs->GetString("utm_source");
  }
  
  // Also check shared preferences as a fallback
  if (utm_source.empty()) {
    // Try to get from shared preferences via JNI
    // This would require implementing a method to access Android shared prefs from C++
    LOG(INFO) << "UTM source from prefs was empty, could try shared prefs";
  }
  
  LOG(INFO) << "Retrieved UTM source in intent_handler: " << utm_source;
  return base::android::ConvertUTF8ToJavaString(env, utm_source).Release();
}



}  // namespace android
}  // namespace chrome
