// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_OKTA_CERTIFICATE_MANAGER_H_
#define CHROME_BROWSER_NET_OKTA_CERTIFICATE_MANAGER_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"
#include "base/memory/scoped_refptr.h"
#include "base/time/time.h"
#include "crypto/rsa_private_key.h"
#include "net/cert/x509_certificate.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace content {
class BrowserContext;
}

class OktaCertificateManager {
 public:
  explicit OktaCertificateManager(content::BrowserContext* context);
  ~OktaCertificateManager();

  // Store a certificate received from the Okta API
  bool StoreCertificate(const std::string& cert_pem);

  // Check if we have a valid certificate in storage (synchronous for now)
  bool HasValidCertificate();

  // Store certificate received from extension API (chrome.wootz.mtlsCert)
  bool StoreAndValidateCertificate(const std::string& certificate_pem);

  // Validate certificate against root CA and time constraints
  bool ValidateCertificate(scoped_refptr<net::X509Certificate> cert);
  bool ValidateCertificate(const std::string& certificate_pem);

  // Get the stored certificate if available
  scoped_refptr<net::X509Certificate> GetStoredCertificate();

  // Clear the stored certificate
  void ClearStoredCertificate();

  // Check if we have a valid certificate that hasn't expired (10 min window)
  bool HasValidStoredCertificate();

 private:
  // Load the hardcoded root CA certificate
  scoped_refptr<net::X509Certificate> LoadRootCACertificate();

  // Validate certificate against the root CA
  bool ValidateAgainstRootCA(scoped_refptr<net::X509Certificate> cert);

  // Check if certificate is within its time validity (includes 10-minute
  // window)
  bool IsCertificateTimeValid(scoped_refptr<net::X509Certificate> cert);

  // Hardcoded root CA PEM
  static const char kRootCAPEM[];

  content::BrowserContext* context_;

  // Static storage for certificate persistence across instances
  static scoped_refptr<net::X509Certificate> stored_certificate_;
  static base::Time certificate_stored_time_;
};

#endif  // CHROME_BROWSER_NET_OKTA_CERTIFICATE_MANAGER_H_
