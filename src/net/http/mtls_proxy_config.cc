// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/mtls_proxy_config.h"

#include "base/files/file_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "build/build_config.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_private_key.h"
#include "net/ssl/openssl_private_key.h"
#include "third_party/boringssl/src/include/openssl/bio.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/x509.h"

#if BUILDFLAG(IS_ANDROID)
#include "net/android/wootz_keystore.h"
#endif

namespace net {

namespace {

base::LazyInstance<MtlsProxyConfig>::Leaky g_mtls_proxy_config =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace
 // namespace

MtlsProxyConfig::MtlsProxyConfig()
    : enabled_(false),
      proxy_endpoint_(HostPortPair("eb.wootzapp.com", 443)) {
#if BUILDFLAG(IS_ANDROID)
  // On Android, try to load from Android Keystore
  if (LoadFromAndroidKeystore()) {
    LOG(INFO) << "Loaded mTLS certificates from Android Keystore";
  } else {
    LOG(WARNING) << "Android Keystore empty. mTLS proxy will remain disabled until certificates are provided.";
  }
#else
  LOG(WARNING) << "mTLS proxy certificates must be provided via SetClientCertificate() or LoadFromFiles()";
#endif
}

MtlsProxyConfig::~MtlsProxyConfig() = default;

void MtlsProxyConfig::SetEnabled(bool enabled) {
  enabled_ = enabled;
  LOG(INFO) << "mTLS Proxy " << (enabled ? "enabled" : "disabled");
}

void MtlsProxyConfig::SetProxyEndpoint(const HostPortPair& endpoint) {
  proxy_endpoint_ = endpoint;
  LOG(INFO) << "mTLS Proxy endpoint set to: " << endpoint.ToString();
}

void MtlsProxyConfig::SetClientCertificate(scoped_refptr<X509Certificate> cert,
                                           scoped_refptr<SSLPrivateKey> key) {
  client_cert_ = std::move(cert);
  client_key_ = std::move(key);
  LOG(INFO) << "mTLS client certificate configured";
}

void MtlsProxyConfig::SetCACertificate(scoped_refptr<X509Certificate> ca_cert) {
  ca_cert_ = std::move(ca_cert);
  LOG(INFO) << "mTLS CA certificate configured";
}

bool MtlsProxyConfig::LoadFromFiles(const base::FilePath& client_cert_path,
                                     const base::FilePath& client_key_path,
                                     const base::FilePath& ca_cert_path) {
  // Read client certificate
  std::string cert_data;
  if (!base::ReadFileToString(client_cert_path, &cert_data)) {
    LOG(ERROR) << "Failed to read client certificate from: "
               << client_cert_path;
    return false;
  }

  // Parse client certificate
  CertificateList cert_list =
      X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(cert_data)),
          X509Certificate::FORMAT_AUTO);
  
  if (cert_list.empty()) {
    LOG(ERROR) << "Failed to parse client certificate";
    return false;
  }

  client_cert_ = cert_list[0];

  // Read private key
  std::string key_data;
  if (!base::ReadFileToString(client_key_path, &key_data)) {
    LOG(ERROR) << "Failed to read private key from: " << client_key_path;
    return false;
  }

  // Parse private key using OpenSSL
  bssl::UniquePtr<BIO> bio(BIO_new_mem_buf(key_data.data(), key_data.size()));
  if (!bio) {
    LOG(ERROR) << "Failed to create BIO for private key";
    return false;
  }

  bssl::UniquePtr<EVP_PKEY> pkey(
      PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr));
  if (!pkey) {
    LOG(ERROR) << "Failed to parse private key";
    return false;
  }

  client_key_ = WrapOpenSSLPrivateKey(std::move(pkey));
  if (!client_key_) {
    LOG(ERROR) << "Failed to wrap private key";
    return false;
  }

  // Read CA certificate if provided
  if (!ca_cert_path.empty()) {
    std::string ca_data;
    if (!base::ReadFileToString(ca_cert_path, &ca_data)) {
      LOG(WARNING) << "Failed to read CA certificate from: " << ca_cert_path;
      // Don't fail completely, CA might be in system store
    } else {
      CertificateList ca_list =
          X509Certificate::CreateCertificateListFromBytes(
              base::as_bytes(base::make_span(ca_data)),
              X509Certificate::FORMAT_AUTO);
      
      if (!ca_list.empty()) {
        ca_cert_ = ca_list[0];
      } else {
        LOG(WARNING) << "Failed to parse CA certificate";
      }
    }
  }

  LOG(INFO) << "Successfully loaded mTLS certificates from files";
  return true;
}

