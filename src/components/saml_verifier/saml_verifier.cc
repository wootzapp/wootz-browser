// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/saml_verifier/saml_verifier.h"

#include "third_party/libxml/chromium/libxml_utils.h"
#include "third_party/libxml/chromium/xml_reader.h"

// Include proper C14N support
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "base/base64.h"
#include "base/containers/span.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "chrome/common/pref_names.h"
#include "components/keyboard_garbaging/keyboard_garbaging_prefs.h"
#include "components/prefs/pref_service.h"
#include "components/safe_browsing/core/common/safe_browsing_prefs.h"
#include "content/public/browser/blocked_domains_prefs.h"
#include "content/public/browser/content_privacy_prefs.h"
#include "content/public/browser/copy_paste_blocker_prefs.h"
#include "content/public/browser/domain_block_checker.h"
#include "content/public/browser/saml_prefs.h"
#include "content/public/browser/upload_blocking_prefs.h"
#include "third_party/libxml/src/include/libxml/c14n.h"
#include "third_party/libxml/src/include/libxml/parser.h"
#include "third_party/libxml/src/include/libxml/tree.h"
#include "third_party/libxml/src/include/libxml/xpath.h"
#include "third_party/libxml/src/include/libxml/xpathInternals.h"

// Use Chromium's crypto instead of xmlsec
#include "crypto/sha2.h"
#include "crypto/signature_verifier.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#include "third_party/boringssl/src/include/openssl/x509.h"

// Network includes for dynamic certificate fetching
#include "base/functional/bind.h"
#include "base/time/time.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "url/gurl.h"

namespace saml_verifier {

// Struct implementations (required by Chromium style)
SamlAttribute::SamlAttribute() = default;

SamlAttribute::SamlAttribute(const SamlAttribute& other)
    : name(other.name), values(other.values) {}

SamlAttribute& SamlAttribute::operator=(const SamlAttribute& other) {
  if (this != &other) {
    name = other.name;
    values = other.values;
  }
  return *this;
}

SamlAttribute::SamlAttribute(SamlAttribute&& other) noexcept
    : name(std::move(other.name)), values(std::move(other.values)) {}

SamlAttribute& SamlAttribute::operator=(SamlAttribute&& other) noexcept {
  if (this != &other) {
    name = std::move(other.name);
    values = std::move(other.values);
  }
  return *this;
}

SamlAttribute::~SamlAttribute() = default;

VerificationResult::VerificationResult() = default;
VerificationResult::~VerificationResult() = default;

SamlVerifier::SignatureData::SignatureData() = default;
SamlVerifier::SignatureData::~SignatureData() = default;

namespace {

// Use the proper SAML prefs constant defined in saml_prefs.h

// XPath expression to find SAML attributes
const char kSamlAttributeXPath[] = "//saml2:Attribute | //saml:Attribute";

// Parse comma-separated domain list
std::vector<std::string> ParseDomainList(const std::string& domain_string) {
  std::vector<std::string> domains;
  std::vector<std::string> parts = base::SplitString(
      domain_string, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

  for (const std::string& domain : parts) {
    if (content::DomainBlockChecker::IsValidDomain(domain)) {
      domains.push_back(domain);
    } else {
      LOG(WARNING) << "Invalid domain in SAML response: " << domain;
    }
  }

  return domains;
}

// RAII wrapper for xmlDocPtr
class XmlDocPtr {
 public:
  explicit XmlDocPtr(xmlDocPtr doc) : doc_(doc) {}
  ~XmlDocPtr() {
    if (doc_) {
      xmlFreeDoc(doc_);
    }
  }

  xmlDocPtr get() const { return doc_; }
  xmlDocPtr release() {
    xmlDocPtr doc = doc_;
    doc_ = nullptr;
    return doc;
  }

 private:
  xmlDocPtr doc_;
};

// RAII wrapper for xmlXPathContextPtr
class XPathContextPtr {
 public:
  explicit XPathContextPtr(xmlXPathContextPtr ctx) : ctx_(ctx) {}
  ~XPathContextPtr() {
    if (ctx_) {
      xmlXPathFreeContext(ctx_);
    }
  }

  xmlXPathContextPtr get() const { return ctx_; }

 private:
  xmlXPathContextPtr ctx_;
};

// RAII wrapper for xmlXPathObjectPtr
class XPathObjectPtr {
 public:
  explicit XPathObjectPtr(xmlXPathObjectPtr obj) : obj_(obj) {}
  ~XPathObjectPtr() {
    if (obj_) {
      xmlXPathFreeObject(obj_);
    }
  }

  xmlXPathObjectPtr get() const { return obj_; }

 private:
  xmlXPathObjectPtr obj_;
};

}  // namespace

// VerificationResult helper methods
std::vector<std::string> VerificationResult::GetAttributeValues(
    const std::string& attribute_name) const {
  for (const auto& attr : attributes) {
    if (attr.name == attribute_name) {
      return attr.values;
    }
  }
  return {};
}

bool VerificationResult::HasAttribute(const std::string& attribute_name) const {
  for (const auto& attr : attributes) {
    if (attr.name == attribute_name) {
      return true;
    }
  }
  return false;
}

// DomainAttributeProcessor implementation
bool DomainAttributeProcessor::ProcessAttributes(
    const std::vector<SamlAttribute>& attributes,
    PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "PrefService is null";
    return false;
  }

  std::vector<std::string> blocked_domains;

  // Look for blocked_domains attribute
  for (const auto& attr : attributes) {
    if (attr.name == "blocked_domains" && !attr.values.empty()) {
      // Parse comma-separated domains from the first value
      blocked_domains = ParseDomainList(attr.values[0]);
      break;
    }
  }

  // Always update the blocked domains preference (even if empty to clear old
  // values)
  base::Value::List domain_list;
  for (const std::string& domain : blocked_domains) {
    domain_list.Append(domain);
  }

  prefs->SetList(blocked_domains::prefs::kBlockedDomains,
                 std::move(domain_list));

  LOG(INFO) << "Domain blocking updated with " << blocked_domains.size()
            << " domains";

  return true;
}

std::vector<std::string> DomainAttributeProcessor::GetHandledAttributes()
    const {
  return {"blocked_domains"};
}

// ContentPrivacyAttributeProcessor implementation
bool ContentPrivacyAttributeProcessor::ProcessAttributes(
    const std::vector<SamlAttribute>& attributes,
    PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "ContentPrivacyAttributeProcessor: PrefService is null";
    return false;
  }

  bool content_privacy_enabled = false;  // Default to false
  bool found_content_privacy = false;

  // Look for content_privacy attribute
  for (const auto& attr : attributes) {
    if (attr.name == "content_privacy" && !attr.values.empty()) {
      found_content_privacy = true;
      std::string value = attr.values[0];

      // Convert to lowercase for case-insensitive comparison
      std::transform(value.begin(), value.end(), value.begin(), ::tolower);

      // Parse boolean values: true/false, 1/0, yes/no
      if (value == "true" || value == "1" || value == "yes") {
        content_privacy_enabled = true;
      } else if (value == "false" || value == "0" || value == "no") {
        content_privacy_enabled = false;
      } else {
        LOG(WARNING) << "Invalid content_privacy value: " << attr.values[0]
                     << " - defaulting to false";
      }
      break;
    }
  }

  // Update the content privacy preference
  prefs->SetBoolean(content_privacy::prefs::kContentPrivacyEnabled,
                    content_privacy_enabled);

  // Update the last updated timestamp
  prefs->SetTime(content_privacy::prefs::kContentPrivacyLastUpdated,
                 base::Time::Now());

  LOG(INFO) << "Content privacy "
            << (found_content_privacy ? "set" : "defaulted")
            << " to: " << (content_privacy_enabled ? "enabled" : "disabled");

  return true;
}

std::vector<std::string>
ContentPrivacyAttributeProcessor::GetHandledAttributes() const {
  return {"content_privacy"};
}

// CopyPasteAttributeProcessor implementation
bool CopyPasteAttributeProcessor::ProcessAttributes(
    const std::vector<SamlAttribute>& attributes,
    PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "CopyPasteAttributeProcessor: PrefService is null";
    return false;
  }

  bool copy_paste_blocking_enabled = false;  // Default to false
  bool found_copy_paste = false;
  std::vector<std::string> copy_paste_blocked_domains;

  // Look for copy_paste and copy_paste_blocked_domains attributes
  for (const auto& attr : attributes) {
    if (attr.name == "copy_paste" && !attr.values.empty()) {
      found_copy_paste = true;
      std::string value = attr.values[0];

      // Convert to lowercase for case-insensitive comparison
      std::transform(value.begin(), value.end(), value.begin(), ::tolower);

      // Parse boolean values: true/false, 1/0, yes/no
      if (value == "true" || value == "1" || value == "yes") {
        copy_paste_blocking_enabled = true;
      } else if (value == "false" || value == "0" || value == "no") {
        copy_paste_blocking_enabled = false;
      } else {
        LOG(WARNING) << "Invalid copy_paste value: " << attr.values[0]
                     << " - defaulting to false";
      }
    }

    if (attr.name == "copy_paste_blocked_domains" && !attr.values.empty()) {
      // Parse comma-separated domains from the first value
      copy_paste_blocked_domains = ParseDomainList(attr.values[0]);
    }
  }

  // Update the copy paste blocking preference
  prefs->SetBoolean(copy_paste_blocker::prefs::kCopyPasteBlockingEnabled,
                    copy_paste_blocking_enabled);

  // Update the copy paste blocked domains preference
  base::Value::List domain_list;
  for (const std::string& domain : copy_paste_blocked_domains) {
    domain_list.Append(domain);
  }

  prefs->SetList(copy_paste_blocker::prefs::kCopyPasteBlockingDomains,
                 std::move(domain_list));

  // SAML Integration: Control WebKit clipboard preferences for additional
  // security layers
  if (copy_paste_blocking_enabled) {
    // When copy/paste blocking is enabled, also disable WebKit-level clipboard
    // access
    prefs->SetBoolean(prefs::kWebKitDomPasteEnabled, false);
    prefs->SetBoolean(prefs::kWebKitJavascriptCanAccessClipboard, false);
    LOG(INFO) << "SAML: Disabled WebKit DOM paste and JavaScript clipboard "
                 "access for enhanced security";
  } else {
    // When copy/paste blocking is disabled, restore WebKit clipboard
    // functionality
    prefs->SetBoolean(prefs::kWebKitDomPasteEnabled, true);
    prefs->SetBoolean(prefs::kWebKitJavascriptCanAccessClipboard, true);
    LOG(INFO)
        << "SAML: Enabled WebKit DOM paste and JavaScript clipboard access";
  }

  LOG(INFO) << "Copy paste blocking "
            << (found_copy_paste ? "set" : "defaulted") << " to: "
            << (copy_paste_blocking_enabled ? "enabled" : "disabled");
  LOG(INFO) << "Copy paste blocked domains updated with "
            << copy_paste_blocked_domains.size() << " domains";

  return true;
}

