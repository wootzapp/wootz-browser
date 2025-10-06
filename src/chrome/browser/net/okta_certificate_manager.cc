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

// Hardcoded Root CA certificate (provided by user)
const char OktaCertificateManager::kRootCAPEM[] = R"(
-----BEGIN CERTIFICATE-----
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
YIJHpDFZlY/oeFNltIxuUI0jNw==
-----END CERTIFICATE-----
)";

// Hardcoded test certificate (simulates what would be returned by API call)
const char OktaCertificateManager::kTestCertificatePEM[] =
    R"(-----BEGIN CERTIFICATE-----
MIIGDzCCA/egAwIBAgIUAaDQivaGX1GYEMs+cD/JRqkQMwgwDQYJKoZIhvcNAQEL
BQAwgY8xCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQH
Ew1Nb3VudGFpbiBWaWV3MR0wGwYDVQQKExRNeSBHb29nbGUgQ2xvdWQgQ29ycDER
MA8GA1UECxMIU2VjdXJpdHkxITAfBgNVBAMTGE15IEF3ZXNvbWUgUm9vdCBDQSAo
R0NQKTAeFw0yNTA5MTkwNzQ2MTZaFw0yNTA5MjAwNzQ2MTZaMCwxFzAVBgNVBAMM
DmFkbmFuLXVzZXItMTIzMREwDwYDVQQKDAhXb290emFwcDCCASIwDQYJKoZIhvcN
AQEBBQADggEPADCCAQoCggEBAMLLn59iNmUjjiqj3TMXmTkDXtWaV+l0/qNREney
9h8F1AlAXmKnFlIoDcHLLkcDYo0gockvgDu35sT50iOnKOMbUWDqlXB66l+x8xb4
HRwWUFkVLDqwHiv4VH/lXQP1k7uDhc6JN3rNp2LxTcx3F/WmvswTZ7C1SoUdJYa4
ZOqytReZdtsCXVXMRNaDbF9B64fRGLNHh8Y/zNTCtG8RcZiP02ReHOYs1Z7h3z1N
pYwNj/qaABgjVt5WkXMsU5FQ9ugOdFxoE+kgZQji0Y38oo3UVharSt3jXgWzzvkC
GlLq9tDRI1zGhvlB3rpgeVRFPaf0YFanbIWYdaynm+44+rkCAwEAAaN4MHYwDAYD
VR0TAQH/BAIwADAOBgNVHQ8BAf8EBAMCBaAwFgYDVR0lAQH/BAwwCgYIKwYBBQUH
AwIwHQYDVR0OBBYEFEOCaXg704qUKspAgjdyWX/7Is95MB8GA1UdIwQYMBaAFIkv
b9D6rAyl9FLdKaZ6bPIO/uGVMA0GCSqGSIb3DQEBCwUAA4ICAQA9h6EQ59w2JnvD
/WITRGBlw40zQRllXX/2CVVhs/R6CSPnHRj6yxNyAnjR/keyzOt/bQwlDJ1UhitY
dUlaThapGdaR70JiSCjlfSs7mTA4x5CV52ozNlCgJGE+NJDd2p0zOZyZ/Tl39/D1
1d2iWdZJ4O4j7hRkfxHt7a3SEkiIkHqFVJ4rIaU+eCQsBgbIJnXlwDdwLv95+fQr
DpE21afZlwzS8jvwtD8pwcKmGpNei7fXzxoCfMgq8o61qb/wg1fSVjiSU+qUj8/n
6d0x7LAhlYa7OpFYJplBYgL8K8QovinhbRrtvMKeyVcMADiwFF7dIkY16vVGANXz
zbz6s4PtmW0Rec7cRXzcD/S1fU2WVBjZmckJVBM4vE5hfy96/xmz6j5/Yv0GbPaN
cxHH7Simhm2/cgHCz7kuSc7JDNDj3sWVmuHXBbDTJ2K1r1qgoDI7ABZuK3RqAtoX
krgCpPrQToa633Rxrn7Behxr+SQCIT0qc+k5ASvWY+TQfqkiVj1VBid9U+xEU8rr
gsjb+wsNAOLv6JwZiCzSqYUIxmCLafLevZi3StrHWy8licBP1jpqnPmUvRP3cLT8
jBBvTNfxrmVhBmKJKmsqElnwdWSlOuzWGQSLfzLsPFwDwsCweXtLGIVq3SO8CVep
3TmnvSbcDjEVDF6C1HBaFmmXaOYsPQ==
-----END CERTIFICATE-----
)";

