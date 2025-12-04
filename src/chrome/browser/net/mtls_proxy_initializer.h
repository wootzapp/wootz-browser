// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_MTLS_PROXY_INITIALIZER_H_
#define CHROME_BROWSER_NET_MTLS_PROXY_INITIALIZER_H_

#include "base/files/file_path.h"

class Profile;

namespace net {
class SSLClientContext;
class HostPortPair;
}  // namespace net

// Initializes the mTLS proxy configuration for Wootz Browser.
// This sets up the connection to eb.wootzapp.com:443 with client certificate
// authentication for all HTTP/HTTPS traffic.
class MtlsProxyInitializer {
 public:
  // Initialize the mTLS proxy for the given profile
  static void Initialize(Profile* profile);

  // Configure mTLS proxy client certificates for the SSL context
  // This should be called during network context setup
  static void ConfigureSSLClientContext(
      net::SSLClientContext* ssl_client_context,
      const net::HostPortPair& proxy_endpoint);

  // Load certificates from Android app's private directory
  // Returns true if certificates were loaded successfully
  static bool LoadCertificatesFromAppData();

  // Load certificates from specified paths
  static bool LoadCertificates(const base::FilePath& cert_path,
                               const base::FilePath& key_path,
                               const base::FilePath& ca_path);

  // Enable/disable the mTLS proxy
  static void SetEnabled(bool enabled);
  static bool IsEnabled();

 private:
  MtlsProxyInitializer() = delete;
  ~MtlsProxyInitializer() = delete;
};

#endif  // CHROME_BROWSER_NET_MTLS_PROXY_INITIALIZER_H_