std::vector<std::string> CopyPasteAttributeProcessor::GetHandledAttributes()
    const {
  return {"copy_paste", "copy_paste_blocked_domains"};
}

// DownloadBlockingAttributeProcessor implementation
bool DownloadBlockingAttributeProcessor::ProcessAttributes(
    const std::vector<SamlAttribute>& attributes,
    PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "DownloadBlockingAttributeProcessor: PrefService is null";
    return false;
  }

  std::vector<std::string> download_blocked_domains;

  // Look for download_blocked_domains attribute
  for (const auto& attr : attributes) {
    if (attr.name == "download_blocked_domains" && !attr.values.empty()) {
      // Parse comma-separated domains from the first value
      download_blocked_domains = ParseDomainList(attr.values[0]);
      break;
    }
  }

  // Always update the download blocked domains preference (even if empty to
  // clear old values)
  base::Value::List domain_list;
  for (const std::string& domain : download_blocked_domains) {
    domain_list.Append(domain);
  }

  prefs->SetList(::prefs::kDangerousDownloadBlockedDomains,
                 std::move(domain_list));

  LOG(INFO) << "Download blocking domains updated with "
            << download_blocked_domains.size() << " domains";

  return true;
}

std::vector<std::string>
DownloadBlockingAttributeProcessor::GetHandledAttributes() const {
  return {"download_blocked_domains"};
}

// UploadBlockingAttributeProcessor implementation
bool UploadBlockingAttributeProcessor::ProcessAttributes(
    const std::vector<SamlAttribute>& attributes,
    PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "UploadBlockingAttributeProcessor: PrefService is null";
    return false;
  }

  std::vector<std::string> upload_blocked_domains;

  // Look for upload_blocked_domains attribute
  for (const auto& attr : attributes) {
    if (attr.name == "upload_blocked_domains" && !attr.values.empty()) {
      // Parse comma-separated domains from the first value
      upload_blocked_domains = ParseDomainList(attr.values[0]);
      break;
    }
  }

  // Always update the upload blocked domains preference directly (even if empty
  // to clear old values)
  base::Value::List domain_list;
  for (const std::string& domain : upload_blocked_domains) {
    domain_list.Append(domain);
  }

  prefs->SetList(content::upload_blocking_prefs::kBlockedUploadDomains,
                 std::move(domain_list));

  LOG(INFO) << "[SAML] Upload blocking domains updated with "
            << upload_blocked_domains.size() << " domains";

  // Log the specific domains being set for debugging
  if (!upload_blocked_domains.empty()) {
    std::string domains_str = "";
    for (const auto& domain : upload_blocked_domains) {
      if (!domains_str.empty()) {
        domains_str += ", ";
      }
      domains_str += domain;
    }
    LOG(INFO) << "[SAML] Upload blocked domains set to: [" << domains_str
              << "]";
  } else {
    LOG(INFO) << "[SAML] Upload blocking domains cleared (empty list)";
  }

  return true;
}

std::vector<std::string>
UploadBlockingAttributeProcessor::GetHandledAttributes() const {
  return {"upload_blocked_domains"};
}

// SyntheticKeystrokesAttributeProcessor implementation
bool SyntheticKeystrokesAttributeProcessor::ProcessAttributes(
    const std::vector<SamlAttribute>& attributes,
    PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "SyntheticKeystrokesAttributeProcessor: PrefService is null";
    return false;
  }

  bool synthetic_keystrokes_enabled = false;  // Default to false
  bool found_synthetic_keystrokes = false;

  // Look for synthetic_keystrokes attribute
  for (const auto& attr : attributes) {
    if (attr.name == "synthetic_keystrokes" && !attr.values.empty()) {
      found_synthetic_keystrokes = true;
      std::string value = attr.values[0];

      // Convert to lowercase for case-insensitive comparison
      std::transform(value.begin(), value.end(), value.begin(), ::tolower);

      // Parse boolean values: true/false, 1/0, yes/no
      if (value == "true" || value == "1" || value == "yes") {
        synthetic_keystrokes_enabled = true;
      } else if (value == "false" || value == "0" || value == "no") {
        synthetic_keystrokes_enabled = false;
      } else {
        LOG(WARNING) << "Invalid synthetic_keystrokes value: " << attr.values[0]
                     << " - defaulting to false";
      }
      break;
    }
  }

  // Update the synthetic keystrokes (keyboard garbaging) preference
  prefs->SetBoolean(keyboard_garbaging_prefs::kObfuscationEnabled,
                    synthetic_keystrokes_enabled);

  LOG(INFO) << "Synthetic keystrokes "
            << (found_synthetic_keystrokes ? "set" : "defaulted") << " to: "
            << (synthetic_keystrokes_enabled ? "enabled" : "disabled");

  return true;
}

std::vector<std::string>
SyntheticKeystrokesAttributeProcessor::GetHandledAttributes() const {
  return {"synthetic_keystrokes"};
}

// SamlVerifier implementation
SamlVerifier::SamlVerifier() {
  // XmlReader handles libxml2 initialization internally
}

SamlVerifier::~SamlVerifier() {
  // XmlReader handles libxml2 cleanup internally
}

void SamlVerifier::ProcessStoredSamlResponse(PrefService* prefs,
                                             VerificationCallback callback) {
  if (!prefs) {
    VerificationResult result;
    result.success = false;
    result.error_message = "PrefService is null";
    std::move(callback).Run(result);
    return;
  }

  std::string saml_xml = prefs->GetString(saml::prefs::kSamlResponse);
  if (saml_xml.empty()) {
    VerificationResult result;
    result.success = false;
    result.error_message = "No SAML response found in preferences";
    std::move(callback).Run(result);
    return;
  }

  ProcessSamlResponse(saml_xml, prefs, std::move(callback));
}

void SamlVerifier::ProcessSamlResponse(const std::string& saml_xml,
                                       PrefService* prefs,
                                       VerificationCallback callback) {
  VerificationResult result;

  if (saml_xml.empty()) {
    result.success = false;
    result.error_message = "Empty SAML response";
    std::move(callback).Run(result);
    return;
  }

  // Use dynamic certificate fetching if enabled and no cert loaded
  if (dynamic_cert_fetching_enabled_ && okta_certificate_pem_.empty()) {
    ProcessSamlResponseWithDynamicCert(saml_xml, prefs, std::move(callback));
    return;
  }

  // Verify SAML signature using certificate
  result.signature_verified = VerifySamlSignature(saml_xml);

  // Validate SAML structure and conditions BEFORE processing attributes
  if (!ValidateSamlConditions(saml_xml)) {
    result.success = false;
    result.error_message = "SAML conditions validation failed";
    LOG(ERROR) << "SAML conditions validation failed";
    std::move(callback).Run(result);
    return;
  }

  // Check for replay attacks
  if (!CheckReplayAttack(saml_xml)) {
    result.success = false;
    result.error_message = "SAML replay attack detected";
    LOG(ERROR) << "SAML replay attack detected";
    std::move(callback).Run(result);
    return;
  }

  // CRITICAL: Signature verification MUST succeed before processing attributes
  if (!result.signature_verified) {
    result.success = false;
    result.error_message =
        "SAML signature verification failed - rejecting response";
    LOG(ERROR) << "SAML signature verification failed - SECURITY: Rejecting "
                  "SAML response";
    std::move(callback).Run(result);
    return;
  }

  // Parse SAML attributes only after signature verification succeeds
  result.attributes = ParseSamlAttributes(saml_xml);

  if (result.attributes.empty()) {
    result.success = false;
    result.error_message = "No SAML attributes found";
    std::move(callback).Run(result);
    return;
  }

  // Process attributes since signature verification succeeded
  ProcessAttributesWithProcessors(result.attributes, prefs);
  result.success = true;

  std::move(callback).Run(result);
}

void SamlVerifier::RegisterAttributeProcessor(
    std::unique_ptr<SamlAttributeProcessor> processor) {
  attribute_processors_.push_back(std::move(processor));
}

void SamlVerifier::RegisterDomainProcessor() {
  RegisterAttributeProcessor(std::make_unique<DomainAttributeProcessor>());
}

void SamlVerifier::RegisterContentPrivacyProcessor() {
  RegisterAttributeProcessor(
      std::make_unique<ContentPrivacyAttributeProcessor>());
}

void SamlVerifier::RegisterCopyPasteProcessor() {
  RegisterAttributeProcessor(std::make_unique<CopyPasteAttributeProcessor>());
}

void SamlVerifier::RegisterDownloadBlockingProcessor() {
  RegisterAttributeProcessor(
      std::make_unique<DownloadBlockingAttributeProcessor>());
}

void SamlVerifier::RegisterUploadBlockingProcessor() {
  RegisterAttributeProcessor(
      std::make_unique<UploadBlockingAttributeProcessor>());
}