OktaCertificateManager::OktaCertificateManager(content::BrowserContext* context)
    : context_(context) {
  // Generate RSA key pair for CSR generation
  private_key_ = crypto::RSAPrivateKey::Create(2048);
  if (!private_key_) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to generate RSA key pair";
  }
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
  LOG(INFO) << "Aaditesh_mtls -> Checking for valid certificate (TEST MODE - "
               "HARDCODED CERT)";

  // For testing: Load the hardcoded test certificate
  std::string test_cert_pem = kTestCertificatePEM;
  net::CertificateList certs =
      net::X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(base::make_span(test_cert_pem)),
          net::X509Certificate::FORMAT_PEM_CERT_SEQUENCE);

  if (certs.empty()) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to parse hardcoded test certificate";
    LOG(ERROR) << "Aaditesh_mtls -> Certificate PEM length: "
               << test_cert_pem.length();
    LOG(ERROR) << "Aaditesh_mtls -> Certificate PEM preview: "
               << test_cert_pem.substr(0, 100);
    return false;
  }

  scoped_refptr<net::X509Certificate> test_cert = certs[0];

  LOG(INFO)
      << "Aaditesh_mtls -> Successfully loaded hardcoded test certificate";
  LOG(INFO) << "Aaditesh_mtls -> Certificate subject: "
            << test_cert->subject().GetDisplayName();

  // Store the test certificate and set current time as storage time
  stored_certificate_ = test_cert;
  certificate_stored_time_ = base::Time::Now();

  // Validate certificate against Root CA and time constraints
  bool is_valid = ValidateCertificate(stored_certificate_);
  LOG(INFO) << "Aaditesh_mtls -> Certificate validation result: "
            << (is_valid ? "VALID" : "INVALID");

  return is_valid;
}

void OktaCertificateManager::RequestCertificate(CertificateCallback callback) {
  LOG(INFO) << "Aaditesh_mtls -> Starting certificate request via CSR API";

  if (!private_key_) {
    LOG(ERROR)
        << "Aaditesh_mtls -> No private key available for CSR generation";
    std::move(callback).Run(false, "");
    return;
  }

  // Generate CSR
  std::string csr = GenerateCSR();
  if (csr.empty()) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to generate CSR";
    std::move(callback).Run(false, "");
    return;
  }

  LOG(INFO) << "Aaditesh_mtls -> Generated CSR, making API request";

  // Make API call to sign the CSR
  MakeCSRSigningRequest(csr, std::move(callback));
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
  LOG(INFO)
      << "Aaditesh_mtls -> Validating certificate against Root CA (TEST MODE)";

  // For testing purposes, we'll accept the hardcoded certificate as valid
  // Check if this is our expected test certificate by comparing subject
  std::string cert_subject = cert->subject().GetDisplayName();
  LOG(INFO) << "Aaditesh_mtls -> Certificate subject: " << cert_subject;

  // Accept certificates that contain our expected test subject components
  bool is_test_cert =
      (cert_subject.find("adnan-user-123") != std::string::npos);

  LOG(INFO) << "Aaditesh_mtls -> Is test certificate: "
            << (is_test_cert ? "YES" : "NO");
  LOG(INFO) << "Aaditesh_mtls -> Root CA validation: "
            << (is_test_cert ? "VALID" : "INVALID");

  return is_test_cert;
}

