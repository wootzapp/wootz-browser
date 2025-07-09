#include "components/wootz_scraping/browser/twitter_scraping_bridge_jni.h"
#include "components/wootz_scraping/browser/wootz_scraping_service.h"

#include <utility>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/task/thread_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/wootz_scraping/wootz_scraping_service_factory.h"
#include "components/wootz_scraping/browser/pref_names.h"
#include "components/prefs/pref_service.h"

//generated JNI header
#include "chrome/android/java/src/org/chromium/chrome/browser/background/twitter_scraping_bridge_jni_jni/TwitterScrapingBridgeJni_jni.h"

namespace wootz_scraping {

namespace {

const JNINativeMethod kTwitterScrapingBridgeJniMethods[] = {
    {"init", "()J",
     reinterpret_cast<void*>(&JNI_TwitterScrapingBridgeJni_Init)},
    {"destroy",
     "(JLorg/chromium/chrome/browser/background/TwitterScrapingBridgeJni;)V",
     reinterpret_cast<void*>(&JNI_TwitterScrapingBridgeJni_Destroy)},
    {"makeTwitterApiCall",
     "(JLorg/chromium/chrome/browser/background/TwitterScrapingBridgeJni;)V",
     reinterpret_cast<void*>(&JNI_TwitterScrapingBridgeJni_MakeTwitterApiCall)}
};

}  // namespace


TwitterScrapingBridgeJni::TwitterScrapingBridgeJni() 
    : is_destroyed_(false) {
  LOG(INFO) << "TwitterScrapingBridgeJni: Constructor called";
}

TwitterScrapingBridgeJni::~TwitterScrapingBridgeJni() {
  LOG(INFO) << "TwitterScrapingBridgeJni: Destructor called";

  // Ensure cleanup if not already done
  if (!is_destroyed_) {
    InternalCleanup();
  }
}

void TwitterScrapingBridgeJni::MakeTwitterApiCallDirect() {
  LOG(INFO) << "TwitterScrapingBridgeJni: Storing credentials for Twitter API call";

  std::vector<std::string> stored_credentials;

  LOG(INFO) << "Ram -> 🔍 Checking g_browser_process...";

  if (!g_browser_process) {
    LOG(ERROR) << "Ram -> ❌ g_browser_process is null";
    NotifyJavaCallback("g_browser_process is null", false);
    return;
  }

  LOG(INFO) << "Ram -> 🔍 Checking ProfileManager...";

  ProfileManager* profile_manager = g_browser_process->profile_manager();
  if (!profile_manager) {
    LOG(ERROR) << "Ram -> ❌ ProfileManager not available";
    NotifyJavaCallback("ProfileManager not available", false);
    return;
  }

  LOG(INFO) << "Ram -> 🔍 Checking Profile...";

  // 2. Defensive: Get a valid profile
  Profile* profile = profile_manager->GetLastUsedProfile();
  if (!profile) {
    profile = profile_manager->GetPrimaryUserProfile();
  }
  if (!profile) {
    LOG(ERROR) << "Ram -> ❌ No profile available for credential retrieval";
    NotifyJavaCallback("No profile available for credential retrieval", false);
    return;
  }

  LOG(INFO) << "Ram -> 🔍 Checking PrefService...";

  // 3. Defensive: Get PrefService
  PrefService* profile_prefs = profile->GetPrefs();
  if (!profile_prefs) {
    LOG(ERROR) << "Ram -> ❌ PrefService not available for credential retrieval";
    NotifyJavaCallback("PrefService not available for credential retrieval", false);
    return;
  }

  LOG(INFO) << "Ram -> 🔍 PrefService pointer: " << profile_prefs;
  LOG(INFO) << "Ram -> 🔍 Preference key: " << kWootzScrapingTwitterAPICredentials;
  LOG(INFO) << "Ram -> 🔍 Profile pointer: " << profile;
  LOG(INFO) << "Ram -> 🔍 ProfileManager pointer: " << profile_manager;
  LOG(INFO) << "Ram -> 🔍 g_browser_process pointer: " << g_browser_process;

  LOG(INFO) << "Ram -> 📂 Retrieving stored credentials from Chrome Storage...";
  const base::Value::Dict& credentials_dict = 
      profile_prefs->GetDict(kWootzScrapingTwitterAPICredentials);

  if (credentials_dict.empty()) {
    LOG(ERROR) << "Ram -> ❌ No stored Twitter API credentials found in Chrome Storage";
    LOG(ERROR) << "Ram -> Cannot make API call without stored credentials";
    NotifyJavaCallback("No stored Twitter API credentials found in Chrome Storage", false);
    return;
  }
  
  // Step 2: Extract credentials from storage
  const std::string* url = credentials_dict.FindString("url");
  const std::string* method = credentials_dict.FindString("method");
  const std::string* headers = credentials_dict.FindString("headers");
  const std::string* body = credentials_dict.FindString("body");
  std::optional<double> stored_time_double = credentials_dict.FindDouble("stored_time");
  std::optional<bool> is_valid = credentials_dict.FindBool("is_valid");
  
  // Step 3: Validate all required fields
  if (!url || !method || !headers || !body || !stored_time_double || !is_valid) {
    LOG(ERROR) << "Ram -> ❌ Missing required credential fields in storage";
    LOG(ERROR) << "  - url: " << (url ? "present" : "missing");
    LOG(ERROR) << "  - method: " << (method ? "present" : "missing");
    LOG(ERROR) << "  - headers: " << (headers ? "present" : "missing");
    LOG(ERROR) << "  - body: " << (body ? "present" : "missing");
    LOG(ERROR) << "  - stored_time: " << (stored_time_double ? "present" : "missing");
    LOG(ERROR) << "  - is_valid: " << (is_valid ? "present" : "missing");
    return;
  }
  
  if (!*is_valid) {
    LOG(ERROR) << "Ram -> ❌ Stored credentials marked as invalid";
    NotifyJavaCallback("Stored credentials marked as invalid", false);
    return;
  }
  
  // Step 4: Log the retrieved credentials
  LOG(INFO) << "Ram -> ✅ SUCCESSFULLY RETRIEVED CREDENTIALS FROM CHROME STORAGE:";
  LOG(INFO) << "  🔗 URL: " << url->substr(0, 150) << "..." << " (Total: " << url->length() << " chars)";
  LOG(INFO) << "  🔧 Method: " << *method;
  LOG(INFO) << "  📋 Headers: " << headers->substr(0, 200) << "..." << " (Total: " << headers->length() << " chars)";
  LOG(INFO) << "  📦 Body Length: " << body->length() << " characters";
  LOG(INFO) << "  ⏰ Stored Time: " << *stored_time_double;
  LOG(INFO) << "  ✅ Valid: " << (*is_valid ? "YES" : "NO");
  
  // Step 5: Verify authentication tokens are present
  if (headers->length() > 0) {
    LOG(INFO) << "Ram -> 🔑 Authentication Token Verification:";
    if (headers->find("authorization") != std::string::npos || 
        headers->find("Authorization") != std::string::npos) {
      LOG(INFO) << "  ✅ Authorization header FOUND in stored credentials";
    }
    if (headers->find("x-csrf-token") != std::string::npos) {
      LOG(INFO) << "  ✅ CSRF token FOUND in stored credentials";
    }
    if (headers->find("cookie") != std::string::npos || 
        headers->find("Cookie") != std::string::npos) {
      LOG(INFO) << "  ✅ Cookie header FOUND in stored credentials";
    }
  }
  
  // Step 6: Check credential age
  base::Time stored_time = base::Time::FromSecondsSinceUnixEpoch(*stored_time_double);
  auto time_diff = base::Time::Now() - stored_time;
  LOG(INFO) << "Ram -> ⏰ Credential Age: " << time_diff.InMinutes() << " minutes";
  
  if (time_diff.InHours() > 24) {
    LOG(WARNING) << "Ram -> ⚠️ Credentials are over 24 hours old, may be expired";
  }
  
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "🎯 Making API call: URL(%zu chars), Headers(%zu chars), Method(%s)",
                     url->length(), headers->length(), method->c_str());
  #endif