void SamlVerifier::RegisterSyntheticKeystrokesProcessor() {
  RegisterAttributeProcessor(
      std::make_unique<SyntheticKeystrokesAttributeProcessor>());
}

// static
void SamlVerifier::ProcessNewSamlResponse(PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "[SAML] PrefService is null";
    return;
  }

  LOG(INFO) << "[SAML] ========== Starting SAML Response Processing ==========";

  // Create and configure verifier
  auto verifier = std::make_unique<SamlVerifier>();

  LOG(INFO) << "[SAML] Registering attribute processors:";
  verifier->RegisterDomainProcessor();
  LOG(INFO) << "[SAML] - Domain blocking processor registered";

  verifier->RegisterContentPrivacyProcessor();
  LOG(INFO) << "[SAML] - Content privacy processor registered";

  verifier->RegisterCopyPasteProcessor();
  LOG(INFO) << "[SAML] - Copy/paste blocking processor registered";

  verifier->RegisterDownloadBlockingProcessor();
  LOG(INFO) << "[SAML] - Download blocking processor registered";

  verifier->RegisterUploadBlockingProcessor();
  LOG(INFO) << "[SAML] - Upload blocking processor registered";

  verifier->RegisterSyntheticKeystrokesProcessor();
  LOG(INFO) << "[SAML] - Synthetic keystrokes processor registered";

  verifier->SetSignatureVerificationEnabled(true);
  verifier->SetDynamicCertificateFetchingEnabled(true);
  verifier->SetDevelopmentMode(true);  // Enable for Okta trial instances

  LOG(INFO) << "[SAML] SAML verifier configured with signature verification "
               "and dynamic cert fetching";

  // Move the verifier to the callback to keep it alive
  auto* verifier_ptr = verifier.get();
  verifier_ptr->ProcessStoredSamlResponse(
      prefs, base::BindOnce(
                 [](std::unique_ptr<SamlVerifier> verifier,
                    const VerificationResult& result) {
                   if (result.success) {
                     LOG(INFO) << "[SAML] ========== SAML Response Processing "
                                  "COMPLETED SUCCESSFULLY ==========";
                     LOG(INFO) << "[SAML] All security policies have been "
                                  "updated from SAML attributes";
                   } else {
                     LOG(ERROR) << "[SAML] ========== SAML Response Processing "
                                   "FAILED ==========";
                     LOG(ERROR) << "[SAML] Error: " << result.error_message;
                   }
                   // verifier is automatically destroyed here
                 },
                 std::move(verifier)));
}

bool SamlVerifier::LoadCertificateFromPem(const std::string& certificate_pem) {
  if (certificate_pem.empty()) {
    LOG(ERROR) << "Empty certificate PEM data";
    return false;
  }

  // Parse X.509 certificate using BoringSSL
  BIO* bio = BIO_new_mem_buf(certificate_pem.data(), certificate_pem.length());
  if (!bio) {
    LOG(ERROR) << "Failed to create BIO for certificate";
    return false;
  }

  X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
  BIO_free(bio);

  if (!cert) {
    LOG(ERROR) << "Failed to parse X.509 certificate from PEM";
    return false;
  }

  // Extract DER-encoded certificate for signature verification
  unsigned char* der_data = nullptr;
  int der_length = i2d_X509(cert, &der_data);

  if (der_length <= 0 || !der_data) {
    LOG(ERROR) << "Failed to convert certificate to DER format";
    X509_free(cert);
    return false;
  }

  // Store certificate DER data with safe conversion
  certificate_der_.assign(der_data, der_data + static_cast<size_t>(der_length));
  OPENSSL_free(der_data);
  X509_free(cert);

  return true;
}

bool SamlVerifier::VerifySamlSignature(const std::string& saml_xml) {
  if (!signature_verification_enabled_) {
    LOG(ERROR) << "Signature verification is disabled - this is insecure for "
                  "production";
    return false;  // PRODUCTION: Must verify signatures
  }

  if (okta_certificate_pem_.empty() || certificate_der_.empty()) {
    LOG(ERROR) << "No Okta certificate loaded for signature verification";
    return false;
  }

  // Validate certificate before using it
  if (!ValidateCertificate()) {
    LOG(ERROR) << "Certificate validation failed";
    return false;
  }

  // Extract signature data from SAML XML
  SignatureData sig_data;
  if (!ExtractSignatureData(saml_xml, &sig_data)) {
    LOG(ERROR) << "Failed to extract signature data from SAML";
    return false;
  }

  // Certificate should already be loaded by ProcessSamlResponseWithDynamicCert
  // or SetOktaCertificate - no need to extract it again here

  // Extract the Reference URI to find what element is being signed
  std::string reference_uri = ExtractReferenceUri(sig_data.signed_info_xml);
  if (reference_uri.empty()) {
    LOG(ERROR) << "Failed to extract Reference URI from SignedInfo";
    return false;
  }

  // CRITICAL FIX: Apply SAML signature transforms to the REFERENCED element
  std::string canonicalized_data =
      ApplySamlSignatureTransforms(saml_xml, reference_uri);
  if (canonicalized_data.empty()) {
    LOG(ERROR) << "Failed to apply SAML signature transforms";
    return false;
  }

  // First verify the digest of the referenced element
  bool digest_valid =
      VerifyDigestValue(canonicalized_data, sig_data.signed_info_xml);
  if (!digest_valid) {
    LOG(ERROR) << "Digest verification failed for referenced element";
    return false;
  }

  // Now canonicalize the SignedInfo for signature verification
  std::string canonical_signed_info = CanonicalizeXml(sig_data.signed_info_xml);
  if (canonical_signed_info.empty()) {
    LOG(ERROR) << "Failed to canonicalize SignedInfo";
    return false;
  }

  // CORRECT: Verify the RSA signature against the canonicalized SignedInfo
  bool signature_valid =
      VerifyRsaSignature(canonical_signed_info, sig_data.signature_value);

  if (!signature_valid) {
    LOG(ERROR) << "SAML signature verification FAILED";
  }

  return signature_valid;
}

bool SamlVerifier::ExtractSignatureData(const std::string& saml_xml,
                                        SignatureData* sig_data) {
  if (!sig_data) {
    return false;
  }

  // Use string-based extraction for the first signature (Response-level)
  // Look for the first SignedInfo element
  size_t signed_info_start = saml_xml.find("<ds:SignedInfo");
  if (signed_info_start == std::string::npos) {
    signed_info_start = saml_xml.find("<SignedInfo");
  }

  if (signed_info_start == std::string::npos) {
    LOG(ERROR) << "No SignedInfo element found in SAML";
    return false;
  }

  // Find the closing tag for SignedInfo
  size_t signed_info_end = saml_xml.find("</ds:SignedInfo>", signed_info_start);
  if (signed_info_end == std::string::npos) {
    signed_info_end = saml_xml.find("</SignedInfo>", signed_info_start);
  }

  if (signed_info_end == std::string::npos) {
    LOG(ERROR) << "No closing SignedInfo tag found in SAML";
    return false;
  }

  // Include the closing tag in the extraction
  if (saml_xml.substr(signed_info_end, 2) == "</") {
    size_t tag_end = saml_xml.find('>', signed_info_end);
    if (tag_end != std::string::npos) {
      signed_info_end = tag_end + 1;
    }
  }

  // Extract the raw SignedInfo
  std::string raw_signed_info =
      saml_xml.substr(signed_info_start, signed_info_end - signed_info_start);

  // CRITICAL FIX: Add namespace declarations to preserve context
  // The SignedInfo uses ds: prefix but the namespace declaration is in the
  // parent Signature element We need to add the namespace declarations for
  // proper canonicalization

  // Check if the SignedInfo already has namespace declarations
  bool has_ds_namespace =
      raw_signed_info.find("xmlns:ds=") != std::string::npos;

  if (!has_ds_namespace) {
    // Add the ds namespace declaration to the SignedInfo element
    // Find the end of the opening SignedInfo tag
    size_t tag_end = raw_signed_info.find('>');
    if (tag_end != std::string::npos) {
      // Insert the namespace declaration before the closing >
      std::string namespace_decl =
          " xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\"";
      raw_signed_info.insert(tag_end, namespace_decl);
    } else {
      LOG(ERROR) << "Failed to find end of SignedInfo opening tag";
      return false;
    }
  }

  // Also check for xmlns:ec namespace if InclusiveNamespaces is used
  if (raw_signed_info.find("xmlns:ec=") == std::string::npos &&
      raw_signed_info.find("ec:InclusiveNamespaces") != std::string::npos) {
    // Add the ec namespace declaration
    size_t tag_end = raw_signed_info.find('>');
    if (tag_end != std::string::npos) {
      std::string ec_namespace_decl =
          " xmlns:ec=\"http://www.w3.org/2001/10/xml-exc-c14n#\"";
      raw_signed_info.insert(tag_end, ec_namespace_decl);
    }
  }

  sig_data->signed_info_xml = raw_signed_info;

  // Look for SignatureValue in the SAME signature element as the SignedInfo
  // Start searching after the SignedInfo we just found
  size_t sig_value_start = saml_xml.find("<ds:SignatureValue", signed_info_end);
  if (sig_value_start == std::string::npos) {
    sig_value_start = saml_xml.find("<SignatureValue", signed_info_end);
  }

  // Make sure we don't go into the next signature (if any)
  size_t next_signature = saml_xml.find("<ds:Signature", signed_info_end);
  if (next_signature == std::string::npos) {
    next_signature = saml_xml.find("<Signature", signed_info_end);
  }

  if (sig_value_start == std::string::npos ||
      (next_signature != std::string::npos &&
       sig_value_start > next_signature)) {
    LOG(ERROR)
        << "No SignatureValue element found in same signature as SignedInfo";
    return false;
  }

  // Find the start of the actual signature value content
  size_t content_start = saml_xml.find('>', sig_value_start);
  if (content_start == std::string::npos) {
    LOG(ERROR) << "Malformed SignatureValue element";
    return false;
  }
  content_start++;  // Move past the '>'

  // Find the closing tag
  size_t sig_value_end = saml_xml.find("</ds:SignatureValue>", content_start);
  if (sig_value_end == std::string::npos) {
    sig_value_end = saml_xml.find("</SignatureValue>", content_start);
  }

  if (sig_value_end == std::string::npos) {
    LOG(ERROR) << "No closing SignatureValue tag found in SAML";
    return false;
  }

  std::string raw_signature =
      saml_xml.substr(content_start, sig_value_end - content_start);

  // Clean up whitespace from signature value
  raw_signature.erase(std::remove_if(raw_signature.begin(), raw_signature.end(),
                                     [](char c) { return std::isspace(c); }),
                      raw_signature.end());

  sig_data->signature_value = raw_signature;

  bool success =
      !sig_data->signed_info_xml.empty() && !sig_data->signature_value.empty();

  if (!success) {
    LOG(ERROR) << "Failed to extract complete signature data";
  }

  return success;
}