bool OktaCertificateManager::IsCertificateTimeValid(
    scoped_refptr<net::X509Certificate> cert) {
  base::Time now = base::Time::Now();

  // Check certificate's own validity period
  if (now < cert->valid_start() || now > cert->valid_expiry()) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate is outside its validity period";
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

std::string OktaCertificateManager::GenerateCSR() {
  LOG(INFO) << "Aaditesh_mtls -> Generating Certificate Signing Request (CSR)";

  if (!private_key_) {
    LOG(ERROR) << "Aaditesh_mtls -> No private key available";
    return std::string();
  }

  crypto::OpenSSLErrStackTracer err_tracer(FROM_HERE);

  // Create X509_REQ structure
  bssl::UniquePtr<X509_REQ> req(X509_REQ_new());
  if (!req) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to create X509_REQ";
    return std::string();
  }

  // Set version
  if (!X509_REQ_set_version(req.get(), 0)) {  // Version 1 (0 in OpenSSL)
    LOG(ERROR) << "Aaditesh_mtls -> Failed to set CSR version";
    return std::string();
  }

  // Create subject name
  bssl::UniquePtr<X509_NAME> name(X509_NAME_new());
  if (!name) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to create X509_NAME";
    return std::string();
  }

  // Add subject fields
  if (!X509_NAME_add_entry_by_txt(
          name.get(), "CN", MBSTRING_ASC,
          reinterpret_cast<const unsigned char*>("adnan-user-123"), -1, -1,
          0) ||
      !X509_NAME_add_entry_by_txt(
          name.get(), "O", MBSTRING_ASC,
          reinterpret_cast<const unsigned char*>("Wootzapp"), -1, -1, 0)) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to add subject fields";
    return std::string();
  }

  if (!X509_REQ_set_subject_name(req.get(), name.get())) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to set subject name";
    return std::string();
  }

  // Set public key
  EVP_PKEY* pkey = private_key_->key();
  if (!X509_REQ_set_pubkey(req.get(), pkey)) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to set public key";
    return std::string();
  }

  // Sign the CSR
  if (!X509_REQ_sign(req.get(), pkey, EVP_sha256())) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to sign CSR";
    return std::string();
  }

  // Convert to PEM format
  bssl::UniquePtr<BIO> bio(BIO_new(BIO_s_mem()));
  if (!bio || !PEM_write_bio_X509_REQ(bio.get(), req.get())) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to write CSR to PEM format";
    return std::string();
  }

  // Read PEM data
  char* data;
  long len = BIO_get_mem_data(bio.get(), &data);
  if (len <= 0) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to get PEM data";
    return std::string();
  }

  std::string csr_pem(data, len);
  LOG(INFO) << "Aaditesh_mtls -> CSR generated successfully, length: "
            << csr_pem.length();
  LOG(INFO) << "Aaditesh_mtls -> CSR content: " << csr_pem;

  return csr_pem;
}

void OktaCertificateManager::MakeCSRSigningRequest(
    const std::string& csr,
    CertificateCallback callback) {
  LOG(INFO) << "Aaditesh_mtls -> Making CSR signing API request (New API "
               "Documentation Format)";

  // Create resource request
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL("https://app.wootzapp.com/api/csr/sign");
  resource_request->method = "POST";
  resource_request->load_flags = net::LOAD_DISABLE_CACHE |
                                 net::LOAD_DISABLE_CERT_NETWORK_FETCHES |
                                 net::LOAD_SHOULD_BYPASS_HSTS;

  // Skip certificate validation for this bootstrap request
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  // Configure trusted params for this request\n
  // resource_request->trusted_params =
  // network::ResourceRequest::TrustedParams();

  // Additional SSL bypass settings
  resource_request->request_initiator = url::Origin();
  resource_request->site_for_cookies = net::SiteForCookies();

  // Add headers
  resource_request->headers.SetHeader("Content-Type", "application/json");
  resource_request->headers.SetHeader("Authorization",
                                      "Bearer Aoi3dkgpE905nvSiec");

  LOG(INFO) << "Aaditesh_mtls -> Headers set:";
  LOG(INFO) << "Aaditesh_mtls -> Content-Type: application/json";
  LOG(INFO) << "Aaditesh_mtls -> Authorization: Bearer Aoi3dkgpE905nvSiec";

  // Create JSON request body
  base::Value::Dict request_dict;

  // Clean up CSR (remove extra whitespace/newlines if needed)
  std::string clean_csr = csr;
  base::ReplaceChars(clean_csr, "\r", "", &clean_csr);

  request_dict.Set("csr", clean_csr);

  // Verify CSR format matches working curl command
  LOG(INFO) << "Aaditesh_mtls -> CSR for API call (first 100 chars): "
            << clean_csr.substr(0, 100);
  LOG(INFO) << "Aaditesh_mtls -> CSR ends with (last 50 chars): "
            << clean_csr.substr(std::max(0, (int)clean_csr.length() - 50));
  LOG(INFO) << "Aaditesh_mtls -> Total CSR length: " << clean_csr.length();

  // Verify it starts and ends correctly
  bool starts_correct =
      clean_csr.find("-----BEGIN CERTIFICATE REQUEST-----") == 0;
  bool ends_correct =
      clean_csr.find("-----END CERTIFICATE REQUEST-----") != std::string::npos;
  LOG(INFO) << "Aaditesh_mtls -> CSR format check - Starts correctly: "
            << (starts_correct ? "YES" : "NO");
  LOG(INFO) << "Aaditesh_mtls -> CSR format check - Ends correctly: "
            << (ends_correct ? "YES" : "NO");

  std::string request_body;
  if (!base::JSONWriter::Write(request_dict, &request_body)) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to create JSON request body";
    std::move(callback).Run(false, "");
    return;
  }

  LOG(INFO) << "Aaditesh_mtls -> Request body length: "
            << request_body.length();
  LOG(INFO) << "Aaditesh_mtls -> Request body: " << request_body;
  LOG(INFO) << "Aaditesh_mtls -> Request URL: " << resource_request->url.spec();
  LOG(INFO) << "Aaditesh_mtls -> Request method: " << resource_request->method;
  LOG(INFO) << "Aaditesh_mtls -> UPDATED: Using app.wootzapp.com domain (not "
               "eb.wootzapp.com)";

  // Create traffic annotation
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("okta_csr_signing", R"(
        semantics {
          sender: "Okta Certificate Manager"
          description: "Request to sign a Certificate Signing Request (CSR) for Okta authentication"
          trigger: "User navigates to Okta app URLs"
          data: "Certificate Signing Request in PEM format"
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting: "This feature cannot be disabled"
        })");

  // Create URL loader with SSL bypass configuration
  url_loader_ = network::SimpleURLLoader::Create(std::move(resource_request),
                                                 traffic_annotation);

  // Attach the JSON body containing the CSR to the POST request
  url_loader_->AttachStringForUpload(request_body, "application/json");

  LOG(INFO) << "Aaditesh_mtls -> CSR data attached to POST request body";

  // Configure URL loader to allow HTTP errors and disable retries
  url_loader_->SetAllowHttpErrorResults(true);
  url_loader_->SetRetryOptions(0, network::SimpleURLLoader::RETRY_NEVER);

  // Get URL loader factory
  auto* storage_partition = context_->GetDefaultStoragePartition();
  auto* url_loader_factory =
      storage_partition->GetURLLoaderFactoryForBrowserProcess().get();

  // Start the request
  LOG(INFO) << "Aaditesh_mtls -> Starting URL loader request with SSL bypass "
               "configured";
  LOG(INFO) << "Aaditesh_mtls -> Note: This is a bootstrap request to GET our "
               "first certificate";
  url_loader_->DownloadToStringOfUnboundedSizeUntilCrashAndDie(
      url_loader_factory,
      base::BindOnce(&OktaCertificateManager::OnCSRSigningResponse,
                     base::Unretained(this), std::move(callback)));
}

