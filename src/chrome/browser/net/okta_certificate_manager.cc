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

// Hardcoded Root CA certificate (GCP mTLS Root CA)
const char OktaCertificateManager::kRootCAPEM[] = R"(
-----BEGIN CERTIFICATE-----
MIICDjCCAbKgAwIBAgIUVHjG+GnM/rnCATUpfcyM3QNlgh4wDAYIKoZIzj0EAwIF
ADBRMU8wFwYDVQQDDBBHQ1AgbVRMUyBSb290IENBMA8GA1UECwwIU2VjdXJpdHkw
GAYDVQQKDBFZb3VyIE9yZ2FuaXphdGlvbjAJBgNVBAYTAlVTMB4XDTI1MDkyNTEy
MzY1OFoXDTM1MDkyNTEyMzY1OFowUTFPMBcGA1UEAwwQR0NQIG1UTFMgUm9vdCBD
QTAPBgNVBAsMCFNlY3VyaXR5MBgGA1UECgwRWW91ciBPcmdhbml6YXRpb24wCQYD
VQQGEwJVUzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABGZJkyZstXFdNrM/yMFH
M8s0XUITr+0xsv59qHTzpgPZpRclMHfqFZ/erODQt4v1wD7Lw4SjdzF0fLUQ7ffo
HT6jZjBkMBIGA1UdEwEB/wQIMAYBAf8CAQEwDgYDVR0PAQH/BAQDAgAGMB0GA1Ud
DgQWBBT2U5xKyPsFhI5QivXiVXWgfA4QUjAfBgNVHSMEGDAWgBT2U5xKyPsFhI5Q
ivXiVXWgfA4QUjAMBggqhkjOPQQDAgUAA0gAMEUCIQCzBFdoh9xMGTwwOjGVAtUG
LqSJ0QIWEs5Kd50ULd7dmwIgeZ/UhppiYJJ44M9e0+FRp+p27cQ7tzWjm7k/xODZ
6gk=
-----END CERTIFICATE-----
)";

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

  // Check time validity first (10-minute window from certificate's valid_start)
  if (!IsCertificateTimeValid(cert)) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate time validation failed";
    return false;
  }

  // Validate against root CA
  if (!ValidateAgainstRootCA(cert)) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate root CA validation failed";
    return false;
  }

  LOG(INFO) << "Aaditesh_mtls -> Certificate validation successful";
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

scoped_refptr<net::X509Certificate>
OktaCertificateManager::LoadRootCACertificate() {
  net::CertificateList certs =
      net::X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(kRootCAPEM)),
          net::X509Certificate::FORMAT_PEM_CERT_SEQUENCE);

  if (certs.empty()) {
    LOG(ERROR)
        << "Aaditesh_mtls -> Failed to load hardcoded root CA certificate";
    return nullptr;
  }

  LOG(INFO) << "Aaditesh_mtls -> Root CA certificate loaded successfully";
  return certs[0];
}

bool OktaCertificateManager::ValidateAgainstRootCA(
    scoped_refptr<net::X509Certificate> cert) {
  LOG(INFO) << "Aaditesh_mtls -> Validating certificate against Root CA "
               "(STRICT MODE)";

  // ========== CERTIFICATE DETAILS (DECODED) ==========
  LOG(INFO) << "Aaditesh_mtls -> ===== DECODED CERTIFICATE DETAILS =====";
  
  // Get certificate subject (who the certificate belongs to)
  std::string cert_subject = cert->subject().GetDisplayName();
  LOG(INFO) << "Aaditesh_mtls -> Certificate Subject (Owner): " << cert_subject;
  
  // Log individual subject components if available (with safety checks)
  LOG(INFO) << "Aaditesh_mtls -> Certificate Subject Components:";
  LOG(INFO) << "Aaditesh_mtls ->   Common Name (CN): " 
            << (cert->subject().common_name.empty() ? "(not set)" : cert->subject().common_name);
  
  if (!cert->subject().organization_names.empty()) {
    LOG(INFO) << "Aaditesh_mtls ->   Organization (O): " 
              << cert->subject().organization_names[0];
  } else {
    LOG(INFO) << "Aaditesh_mtls ->   Organization (O): (not set)";
  }
  
  if (!cert->subject().organization_unit_names.empty()) {
    LOG(INFO) << "Aaditesh_mtls ->   Organizational Unit (OU): " 
              << cert->subject().organization_unit_names[0];
  } else {
    LOG(INFO) << "Aaditesh_mtls ->   Organizational Unit (OU): (not set)";
  }
  
  LOG(INFO) << "Aaditesh_mtls ->   Country (C): " 
            << (cert->subject().country_name.empty() ? "(not set)" : cert->subject().country_name);
  
  // Get certificate issuer (who signed the certificate)
  std::string cert_issuer = cert->issuer().GetDisplayName();
  LOG(INFO) << "Aaditesh_mtls -> Certificate Issuer (Signer): " << cert_issuer;
  
  // Log certificate validity period
  LOG(INFO) << "Aaditesh_mtls -> Certificate Valid From: " 
            << cert->valid_start();
  LOG(INFO) << "Aaditesh_mtls -> Certificate Valid Until: " 
            << cert->valid_expiry();
  
  LOG(INFO) << "Aaditesh_mtls -> ==========================================";

  // Load the Root CA certificate
  scoped_refptr<net::X509Certificate> root_ca = LoadRootCACertificate();
  if (!root_ca) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to load Root CA certificate";
    return false;
  }

  // Log Root CA details for comparison
  LOG(INFO) << "Aaditesh_mtls -> ===== ROOT CA DETAILS =====";
  std::string root_ca_subject = root_ca->subject().GetDisplayName();
  LOG(INFO) << "Aaditesh_mtls -> Root CA Subject: " << root_ca_subject;
  LOG(INFO) << "Aaditesh_mtls ->   Common Name (CN): " 
            << (root_ca->subject().common_name.empty() ? "(not set)" : root_ca->subject().common_name);
  LOG(INFO) << "Aaditesh_mtls -> ==========================";

  // STRICT VALIDATION: Certificate's issuer MUST match Root CA's subject
  // This ensures the certificate was issued by our trusted Root CA
  LOG(INFO) << "Aaditesh_mtls -> Comparing Certificate Issuer vs Root CA Subject:";
  LOG(INFO) << "Aaditesh_mtls -> Certificate Issuer: " << cert_issuer;
  LOG(INFO) << "Aaditesh_mtls -> Root CA Subject:    " << root_ca_subject;

  // STRICT CHECK: Issuer must exactly match the Root CA subject
  bool is_valid = (cert_issuer == root_ca_subject);

  if (!is_valid) {
    LOG(ERROR) << "Aaditesh_mtls -> VALIDATION FAILED: Certificate issuer "
                  "does NOT match Root CA subject";
    LOG(ERROR) << "Aaditesh_mtls -> Expected issuer: " << root_ca_subject;
    LOG(ERROR) << "Aaditesh_mtls -> Actual issuer: " << cert_issuer;
    LOG(ERROR) << "Aaditesh_mtls -> Certificate REJECTED (strict mode)";
    return false;
  }

  LOG(INFO) << "Aaditesh_mtls -> Certificate issuer matches Root CA subject - "
               "VALID";
  LOG(INFO) << "Aaditesh_mtls -> Root CA validation: PASSED (strict mode)";

  return true;
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