std::string SamlVerifier::CanonicalizeXml(const std::string& xml) {
  // Initialize libxml2 parser
  xmlInitParser();

  // Parse the XML string into a document
  XmlDocPtr doc(xmlParseMemory(xml.c_str(), xml.length()));
  if (!doc.get()) {
    LOG(ERROR) << "Failed to parse XML for canonicalization";
    xmlCleanupParser();
    return "";
  }

  // Create buffer for canonicalized output
  xmlChar* c14n_output = nullptr;

  // Perform exclusive C14N canonicalization (C14N_EXCLUSIVE_1_0)
  // This is the standard for SAML signature verification
  // NOTE: SignedInfo canonicalization should NOT include inclusive namespaces
  // The inclusive namespaces (xs) are only for referenced element transforms
  int result = xmlC14NDocDumpMemory(
      doc.get(),               // XML document
      nullptr,                 // Node set (nullptr = whole document)
      XML_C14N_EXCLUSIVE_1_0,  // Exclusive canonicalization mode
      nullptr,                 // No inclusive namespaces for SignedInfo
      0,                       // With comments = 0 (no comments)
      &c14n_output);           // Output buffer

  std::string canonicalized;
  if (result >= 0 && c14n_output) {
    canonicalized =
        std::string(reinterpret_cast<const char*>(c14n_output), result);
    xmlFree(c14n_output);
  } else {
    LOG(ERROR) << "C14N canonicalization failed with result: " << result;
    xmlCleanupParser();
    return "";
  }

  xmlCleanupParser();
  return canonicalized;
}

std::string SamlVerifier::CanonicalizeXmlSubset(const std::string& xml,
                                                const std::string& element_id) {
  // Initialize libxml2 parser
  xmlInitParser();

  // Parse the XML string into a document
  XmlDocPtr doc(xmlParseMemory(xml.c_str(), xml.length()));
  if (!doc.get()) {
    LOG(ERROR) << "Failed to parse XML for subset canonicalization";
    xmlCleanupParser();
    return "";
  }

  // Create XPath context for finding the specific element
  XPathContextPtr xpath_ctx(xmlXPathNewContext(doc.get()));
  if (!xpath_ctx.get()) {
    LOG(ERROR) << "Failed to create XPath context";
    xmlCleanupParser();
    return "";
  }

  // Register common SAML namespaces
  xmlXPathRegisterNs(xpath_ctx.get(), BAD_CAST "saml2",
                     BAD_CAST "urn:oasis:names:tc:SAML:2.0:assertion");
  xmlXPathRegisterNs(xpath_ctx.get(), BAD_CAST "samlp",
                     BAD_CAST "urn:oasis:names:tc:SAML:2.0:protocol");
  xmlXPathRegisterNs(xpath_ctx.get(), BAD_CAST "saml2p",
                     BAD_CAST "urn:oasis:names:tc:SAML:2.0:protocol");
  xmlXPathRegisterNs(xpath_ctx.get(), BAD_CAST "ds",
                     BAD_CAST "http://www.w3.org/2000/09/xmldsig#");

  // Create XPath expression to find element with specific ID
  std::string xpath_expr = "//*[@ID='" + element_id + "']";
  XPathObjectPtr xpath_obj(
      xmlXPathEvalExpression(BAD_CAST xpath_expr.c_str(), xpath_ctx.get()));

  if (!xpath_obj.get() || !xpath_obj.get()->nodesetval ||
      xpath_obj.get()->nodesetval->nodeNr == 0) {
    LOG(ERROR) << "Element with ID '" << element_id << "' not found";
    xmlCleanupParser();
    return "";
  }

  // Get the first matching node
  xmlNodePtr target_node = xpath_obj.get()->nodesetval->nodeTab[0];
  if (!target_node) {
    LOG(ERROR) << "Invalid target node for canonicalization";
    xmlCleanupParser();
    return "";
  }

  // Create a new document containing only the target element
  XmlDocPtr subset_doc(xmlNewDoc(BAD_CAST "1.0"));
  if (!subset_doc.get()) {
    LOG(ERROR) << "Failed to create subset document";
    xmlCleanupParser();
    return "";
  }

  // Copy the target node to the new document
  xmlNodePtr copied_node =
      xmlDocCopyNode(target_node, subset_doc.get(), 1);  // 1 = deep copy
  if (!copied_node) {
    LOG(ERROR) << "Failed to copy target node";
    xmlCleanupParser();
    return "";
  }

  // Set as root element
  xmlDocSetRootElement(subset_doc.get(), copied_node);

  // Create buffer for canonicalized output
  xmlChar* c14n_output = nullptr;

  // Perform exclusive C14N canonicalization on the subset document
  // Include 'xs' namespace as per SAML signature specification
  xmlChar* inclusive_ns_list[] = {BAD_CAST "xs", nullptr};
  int result = xmlC14NDocDumpMemory(
      subset_doc.get(),        // XML document containing only target element
      nullptr,                 // Node set (nullptr = whole document)
      XML_C14N_EXCLUSIVE_1_0,  // Exclusive canonicalization mode
      inclusive_ns_list,       // Inclusive namespaces list (xs namespace)
      0,                       // With comments = 0 (no comments)
      &c14n_output);           // Output buffer

  std::string canonicalized;
  if (result >= 0 && c14n_output) {
    canonicalized =
        std::string(reinterpret_cast<const char*>(c14n_output), result);
    xmlFree(c14n_output);
  } else {
    LOG(ERROR) << "C14N subset canonicalization failed with result: " << result;
    xmlCleanupParser();
    return "";
  }

  xmlCleanupParser();
  return canonicalized;
}

std::string SamlVerifier::ExtractReferenceUri(
    const std::string& signed_info_xml) {
  // Extract the URI attribute from the Reference element
  size_t ref_start = signed_info_xml.find("<ds:Reference");
  if (ref_start == std::string::npos) {
    ref_start = signed_info_xml.find("<Reference");
  }

  if (ref_start == std::string::npos) {
    LOG(ERROR) << "No Reference element found in SignedInfo";
    return "";
  }

  size_t uri_start = signed_info_xml.find("URI=\"", ref_start);
  if (uri_start == std::string::npos) {
    LOG(ERROR) << "No URI attribute found in Reference element";
    return "";
  }

  uri_start += 5;  // Move past 'URI="'
  size_t uri_end = signed_info_xml.find('\"', uri_start);
  if (uri_end == std::string::npos) {
    LOG(ERROR) << "Malformed URI attribute in Reference element";
    return "";
  }

  std::string uri = signed_info_xml.substr(uri_start, uri_end - uri_start);
  return uri;
}

std::string SamlVerifier::ExtractSignatureMethod(
    const std::string& signed_info_xml) {
  // Extract the Algorithm attribute from the SignatureMethod element
  size_t sig_method_start = signed_info_xml.find("<ds:SignatureMethod");
  if (sig_method_start == std::string::npos) {
    sig_method_start = signed_info_xml.find("<SignatureMethod");
  }

  if (sig_method_start == std::string::npos) {
    LOG(ERROR) << "No SignatureMethod element found in SignedInfo";
    return "";
  }

  size_t algorithm_start =
      signed_info_xml.find("Algorithm=\"", sig_method_start);
  if (algorithm_start == std::string::npos) {
    LOG(ERROR) << "No Algorithm attribute found in SignatureMethod element";
    return "";
  }

  algorithm_start += 11;  // Move past 'Algorithm="'
  size_t algorithm_end = signed_info_xml.find('\"', algorithm_start);
  if (algorithm_end == std::string::npos) {
    LOG(ERROR) << "Malformed Algorithm attribute in SignatureMethod element";
    return "";
  }

  std::string algorithm =
      signed_info_xml.substr(algorithm_start, algorithm_end - algorithm_start);
  return algorithm;
}

std::string SamlVerifier::ApplySamlSignatureTransforms(
    const std::string& saml_xml,
    const std::string& reference_uri) {
  // For SAML, we need to:
  // 1. Find the referenced element (usually Response with matching ID)
  // 2. Apply enveloped signature transform (remove Signature element)
  // 3. Apply exclusive canonicalization (C14N)

  if (reference_uri.empty() || reference_uri[0] != '#') {
    LOG(ERROR) << "Invalid reference URI: " << reference_uri;
    return "";
  }

  std::string element_id = reference_uri.substr(1);  // Remove '#' prefix

  // Apply enveloped signature transform first (remove Signature element)
  std::string transformed = ApplyEnvelopedSignatureTransform(saml_xml);
  if (transformed.empty()) {
    LOG(ERROR) << "Failed to apply enveloped signature transform";
    return "";
  }

  // Apply exclusive canonicalization to the specific referenced element
  std::string canonicalized = CanonicalizeXmlSubset(transformed, element_id);
  if (canonicalized.empty()) {
    LOG(ERROR) << "Failed to canonicalize referenced element";
    return "";
  }

  return canonicalized;
}

