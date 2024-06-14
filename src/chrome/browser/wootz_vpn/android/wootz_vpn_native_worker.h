#include <jni.h>
#include <string>

#include "base/android/jni_weak_ref.h"
#include "base/memory/weak_ptr.h"

namespace chrome {
namespace android {
class WootzVpnNativeWorker {
 public:
  WootzVpnNativeWorker(JNIEnv* env, const base::android::JavaRef<jobject>& obj);
  ~WootzVpnNativeWorker();

  WootzVpnNativeWorker(const WootzVpnNativeWorker&) = delete;
  WootzVpnNativeWorker& operator=(const WootzVpnNativeWorker&) = delete;

  void Destroy(JNIEnv* env,
               const base::android::JavaParamRef<jobject>& jcaller);

  void GetAllServerRegions(JNIEnv* env);

  void OnGetAllServerRegions(const std::string& server_regions_json,
                             bool success);

  void GetTimezonesForRegions(JNIEnv* env);

  void OnGetTimezonesForRegions(const std::string& timezones_json,
                                bool success);

  void GetHostnamesForRegion(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& region);

  void OnGetHostnamesForRegion(const std::string& hostname_json, bool success);

  void GetWireguardProfileCredentials(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& public_key,
      const base::android::JavaParamRef<jstring>& hostname);

  void OnGetWireguardProfileCredentials(
      const std::string& wireguard_profile_credentials_json,
      bool success);

  void VerifyCredentials(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& hostname,
      const base::android::JavaParamRef<jstring>& client_id,
      const base::android::JavaParamRef<jstring>& api_auth_token);

  void OnVerifyCredentials(const std::string& verify_credentials_json,
                           bool success);
  void InvalidateCredentials(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& hostname,
      const base::android::JavaParamRef<jstring>& client_id,
      const base::android::JavaParamRef<jstring>& api_auth_token);

  void OnInvalidateCredentials(const std::string& invalidate_credentials_json,
                               bool success);

  void ReportForegroundP3A(JNIEnv* env);
  void ReportBackgroundP3A(JNIEnv* env,
                           jlong session_start_time_ms,
                           jlong session_end_time_ms);

 private:
  JavaObjectWeakGlobalRef weak_java_wootz_vpn_native_worker_;
  base::WeakPtrFactory<WootzVpnNativeWorker> weak_factory_;
};
}  // namespace android
}  // namespace chrome