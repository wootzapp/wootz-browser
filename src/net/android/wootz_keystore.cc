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

// OpenSSL includes for CSR generation
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

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

// CSR Generation Implementation

namespace {

// Helper function to create X509_NAME from device ID
X509_NAME* CreateSubjectName(const std::string& device_id) {
  X509_NAME* name = X509_NAME_new();
  if (!name) {
    LOG(ERROR) << "Failed to create X509_NAME";
    return nullptr;
  }
  
  // Add subject components: CN=deviceId, OU=Wootz Browser, O=Wootz, C=US
  if (!X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
                                  reinterpret_cast<const unsigned char*>("US"),
                                  -1, -1, 0) ||
      !X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
                                  reinterpret_cast<const unsigned char*>("Wootz"),
                                  -1, -1, 0) ||
      !X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC,
                                  reinterpret_cast<const unsigned char*>("Wootz Browser"),
                                  -1, -1, 0) ||
      !X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                  reinterpret_cast<const unsigned char*>(device_id.c_str()),
                                  -1, -1, 0)) {
    LOG(ERROR) << "Failed to add subject name entries";
    X509_NAME_free(name);
    return nullptr;
  }
  
  return name;
}

// Helper function to create EVP_PKEY from EC public key bytes
EVP_PKEY* CreatePublicKeyFromBytes(base::span<const uint8_t> public_key_bytes) {
  const unsigned char* key_data = public_key_bytes.data();
  EVP_PKEY* pkey = d2i_PUBKEY(nullptr, &key_data, public_key_bytes.size());
  
  if (!pkey) {
    LOG(ERROR) << "Failed to parse public key from bytes";
    return nullptr;
  }
  
  // Verify this is an EC P-256 key
  if (EVP_PKEY_id(pkey) != EVP_PKEY_EC) {
    LOG(ERROR) << "Public key is not an EC key";
    EVP_PKEY_free(pkey);
    return nullptr;
  }
  
  EC_KEY* ec_key = EVP_PKEY_get1_EC_KEY(pkey);
  if (!ec_key) {
    LOG(ERROR) << "Failed to get EC_KEY from EVP_PKEY";
    EVP_PKEY_free(pkey);
    return nullptr;
  }
  
  const EC_GROUP* group = EC_KEY_get0_group(ec_key);
  if (!group || EC_GROUP_get_curve_name(group) != NID_X9_62_prime256v1) {
    LOG(ERROR) << "Public key is not P-256";
    EC_KEY_free(ec_key);
    EVP_PKEY_free(pkey);
    return nullptr;
  }
  
  EC_KEY_free(ec_key);
  return pkey;
}

// Helper function to add CSR extensions
bool AddCSRExtensions(X509_REQ* req) {
  STACK_OF(X509_EXTENSION)* exts = sk_X509_EXTENSION_new_null();
  if (!exts) {
    LOG(ERROR) << "Failed to create extension stack";
    return false;
  }
  
  // Add Key Usage extension (critical): digitalSignature
  X509_EXTENSION* key_usage_ext = X509V3_EXT_conf_nid(
      nullptr, nullptr, NID_key_usage, "critical,digitalSignature");
  if (!key_usage_ext) {
    LOG(ERROR) << "Failed to create Key Usage extension";
    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
    return false;
  }
  sk_X509_EXTENSION_push(exts, key_usage_ext);
  
  // Add Extended Key Usage extension (critical): clientAuth
  X509_EXTENSION* ext_key_usage_ext = X509V3_EXT_conf_nid(
      nullptr, nullptr, NID_ext_key_usage, "critical,clientAuth");
  if (!ext_key_usage_ext) {
    LOG(ERROR) << "Failed to create Extended Key Usage extension";
    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
    return false;
  }
  sk_X509_EXTENSION_push(exts, ext_key_usage_ext);
  
  // Add extensions to the CSR
  if (!X509_REQ_add_extensions(req, exts)) {
    LOG(ERROR) << "Failed to add extensions to CSR";
    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
    return false;
  }
  
  sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
  return true;
}

// Helper function to sign CSR using Android KeyStore via existing SignWithHardwareKey
bool SignCSRWithHardwareKey(X509_REQ* req, const std::string& private_key_alias) {
  // Get the data to be signed (CertificationRequestInfo)
  unsigned char* req_info_data = nullptr;
  int req_info_len = i2d_re_X509_REQ_tbs(req, &req_info_data);
  if (req_info_len <= 0 || !req_info_data) {
    LOG(ERROR) << "Failed to encode CertificationRequestInfo for signing";
    return false;
  }
  
  // Create span for the data to sign
  base::span<const uint8_t> data_to_sign(req_info_data, static_cast<size_t>(req_info_len));  
  // Sign using hardware key via existing SignWithHardwareKey function
  // This calls Java_WootzHardwareKeyStore_signWithHardwareKey internally
  std::vector<uint8_t> signature = SignWithHardwareKey(data_to_sign);
  
  // Free the encoded data
  OPENSSL_free(req_info_data);
  
  if (signature.empty()) {
    LOG(ERROR) << "Failed to sign CSR with hardware key";
    return false;
  }
  
  // Create signature algorithm identifier for ECDSA with SHA-256
  X509_ALGOR* sig_alg = X509_ALGOR_new();
  if (!sig_alg) {
    LOG(ERROR) << "Failed to create signature algorithm";
    return false;
  }
  
  // Set the algorithm OID for ecdsa-with-SHA256
  if (!X509_ALGOR_set0(sig_alg, OBJ_nid2obj(NID_ecdsa_with_SHA256), V_ASN1_NULL, nullptr)) {
    LOG(ERROR) << "Failed to set signature algorithm OID";
    X509_ALGOR_free(sig_alg);
    return false;
  }
  
  // Create signature bit string
  ASN1_BIT_STRING* sig_bit_string = ASN1_BIT_STRING_new();
  if (!sig_bit_string) {
    LOG(ERROR) << "Failed to create signature bit string";
    X509_ALGOR_free(sig_alg);
    return false;
  }
  
  if (!ASN1_BIT_STRING_set(sig_bit_string, signature.data(), signature.size())) {
    LOG(ERROR) << "Failed to set signature data";
    ASN1_BIT_STRING_free(sig_bit_string);
    X509_ALGOR_free(sig_alg);
    return false;
  }
  
  // Set signature and algorithm on CSR (this takes ownership)
  X509_REQ_set0_signature(req, sig_bit_string, sig_alg);
  
  return true;
}

}  // anonymous namespace