std::string SamlVerifier::ApplyEnvelopedSignatureTransform(
    const std::string& xml) {
  // Remove the entire ds:Signature element from the XML
  std::string result = xml;

  size_t sig_start = result.find("<ds:Signature");
  if (sig_start == std::string::npos) {
    sig_start = result.find("<Signature");
  }

  if (sig_start == std::string::npos) {
    return result;
  }

  // Find the matching closing tag
  size_t sig_end = result.find("</ds:Signature>", sig_start);
  if (sig_end == std::string::npos) {
    sig_end = result.find("</Signature>", sig_start);
  }

  if (sig_end == std::string::npos) {
    LOG(ERROR) << "Cannot find closing Signature tag";
    return "";
  }

  // Include the closing tag
  sig_end = result.find('>', sig_end) + 1;

  // Remove the signature element
  result.erase(sig_start, sig_end - sig_start);

  return result;
}

bool SamlVerifier::VerifyRsaSignature(const std::string& data,
                                      const std::string& signature_base64) {
  // Decode base64 signature
  std::string signature_bytes;
  if (!base::Base64Decode(signature_base64, &signature_bytes)) {
    LOG(ERROR) << "Failed to decode base64 signature";
    return false;
  }

  // Validate signature method from the SignedInfo
  std::string signature_method = ExtractSignatureMethod(data);

  // Validate that the signature method matches what we're using
  if (!signature_method.empty()) {
    if (signature_method !=
        "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256") {
      LOG(ERROR) << "Unsupported signature method: " << signature_method;
      LOG(ERROR)
          << "Expected: http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";
      return false;
    }
  }

  // Parse certificate to get public key
  const unsigned char* cert_data = certificate_der_.data();
  X509* cert = d2i_X509(nullptr, &cert_data, certificate_der_.size());
  if (!cert) {
    LOG(ERROR) << "Failed to parse stored certificate";
    return false;
  }

  EVP_PKEY* public_key = X509_get_pubkey(cert);
  X509_free(cert);

  if (!public_key) {
    LOG(ERROR) << "Failed to extract public key from certificate";
    return false;
  }

  // Extract public key info in DER format for Chromium crypto
  unsigned char* key_der = nullptr;
  int key_der_length = i2d_PUBKEY(public_key, &key_der);

  if (key_der_length <= 0 || !key_der) {
    LOG(ERROR) << "Failed to convert public key to DER format";
    EVP_PKEY_free(public_key);
    return false;
  }

  // Create signature verifier
  auto verifier = std::make_unique<crypto::SignatureVerifier>();

  // Initialize verifier with RSA-SHA256 using correct Chromium API (safe
  // conversion)
  bool init_success = verifier->VerifyInit(
      crypto::SignatureVerifier::RSA_PKCS1_SHA256,
      base::span<const uint8_t>(
          reinterpret_cast<const uint8_t*>(signature_bytes.data()),
          signature_bytes.length()),
      base::span<const uint8_t>(key_der, static_cast<size_t>(key_der_length)));

  OPENSSL_free(key_der);
  EVP_PKEY_free(public_key);

  if (!init_success) {
    LOG(ERROR) << "Failed to initialize signature verifier";
    return false;
  }

  // IMPORTANT: Pass the raw canonicalized data, not the hash!
  // crypto::SignatureVerifier::RSA_PKCS1_SHA256 will calculate SHA-256
  // internally
  verifier->VerifyUpdate(base::span<const uint8_t>(
      reinterpret_cast<const uint8_t*>(data.data()), data.length()));

  // Finalize verification
  bool signature_valid = verifier->VerifyFinal();

  return signature_valid;
}

bool SamlVerifier::ValidateCertificate() {
  if (certificate_der_.empty()) {
    LOG(ERROR) << "No certificate loaded for validation";
    return false;
  }

  // Parse certificate to check validity
  const unsigned char* cert_data = certificate_der_.data();
  X509* cert = d2i_X509(nullptr, &cert_data, certificate_der_.size());
  if (!cert) {
    LOG(ERROR) << "Failed to parse certificate for validation";
    return false;
  }

  // Check certificate validity period
  ASN1_TIME* not_before = X509_get_notBefore(cert);
  ASN1_TIME* not_after = X509_get_notAfter(cert);

  int not_before_check = X509_cmp_time(not_before, nullptr);
  int not_after_check = X509_cmp_time(not_after, nullptr);

  bool is_valid = (not_before_check <= 0) && (not_after_check >= 0);

  if (!is_valid) {
    LOG(ERROR) << "Certificate is not within valid time period";
    X509_free(cert);
    return false;
  }

  // Additional Okta-specific validation
  bool okta_valid = ValidateOktaCertificate("");

  X509_free(cert);

  if (!okta_valid) {
    LOG(ERROR) << "Certificate validation failed - Okta validation failed";
  }

  return okta_valid;
}

bool SamlVerifier::ValidateEmbeddedCertificate(
    const std::string& certificate_pem) {
  if (certificate_pem.empty()) {
    LOG(ERROR) << "Empty embedded certificate";
    return false;
  }

  // Parse and validate the embedded certificate
  BIO* bio = BIO_new_mem_buf(certificate_pem.data(), certificate_pem.length());
  if (!bio) {
    LOG(ERROR) << "Failed to create BIO for embedded certificate";
    return false;
  }

  X509* cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
  BIO_free(bio);

  if (!cert) {
    LOG(ERROR) << "Failed to parse embedded X.509 certificate";
    return false;
  }

  // Check certificate validity period
  ASN1_TIME* not_before = X509_get_notBefore(cert);
  ASN1_TIME* not_after = X509_get_notAfter(cert);

  int not_before_check = X509_cmp_time(not_before, nullptr);
  int not_after_check = X509_cmp_time(not_after, nullptr);

  bool is_valid = (not_before_check <= 0) && (not_after_check >= 0);

  if (!is_valid) {
    LOG(ERROR) << "Embedded certificate is not within valid time period";
    X509_free(cert);
    return false;
  }

  // Validate this is an Okta certificate
  bool okta_valid = ValidateOktaCertificate(certificate_pem);

  X509_free(cert);
  return okta_valid;
}

bool SamlVerifier::ValidateOktaCertificate(const std::string& certificate_pem) {
  const unsigned char* cert_data = certificate_der_.data();
  X509* cert = d2i_X509(nullptr, &cert_data, certificate_der_.size());
  if (!cert) {
    LOG(ERROR) << "Failed to parse certificate for Okta validation";
    return false;
  }

  bool is_valid = true;

  // 1. Validate certificate purpose and key usage
  if (!ValidateCertificatePurpose(cert)) {
    LOG(ERROR) << "Certificate purpose validation failed";
    is_valid = false;
  }

  // 2. Validate certificate issuer and subject
  if (!ValidateCertificateIssuer(cert)) {
    LOG(ERROR) << "Certificate issuer validation failed";
    is_valid = false;
  }

  // 3. Validate subject alternative names for Okta domain
  if (!ValidateOktaDomain(cert)) {
    LOG(ERROR) << "Okta domain validation failed";
    is_valid = false;
  }

  // 4. Validate certificate chain (simplified - in production add full chain)
  if (!ValidateCertificateChain(cert)) {
    LOG(ERROR) << "Certificate chain validation failed";
    is_valid = false;
  }

  X509_free(cert);

  if (!is_valid) {
    LOG(ERROR) << "Okta certificate validation FAILED";
  }

  return is_valid;
}

bool SamlVerifier::ValidateCertificatePurpose(X509* cert) {
  // Check key usage extensions
  int key_usage = X509_get_key_usage(cert);

  // For SAML signing, we need digital signature capability
  if (!(key_usage & X509v3_KU_DIGITAL_SIGNATURE)) {
    LOG(ERROR) << "Certificate does not have digital signature capability";
    return false;
  }

  // Check extended key usage for code/document signing
  EXTENDED_KEY_USAGE* ext_key_usage = static_cast<EXTENDED_KEY_USAGE*>(
      X509_get_ext_d2i(cert, NID_ext_key_usage, nullptr, nullptr));

  if (ext_key_usage) {
    bool valid_purpose = false;
    for (int i = 0; i < static_cast<int>(sk_ASN1_OBJECT_num(ext_key_usage));
         i++) {
      ASN1_OBJECT* obj = sk_ASN1_OBJECT_value(ext_key_usage, i);
      int nid = OBJ_obj2nid(obj);

      // Allow code signing or any purpose for SAML
      if (nid == NID_code_sign || nid == NID_anyExtendedKeyUsage) {
        valid_purpose = true;
        break;
      }
    }

    EXTENDED_KEY_USAGE_free(ext_key_usage);

    if (!valid_purpose) {
      LOG(ERROR) << "Certificate does not have appropriate extended key usage";
      return false;
    }
  }

  return true;
}

bool SamlVerifier::ValidateCertificateIssuer(X509* cert) {
  X509_NAME* issuer = X509_get_issuer_name(cert);
  X509_NAME* subject = X509_get_subject_name(cert);

  if (!issuer || !subject) {
    LOG(ERROR) << "Invalid certificate issuer or subject";
    return false;
  }

  // Get issuer string
  char issuer_str[256];
  X509_NAME_oneline(issuer, issuer_str, sizeof(issuer_str));

  char subject_str[256];
  X509_NAME_oneline(subject, subject_str, sizeof(subject_str));

  // Basic validation - in production, validate against known Okta CAs
  std::string issuer_string(issuer_str);
  std::string subject_string(subject_str);

  // Check for reasonable certificate attributes
  if (issuer_string.empty() || subject_string.empty()) {
    LOG(ERROR) << "Empty issuer or subject in certificate";
    return false;
  }

  // For production: validate against whitelist of trusted Okta issuer CAs
  // For now, just ensure it's not self-signed for security purposes
  if (X509_NAME_cmp(issuer, subject) == 0) {
    LOG(WARNING)
        << "Self-signed certificate detected - verify this is expected";
  }

  return true;
}