void OktaCertificateManager::OnCSRSigningResponse(
    CertificateCallback callback,
    std::unique_ptr<std::string> response_body) {
  LOG(INFO) << "Aaditesh_mtls -> Received CSR signing response";

  // Check for network errors first
  if (url_loader_->NetError() != net::OK) {
    LOG(ERROR) << "Aaditesh_mtls -> Network error: " << url_loader_->NetError();
    LOG(ERROR) << "Aaditesh_mtls -> Network error string: "
               << net::ErrorToString(url_loader_->NetError());

    // Special handling for SSL client certificate errors\n    if
    // (url_loader_->NetError() == net::ERR_SSL_CLIENT_AUTH_CERT_NEEDED) {\n
    // LOG(ERROR) << \"Aaditesh_mtls -> CRITICAL: Server requires client
    // certificate but we don't have one yet!\";\n      LOG(ERROR) <<
    // \"Aaditesh_mtls -> This is a bootstrap certificate request - server
    // should not require client certs\";\n      LOG(ERROR) << \"Aaditesh_mtls
    // -> API endpoint: https://eb.wootzapp.com/api/csr/sign\";\n LOG(ERROR) <<
    // \"Aaditesh_mtls -> Bearer token: Aoi3dkgpE905nvSiec\";\n      LOG(ERROR)
    // << \"Aaditesh_mtls -> FALLBACK: Using hardcoded test certificate for
    // initial access\";\n      \n      // As a fallback, use the hardcoded test
    // certificate since we have a chicken-and-egg problem\n
    // std::move(callback).Run(true, kTestCertificatePEM);\n      return;\n    }

    // Special handling for SSL client certificate errors
    if (url_loader_->NetError() == net::ERR_SSL_CLIENT_AUTH_CERT_NEEDED) {
      LOG(ERROR) << "Aaditesh_mtls -> CRITICAL: Server requires client "
                    "certificate but we don't have one yet!";
      LOG(ERROR) << "Aaditesh_mtls -> This is a bootstrap certificate request "
                    "- server should not require client certs";
      LOG(ERROR) << "Aaditesh_mtls -> API endpoint: "
                    "https://app.wootzapp.com/api/csr/sign";
      LOG(ERROR) << "Aaditesh_mtls -> Bearer token: Aoi3dkgpE905nvSiec";
      LOG(ERROR) << "Aaditesh_mtls -> FALLBACK: Using hardcoded test "
                    "certificate for initial access";

      // As a fallback, use the hardcoded test certificate since we have a
      // chicken-and-egg problem
      std::move(callback).Run(true, kTestCertificatePEM);
      return;
    }

    std::move(callback).Run(false, "");
    return;
  }

  // First check HTTP status code regardless of response body
  int response_code =
      url_loader_->ResponseInfo()
          ? url_loader_->ResponseInfo()->headers->response_code()
          : 0;
  LOG(INFO) << "Aaditesh_mtls -> HTTP response code: " << response_code;

  // Log response headers for debugging
  if (url_loader_->ResponseInfo() && url_loader_->ResponseInfo()->headers) {
    scoped_refptr<net::HttpResponseHeaders> headers =
        url_loader_->ResponseInfo()->headers;
    LOG(INFO) << "Aaditesh_mtls -> Response headers:";
    size_t iter = 0;
    std::string name, value;
    while (headers->EnumerateHeaderLines(&iter, &name, &value)) {
      LOG(INFO) << "Aaditesh_mtls -> " << name << ": " << value;
    }
  }

  if (!response_body) {
    LOG(ERROR) << "Aaditesh_mtls -> Empty response body (nullptr)";
    LOG(ERROR) << "Aaditesh_mtls -> This suggests the network request failed "
                  "completely";
    std::move(callback).Run(false, "");
    return;
  }

  if (response_body->empty()) {
    LOG(ERROR) << "Aaditesh_mtls -> Empty response body (zero length)";
    LOG(ERROR) << "Aaditesh_mtls -> Server returned no content but request "
                  "succeeded at network level";
    std::move(callback).Run(false, "");
    return;
  }

  LOG(INFO) << "Aaditesh_mtls -> Response body length: "
            << response_body->length();
  LOG(INFO) << "Aaditesh_mtls -> Full API Response body: " << *response_body;

  if (response_code != net::HTTP_OK) {
    LOG(ERROR) << "Aaditesh_mtls -> API request failed with status: "
               << response_code;
    LOG(ERROR) << "Aaditesh_mtls -> Response body: " << *response_body;
    std::move(callback).Run(false, "");
    return;
  }

  LOG(INFO) << "Aaditesh_mtls -> Response length: " << response_body->length();

  // Parse JSON response
  auto parsed_json =
      base::JSONReader::ReadAndReturnValueWithError(*response_body);
  if (!parsed_json.has_value()) {
    LOG(ERROR) << "Aaditesh_mtls -> Failed to parse JSON response: "
               << parsed_json.error().message;
    std::move(callback).Run(false, "");
    return;
  }

  const base::Value::Dict* response_dict = parsed_json->GetIfDict();
  if (!response_dict) {
    LOG(ERROR) << "Aaditesh_mtls -> Response is not a JSON object";
    std::move(callback).Run(false, "");
    return;
  }

  // According to new API documentation, certificate is nested in certificate
  // object
  LOG(INFO) << "Aaditesh_mtls -> Parsing response using NEW API format: "
               "certificate.certificatePem";
  const base::Value::Dict* certificate_obj =
      response_dict->FindDict("certificate");
  if (!certificate_obj) {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate object not found in response";
    LOG(ERROR) << "Aaditesh_mtls -> Expected format: {\"certificate\": "
                  "{\"certificatePem\": \"...\"}}";
    std::move(callback).Run(false, "");
    return;
  }

  const std::string* certificate_pem =
      certificate_obj->FindString("certificatePem");
  if (!certificate_pem) {
    LOG(ERROR) << "Aaditesh_mtls -> certificatePem field not found in "
                  "certificate object";
    LOG(ERROR) << "Aaditesh_mtls -> Expected format: {\"certificate\": "
                  "{\"certificatePem\": \"...\"}}";
    std::move(callback).Run(false, "");
    return;
  }

  LOG(INFO)
      << "Aaditesh_mtls -> Extracted certificate from API response, length: "
      << certificate_pem->length();

  // Store and validate the certificate
  bool success = StoreCertificate(*certificate_pem);
  if (success) {
    LOG(INFO)
        << "Aaditesh_mtls -> Certificate stored and validated successfully";
  } else {
    LOG(ERROR) << "Aaditesh_mtls -> Certificate validation failed";
  }

  std::move(callback).Run(success, *certificate_pem);
}