  stored_credentials.push_back(*url);
  stored_credentials.push_back(*method);
  stored_credentials.push_back(*headers);
  stored_credentials.push_back(*body);

  SendCredentialsToJava(stored_credentials);

  NotifyJavaCallback("Twitter API call executed successfully", true);
}

void TwitterScrapingBridgeJni::MakeTwitterApiCall(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_caller) {
  
  if (is_destroyed_) {
    LOG(ERROR) << "TwitterScrapingBridgeJni: Called after destruction";
    return;
  }
  
  // Store Java caller reference for callbacks
  java_caller_.Reset(env, j_caller);

  this->MakeTwitterApiCallDirect();
}


void TwitterScrapingBridgeJni::NotifyJavaCallback(const std::string& result,
                                                 bool success) {
  if (is_destroyed_) {
    LOG(ERROR) << "TwitterScrapingBridgeJni: NotifyJavaCallback called after destruction";
    return;
  }

  LOG(INFO) << "TwitterScrapingBridgeJni: Notifying Java callback - Success: " << success;
  LOG(INFO) << "TwitterScrapingBridgeJni: Result: " << result;

  JNIEnv* env = base::android::AttachCurrentThread();
  if (!env) {
    LOG(ERROR) << "TwitterScrapingBridgeJni: Failed to attach to current thread";
    return;
  }

  // Use the stored Java caller reference
  if (!java_caller_.is_null()) {
    Java_TwitterScrapingBridgeJni_onNativeCallback(
        env,
        java_caller_,
        base::android::ConvertUTF8ToJavaString(env, result),
        success);
  } else {
    LOG(ERROR) << "TwitterScrapingBridgeJni: Java caller reference is null";
  }
}