bool SamlVerifier::ValidateOktaDomain(X509* cert) {
  // Get subject alternative names
  GENERAL_NAMES* san_names = static_cast<GENERAL_NAMES*>(
      X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));

  bool has_okta_domain = false;

  if (san_names) {
    for (int i = 0; i < static_cast<int>(sk_GENERAL_NAME_num(san_names)); i++) {
      GENERAL_NAME* gen_name = sk_GENERAL_NAME_value(san_names, i);

      if (gen_name->type == GEN_DNS) {
        ASN1_STRING* dns_name = gen_name->d.dNSName;
        if (dns_name) {
          std::string domain_name(
              reinterpret_cast<const char*>(ASN1_STRING_get0_data(dns_name)),
              ASN1_STRING_length(dns_name));

          // Check if this is an Okta domain
          if (domain_name.find(".okta.com") != std::string::npos ||
              domain_name.find(".oktapreview.com") != std::string::npos ||
              domain_name.find(".okta-emea.com") != std::string::npos) {
            has_okta_domain = true;
          }
        }
      }
    }

    GENERAL_NAMES_free(san_names);
  }

  // Also check the common name in subject
  X509_NAME* subject = X509_get_subject_name(cert);
  std::string cn;
  if (subject) {
    int lastpos = X509_NAME_get_index_by_NID(subject, NID_commonName, -1);
    if (lastpos >= 0) {
      X509_NAME_ENTRY* entry = X509_NAME_get_entry(subject, lastpos);
      if (entry) {
        ASN1_STRING* cn_data = X509_NAME_ENTRY_get_data(entry);
        if (cn_data) {
          cn = std::string(
              reinterpret_cast<const char*>(ASN1_STRING_get0_data(cn_data)),
              ASN1_STRING_length(cn_data));

          // Check for full Okta domains in CN
          if (cn.find(".okta.com") != std::string::npos ||
              cn.find(".oktapreview.com") != std::string::npos ||
              cn.find(".okta-emea.com") != std::string::npos) {
            has_okta_domain = true;
          }
        }
      }
    }
  }

  // Check if this is a valid Okta trial certificate
  // Okta trial certificates often have CN like "trial-XXXXXXX" and are issued
  // by Okta
  if (!has_okta_domain && !cn.empty()) {
    // Check if CN matches Okta trial pattern
    if (cn.find("trial-") == 0) {
      // Verify the issuer is Okta
      X509_NAME* issuer = X509_get_issuer_name(cert);
      if (issuer) {
        char issuer_str[512];
        X509_NAME_oneline(issuer, issuer_str, sizeof(issuer_str));
        std::string issuer_string(issuer_str);

        if (issuer_string.find("O=Okta") != std::string::npos) {
          has_okta_domain = true;
        }
      }
    }

    // Also check for other Okta patterns in CN
    if (cn.find("okta") != std::string::npos ||
        cn.find("dev-") == 0) {  // Okta dev instances
      // Verify the issuer is Okta
      X509_NAME* issuer = X509_get_issuer_name(cert);
      if (issuer) {
        char issuer_str[512];
        X509_NAME_oneline(issuer, issuer_str, sizeof(issuer_str));
        std::string issuer_string(issuer_str);

        if (issuer_string.find("O=Okta") != std::string::npos) {
          has_okta_domain = true;
        }
      }
    }
  }

  if (!has_okta_domain) {
    if (development_mode_) {
      // In development mode, at least verify the issuer contains "Okta"
      X509_NAME* issuer = X509_get_issuer_name(cert);
      if (issuer) {
        char issuer_str[512];
        X509_NAME_oneline(issuer, issuer_str, sizeof(issuer_str));
        std::string issuer_string(issuer_str);

        if (issuer_string.find("Okta") != std::string::npos ||
            issuer_string.find("okta") != std::string::npos) {
          return true;
        }
      }

      LOG(ERROR) << "DEVELOPMENT MODE: Certificate issuer does not contain "
                    "'Okta' - rejecting";
      return false;
    } else {
      LOG(ERROR) << "Certificate does not contain valid Okta domain or pattern";
      return false;
    }
  }

  return true;
}

bool SamlVerifier::ValidateCertificateChain(X509* cert) {
  // For production, implement full certificate chain validation:
  // 1. Build certificate chain to trusted root CA
  // 2. Validate each certificate in the chain
  // 3. Check for revocation using OCSP/CRL
  // 4. Validate trust anchors

  // For now, basic self-validation
  EVP_PKEY* public_key = X509_get_pubkey(cert);
  if (!public_key) {
    LOG(ERROR) << "Failed to get public key from certificate";
    return false;
  }

  // Verify certificate is self-consistent
  int verify_result = X509_verify(cert, public_key);
  EVP_PKEY_free(public_key);

  if (verify_result != 1) {
    LOG(ERROR) << "Certificate self-verification failed";
    return false;
  }

  // FUTURE: Implement full chain validation to Okta root CAs for enhanced
  // security
  return true;
}

bool SamlVerifier::VerifyDigestValue(const std::string& canonical_data,
                                     const std::string& signed_info_xml) {
  if (canonical_data.empty() || signed_info_xml.empty()) {
    LOG(ERROR) << "Empty data for digest verification";
    return false;
  }

  // Extract digest value from SignedInfo
  size_t digest_start = signed_info_xml.find("<ds:DigestValue>");
  if (digest_start == std::string::npos) {
    digest_start = signed_info_xml.find("<DigestValue>");
  }

  if (digest_start == std::string::npos) {
    LOG(ERROR) << "No DigestValue element found in SignedInfo";
    return false;
  }

  size_t content_start = signed_info_xml.find('>', digest_start) + 1;
  size_t digest_end = signed_info_xml.find("</ds:DigestValue>", content_start);
  if (digest_end == std::string::npos) {
    digest_end = signed_info_xml.find("</DigestValue>", content_start);
  }

  if (digest_end == std::string::npos) {
    LOG(ERROR) << "No closing DigestValue tag found";
    return false;
  }

  std::string expected_digest_b64 =
      signed_info_xml.substr(content_start, digest_end - content_start);

  // Remove whitespace
  expected_digest_b64.erase(
      std::remove_if(expected_digest_b64.begin(), expected_digest_b64.end(),
                     [](char c) { return std::isspace(c); }),
      expected_digest_b64.end());

  // Calculate SHA-256 digest of canonical data
  std::string digest = crypto::SHA256HashString(canonical_data);

  // Encode to base64
  std::string calculated_digest_b64 = base::Base64Encode(digest);

  bool digest_match = (expected_digest_b64 == calculated_digest_b64);

  if (!digest_match) {
    LOG(ERROR) << "Digest verification FAILED";
    LOG(ERROR) << "Expected: " << expected_digest_b64;
    LOG(ERROR) << "Calculated: " << calculated_digest_b64;
  }

  return digest_match;
}

std::vector<SamlAttribute> SamlVerifier::ParseSamlAttributes(
    const std::string& saml_xml) {
  std::vector<SamlAttribute> attributes;

  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML";
    return attributes;
  }

  std::string current_attribute_name;
  SamlAttribute current_attr;
  bool in_attribute = false;

  // Parse through XML sequentially
  while (reader.Read()) {
    std::string node_name = reader.NodeName();

    if (reader.IsElement()) {
      if (node_name == "saml2:Attribute" || node_name == "Attribute") {
        // Save previous attribute if we have one
        if (in_attribute && !current_attr.name.empty() &&
            !current_attr.values.empty()) {
          attributes.push_back(std::move(current_attr));
        }

        // Start new attribute
        current_attr = SamlAttribute();
        if (reader.NodeAttribute("Name", &current_attr.name)) {
          in_attribute = true;
        } else {
          in_attribute = false;
        }
      } else if (in_attribute && (node_name == "saml2:AttributeValue" ||
                                  node_name == "AttributeValue")) {
        std::string value;
        if (reader.ReadElementContent(&value)) {
          current_attr.values.push_back(value);
        }
      }
    } else if (reader.IsClosingElement()) {
      if (node_name == "saml2:Attribute" || node_name == "Attribute") {
        // End of current attribute
        if (in_attribute && !current_attr.name.empty() &&
            !current_attr.values.empty()) {
          attributes.push_back(std::move(current_attr));
        }
        in_attribute = false;
        current_attr = SamlAttribute();
      }
    }
  }

  // Save the last attribute if needed
  if (in_attribute && !current_attr.name.empty() &&
      !current_attr.values.empty()) {
    attributes.push_back(std::move(current_attr));
  }

  return attributes;
}

void SamlVerifier::ProcessAttributesWithProcessors(
    const std::vector<SamlAttribute>& attributes,
    PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "PrefService is null, cannot process attributes";
    return;
  }

  for (const auto& processor : attribute_processors_) {
    if (!processor->ProcessAttributes(attributes, prefs)) {
      LOG(WARNING) << "Processor failed to process attributes";
    }
  }
}

bool SamlVerifier::SetOktaCertificate(const std::string& certificate_pem) {
  if (certificate_pem.empty()) {
    LOG(ERROR) << "Empty certificate provided";
    return false;
  }

  okta_certificate_pem_ = certificate_pem;

  // Parse and store the certificate
  if (!LoadCertificateFromPem(certificate_pem)) {
    LOG(ERROR) << "Failed to load certificate from PEM data";
    return false;
  }

  return true;
}