bool MtlsProxyConfig::LoadHardcodedCertificates() {
  LOG(WARNING) << "Hardcoded certificates are not available. mTLS proxy will remain disabled.";
  // No hardcoded certificates - must use Android Keystore or provide via SetClientCertificate
  return false;
}

#if BUILDFLAG(IS_ANDROID)
bool MtlsProxyConfig::LoadFromAndroidKeystore() {
  LOG(INFO) << "Loading mTLS certificates from Android Keystore";

  // Check if certificate exists in Android Keystore
  if (!net::android::wootz::HasMTLSProxyCertificate()) {
    LOG(WARNING) << "No mTLS proxy certificate found in Android Keystore";
    return false;
  }

  // Get certificate from Android Keystore
  std::vector<uint8_t> cert_der = net::android::wootz::GetMTLSProxyCertificate();
  if (cert_der.empty()) {
    LOG(ERROR) << "Failed to retrieve mTLS certificate from Android Keystore";
    return false;
  }

  // Parse certificate
  client_cert_ = X509Certificate::CreateFromBytes(base::as_bytes(base::make_span(cert_der)));
  if (!client_cert_) {
    LOG(ERROR) << "Failed to parse mTLS certificate from Android Keystore";
    return false;
  }

  LOG(INFO) << "Successfully parsed mTLS certificate from Android Keystore";

  // Get private key from Android Keystore
  std::vector<uint8_t> key_pkcs8 = net::android::wootz::GetMTLSProxyPrivateKey();
  if (key_pkcs8.empty()) {
    LOG(ERROR) << "Failed to retrieve mTLS private key from Android Keystore";
    client_cert_ = nullptr;
    return false;
  }

  // Parse private key (PKCS#8 format)
  const uint8_t* key_data = key_pkcs8.data();
  bssl::UniquePtr<EVP_PKEY> pkey(
      d2i_PrivateKey(EVP_PKEY_EC, nullptr, &key_data, key_pkcs8.size()));
  
  if (!pkey) {
    // Try RSA if EC fails
    key_data = key_pkcs8.data();
    pkey.reset(d2i_PrivateKey(EVP_PKEY_RSA, nullptr, &key_data, key_pkcs8.size()));
  }

  if (!pkey) {
    LOG(ERROR) << "Failed to parse mTLS private key from Android Keystore";
    client_cert_ = nullptr;
    return false;
  }

  // Wrap the private key
  client_key_ = WrapOpenSSLPrivateKey(std::move(pkey));
  if (!client_key_) {
    LOG(ERROR) << "Failed to wrap mTLS private key from Android Keystore";
    client_cert_ = nullptr;
    return false;
  }

  LOG(INFO) << "Successfully loaded mTLS certificates from Android Keystore";
  LOG(INFO) << "  Certificate: " << client_cert_.get();
  LOG(INFO) << "  Private Key: " << client_key_.get();

  // Enable mTLS proxy since we have valid credentials
  SetEnabled(true);

  return true;
}

bool MtlsProxyConfig::ReloadFromAndroidKeystore() {
  LOG(INFO) << "Reloading mTLS certificates from Android Keystore";
  
  // Clear existing certificates
  client_cert_ = nullptr;
  client_key_ = nullptr;
  
  // Try to load from Android Keystore
  if (LoadFromAndroidKeystore()) {
    LOG(INFO) << "Successfully reloaded mTLS certificates from Android Keystore";
    return true;
  } else {
    LOG(ERROR) << "Failed to reload from Android Keystore. mTLS proxy will be disabled.";
    return false;
  }
}
#endif

// static
MtlsProxyConfig* MtlsProxyConfig::GetInstance() {
  return g_mtls_proxy_config.Pointer();
}

}  // namespace net