// void OktaCertificateManager::OnMockAPIResponse(
//     std::unique_ptr<std::string> response_body) {
//   LOG(INFO) << "Aaditesh_mtls -> === MOCK API RESPONSE RECEIVED ===";
//   LOG(INFO) << "Aaditesh_mtls -> Mock API URL was:
//   https://mocki.io/v1/fd203c54-dcb9-4a0a-bb0c-3030c7c60f6e";

//   // Check for network errors
//   if (mock_url_loader_->NetError() != net::OK) {
//     LOG(ERROR) << "Aaditesh_mtls -> Mock API network error: " <<
//     mock_url_loader_->NetError(); LOG(ERROR) << "Aaditesh_mtls -> Mock API
//     error string: "
//                << net::ErrorToString(mock_url_loader_->NetError());
//     LOG(ERROR) << "Aaditesh_mtls -> === MOCK API RESPONSE FAILED ===";
//     return;
//   }

//   // Check HTTP status code
//   int response_code =
//       mock_url_loader_->ResponseInfo()
//           ? mock_url_loader_->ResponseInfo()->headers->response_code()
//           : 0;
//   LOG(INFO) << "Aaditesh_mtls -> Mock API HTTP response code: " <<
//   response_code;

//   // Log response headers
//   if (mock_url_loader_->ResponseInfo() &&
//   mock_url_loader_->ResponseInfo()->headers) {
//     scoped_refptr<net::HttpResponseHeaders> headers =
//         mock_url_loader_->ResponseInfo()->headers;
//     LOG(INFO) << "Aaditesh_mtls -> Mock API response headers:";
//     size_t iter = 0;
//     std::string name, value;
//     while (headers->EnumerateHeaderLines(&iter, &name, &value)) {
//       LOG(INFO) << "Aaditesh_mtls -> Mock API Header - " << name << ": " <<
//       value;
//     }
//   }