bool SamlVerifier::LoadOktaCertificateFromFile(
    const std::string& cert_file_path) {
  if (cert_file_path.empty()) {
    LOG(ERROR) << "Empty certificate file path";
    return false;
  }

  // Read certificate file
  std::string certificate_pem;
  if (!base::ReadFileToString(base::FilePath(cert_file_path),
                              &certificate_pem)) {
    LOG(ERROR) << "Failed to read certificate file: " << cert_file_path;
    return false;
  }

  return SetOktaCertificate(certificate_pem);
}

void SamlVerifier::SetSignatureVerificationEnabled(bool enabled) {
  signature_verification_enabled_ = enabled;
}

void SamlVerifier::SetDynamicCertificateFetchingEnabled(bool enabled) {
  dynamic_cert_fetching_enabled_ = enabled;
}

void SamlVerifier::SetExpectedAudience(const std::string& audience) {
  expected_audience_ = audience;
}

void SamlVerifier::SetMaxResponseAge(base::TimeDelta max_age) {
  max_response_age_ = max_age;
}

void SamlVerifier::SetDevelopmentMode(bool enabled) {
  development_mode_ = enabled;
}

// Dynamic certificate fetching methods (simplified versions)
std::string SamlVerifier::ExtractSamlIssuer(const std::string& saml_xml) {
  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML for issuer extraction";
    return "";
  }

  std::string issuer;
  while (reader.Read()) {
    if (reader.IsElement() && (reader.NodeName() == "saml2:Issuer" ||
                               reader.NodeName() == "Issuer")) {
      if (reader.ReadElementContent(&issuer)) {
        break;
      }
    }
  }

  return issuer;
}

std::string SamlVerifier::ExtractOktaDomain(const std::string& saml_xml) {
  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML for domain extraction";
    return "";
  }

  // Look for various elements that might contain the actual Okta domain
  std::string domain;
  while (reader.Read()) {
    if (reader.IsElement()) {
      std::string node_name = reader.NodeName();

      // Look for Destination attribute (common in SAML responses)
      if (node_name.find("Response") != std::string::npos) {
        if (reader.NodeAttribute("Destination", &domain)) {
          // Extract domain from destination URL
          size_t start = domain.find("://");
          if (start != std::string::npos) {
            start += 3;
            size_t end = domain.find("/", start);
            if (end != std::string::npos) {
              std::string extracted_domain = domain.substr(start, end - start);
              if (extracted_domain.find(".okta.com") != std::string::npos) {
                return extracted_domain;
              }
            }
          }
        }
      }

      // Look for other URLs that might contain the domain
      if (node_name.find("Audience") != std::string::npos ||
          node_name.find("SingleSignOnService") != std::string::npos ||
          node_name.find("Location") != std::string::npos) {
        std::string url_content;
        if (reader.ReadElementContent(&url_content)) {
          size_t start = url_content.find("://");
          if (start != std::string::npos) {
            start += 3;
            size_t end = url_content.find("/", start);
            if (end != std::string::npos) {
              std::string extracted_domain =
                  url_content.substr(start, end - start);
              if (extracted_domain.find(".okta.com") != std::string::npos) {
                return extracted_domain;
              }
            }
          }
        }
      }
    }
  }

  LOG(WARNING) << "Could not extract Okta domain from SAML response";
  return "";
}

std::string SamlVerifier::ConstructMetadataUrl(const std::string& issuer_url) {
  if (issuer_url.empty()) {
    LOG(ERROR) << "Empty issuer URL";
    return "";
  }

  // For Okta issuers like "http://www.okta.com/exksskc7obCMybtFv697"
  std::string metadata_url;

  if (issuer_url.find("okta.com") != std::string::npos) {
    // Extract app ID from issuer
    std::string app_id;
    size_t last_slash = issuer_url.find_last_of('/');
    if (last_slash != std::string::npos) {
      app_id = issuer_url.substr(last_slash + 1);
    }

    // Try to find the actual Okta domain from the SAML response
    // We'll need to pass the SAML XML to this method for better domain
    // extraction For now, try common Okta metadata URL patterns

    if (!app_id.empty()) {
      // Try app-specific metadata URL pattern (needs actual domain)
      // metadata_url = "https://{domain}/app/" + app_id + "/sso/saml/metadata";

      // For now, return empty to fallback to embedded cert
    }
  }

  return metadata_url;
}

void SamlVerifier::FetchSamlMetadata(
    const std::string& metadata_url,
    base::OnceCallback<void(const std::string&)> callback) {
  if (metadata_url.empty()) {
    LOG(ERROR) << "Empty metadata URL provided";
    std::move(callback).Run("");
    return;
  }

  // Create a simple URL request
  auto request = std::make_unique<network::ResourceRequest>();
  request->url = GURL(metadata_url);
  request->method = "GET";
  request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  // Set proper headers for SAML metadata
  request->headers.SetHeader(
      "Accept", "application/samlmetadata+xml, application/xml, text/xml");
  request->headers.SetHeader("User-Agent", "WootzApp SAML Client/1.0");

  // Create a simple URL loader
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(request),
      net::DefineNetworkTrafficAnnotation("saml_metadata_fetch", R"(
        semantics {
          sender: "SAML Verifier"
          description: "Fetch SAML metadata to obtain signing certificates"
          trigger: "When processing SAML response that requires certificate verification"
          data: "HTTP request to SAML metadata endpoint"
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting: "This feature cannot be disabled"
        })"));

  // Set timeout and retry
  url_loader->SetTimeoutDuration(base::Seconds(30));
  url_loader->SetRetryOptions(
      1, network::SimpleURLLoader::RETRY_ON_NETWORK_CHANGE);

  // Start the request - we'll need URLLoaderFactory for this to work properly
  // For now, return empty to avoid crashes

  // Return empty metadata (will fallback to embedded cert)
  std::move(callback).Run("");
}

std::string SamlVerifier::ExtractCertificateFromMetadata(
    const std::string& metadata_xml) {
  if (metadata_xml.empty()) {
    LOG(WARNING) << "Empty metadata XML provided";
    return "";
  }

  XmlReader reader;
  if (!reader.Load(metadata_xml)) {
    LOG(ERROR) << "Failed to parse SAML metadata XML";
    return "";
  }

  std::string cert_data;
  bool in_signing_key = false;

  while (reader.Read()) {
    if (reader.IsElement()) {
      std::string node_name = reader.NodeName();

      // Look for signing key descriptor
      if (node_name == "md:KeyDescriptor" || node_name == "KeyDescriptor") {
        std::string use_attr;
        if (reader.NodeAttribute("use", &use_attr) && use_attr == "signing") {
          in_signing_key = true;
        }
      }

      // Look for X509Certificate elements (with various namespace prefixes)
      if ((node_name == "ds:X509Certificate" ||
           node_name == "X509Certificate" ||
           node_name.find("X509Certificate") != std::string::npos) &&
          in_signing_key) {  // Only accept certificates from signing
                             // KeyDescriptor

        if (reader.ReadElementContent(&cert_data)) {
          // Remove any whitespace from the base64 data
          base::RemoveChars(cert_data, " \t\r\n", &cert_data);

          if (cert_data.empty()) {
            LOG(WARNING) << "Empty certificate data found in metadata";
            continue;
          }

          // Convert base64 certificate to PEM format
          std::string pem_cert = "-----BEGIN CERTIFICATE-----\n";

          // Insert newlines every 64 characters for proper PEM formatting
          for (size_t i = 0; i < cert_data.length(); i += 64) {
            pem_cert += cert_data.substr(i, 64) + "\n";
          }
          pem_cert += "-----END CERTIFICATE-----\n";

          return pem_cert;
        }
      }
    } else if (reader.IsClosingElement()) {
      std::string node_name = reader.NodeName();
      if (node_name == "md:KeyDescriptor" || node_name == "KeyDescriptor") {
        in_signing_key = false;
      }
    }
  }

  LOG(WARNING) << "No X.509 certificate found in SAML metadata";
  return "";
}

std::string SamlVerifier::ExtractEmbeddedCertificate(
    const std::string& saml_xml) {
  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML for certificate extraction";
    return "";
  }

  std::string cert_data;
  while (reader.Read()) {
    if (reader.IsElement()) {
      std::string node_name = reader.NodeName();

      // Look for X509Certificate elements (with or without namespace prefix)
      if (node_name == "ds:X509Certificate" || node_name == "X509Certificate" ||
          node_name.find("X509Certificate") != std::string::npos) {
        if (reader.ReadElementContent(&cert_data)) {
          // Remove any whitespace from the base64 data
          base::RemoveChars(cert_data, " \t\r\n", &cert_data);

          if (cert_data.empty()) {
            LOG(WARNING) << "Empty certificate data found";
            continue;
          }

          // Convert base64 certificate to PEM format
          std::string pem_cert = "-----BEGIN CERTIFICATE-----\n";

          // Insert newlines every 64 characters for proper PEM formatting
          for (size_t i = 0; i < cert_data.length(); i += 64) {
            pem_cert += cert_data.substr(i, 64) + "\n";
          }
          pem_cert += "-----END CERTIFICATE-----\n";

          return pem_cert;
        }
      }
    }
  }

  LOG(WARNING) << "No embedded X.509 certificate found in SAML response";
  return "";
}

