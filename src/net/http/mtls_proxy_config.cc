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

namespace {

// Hardcoded certificates for testing/deployment
const char kHardcodedClientKey[] = 
    "-----BEGIN PRIVATE KEY-----\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgUbDnhOQwbo7ZjXoH\n"
    "9tkSHFmfMJdYA83kzuWPLl8r9sShRANCAARL89GBQyGt6NEGgLfqdOwTGli2C9d2\n"
    "Imhwbn9vBhN0ghdy/Vdf7/KrYJS9xQTtYVyeDIH1giR0/zKxZVyAM28l\n"
    "-----END PRIVATE KEY-----\n";

const char kHardcodedClientCert[] = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIICBTCCAaugAwIBAgIUKbb1JYK8M1rC8msAhlEJSR8HcfUwCgYIKoZIzj0EAwIw\n"
    "UTFPMBcGA1UEAwwQR0NQIG1UTFMgUm9vdCBDQTAPBgNVBAsMCFNlY3VyaXR5MBgG\n"
    "A1UECgwRWW91ciBPcmdhbml6YXRpb24wCQYDVQQGEwJVUzAeFw0yNTEwMTMxMzI0\n"
    "MTFaFw0zNTEwMTExMzI0MTFaMDoxODAJBgNVBAYTAlVTMAoGA1UECAwDQ2FsMAwG\n"
    "A1UECgwFV29vdHowEQYDVQQDDApkZXZpY2UtMTIzMFkwEwYHKoZIzj0CAQYIKoZI\n"
    "zj0DAQcDQgAES/PRgUMhrejRBoC36nTsExpYtgvXdiJocG5/bwYTdIIXcv1XX+/y\n"
    "q2CUvcUE7WFcngyB9YIkdP8ysWVcgDNvJaN4MHYwDwYDVR0TAQH/BAUwAwEBADAO\n"
    "BgNVHQ8BAf8EBAMCB4AwEwYDVR0lBAwwCgYIKwYBBQUHAwIwHQYDVR0OBBYEFNV+\n"
    "LDTRkPNQFbdLJiPftUnrRmcnMB8GA1UdIwQYMBaAFPZTnErI+wWEjlCK9eJVdaB8\n"
    "DhBSMAoGCCqGSM49BAMCA0gAMEUCIQCa3wBIObIyeWw2mwHHYHPaI42M1GBQnrjY\n"
    "90WlcOkxjQIgYcwa1lTo1L6D2ZyJzbOIsSesoCwzteag4eOfH2PvK+0=\n"
    "-----END CERTIFICATE-----\n";

const char kHardcodedCACert[] = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIICDjCCAbKgAwIBAgIUVHjG+GnM/rnCATUpfcyM3QNlgh4wDAYIKoZIzj0EAwIF\n"
    "ADBRMU8wFwYDVQQDDBBHQ1AgbVRMUyBSb290IENBMA8GA1UECwwIU2VjdXJpdHkw\n"
    "GAYDVQQKDBFZb3VyIE9yZ2FuaXphdGlvbjAJBgNVBAYTAlVTMB4XDTI1MDkyNTEy\n"
    "MzY1OFoXDTM1MDkyNTEyMzY1OFowUTFPMBcGA1UEAwwQR0NQIG1UTFMgUm9vdCBD\n"
    "QTAPBgNVBAsMCFNlY3VyaXR5MBgGA1UECgwRWW91ciBPcmdhbml6YXRpb24wCQYD\n"
    "VQQGEwJVUzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABGZJkyZstXFdNrM/yMFH\n"
    "M8s0XUITr+0xsv59qHTzpgPZpRclMHfqFZ/erODQt4v1wD7Lw4SjdzF0fLUQ7ffo\n"
    "HT6jZjBkMBIGA1UdEwEB/wQIMAYBAf8CAQEwDgYDVR0PAQH/BAQDAgAGMB0GA1Ud\n"
    "DgQWBBT2U5xKyPsFhI5QivXiVXWgfA4QUjAfBgNVHSMEGDAWgBT2U5xKyPsFhI5Q\n"
    "ivXiVXWgfA4QUjAMBggqhkjOPQQDAgUAA0gAMEUCIQCzBFdoh9xMGTwwOjGVAtUG\n"
    "LqSJ0QIWEs5Kd50ULd7dmwIgeZ/UhppiYJJ44M9e0+FRp+p27cQ7tzWjm7k/xODZ\n"
    "6gk=\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n"
    "iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
    "cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
    "BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n"
    "MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n"
    "BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n"
    "aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n"
    "dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n"
    "AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n"
    "3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n"
    "tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n"
    "Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n"
    "VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n"
    "79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n"
    "c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n"
    "Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n"
    "c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n"
    "UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n"
    "Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n"
    "BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n"
    "A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n"
    "Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n"
    "VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n"
    "ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n"
    "8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n"
    "iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n"
    "Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n"
    "XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n"
    "qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n"
    "VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n"
    "L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n"
    "jjxDah2nGN59PRbxYvnKkKj9\n"
    "-----END CERTIFICATE-----\n";

}  // namespace