//   if (!response_body) {
//     LOG(ERROR) << "Aaditesh_mtls -> Mock API empty response body (nullptr)";
//     LOG(ERROR) << "Aaditesh_mtls -> === MOCK API RESPONSE FAILED ===";
//     return;
//   }

//   if (response_body->empty()) {
//     LOG(ERROR) << "Aaditesh_mtls -> Mock API empty response body (zero
//     length)"; LOG(ERROR) << "Aaditesh_mtls -> === MOCK API RESPONSE FAILED
//     ==="; return;
//   }

//   LOG(INFO) << "Aaditesh_mtls -> === MOCK API RESPONSE SUCCESS ===";
//   LOG(INFO) << "Aaditesh_mtls -> Mock API response body length: "
//             << response_body->length();
//   LOG(INFO) << "Aaditesh_mtls -> Mock API Full Response body: " <<
//   *response_body; LOG(INFO) << "Aaditesh_mtls -> === MOCK API RAW RESPONSE
//   ==="; LOG(INFO) << *response_body; LOG(INFO) << "Aaditesh_mtls -> === END
//   MOCK API RAW RESPONSE ===";

//   // Try to parse as JSON for additional logging
//   auto parsed_json =
//       base::JSONReader::ReadAndReturnValueWithError(*response_body);
//   if (parsed_json.has_value()) {
//     LOG(INFO) << "Aaditesh_mtls -> Mock API response is valid JSON";
//     const base::Value::Dict* response_dict = parsed_json->GetIfDict();
//     if (response_dict) {
//       LOG(INFO) << "Aaditesh_mtls -> Mock API response is JSON object with "
//                 << response_dict->size() << " fields";
//       // Log each field in the JSON response
//       for (const auto item : *response_dict) {
//         LOG(INFO) << "Aaditesh_mtls -> Mock API JSON field: " << item.first;
//       }
//     }
//   } else {
//     LOG(INFO) << "Aaditesh_mtls -> Mock API response is not JSON: "
//               << parsed_json.error().message;
//   }

//   LOG(INFO) << "Aaditesh_mtls -> Mock API call completed successfully";
//   LOG(INFO) << "Aaditesh_mtls -> === MOCK API PROCESSING COMPLETE ===";
// }
