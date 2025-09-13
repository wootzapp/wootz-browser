// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/android/wootz_keystore.h"

#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/check.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "net/net_jni_headers/WootzHardwareKeyStore_jni.h"

using base::android::AttachCurrentThread;
using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertUTF8ToJavaString;
using base::android::HasException;
using base::android::JavaByteArrayToByteVector;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;
using base::android::ToJavaByteArray;

namespace net::android::wootz {

bool InitializeHardwareKeystore() {
  JNIEnv* env = AttachCurrentThread();
  
  // Check if we already have a hardware-backed key
  if (Java_WootzHardwareKeyStore_isKeyHardwareBacked(env)) {
    LOG(INFO) << "Hardware-backed key already exists, skipping initialization";
    return true;
  }
  
  LOG(INFO) << "Initializing hardware-backed certificate system";
  
  // Generate attestation challenge from browser startup context
  std::string challenge_data = "wootz-browser-" + 
      std::to_string(base::Time::Now().InMillisecondsSinceUnixEpoch());
  std::vector<uint8_t> attestation_challenge(challenge_data.begin(), challenge_data.end());
  
  // Generate hardware-backed key with attestation
  bool success = GenerateHardwareBackedKeyWithAttestation(attestation_challenge);
  
  if (success) {
    LOG(INFO) << "Hardware-backed certificate system initialized successfully";
    
    // Log the security level
    if (Java_WootzHardwareKeyStore_isKeyStrongboxBacked(env)) {
      LOG(INFO) << "Certificate system using Strongbox security level";
    } else if (Java_WootzHardwareKeyStore_isKeyHardwareBacked(env)) {
      LOG(INFO) << "Certificate system using TEE security level";
    } else {
      LOG(WARNING) << "Certificate system not hardware-backed";
    }
  } else {
    LOG(ERROR) << "Failed to initialize hardware-backed certificate system";
  }
  
  return success;
}

bool GenerateHardwareBackedKeyWithAttestation(
    base::span<const uint8_t> attestation_challenge) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jbyteArray> challenge_array =
      ToJavaByteArray(env, attestation_challenge);
  
  return Java_WootzHardwareKeyStore_generateHardwareBackedKeyWithAttestation(
      env, challenge_array);
}

std::string GetDevicePublicKeyPem() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> pem_string =
      Java_WootzHardwareKeyStore_getDevicePublicKeyPem(env);
  
  if (pem_string.is_null()) {
    return std::string();
  }
  
  return ConvertJavaStringToUTF8(env, pem_string);
}

std::string GetAttestationChainPem() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> chain_string =
      Java_WootzHardwareKeyStore_getAttestationChainPem(env);
  
  if (chain_string.is_null()) {
    return std::string();
  }
  
  return ConvertJavaStringToUTF8(env, chain_string);
}

bool IsKeyStrongboxBacked() {
  JNIEnv* env = AttachCurrentThread();
  return Java_WootzHardwareKeyStore_isKeyStrongboxBacked(env);
}

bool IsKeyHardwareBacked() {
  JNIEnv* env = AttachCurrentThread();
  return Java_WootzHardwareKeyStore_isKeyHardwareBacked(env);
}

std::vector<uint8_t> SignWithHardwareKey(base::span<const uint8_t> data) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jbyteArray> data_array = ToJavaByteArray(env, data);
  
  ScopedJavaLocalRef<jbyteArray> signature_array =
      Java_WootzHardwareKeyStore_signWithHardwareKey(env, data_array);
  
  if (signature_array.is_null()) {
    return std::vector<uint8_t>();
  }
  
  std::vector<uint8_t> signature;
  JavaByteArrayToByteVector(env, signature_array, &signature);
  return signature;
}

// mTLS Client Certificate Functions

bool IsDicAvailableForMTLS() {
  JNIEnv* env = AttachCurrentThread();
  return Java_WootzHardwareKeyStore_isDicAvailableForMTLS(env);
}

std::vector<uint8_t> GetMTLSClientCertificate() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jbyteArray> cert_array =
      Java_WootzHardwareKeyStore_getMTLSClientCertificate(env);
  
  if (cert_array.is_null()) {
    return std::vector<uint8_t>();
  }
  
  std::vector<uint8_t> certificate;
  JavaByteArrayToByteVector(env, cert_array, &certificate);
  return certificate;
}

std::vector<uint8_t> SignMTLSHandshake(base::span<const uint8_t> handshake_data) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jbyteArray> data_array = ToJavaByteArray(env, handshake_data);
  
  ScopedJavaLocalRef<jbyteArray> signature_array =
      Java_WootzHardwareKeyStore_signMTLSHandshake(env, data_array);
  
  if (signature_array.is_null()) {
    LOG(ERROR) << "Failed to sign mTLS handshake data";
    return std::vector<uint8_t>();
  }
  
  std::vector<uint8_t> signature;
  JavaByteArrayToByteVector(env, signature_array, &signature);
  
  LOG(INFO) << "Successfully signed mTLS handshake with hardware key, signature size: " 
            << signature.size();
  return signature;
}

std::vector<std::vector<uint8_t>> GetMTLSCertificateChain() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobjectArray> chain_array =
      Java_WootzHardwareKeyStore_getMTLSCertificateChain(env);
  
  if (chain_array.is_null()) {
    return std::vector<std::vector<uint8_t>>();
  }
  
  std::vector<std::vector<uint8_t>> certificate_chain;
  jsize chain_length = env->GetArrayLength(chain_array.obj());
  
  for (jsize i = 0; i < chain_length; ++i) {
    ScopedJavaLocalRef<jbyteArray> cert_array(
        env, static_cast<jbyteArray>(
            env->GetObjectArrayElement(chain_array.obj(), i)));
    
    if (!cert_array.is_null()) {
      std::vector<uint8_t> certificate;
      JavaByteArrayToByteVector(env, cert_array, &certificate);
      certificate_chain.push_back(std::move(certificate));
    }
  }
  
  return certificate_chain;
}

std::string GetMTLSSecurityInfo() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jstring> info_string =
      Java_WootzHardwareKeyStore_getMTLSSecurityInfo(env);
  
  if (info_string.is_null()) {
    return std::string();
  }
  
  return ConvertJavaStringToUTF8(env, info_string);
}

}  // namespace net::android::wootz
