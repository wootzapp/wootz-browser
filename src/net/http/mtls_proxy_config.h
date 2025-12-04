// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_MTLS_PROXY_CONFIG_H_
#define NET_HTTP_MTLS_PROXY_CONFIG_H_

#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/scoped_refptr.h"
#include "build/build_config.h"
#include "net/base/host_port_pair.h"
#include "net/base/net_export.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_private_key.h"

namespace net {

// Configuration for mTLS proxy connection to eb.wootzapp.com:443
// This class manages the proxy endpoint and client certificate credentials
// for mutual TLS authentication.
class NET_EXPORT_PRIVATE MtlsProxyConfig {
 public:
  MtlsProxyConfig();
  ~MtlsProxyConfig();

  MtlsProxyConfig(const MtlsProxyConfig&) = delete;
  MtlsProxyConfig& operator=(const MtlsProxyConfig&) = delete;

  // Enable or disable the mTLS proxy
  void SetEnabled(bool enabled);
  bool IsEnabled() const { return enabled_; }

  // Set the proxy endpoint (default: eb.wootzapp.com:443)
  void SetProxyEndpoint(const HostPortPair& endpoint);
  const HostPortPair& GetProxyEndpoint() const { return proxy_endpoint_; }

  // Set client certificate and private key for mTLS
  // These will be used to authenticate to the proxy server
  void SetClientCertificate(scoped_refptr<X509Certificate> cert,
                            scoped_refptr<SSLPrivateKey> key);
  
  scoped_refptr<X509Certificate> GetClientCertificate() const {
    return client_cert_;
  }
  
  scoped_refptr<SSLPrivateKey> GetClientPrivateKey() const {
    return client_key_;
  }

  bool HasClientCertificate() const {
    return client_cert_ != nullptr && client_key_ != nullptr;
  }

  // Set CA certificate for verifying the proxy server
  void SetCACertificate(scoped_refptr<X509Certificate> ca_cert);
  scoped_refptr<X509Certificate> GetCACertificate() const { return ca_cert_; }

  // Load configuration from certificate files
  // Returns true on success
  bool LoadFromFiles(const base::FilePath& client_cert_path,
                     const base::FilePath& client_key_path,
                     const base::FilePath& ca_cert_path);

  // Load hardcoded certificates (for testing/deployment)
  // Returns true on success
  bool LoadHardcodedCertificates();

#if BUILDFLAG(IS_ANDROID)
  // Load certificates from Android Keystore
  // Returns true on success
  bool LoadFromAndroidKeystore();
  
  // Reload certificates from Android Keystore (useful after update from extension)
  // Returns true if successfully reloaded
  bool ReloadFromAndroidKeystore();
#endif

  // Get a singleton instance (for convenience)
  static MtlsProxyConfig* GetInstance();

 private:
  bool enabled_;
  HostPortPair proxy_endpoint_;
  scoped_refptr<X509Certificate> client_cert_;
  scoped_refptr<SSLPrivateKey> client_key_;
  scoped_refptr<X509Certificate> ca_cert_;
};

}  // namespace net

#endif  // NET_HTTP_MTLS_PROXY_CONFIG_H_