std::string GenerateCSR(const std::string& device_id,
                       base::span<const uint8_t> public_key_bytes,
                       const std::string& private_key_alias) {
  LOG(INFO) << "Generating CSR for device: " << device_id;
  
  // Create new CSR
  X509_REQ* req = X509_REQ_new();
  if (!req) {
    LOG(ERROR) << "Failed to create X509_REQ";
    return std::string();
  }
  
  // Set version (PKCS#10 v1.0)
  if (!X509_REQ_set_version(req, 0L)) {
    LOG(ERROR) << "Failed to set CSR version";
    X509_REQ_free(req);
    return std::string();
  }
  
  // Create and set subject name
  X509_NAME* subject = CreateSubjectName(device_id);
  if (!subject) {
    LOG(ERROR) << "Failed to create subject name";
    X509_REQ_free(req);
    return std::string();
  }
  
  if (!X509_REQ_set_subject_name(req, subject)) {
    LOG(ERROR) << "Failed to set subject name";
    X509_NAME_free(subject);
    X509_REQ_free(req);
    return std::string();
  }
  X509_NAME_free(subject);
  
  // Create and set public key
  EVP_PKEY* pkey = CreatePublicKeyFromBytes(public_key_bytes);
  if (!pkey) {
    LOG(ERROR) << "Failed to create public key";
    X509_REQ_free(req);
    return std::string();
  }
  
  if (!X509_REQ_set_pubkey(req, pkey)) {
    LOG(ERROR) << "Failed to set public key";
    EVP_PKEY_free(pkey);
    X509_REQ_free(req);
    return std::string();
  }
  EVP_PKEY_free(pkey);
  
  // Add extensions
  if (!AddCSRExtensions(req)) {
    LOG(ERROR) << "Failed to add CSR extensions";
    X509_REQ_free(req);
    return std::string();
  }
  
  // Sign the CSR with hardware key
  if (!SignCSRWithHardwareKey(req, private_key_alias)) {
    LOG(ERROR) << "Failed to sign CSR with hardware key";
    X509_REQ_free(req);
    return std::string();
  }
  
  // Convert to PEM format
  BIO* bio = BIO_new(BIO_s_mem());
  if (!bio) {
    LOG(ERROR) << "Failed to create BIO";
    X509_REQ_free(req);
    return std::string();
  }
  
  if (!PEM_write_bio_X509_REQ(bio, req)) {
    LOG(ERROR) << "Failed to write CSR to PEM";
    BIO_free(bio);
    X509_REQ_free(req);
    return std::string();
  }
  
  // Extract PEM string
  char* pem_data;
  long pem_len = BIO_get_mem_data(bio, &pem_data);
  std::string pem_string(pem_data, pem_len);
  
  // Cleanup
  BIO_free(bio);
  X509_REQ_free(req);
  
  LOG(INFO) << "Successfully generated CSR for device: " << device_id;
  return pem_string;
}

}  // namespace net::android::wootz

// JNI method implementation following Chromium's @NativeMethods pattern
// This function is called by WootzHardwareKeyStoreJni.get().generateCSR()
static jstring JNI_WootzHardwareKeyStore_GenerateCSR(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& j_device_id,
    const base::android::JavaParamRef<jbyteArray>& j_public_key_bytes,
    const base::android::JavaParamRef<jstring>& j_private_key_alias) {
  
  // Convert Java parameters to C++ types
  std::string device_id = base::android::ConvertJavaStringToUTF8(env, j_device_id);
  std::string private_key_alias = base::android::ConvertJavaStringToUTF8(env, j_private_key_alias);
  
  std::vector<uint8_t> public_key_bytes;
  base::android::JavaByteArrayToByteVector(env, j_public_key_bytes, &public_key_bytes);
  
  // Generate CSR using OpenSSL implementation
  std::string csr_pem = net::android::wootz::GenerateCSR(device_id, public_key_bytes, private_key_alias);
  
  // Return result (empty string becomes null in Java)
  if (csr_pem.empty()) {
    return nullptr;
  }
  
  return base::android::ConvertUTF8ToJavaString(env, csr_pem).Release();
}

