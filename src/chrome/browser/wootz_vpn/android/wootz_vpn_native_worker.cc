#include "chrome/browser/wootz_vpn/android/wootz_vpn_native_worker.h"

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/functional/bind.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include "chrome/browser/wootz_vpn/wootz_vpn_service_factory.h"
#include "chrome/build/android/jni_headers/WootzVpnNativeWorker_jni.h" /////////
#include "chrome/components/wootz_vpn/browser/wootz_vpn_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"

using wootz_vpn::WootzVpnService;

namespace {

WootzVpnService* GetWootzVpnService() {
  return wootz_vpn::WootzVpnServiceFactory::GetForProfile(
      ProfileManager::GetActiveUserProfile()->GetOriginalProfile());
}

}  // namespace

namespace chrome {
namespace android {

WootzVpnNativeWorker::WootzVpnNativeWorker(
    JNIEnv* env,
    const base::android::JavaRef<jobject>& obj)
    : weak_java_wootz_vpn_native_worker_(env, obj), weak_factory_(this) {
  Java_WootzVpnNativeWorker_setNativePtr(env, obj,
                                         reinterpret_cast<intptr_t>(this));
}

WootzVpnNativeWorker::~WootzVpnNativeWorker() {}

void WootzVpnNativeWorker::Destroy(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller) {
  delete this;
}

void WootzVpnNativeWorker::GetAllServerRegions(JNIEnv* env) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    wootz_vpn_service->GetAllServerRegions(
        base::BindOnce(&WootzVpnNativeWorker::OnGetAllServerRegions,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzVpnNativeWorker::OnGetAllServerRegions(
    const std::string& server_regions_json,
    bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzVpnNativeWorker_onGetAllServerRegions(
      env, weak_java_wootz_vpn_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, server_regions_json),
      success);
}

void WootzVpnNativeWorker::GetTimezonesForRegions(JNIEnv* env) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    wootz_vpn_service->GetTimezonesForRegions(
        base::BindOnce(&WootzVpnNativeWorker::OnGetTimezonesForRegions,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzVpnNativeWorker::OnGetTimezonesForRegions(
    const std::string& timezones_json,
    bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzVpnNativeWorker_onGetTimezonesForRegions(
      env, weak_java_wootz_vpn_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, timezones_json), success);
}

void WootzVpnNativeWorker::GetHostnamesForRegion(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& region) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    wootz_vpn_service->GetHostnamesForRegion(
        base::BindOnce(&WootzVpnNativeWorker::OnGetHostnamesForRegion,
                       weak_factory_.GetWeakPtr()),
        base::android::ConvertJavaStringToUTF8(env, region));
  }
}

void WootzVpnNativeWorker::OnGetHostnamesForRegion(
    const std::string& hostnames_json,
    bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzVpnNativeWorker_onGetHostnamesForRegion(
      env, weak_java_wootz_vpn_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, hostnames_json), success);
}

void WootzVpnNativeWorker::GetWireguardProfileCredentials(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& public_key,
    const base::android::JavaParamRef<jstring>& hostname) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    wootz_vpn_service->GetWireguardProfileCredentials(
        base::BindOnce(&WootzVpnNativeWorker::OnGetWireguardProfileCredentials,
                       weak_factory_.GetWeakPtr()),
        base::android::ConvertJavaStringToUTF8(env, public_key),
        base::android::ConvertJavaStringToUTF8(env, hostname));
  }
}

void WootzVpnNativeWorker::OnGetWireguardProfileCredentials(
    const std::string& wireguard_profile_credentials_json,
    bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzVpnNativeWorker_onGetWireguardProfileCredentials(
      env, weak_java_wootz_vpn_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(
          env, wireguard_profile_credentials_json),
      success);
}

void WootzVpnNativeWorker::VerifyCredentials(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& hostname,
    const base::android::JavaParamRef<jstring>& client_id,
    const base::android::JavaParamRef<jstring>& api_auth_token) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    wootz_vpn_service->VerifyCredentials(
        base::BindOnce(&WootzVpnNativeWorker::OnVerifyCredentials,
                       weak_factory_.GetWeakPtr()),
        base::android::ConvertJavaStringToUTF8(env, hostname),
        base::android::ConvertJavaStringToUTF8(env, client_id),
        base::android::ConvertJavaStringToUTF8(env, api_auth_token));
  }
}

void WootzVpnNativeWorker::OnVerifyCredentials(
    const std::string& verify_credentials_json,
    bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzVpnNativeWorker_onVerifyCredentials(
      env, weak_java_wootz_vpn_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, verify_credentials_json),
      success);
}

void WootzVpnNativeWorker::InvalidateCredentials(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& hostname,
    const base::android::JavaParamRef<jstring>& client_id,
    const base::android::JavaParamRef<jstring>& api_auth_token) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    wootz_vpn_service->InvalidateCredentials(
        base::BindOnce(&WootzVpnNativeWorker::OnInvalidateCredentials,
                       weak_factory_.GetWeakPtr()),
        base::android::ConvertJavaStringToUTF8(env, hostname),
        base::android::ConvertJavaStringToUTF8(env, client_id),
        base::android::ConvertJavaStringToUTF8(env, api_auth_token));
  }
}

void WootzVpnNativeWorker::OnInvalidateCredentials(
    const std::string& invalidate_credentials_json,
    bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzVpnNativeWorker_onInvalidateCredentials(
      env, weak_java_wootz_vpn_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, invalidate_credentials_json),
      success);
}

void WootzVpnNativeWorker::ReportForegroundP3A(JNIEnv* env) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    // Reporting a new session to P3A functions.
    wootz_vpn_service->RecordP3A(true);
  }
}

void WootzVpnNativeWorker::ReportBackgroundP3A(JNIEnv* env,
                                               jlong session_start_time_ms,
                                               jlong session_end_time_ms) {
  WootzVpnService* wootz_vpn_service = GetWootzVpnService();
  if (wootz_vpn_service) {
    wootz_vpn_service->RecordAndroidBackgroundP3A(session_start_time_ms,
                                                  session_end_time_ms);
  }
}

static void JNI_WootzVpnNativeWorker_Init(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller) {
  new WootzVpnNativeWorker(env, jcaller);
}

}  // namespace android
}  // namespace chrome