void TwitterScrapingBridgeJni::SendCredentialsToJava(const std::vector<std::string>& stored_credentials) {
  JNIEnv* env = base::android::AttachCurrentThread();
  if (!env) {
    LOG(ERROR) << "Ram -> TwitterScrapingBridgeJni: Failed to attach to current thread";
    return;
  }

  // Convert std::vector<std::string> to Java String[]
  jobjectArray j_credentials = env->NewObjectArray(
      stored_credentials.size(),
      env->FindClass("java/lang/String"),
      nullptr);

  for (size_t i = 0; i < stored_credentials.size(); ++i) {
    jstring j_str = env->NewStringUTF(stored_credentials[i].c_str());
    env->SetObjectArrayElement(j_credentials, i, j_str);
    env->DeleteLocalRef(j_str);
  }

  // Call the Java method
  if (!java_caller_.is_null()) {
    jclass cls = env->GetObjectClass(java_caller_.obj());
    jmethodID method = env->GetMethodID(cls, "onReceiveCredentials", "([Ljava/lang/String;)V");
    if (method) {
      env->CallVoidMethod(java_caller_.obj(), method, j_credentials);
    } else {
      LOG(ERROR) << "Ram -> TwitterScrapingBridgeJni: Failed to find onReceiveCredentials method";
    }
    env->DeleteLocalRef(cls);
  } else {
    LOG(ERROR) << "Ram -> TwitterScrapingBridgeJni: Java caller reference is null";
  }

  env->DeleteLocalRef(j_credentials);
}

void TwitterScrapingBridgeJni::Destroy(JNIEnv* env, 
                                      const base::android::JavaParamRef<jobject>& j_caller) {
  LOG(INFO) << "TwitterScrapingBridgeJni: Destroy called";
  InternalCleanup();
}

void TwitterScrapingBridgeJni::InternalCleanup() {
  if (is_destroyed_) {
    return;  // Already cleaned up
  }
  
  LOG(INFO) << "TwitterScrapingBridgeJni: Performing internal cleanup";
  
  is_destroyed_ = true;
  
  // Clear references
  java_caller_.Reset();
  
  // Invalidate all weak pointers to prevent callbacks
  weak_ptr_factory_.InvalidateWeakPtrs();
}

// Register native methods
bool RegisterNativeMethods(JNIEnv* env) {
    const char kClassName[] =
        "org/chromium/chrome/browser/background/TwitterScrapingBridgeJni";
    
    jclass clazz = env->FindClass(kClassName);
    if (!clazz) {
        LOG(ERROR) << "Failed to find TwitterScrapingBridgeJni class";
        return false;
    }

    if (env->RegisterNatives(clazz, kTwitterScrapingBridgeJniMethods,
                           std::size(kTwitterScrapingBridgeJniMethods)) < 0) {
        LOG(ERROR) << "Failed to register native methods for TwitterScrapingBridgeJni";
        return false;
    }

    return true;
}

// Implementation of JNI methods
static jlong JNI_TwitterScrapingBridgeJni_Init(JNIEnv* env) {
    LOG(INFO) << "TwitterScrapingBridgeJni: JNI Init called";
    return reinterpret_cast<jlong>(new TwitterScrapingBridgeJni());
}

static void JNI_TwitterScrapingBridgeJni_Destroy(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jlong native_bridge) {
    TwitterScrapingBridgeJni* bridge =
        reinterpret_cast<TwitterScrapingBridgeJni*>(native_bridge);
    if (bridge) {
        bridge->Destroy(env, caller);
        delete bridge;
    }
}

static void JNI_TwitterScrapingBridgeJni_MakeTwitterApiCall(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jlong native_bridge) {
    TwitterScrapingBridgeJni* bridge =
        reinterpret_cast<TwitterScrapingBridgeJni*>(native_bridge);
    if (bridge) {
        bridge->MakeTwitterApiCall(env, caller);
    }
}

// Register JNI for this class
bool RegisterTwitterScrapingBridgeJni(JNIEnv* env) {
    return RegisterNativeMethods(env);
}

}  // namespace wootz_scraping