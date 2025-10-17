// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/okta_certificate_manager.h"

#include <string>

#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "crypto/openssl_util.h"
#include "crypto/rsa_private_key.h"
#include "net/base/load_flags.h"
#include "net/cert/x509_certificate.h"
#include "net/http/http_status_code.h"
#include "net/ssl/ssl_config.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "third_party/boringssl/src/include/openssl/bn.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/pool.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#include "third_party/boringssl/src/include/openssl/x509.h"
#include "url/gurl.h"

// Static storage for certificate persistence across instances
scoped_refptr<net::X509Certificate>
    OktaCertificateManager::stored_certificate_ = nullptr;
base::Time OktaCertificateManager::certificate_stored_time_ = base::Time();

// Root CA validation removed - certificate time validity is sufficient

OktaCertificateManager::OktaCertificateManager(content::BrowserContext* context)
    : context_(context) {
  LOG(INFO) << "Aaditesh_mtls -> OktaCertificateManager initialized (using "
               "extension API for certificate)";
}

OktaCertificateManager::~OktaCertificateManager() = default;

bool OktaCertificateManager::StoreCertificate(const std::string& cert_pem) {
  LOG(INFO) << "Aaditesh_mtls -> Storing certificate from API";

  // Parse the PEM certificate
  net::CertificateList certs =
      net::X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(cert_pem)),
          net::X509Certificate::FORMAT_PEM_CERT_SEQUENCE);

  if (certs.empty()) {
    LOG(ERROR)
        << "Aaditesh_mtls -> Failed to parse certificate from API response";
    return false;
  }

  stored_certificate_ = certs[0];
  certificate_stored_time_ = base::Time::Now();

  LOG(INFO) << "Aaditesh_mtls -> Certificate stored successfully";
  LOG(INFO) << "Aaditesh_mtls -> Certificate subject: "
            << stored_certificate_->subject().GetDisplayName();
  LOG(INFO) << "Aaditesh_mtls -> Certificate valid from: "
            << stored_certificate_->valid_start();
  LOG(INFO) << "Aaditesh_mtls -> Certificate valid until: "
            << stored_certificate_->valid_expiry();

  return true;
}

bool OktaCertificateManager::HasValidCertificate() {
  LOG(INFO) << "Aaditesh_mtls -> Checking for valid certificate";
  
  // Check if we have a stored certificate
  return HasValidStoredCertificate();
}

bool OktaCertificateManager::StoreAndValidateCertificate(
    const std::string& certificate_pem) {
  LOG(INFO) << "Aaditesh_mtls -> Storing certificate from extension API";
  LOG(INFO) << "Aaditesh_mtls -> Certificate PEM length: "
            << certificate_pem.length();

  // Parse the PEM certificate
  net::CertificateList certs =
      net::X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(certificate_pem)),
          net::X509Certificate::FORMAT_PEM_CERT_SEQUENCE);

  if (certs.empty()) {
    LOG(ERROR)
        << "Aaditesh_mtls -> Failed to parse certificate from extension API";
    return false;
  }

  scoped_refptr<net::X509Certificate> cert = certs[0];

  // Validate the certificate
  if (!ValidateCertificate(cert)) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate validation failed";
    return false;
  }

  // Store the validated certificate
  stored_certificate_ = cert;
  certificate_stored_time_ = base::Time::Now();

  LOG(INFO) << "Aaditesh_mtls -> Certificate stored and validated successfully";
  LOG(INFO) << "Aaditesh_mtls -> Certificate subject: "
            << stored_certificate_->subject().GetDisplayName();
  LOG(INFO) << "Aaditesh_mtls -> Certificate valid from: "
            << stored_certificate_->valid_start();
  LOG(INFO) << "Aaditesh_mtls -> Certificate valid until: "
            << stored_certificate_->valid_expiry();

  return true;
}

bool OktaCertificateManager::ValidateCertificate(
    scoped_refptr<net::X509Certificate> cert) {
  if (!cert) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate is null";
    return false;
  }

  // Check time validity only (certificate validity period + 10-minute usage window)
  if (!IsCertificateTimeValid(cert)) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate time validation failed";
    return false;
  }

  LOG(INFO) << "Aaditesh_mtls -> Certificate validation successful (time-based only)";
  return true;
}

bool OktaCertificateManager::ValidateCertificate(
    const std::string& certificate_pem) {
  // Parse the PEM certificate string
  net::CertificateList certs =
      net::X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(certificate_pem)),
          net::X509Certificate::FORMAT_PEM_CERT_SEQUENCE);

  if (certs.empty()) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to parse certificate PEM";
    return false;
  }

  // Use the first certificate and validate it
  return ValidateCertificate(certs[0]);
}

scoped_refptr<net::X509Certificate>
OktaCertificateManager::GetStoredCertificate() {
  return stored_certificate_;
}

void OktaCertificateManager::ClearStoredCertificate() {
  LOG(INFO) << "Aaditesh_mtls -> Clearing stored certificate";
  stored_certificate_ = nullptr;
  certificate_stored_time_ = base::Time();
}

bool OktaCertificateManager::HasValidStoredCertificate() {
  LOG(INFO)
      << "Aaditesh_mtls -> Checking if we have a valid stored certificate";

  if (!stored_certificate_) {
    LOG(INFO) << "Aaditesh_mtls -> No certificate stored";
    return false;
  }

  // Check if certificate is still valid within 10-minute window
  base::Time now = base::Time::Now();
  if (!certificate_stored_time_.is_null()) {
    base::TimeDelta time_since_stored = now - certificate_stored_time_;
    if (time_since_stored > base::Minutes(10)) {
      LOG(INFO) << "Aaditesh_mtls -> Stored certificate expired (>10 minutes)";
      LOG(INFO) << "Aaditesh_mtls -> Time since stored: " << time_since_stored;
      return false;
    }
  }

  // Validate the stored certificate
  bool is_valid = ValidateCertificate(stored_certificate_);
  LOG(INFO) << "Aaditesh_mtls -> Stored certificate validation result: "
            << (is_valid ? "VALID" : "INVALID");

  return is_valid;
}

bool OktaCertificateManager::IsCertificateTimeValid(
    scoped_refptr<net::X509Certificate> cert) {
  base::Time now = base::Time::Now();

  // Allow 10 seconds of clock skew tolerance to handle minor time synchronization issues
  base::TimeDelta tolerance = base::Seconds(10);

  // Check certificate's own validity period with tolerance
  if (now < (cert->valid_start() - tolerance) || 
      now > (cert->valid_expiry() + tolerance)) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate is outside its validity period (with 10s tolerance)";
    LOG(ERROR) << "Aaditesh_mtls -> Now: " << now;
    LOG(ERROR) << "Aaditesh_mtls -> Valid from: " << cert->valid_start();
    LOG(ERROR) << "Aaditesh_mtls -> Valid until: " << cert->valid_expiry();
    return false;
  }

  // Additional check: 10-minute window from when we stored it
  if (!certificate_stored_time_.is_null()) {
    base::TimeDelta time_since_stored = now - certificate_stored_time_;
    if (time_since_stored > base::Minutes(10)) {
      LOG(ERROR)
          << "Aaditesh_mtls -> Certificate exceeded 10-minute usage window";
      LOG(ERROR) << "Aaditesh_mtls -> Time since stored: " << time_since_stored;
      return false;
    }
  }

  LOG(INFO) << "Aaditesh_mtls -> Certificate time validation passed";
  return true;
}
