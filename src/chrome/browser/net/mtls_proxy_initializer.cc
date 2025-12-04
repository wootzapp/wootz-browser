// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/mtls_proxy_initializer.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_paths.h"
#include "net/cert/x509_certificate.h"
#include "net/http/mtls_proxy_config.h"
#include "net/socket/ssl_client_socket.h"
#include "net/ssl/ssl_private_key.h"

#if BUILDFLAG(IS_ANDROID)
#include "base/android/path_utils.h"
#endif

namespace {

// Default paths for mTLS certificates (relative to app data directory)
constexpr char kClientCertFileName[] = "app.pem";
constexpr char kClientKeyFileName[] = "app.key";
constexpr char kCACertFileName[] = "ca-bundle.pem";

}  // namespace

// static
void MtlsProxyInitializer::Initialize(Profile* profile) {
  if (!profile) {
    LOG(ERROR) << "Cannot initialize mTLS proxy: null profile";
    return;
  }

  LOG(INFO) << "Initializing mTLS proxy for Wootz Browser";

  // Check if hardcoded certificates are already loaded
  net::MtlsProxyConfig* config = net::MtlsProxyConfig::GetInstance();
  if (config && config->HasClientCertificate() && config->IsEnabled()) {
    LOG(INFO) << "mTLS proxy already initialized with hardcoded certificates";
    return;
  }

  // Try to load certificates from app data directory (optional override)
  if (LoadCertificatesFromAppData()) {
    LOG(INFO) << "mTLS proxy certificates loaded from files";
    SetEnabled(true);
  } else {
    LOG(INFO) << "No certificate files found, using hardcoded certificates";
    // Don't disable - hardcoded certs are already loaded
  }
}

// static
void MtlsProxyInitializer::ConfigureSSLClientContext(
    net::SSLClientContext* ssl_client_context,
    const net::HostPortPair& proxy_endpoint) {
  
  // NOTE: We do NOT call SetClientCertificate() here!
  // This would globally cache the proxy certificate for eb.wootzapp.com:443
  // and interfere with direct connections to eb.wootzapp.com/okta (which uses DIC cert)
  //
  // Instead, the proxy certificate is provided per-connection via SSLConfig
  // in HttpProxyConnectJob::DoHttpProxyConnect()
  //
  // This allows:
  // 1. Proxy connections → use hardcoded device-123 cert (via ssl_config)
  // 2. Direct Okta connections → use DIC cert (via SelectClientCertificate)
  
  LOG(INFO) << "mTLS proxy certificate will be provided per-connection for: "
            << proxy_endpoint.ToString();
}

// static
bool MtlsProxyInitializer::LoadCertificatesFromAppData() {
#if BUILDFLAG(IS_ANDROID)
  // On Android, certificates are stored in the app's private data directory
  base::FilePath data_dir;
  if (!base::android::GetDataDirectory(&data_dir)) {
    LOG(ERROR) << "Failed to get Android data directory";
    return false;
  }
  
  base::FilePath cert_dir = data_dir.Append("mtls_certs");
#else
  // On other platforms, use the user data directory
  base::FilePath user_data_dir;
  if (!base::PathService::Get(chrome::DIR_USER_DATA, &user_data_dir)) {
    LOG(ERROR) << "Failed to get user data directory";
    return false;
  }
  
  base::FilePath cert_dir = user_data_dir.Append("mtls_certs");
#endif

  base::FilePath cert_path = cert_dir.Append(kClientCertFileName);
  base::FilePath key_path = cert_dir.Append(kClientKeyFileName);
  base::FilePath ca_path = cert_dir.Append(kCACertFileName);

  // Check if certificate files exist
  if (!base::PathExists(cert_path)) {
    LOG(WARNING) << "Client certificate not found at: " << cert_path;
    return false;
  }

  if (!base::PathExists(key_path)) {
    LOG(WARNING) << "Client key not found at: " << key_path;
    return false;
  }

  return LoadCertificates(cert_path, key_path, ca_path);
}

// static
bool MtlsProxyInitializer::LoadCertificates(const base::FilePath& cert_path,
                                             const base::FilePath& key_path,
                                             const base::FilePath& ca_path) {
  net::MtlsProxyConfig* config = net::MtlsProxyConfig::GetInstance();
  if (!config) {
    LOG(ERROR) << "Failed to get MtlsProxyConfig instance";
    return false;
  }

  bool success = config->LoadFromFiles(cert_path, key_path, ca_path);
  if (success) {
    LOG(INFO) << "Successfully loaded mTLS certificates";
    LOG(INFO) << "  Cert: " << cert_path;
    LOG(INFO) << "  Key:  " << key_path;
    if (!ca_path.empty() && base::PathExists(ca_path)) {
      LOG(INFO) << "  CA:   " << ca_path;
    }
  } else {
    LOG(ERROR) << "Failed to load mTLS certificates";
  }

  return success;
}

// static
void MtlsProxyInitializer::SetEnabled(bool enabled) {
  net::MtlsProxyConfig* config = net::MtlsProxyConfig::GetInstance();
  if (config) {
    config->SetEnabled(enabled);
  }
}

// static
bool MtlsProxyInitializer::IsEnabled() {
  net::MtlsProxyConfig* config = net::MtlsProxyConfig::GetInstance();
  return config && config->IsEnabled();
}

