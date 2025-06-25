#ifndef COMPONENTS_WOOTZ_SCRAPING_BROWSER_TWITTER_SCRAPING_BRIDGE_JNI_H_
#define COMPONENTS_WOOTZ_SCRAPING_BROWSER_TWITTER_SCRAPING_BRIDGE_JNI_H_

#include <vector>
#include <jni.h>
#include <string>
#include <atomic>
#include <memory>
#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/task/sequenced_task_runner.h"
#include "base/android/jni_registrar.h"
#include "content/public/browser/browser_context.h"
#include "wootz_scraping_service.h"

class PrefService;

namespace wootz_scraping {

static jlong JNI_TwitterScrapingBridgeJni_Init(JNIEnv* env);

static void JNI_TwitterScrapingBridgeJni_Destroy(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jlong native_bridge);

static void JNI_TwitterScrapingBridgeJni_MakeTwitterApiCall(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& caller,
    jlong native_bridge);

class TwitterScrapingBridgeJni {
 public:
  TwitterScrapingBridgeJni();
  ~TwitterScrapingBridgeJni();

  TwitterScrapingBridgeJni(const TwitterScrapingBridgeJni&) = delete;
  TwitterScrapingBridgeJni& operator=(const TwitterScrapingBridgeJni&) = delete;

  void MakeTwitterApiCall(JNIEnv* env,
                         const base::android::JavaParamRef<jobject>& j_caller);

  void Destroy(JNIEnv* env, const base::android::JavaParamRef<jobject>& j_caller);

  // Notify Java callback
  void NotifyJavaCallback(const std::string& result, bool success);

  void SendCredentialsToJava(const std::vector<std::string>& stored_credentials);

  void MakeTwitterApiCallDirect();

 private: 

  void InternalCleanup();
  
  // Java reference for callbacks
  base::android::ScopedJavaGlobalRef<jobject> java_caller_;
  
  // State management
  std::atomic<bool> is_destroyed_{false};
  
  base::WeakPtrFactory<TwitterScrapingBridgeJni> weak_ptr_factory_{this};
};

// Registration function
bool RegisterTwitterScrapingBridgeJni(JNIEnv* env);

}  // namespace wootz_scraping

#endif