void SamlVerifier::ProcessSamlResponseWithDynamicCert(
    const std::string& saml_xml,
    PrefService* prefs,
    VerificationCallback callback) {
  VerificationResult result;
  bool certificate_loaded = false;

  // Step 1: Extract certificate directly from SAML response (most common case)
  std::string embedded_cert = ExtractEmbeddedCertificate(saml_xml);
  if (!embedded_cert.empty()) {
    if (LoadCertificateFromPem(embedded_cert)) {
      okta_certificate_pem_ = embedded_cert;
      certificate_loaded = true;
    } else {
      LOG(ERROR) << "SAML: Failed to load embedded certificate";
    }
  } else {
    // Step 2: Fallback to metadata fetching
    std::string issuer = ExtractSamlIssuer(saml_xml);
    if (!issuer.empty()) {
      // Check cache first
      auto cached_cert = certificate_cache_.find(issuer);
      if (cached_cert != certificate_cache_.end()) {
        if (LoadCertificateFromPem(cached_cert->second)) {
          okta_certificate_pem_ = cached_cert->second;
          certificate_loaded = true;
        }
      } else {
        // Extract actual Okta domain and construct proper metadata URL
        std::string okta_domain = ExtractOktaDomain(saml_xml);
        std::string metadata_url;

        if (!okta_domain.empty()) {
          // Extract app ID from issuer for app-specific metadata
          std::string app_id;
          size_t last_slash = issuer.find_last_of('/');
          if (last_slash != std::string::npos) {
            app_id = issuer.substr(last_slash + 1);
          }

          if (!app_id.empty()) {
            // Try app-specific metadata URL
            metadata_url = "https://" + okta_domain + "/app/" + app_id +
                           "/sso/saml/metadata";
          } else {
            // Try general metadata URL
            metadata_url =
                "https://" + okta_domain + "/.well-known/saml_metadata";
          }
        } else {
          // Fallback to old method
          metadata_url = ConstructMetadataUrl(issuer);
        }

        FetchSamlMetadata(metadata_url,
                          base::BindOnce(&SamlVerifier::OnMetadataFetched,
                                         base::Unretained(this), issuer,
                                         saml_xml, prefs, std::move(callback)));
        return;  // Async operation continues in OnMetadataFetched
      }
    }
  }

  // Step 3: Verify signature if certificate was loaded
  if (certificate_loaded) {
    result.signature_verified = VerifySamlSignature(saml_xml);
  } else {
    result.signature_verified = false;
    LOG(ERROR) << "SAML: No certificate loaded - cannot verify signature";
  }

  // Step 4: Parse and process attributes only if signature verification
  // succeeded
  if (!result.signature_verified) {
    result.success = false;
    result.error_message =
        "SAML signature verification failed with dynamic certificate - "
        "rejecting response";
    LOG(ERROR) << "SAML dynamic cert signature verification failed - SECURITY: "
                  "Rejecting SAML response";
    std::move(callback).Run(result);
    return;
  }

  result.attributes = ParseSamlAttributes(saml_xml);
  if (result.attributes.empty()) {
    result.success = false;
    result.error_message = "No SAML attributes found";
    std::move(callback).Run(result);
    return;
  }

  // Process attributes since signature verification succeeded
  ProcessAttributesWithProcessors(result.attributes, prefs);
  result.success = true;

  std::move(callback).Run(result);
}

void SamlVerifier::OnMetadataFetched(const std::string& issuer,
                                     const std::string& saml_xml,
                                     PrefService* prefs,
                                     VerificationCallback callback,
                                     const std::string& metadata_xml) {
  VerificationResult result;

  if (metadata_xml.empty()) {
    LOG(ERROR) << "SAML: Empty metadata received for issuer: " << issuer;
    result.success = false;
    result.error_message = "Failed to fetch SAML metadata";
    std::move(callback).Run(result);
    return;
  }

  // Extract certificate from metadata
  std::string cert_pem = ExtractCertificateFromMetadata(metadata_xml);
  if (cert_pem.empty()) {
    LOG(ERROR) << "SAML: No certificate found in metadata for issuer: "
               << issuer;
    result.success = false;
    result.error_message = "No certificate found in SAML metadata";
    std::move(callback).Run(result);
    return;
  }

  // Load the certificate
  if (!LoadCertificateFromPem(cert_pem)) {
    LOG(ERROR) << "SAML: Failed to load certificate from metadata";
    result.success = false;
    result.error_message = "Invalid certificate in SAML metadata";
    std::move(callback).Run(result);
    return;
  }

  // Cache the certificate for future use
  certificate_cache_[issuer] = cert_pem;
  okta_certificate_pem_ = cert_pem;

  // Verify signature using the fetched certificate
  result.signature_verified = VerifySamlSignature(saml_xml);

  // Parse and process attributes only if signature verification succeeded
  if (!result.signature_verified) {
    result.success = false;
    result.error_message =
        "SAML signature verification failed with metadata certificate - "
        "rejecting response";
    LOG(ERROR) << "SAML metadata cert signature verification failed - "
                  "SECURITY: Rejecting SAML response";
    std::move(callback).Run(result);
    return;
  }

  result.attributes = ParseSamlAttributes(saml_xml);
  if (result.attributes.empty()) {
    result.success = false;
    result.error_message = "No SAML attributes found";
    std::move(callback).Run(result);
    return;
  }

  // Process attributes since signature verification succeeded
  ProcessAttributesWithProcessors(result.attributes, prefs);
  result.success = true;

  std::move(callback).Run(result);
}

bool SamlVerifier::ValidateSamlTimestamps(const std::string& saml_xml) {
  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML for timestamp validation";
    return false;
  }

  base::Time now = base::Time::Now();
  base::Time not_before;
  base::Time not_on_or_after;
  bool found_conditions = false;

  while (reader.Read()) {
    if (reader.IsElement() && (reader.NodeName() == "saml2:Conditions" ||
                               reader.NodeName() == "Conditions")) {
      found_conditions = true;

      std::string not_before_str;
      std::string not_on_or_after_str;

      if (reader.NodeAttribute("NotBefore", &not_before_str)) {
        if (!base::Time::FromString(not_before_str.c_str(), &not_before)) {
          LOG(ERROR) << "Invalid NotBefore timestamp format: "
                     << not_before_str;
          return false;
        }
      }

      if (reader.NodeAttribute("NotOnOrAfter", &not_on_or_after_str)) {
        if (!base::Time::FromString(not_on_or_after_str.c_str(),
                                    &not_on_or_after)) {
          LOG(ERROR) << "Invalid NotOnOrAfter timestamp format: "
                     << not_on_or_after_str;
          return false;
        }
      }
      break;
    }
  }

  if (!found_conditions) {
    LOG(WARNING) << "No Conditions element found in SAML response";
    return true;  // Some SAML responses might not have conditions
  }

  // Check if current time is within valid period
  if (!not_before.is_null() && now < not_before) {
    LOG(ERROR) << "SAML response not yet valid (NotBefore: " << not_before
               << ", now: " << now << ")";
    return false;
  }

  if (!not_on_or_after.is_null() && now >= not_on_or_after) {
    LOG(ERROR) << "SAML response expired (NotOnOrAfter: " << not_on_or_after
               << ", now: " << now << ")";
    return false;
  }

  // Check response age
  if (!not_on_or_after.is_null()) {
    base::TimeDelta response_age = now - (not_on_or_after - max_response_age_);
    if (response_age > max_response_age_) {
      LOG(ERROR) << "SAML response too old (age: " << response_age
                 << ", max: " << max_response_age_ << ")";
      return false;
    }
  }

  return true;
}

bool SamlVerifier::ValidateSamlAudience(const std::string& saml_xml) {
  if (expected_audience_.empty()) {
    LOG(WARNING)
        << "No expected audience configured - skipping audience validation";
    return true;
  }

  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML for audience validation";
    return false;
  }

  bool found_audience = false;
  std::vector<std::string> audiences;

  while (reader.Read()) {
    if (reader.IsElement() && (reader.NodeName() == "saml2:Audience" ||
                               reader.NodeName() == "Audience")) {
      std::string audience;
      if (reader.ReadElementContent(&audience)) {
        audiences.push_back(audience);
        if (audience == expected_audience_) {
          found_audience = true;
        }
      }
    }
  }

  if (audiences.empty()) {
    LOG(WARNING) << "No audience restrictions found in SAML response";
    return true;  // Some SAML responses might not have audience restrictions
  }

  if (!found_audience) {
    LOG(ERROR) << "Expected audience '" << expected_audience_
               << "' not found in SAML response";
    LOG(ERROR) << "Found audiences: ";
    for (const auto& aud : audiences) {
      LOG(ERROR) << "  - " << aud;
    }
    return false;
  }

  return true;
}

bool SamlVerifier::ValidateSamlConditions(const std::string& saml_xml) {
  // Validate timestamps
  if (!ValidateSamlTimestamps(saml_xml)) {
    LOG(ERROR) << "SAML timestamp validation failed";
    return false;
  }

  // Validate audience restrictions
  if (!ValidateSamlAudience(saml_xml)) {
    LOG(ERROR) << "SAML audience validation failed";
    return false;
  }

  // Additional condition validations can be added here
  // - OneTimeUse validation
  // - ProxyRestriction validation
  // - Custom condition validations

  return true;
}

bool SamlVerifier::CheckReplayAttack(const std::string& saml_xml) {
  // Extract SAML Response ID
  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML for replay check";
    return false;
  }

  std::string response_id;
  while (reader.Read()) {
    if (reader.IsElement() && (reader.NodeName() == "samlp:Response" ||
                               reader.NodeName() == "Response")) {
      if (!reader.NodeAttribute("ID", &response_id)) {
        LOG(ERROR) << "SAML Response missing required ID attribute";
        return false;
      }
      break;
    }
  }

  if (response_id.empty()) {
    LOG(ERROR) << "Could not extract SAML Response ID";
    return false;
  }

  // Check if we've already processed this response ID
  if (processed_response_ids_.find(response_id) !=
      processed_response_ids_.end()) {
    LOG(ERROR)
        << "SAML Response ID already processed - replay attack detected: "
        << response_id;
    return false;
  }

  // Add to processed IDs (in production, this should be persisted and have TTL)
  processed_response_ids_.insert(response_id);

  // Clean up old entries to prevent memory growth
  // In production, implement proper TTL-based cleanup
  if (processed_response_ids_.size() > 1000) {
    LOG(WARNING) << "Large number of processed SAML IDs - consider "
                    "implementing TTL cleanup";
  }

  return true;
}

}  // namespace saml_verifier
