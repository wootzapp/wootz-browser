// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_client_cert_identity.h"

#include "base/functional/bind.h"
#include "base/logging.h"
#include "build/build_config.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_private_key.h"
#include "net/ssl/ssl_platform_key_util.h"
#include "third_party/boringssl/src/include/openssl/ssl.h"

#if BUILDFLAG(IS_ANDROID)
#include "net/android/wootz_keystore.h"
#endif

namespace {

// SSLPrivateKey implementation that delegates to Wootz hardware key
class WootzSSLPrivateKey : public net::SSLPrivateKey {
 public:
  WootzSSLPrivateKey() = default;

 protected:
  ~WootzSSLPrivateKey() override = default;

 private:
  friend class base::RefCountedThreadSafe<WootzSSLPrivateKey>;

 public:
  // net::SSLPrivateKey implementation:
  std::string GetProviderName() override {
    return "Wootz Hardware Key (TEE/Strongbox)";
  }

  std::vector<uint16_t> GetAlgorithmPreferences() override {
    // Support common signature algorithms for hardware keys
    return {
      SSL_SIGN_ECDSA_SECP256R1_SHA256,
      SSL_SIGN_ECDSA_SECP384R1_SHA384,
      SSL_SIGN_RSA_PKCS1_SHA256,
      SSL_SIGN_RSA_PKCS1_SHA384,
    };
  }

  void Sign(uint16_t algorithm,
            base::span<const uint8_t> input,
            SignCallback callback) override {
    DVLOG(1) << "WootzSSLPrivateKey::Sign called";
    
#if BUILDFLAG(IS_ANDROID)
    // Delegate directly to our existing hardware signing function
    std::vector<uint8_t> signature = net::android::wootz::SignMTLSHandshake(input);
    
    if (!signature.empty()) {
      DVLOG(1) << "Wootz hardware key signing successful";
      std::move(callback).Run(net::OK, std::move(signature));
    } else {
      LOG(ERROR) << "Wootz hardware key signing failed";
      std::move(callback).Run(net::ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED, 
                             std::vector<uint8_t>());
    }
#else
    LOG(ERROR) << "Wootz hardware key only supported on Android";
    std::move(callback).Run(net::ERR_NOT_IMPLEMENTED, std::vector<uint8_t>());
#endif
  }

 private:
  WootzSSLPrivateKey(const WootzSSLPrivateKey&) = delete;
  WootzSSLPrivateKey& operator=(const WootzSSLPrivateKey&) = delete;
};

}  // namespace

WootzClientCertIdentity::WootzClientCertIdentity(
    scoped_refptr<net::X509Certificate> certificate)
    : net::ClientCertIdentity(std::move(certificate)) {}

WootzClientCertIdentity::~WootzClientCertIdentity() = default;

void WootzClientCertIdentity::AcquirePrivateKey(base::OnceCallback<void(scoped_refptr<net::SSLPrivateKey>)> callback) {
  DVLOG(1) << "WootzClientCertIdentity::AcquirePrivateKey called";
  
  // Create hardware key proxy that delegates to our existing infrastructure
  auto hardware_key = base::MakeRefCounted<WootzSSLPrivateKey>();
  std::move(callback).Run(std::move(hardware_key));
}