MtlsProxyConfig::MtlsProxyConfig()
    : enabled_(false),
      proxy_endpoint_(HostPortPair("eb.wootzapp.com", 443)) {
#if BUILDFLAG(IS_ANDROID)
  // On Android, try to load from Android Keystore first
  if (LoadFromAndroidKeystore()) {
    LOG(INFO) << "Loaded mTLS certificates from Android Keystore";
  } else {
    // Fall back to hardcoded certificates if keystore is empty
    LOG(INFO) << "Android Keystore empty, loading hardcoded certificates";
    LoadHardcodedCertificates();
  }
#else
  // On other platforms, use hardcoded certificates
  LoadHardcodedCertificates();
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
  LOG(INFO) << "Loading hardcoded mTLS certificates";

  // Parse client certificate
  CertificateList cert_list =
      X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(kHardcodedClientCert)),
          X509Certificate::FORMAT_PEM_CERT_SEQUENCE);
  
  if (cert_list.empty()) {
    LOG(ERROR) << "Failed to parse hardcoded client certificate";
    return false;
  }

  client_cert_ = cert_list[0];

  // Parse private key using OpenSSL
  bssl::UniquePtr<BIO> bio(
      BIO_new_mem_buf(kHardcodedClientKey, strlen(kHardcodedClientKey)));
  if (!bio) {
    LOG(ERROR) << "Failed to create BIO for hardcoded private key";
    return false;
  }

  bssl::UniquePtr<EVP_PKEY> pkey(
      PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr));
  if (!pkey) {
    LOG(ERROR) << "Failed to parse hardcoded private key";
    return false;
  }

  client_key_ = WrapOpenSSLPrivateKey(std::move(pkey));
  if (!client_key_) {
    LOG(ERROR) << "Failed to wrap hardcoded private key";
    return false;
  }

  // Parse CA certificate
  CertificateList ca_list =
      X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(kHardcodedCACert)),
          X509Certificate::FORMAT_PEM_CERT_SEQUENCE);
  
  if (!ca_list.empty()) {
    ca_cert_ = ca_list[0];
  } else {
    LOG(WARNING) << "Failed to parse hardcoded CA certificate";
  }

  LOG(INFO) << "Successfully loaded hardcoded mTLS certificates";
  
  // Verify the loaded certificates are valid
  if (!client_cert_) {
    LOG(ERROR) << "client_cert_ is null after loading!";
    return false;
  }
  if (!client_key_) {
    LOG(ERROR) << "client_key_ is null after loading!";
    return false;
  }
  
  LOG(INFO) << "Certificates verified: client_cert=" << client_cert_.get() 
            << " client_key=" << client_key_.get();
  
  // Enable by default when certificates are loaded
  SetEnabled(true);
  
  return true;
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
    // If reload fails, fall back to hardcoded certificates
    LOG(WARNING) << "Failed to reload from Android Keystore, falling back to hardcoded";
    return LoadHardcodedCertificates();
  }
}
#endif

// static
MtlsProxyConfig* MtlsProxyConfig::GetInstance() {
  return g_mtls_proxy_config.Pointer();
}

}  // namespace net

