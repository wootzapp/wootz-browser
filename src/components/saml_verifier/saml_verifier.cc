// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/saml_verifier/saml_verifier.h"

#include "third_party/libxml/chromium/xml_reader.h"
#include "third_party/libxml/chromium/libxml_utils.h"

// Only use libxml2_full for C14N functionality  
#include <libxml/c14n.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string>
#include <vector>
#include <memory>

#include "base/logging.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/base64.h"
#include "base/containers/span.h"
#include "components/prefs/pref_service.h"

#include "content/public/browser/blocked_domains_prefs.h"
#include "content/public/browser/saml_prefs.h"
#include "content/public/browser/domain_block_checker.h"

// Use Chromium's crypto instead of xmlsec
#include "crypto/signature_verifier.h"
#include "crypto/sha2.h"
#include "third_party/boringssl/src/include/openssl/x509.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"

// Network includes for dynamic certificate fetching
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "url/gurl.h"
#include "base/time/time.h"
#include "base/functional/bind.h"

namespace saml_verifier {

// Struct implementations (required by Chromium style)
SamlAttribute::SamlAttribute() = default;
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

  if (blocked_domains.empty()) {
    LOG(INFO) << "No blocked domains found in SAML response";
    return true;  // Not an error, just no domains to block
  }

  // Update the blocked domains preference
  base::Value::List domain_list;
  for (const std::string& domain : blocked_domains) {
    domain_list.Append(domain);
  }

  prefs->SetList(blocked_domains::prefs::kBlockedDomains, std::move(domain_list));
  
  LOG(INFO) << "Updated blocked domains with " << blocked_domains.size() 
            << " domains from SAML response";
  
  return true;
}

std::vector<std::string> DomainAttributeProcessor::GetHandledAttributes() const {
  return {"blocked_domains"};
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

  LOG(INFO) << "Processing stored SAML response";
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
  
  if (signature_verification_enabled_ && !result.signature_verified) {
    result.success = false;
    result.error_message = "SAML signature verification failed";
    std::move(callback).Run(result);
    return;
  }
  
  // Parse SAML attributes
  result.attributes = ParseSamlAttributes(saml_xml);
  
  if (result.attributes.empty()) {
    result.success = false;
    result.error_message = "No SAML attributes found";
    std::move(callback).Run(result);
    return;
  }

  // Process attributes with registered processors
  ProcessAttributesWithProcessors(result.attributes, prefs);
  
  result.success = true;
  LOG(INFO) << "Successfully processed SAML response with " 
            << result.attributes.size() << " attributes";
  
  std::move(callback).Run(result);
}

void SamlVerifier::RegisterAttributeProcessor(
    std::unique_ptr<SamlAttributeProcessor> processor) {
  attribute_processors_.push_back(std::move(processor));
}

void SamlVerifier::RegisterDomainProcessor() {
  RegisterAttributeProcessor(std::make_unique<DomainAttributeProcessor>());
}

// static
void SamlVerifier::ProcessNewSamlResponse(PrefService* prefs) {
  if (!prefs) {
    LOG(ERROR) << "SAML: PrefService is null";
    return;
  }
  
  LOG(INFO) << "SAML: Processing new SAML response automatically";
  
  // Create and configure verifier
  auto verifier = std::make_unique<SamlVerifier>();
  verifier->RegisterDomainProcessor();
  verifier->SetSignatureVerificationEnabled(true);
  verifier->SetDynamicCertificateFetchingEnabled(true);
  
  // Move the verifier to the callback to keep it alive
  auto* verifier_ptr = verifier.get();
  verifier_ptr->ProcessStoredSamlResponse(
      prefs,
      base::BindOnce([](std::unique_ptr<SamlVerifier> verifier,
                        const VerificationResult& result) {
        if (result.success) {
          LOG(INFO) << "SAML: Successfully processed and verified SAML response";
          LOG(INFO) << "SAML: Signature verified: " << result.signature_verified;
          LOG(INFO) << "SAML: Found " << result.attributes.size() << " attributes";
          
          // Log blocked domains if found
          auto blocked_domains = result.GetAttributeValues("blocked_domains");
          if (!blocked_domains.empty()) {
            LOG(INFO) << "SAML: Updated blocked domains with " << blocked_domains.size() << " entries";
          }
        } else {
          LOG(ERROR) << "SAML: Failed to process SAML response: " << result.error_message;
        }
        // verifier is automatically destroyed here
      }, std::move(verifier)));
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

  LOG(INFO) << "Successfully loaded X.509 certificate (" << certificate_der_.size() << " bytes)";
  return true;
}

bool SamlVerifier::VerifySamlSignature(const std::string& saml_xml) {
  if (!signature_verification_enabled_) {
    LOG(INFO) << "Signature verification is disabled";
    return true;  // Consider as verified when disabled
  }

  if (okta_certificate_pem_.empty() || certificate_der_.empty()) {
    LOG(ERROR) << "No Okta certificate loaded for signature verification";
    return false;
  }

  // Extract signature data from SAML XML
  SignatureData sig_data;
  if (!ExtractSignatureData(saml_xml, &sig_data)) {
    LOG(ERROR) << "Failed to extract signature data from SAML";
    return false;
  }

  // Extract certificate from the same signature element we're verifying
  std::string signature_cert = ExtractCertificateFromSignature(saml_xml, sig_data.signed_info_xml);
  if (!signature_cert.empty() && signature_cert != okta_certificate_pem_) {
    LOG(INFO) << "Using certificate from specific signature instead of cached certificate";
    if (!LoadCertificateFromPem(signature_cert)) {
      LOG(ERROR) << "Failed to load certificate from signature element";
      return false;
    }
  }

  // Extract the Reference URI to find what element is being signed
  std::string reference_uri = ExtractReferenceUri(sig_data.signed_info_xml);
  if (reference_uri.empty()) {
    LOG(ERROR) << "Failed to extract Reference URI from SignedInfo";
    return false;
  }

  LOG(INFO) << "SAML signature references element: " << reference_uri;

  // Apply SAML signature transforms to get the canonicalized signed data
  std::string canonicalized_data = ApplySamlSignatureTransforms(saml_xml, reference_uri);
  if (canonicalized_data.empty()) {
    LOG(ERROR) << "Failed to apply SAML signature transforms";
    return false;
  }

  LOG(INFO) << "Canonicalized signed data length: " << canonicalized_data.length();

  // Verify the RSA signature against the properly transformed data
  bool signature_valid = VerifyRsaSignature(canonicalized_data, sig_data.signature_value);

  if (signature_valid) {
    LOG(INFO) << "SAML signature verification SUCCESSFUL";
  } else {
    LOG(ERROR) << "SAML signature verification FAILED";
  }

  return signature_valid;
}

bool SamlVerifier::ExtractSignatureData(const std::string& saml_xml, SignatureData* sig_data) {
  if (!sig_data) {
    return false;
  }

  LOG(INFO) << "Starting signature data extraction from SAML XML";

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
  
  sig_data->signed_info_xml = saml_xml.substr(signed_info_start, signed_info_end - signed_info_start);
  sig_data->canonical_xml = CanonicalizeXml(sig_data->signed_info_xml);
  
  LOG(INFO) << "Extracted SignedInfo XML (" << sig_data->signed_info_xml.length() << " chars)";
  LOG(INFO) << "Canonicalized XML (" << sig_data->canonical_xml.length() << " chars)";
  LOG(INFO) << "Complete SignedInfo: " << sig_data->signed_info_xml;

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
      (next_signature != std::string::npos && sig_value_start > next_signature)) {
    LOG(ERROR) << "No SignatureValue element found in same signature as SignedInfo";
    return false;
  }
  
  // Find the start of the actual signature value content
  size_t content_start = saml_xml.find('>', sig_value_start);
  if (content_start == std::string::npos) {
    LOG(ERROR) << "Malformed SignatureValue element";
    return false;
  }
  content_start++; // Move past the '>'
  
  // Find the closing tag
  size_t sig_value_end = saml_xml.find("</ds:SignatureValue>", content_start);
  if (sig_value_end == std::string::npos) {
    sig_value_end = saml_xml.find("</SignatureValue>", content_start);
  }
  
  if (sig_value_end == std::string::npos) {
    LOG(ERROR) << "No closing SignatureValue tag found in SAML";
    return false;
  }
  
  std::string raw_signature = saml_xml.substr(content_start, sig_value_end - content_start);
  
  // Clean up whitespace from signature value
  raw_signature.erase(
    std::remove_if(raw_signature.begin(), raw_signature.end(),
                   [](char c) { return std::isspace(c); }),
    raw_signature.end());
  
  sig_data->signature_value = raw_signature;
  
  LOG(INFO) << "Extracted SignatureValue (" << sig_data->signature_value.length() << " chars)";
  LOG(INFO) << "First 50 chars of SignatureValue: " << sig_data->signature_value.substr(0, 50);
  
  bool success = !sig_data->canonical_xml.empty() && !sig_data->signature_value.empty();
  
  if (!success) {
    LOG(ERROR) << "Failed to extract complete signature data";
  } else {
    LOG(INFO) << "Successfully extracted signature data";
  }

  return success;
}

std::string SamlVerifier::CanonicalizeXml(const std::string& xml) {
  LOG(INFO) << "Starting libxml2 Exclusive C14N canonicalization (input length: " << xml.length() << ")";
  
  // Parse XML document using libxml2
  xmlDocPtr doc = xmlParseMemory(xml.c_str(), static_cast<int>(xml.length()));
  if (!doc) {
    LOG(ERROR) << "Failed to parse XML for canonicalization";
    return "";
  }

  // Perform Exclusive Canonicalization (Exc-C14N) as required by SAML
  xmlChar* canonical_output = nullptr;
  int canonical_size = xmlC14NDocDumpMemory(
      doc,                           // XML document
      nullptr,                       // nodes (nullptr = entire document)
      XML_C14N_EXCLUSIVE_1_0,       // Exclusive C14N mode (used by SAML)
      nullptr,                       // inclusive namespace prefixes
      0,                            // with_comments (0 = without comments)
      &canonical_output             // output buffer
  );
  
  std::string result;
  if (canonical_size > 0 && canonical_output) {
    result.assign(reinterpret_cast<char*>(canonical_output), canonical_size);
    xmlFree(canonical_output);
    
    LOG(INFO) << "libxml2 C14N canonicalization successful";
    LOG(INFO) << "Canonical XML length: " << result.length() 
              << " (input: " << xml.length() << ")";
    LOG(INFO) << "First 200 chars of canonical XML: " << result.substr(0, 200);
  } else {
    LOG(ERROR) << "libxml2 canonicalization failed, error code: " << canonical_size;
    result = xml; // Fallback to original XML
  }
  
  xmlFreeDoc(doc);
  return result;
}

std::string SamlVerifier::ExtractReferenceUri(const std::string& signed_info_xml) {
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
  
  uri_start += 5; // Move past 'URI="'
  size_t uri_end = signed_info_xml.find('\"', uri_start);
  if (uri_end == std::string::npos) {
    LOG(ERROR) << "Malformed URI attribute in Reference element";
    return "";
  }
  
  std::string uri = signed_info_xml.substr(uri_start, uri_end - uri_start);
  LOG(INFO) << "Extracted Reference URI: " << uri;
  return uri;
}

std::string SamlVerifier::ApplySamlSignatureTransforms(const std::string& saml_xml, const std::string& reference_uri) {
  // For SAML, we need to:
  // 1. Find the referenced element (usually Response with matching ID)
  // 2. Apply enveloped signature transform (remove Signature element)
  // 3. Apply exclusive canonicalization
  
  if (reference_uri.empty() || reference_uri[0] != '#') {
    LOG(ERROR) << "Invalid reference URI: " << reference_uri;
    return "";
  }
  
  std::string element_id = reference_uri.substr(1); // Remove '#' prefix
  LOG(INFO) << "Looking for element with ID: " << element_id;
  
  // Find the element with the matching ID attribute
  std::string id_pattern = "ID=\"" + element_id + "\"";
  size_t element_start = saml_xml.find(id_pattern);
  if (element_start == std::string::npos) {
    LOG(ERROR) << "Cannot find element with ID: " << element_id;
    return "";
  }
  
  // Find the start of the element that contains this ID
  size_t tag_start = saml_xml.rfind('<', element_start);
  if (tag_start == std::string::npos) {
    LOG(ERROR) << "Cannot find element start for ID: " << element_id;
    return "";
  }
  
  // Extract the element name
  size_t name_end = saml_xml.find_first_of(" >", tag_start + 1);
  if (name_end == std::string::npos) {
    LOG(ERROR) << "Cannot parse element name";
    return "";
  }
  
  std::string element_name = saml_xml.substr(tag_start + 1, name_end - tag_start - 1);
  LOG(INFO) << "Found referenced element: " << element_name;
  
  // Find the closing tag for this element
  std::string closing_tag = "</" + element_name + ">";
  size_t element_end = saml_xml.find(closing_tag, element_start);
  if (element_end == std::string::npos) {
    LOG(ERROR) << "Cannot find closing tag for element: " << element_name;
    return "";
  }
  element_end += closing_tag.length();
  
  // Extract the complete referenced element
  std::string referenced_element = saml_xml.substr(tag_start, element_end - tag_start);
  LOG(INFO) << "Extracted referenced element (" << referenced_element.length() << " chars)";
  
  // Apply enveloped signature transform - remove the Signature element
  std::string transformed = ApplyEnvelopedSignatureTransform(referenced_element);
  if (transformed.empty()) {
    LOG(ERROR) << "Failed to apply enveloped signature transform";
    return "";
  }
  
  // Apply exclusive canonicalization
  std::string canonicalized = CanonicalizeXml(transformed);
  LOG(INFO) << "Applied signature transforms, final length: " << canonicalized.length();
  
  return canonicalized;
}

std::string SamlVerifier::ApplyEnvelopedSignatureTransform(const std::string& xml) {
  // Remove the entire ds:Signature element from the XML
  std::string result = xml;
  
  size_t sig_start = result.find("<ds:Signature");
  if (sig_start == std::string::npos) {
    sig_start = result.find("<Signature");
  }
  
  if (sig_start == std::string::npos) {
    LOG(INFO) << "No Signature element found to remove";
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
  
  LOG(INFO) << "Removed Signature element, remaining length: " << result.length();
  return result;
}

std::string SamlVerifier::ExtractCertificateFromSignature(const std::string& saml_xml, const std::string& signed_info_xml) {
  // Find the signature element that contains this SignedInfo
  size_t signed_info_pos = saml_xml.find(signed_info_xml);
  if (signed_info_pos == std::string::npos) {
    LOG(WARNING) << "Cannot find SignedInfo in full SAML XML";
    return "";
  }
  
  // Find the signature element that contains this SignedInfo
  size_t sig_start = saml_xml.rfind("<ds:Signature", signed_info_pos);
  if (sig_start == std::string::npos) {
    sig_start = saml_xml.rfind("<Signature", signed_info_pos);
  }
  
  if (sig_start == std::string::npos) {
    LOG(WARNING) << "Cannot find Signature element containing SignedInfo";
    return "";
  }
  
  // Find the end of this signature
  size_t sig_end = saml_xml.find("</ds:Signature>", sig_start);
  if (sig_end == std::string::npos) {
    sig_end = saml_xml.find("</Signature>", sig_start);
  }
  
  if (sig_end == std::string::npos) {
    LOG(WARNING) << "Cannot find end of Signature element";
    return "";
  }
  sig_end = saml_xml.find('>', sig_end) + 1;
  
  // Extract just this signature element
  std::string signature_element = saml_xml.substr(sig_start, sig_end - sig_start);
  LOG(INFO) << "Extracting certificate from signature element (" << signature_element.length() << " chars)";
  
  // Extract certificate from this specific signature
  return ExtractEmbeddedCertificate(signature_element);
}

bool SamlVerifier::VerifyRsaSignature(const std::string& data, 
                                      const std::string& signature_base64) {
  // Decode base64 signature
  std::string signature_bytes;
  if (!base::Base64Decode(signature_base64, &signature_bytes)) {
    LOG(ERROR) << "Failed to decode base64 signature";
    return false;
  }

  LOG(INFO) << "Verifying RSA signature - Data size: " << data.length() 
            << " bytes, Signature size: " << signature_bytes.length() << " bytes";

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
  
  // Initialize verifier with RSA-SHA256 using correct Chromium API (safe conversion)
  bool init_success = verifier->VerifyInit(
      crypto::SignatureVerifier::RSA_PKCS1_SHA256,
      base::span<const uint8_t>(reinterpret_cast<const uint8_t*>(signature_bytes.data()), signature_bytes.length()),
      base::span<const uint8_t>(key_der, static_cast<size_t>(key_der_length)));

  OPENSSL_free(key_der);
  EVP_PKEY_free(public_key);

  if (!init_success) {
    LOG(ERROR) << "Failed to initialize signature verifier";
    return false;
  }

  // IMPORTANT: Pass the raw canonicalized data, not the hash!
  // crypto::SignatureVerifier::RSA_PKCS1_SHA256 will calculate SHA-256 internally
  verifier->VerifyUpdate(base::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.length()));

  // Finalize verification
  bool signature_valid = verifier->VerifyFinal();
  
  LOG(INFO) << "RSA signature verification result: " << (signature_valid ? "VALID" : "INVALID");
  
  if (!signature_valid) {
    LOG(ERROR) << "Signature verification failed - this could be due to:";
    LOG(ERROR) << "1. Incorrect XML canonicalization";
    LOG(ERROR) << "2. Wrong certificate used for verification";
    LOG(ERROR) << "3. Signature algorithm mismatch";
    LOG(ERROR) << "First 100 chars of canonicalized data: " << data.substr(0, 100);
  }
  
  return signature_valid;
}

std::vector<SamlAttribute> SamlVerifier::ParseSamlAttributes(
    const std::string& saml_xml) {
  std::vector<SamlAttribute> attributes;

  XmlReader reader;
  if (!reader.Load(saml_xml)) {
    LOG(ERROR) << "Failed to parse SAML XML";
    return attributes;
  }

  // Parse through XML looking for Attribute elements
  while (reader.Read()) {
    if (reader.IsElement() && 
        (reader.NodeName() == "saml2:Attribute" || reader.NodeName() == "Attribute")) {
      
      SamlAttribute saml_attr;
      
      // Get attribute name
      if (!reader.NodeAttribute("Name", &saml_attr.name)) {
        continue;
      }
      
      // Read attribute values by looking ahead until we find the closing Attribute tag
      int attribute_depth = reader.Depth();
      bool reading_attribute = true;
      
      while (reading_attribute && reader.Read()) {
        if (reader.Depth() <= attribute_depth && reader.IsClosingElement()) {
          // We've reached the closing tag of the current Attribute
          reading_attribute = false;
        } else if (reader.IsElement() && 
                   (reader.NodeName() == "saml2:AttributeValue" || 
                    reader.NodeName() == "AttributeValue")) {
          std::string value;
          if (reader.ReadElementContent(&value)) {
            saml_attr.values.push_back(value);
          }
        }
      }
      
      if (!saml_attr.values.empty()) {
        attributes.push_back(std::move(saml_attr));
        LOG(INFO) << "Found SAML attribute: " << saml_attr.name 
                  << " with " << saml_attr.values.size() << " values";
      }
    }
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
    if (processor->ProcessAttributes(attributes, prefs)) {
      LOG(INFO) << "Successfully processed attributes with processor";
    } else {
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
  
  LOG(INFO) << "Okta certificate loaded successfully";
  return true;
}

bool SamlVerifier::LoadOktaCertificateFromFile(const std::string& cert_file_path) {
  if (cert_file_path.empty()) {
    LOG(ERROR) << "Empty certificate file path";
    return false;
  }
  
  // Read certificate file
  std::string certificate_pem;
  if (!base::ReadFileToString(base::FilePath(cert_file_path), &certificate_pem)) {
    LOG(ERROR) << "Failed to read certificate file: " << cert_file_path;
    return false;
  }
  
  return SetOktaCertificate(certificate_pem);
}

void SamlVerifier::SetSignatureVerificationEnabled(bool enabled) {
  signature_verification_enabled_ = enabled;
  LOG(INFO) << "Signature verification " << (enabled ? "enabled" : "disabled");
}

void SamlVerifier::SetDynamicCertificateFetchingEnabled(bool enabled) {
  dynamic_cert_fetching_enabled_ = enabled;
  LOG(INFO) << "Dynamic certificate fetching " << (enabled ? "enabled" : "disabled");
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
    if (reader.IsElement() && 
        (reader.NodeName() == "saml2:Issuer" || reader.NodeName() == "Issuer")) {
      if (reader.ReadElementContent(&issuer)) {
        break;
      }
    }
  }

  LOG(INFO) << "Extracted SAML issuer: " << issuer;
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
                LOG(INFO) << "Found Okta domain from Destination: " << extracted_domain;
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
              std::string extracted_domain = url_content.substr(start, end - start);
              if (extracted_domain.find(".okta.com") != std::string::npos) {
                LOG(INFO) << "Found Okta domain from " << node_name << ": " << extracted_domain;
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

  LOG(INFO) << "Constructing metadata URL for issuer: " << issuer_url;

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
    // We'll need to pass the SAML XML to this method for better domain extraction
    // For now, try common Okta metadata URL patterns
    
    if (!app_id.empty()) {
      // Try app-specific metadata URL pattern (needs actual domain)
      // metadata_url = "https://{domain}/app/" + app_id + "/sso/saml/metadata";
      
      // For now, log what we would try and return empty to fallback to embedded cert
      LOG(INFO) << "Would try app-specific metadata for app ID: " << app_id;
      LOG(INFO) << "Need actual Okta domain to construct proper metadata URL";
    }
  }
  
  LOG(INFO) << "Constructed metadata URL: " << metadata_url;
  return metadata_url;
}

void SamlVerifier::FetchSamlMetadata(const std::string& metadata_url,
                                     base::OnceCallback<void(const std::string&)> callback) {
  if (metadata_url.empty()) {
    LOG(ERROR) << "Empty metadata URL provided";
    std::move(callback).Run("");
    return;
  }

  LOG(INFO) << "SAML: Fetching metadata from: " << metadata_url;
  
  // Create a simple URL request
  auto request = std::make_unique<network::ResourceRequest>();
  request->url = GURL(metadata_url);
  request->method = "GET";
  request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  
  // Set proper headers for SAML metadata
  request->headers.SetHeader("Accept", "application/samlmetadata+xml, application/xml, text/xml");
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
  url_loader->SetRetryOptions(1, network::SimpleURLLoader::RETRY_ON_NETWORK_CHANGE);

  // Start the request - we'll need URLLoaderFactory for this to work properly
  // For now, return empty to avoid crashes, but log that we tried
  LOG(WARNING) << "SAML: Network fetching requires URLLoaderFactory integration";
  LOG(INFO) << "SAML: Would fetch from: " << metadata_url;
  
  // Return empty metadata (will fallback to embedded cert)
  std::move(callback).Run("");
}

std::string SamlVerifier::ExtractCertificateFromMetadata(const std::string& metadata_xml) {
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
          LOG(INFO) << "Found signing KeyDescriptor in metadata";
        }
      }
      
      // Look for X509Certificate elements (with various namespace prefixes)
      if ((node_name == "ds:X509Certificate" || 
           node_name == "X509Certificate" ||
           node_name.find("X509Certificate") != std::string::npos) && 
          (in_signing_key || !in_signing_key)) { // Accept any cert for now
        
        LOG(INFO) << "Found certificate element in metadata: " << node_name;
        
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
          
          LOG(INFO) << "Successfully extracted certificate from SAML metadata (" 
                    << cert_data.length() << " chars base64)";
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

std::string SamlVerifier::ExtractEmbeddedCertificate(const std::string& saml_xml) {
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
      if (node_name == "ds:X509Certificate" || 
          node_name == "X509Certificate" ||
          node_name.find("X509Certificate") != std::string::npos) {
        
        LOG(INFO) << "Found certificate element: " << node_name;
        
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
          
          LOG(INFO) << "Successfully extracted embedded X.509 certificate (" 
                    << cert_data.length() << " chars base64)";
          return pem_cert;
        }
      }
    }
  }

  LOG(WARNING) << "No embedded X.509 certificate found in SAML response";
  return "";
}

void SamlVerifier::ProcessSamlResponseWithDynamicCert(const std::string& saml_xml,
                                                      PrefService* prefs,
                                                      VerificationCallback callback) {
  LOG(INFO) << "SAML: Starting dynamic certificate processing";
  
  VerificationResult result;
  
  // Step 1: Extract certificate directly from SAML response (most common case)
  std::string embedded_cert = ExtractEmbeddedCertificate(saml_xml);
  if (!embedded_cert.empty()) {
    LOG(INFO) << "SAML: Found embedded certificate in response";
    if (LoadCertificateFromPem(embedded_cert)) {
      LOG(INFO) << "SAML: Successfully loaded embedded certificate";
      okta_certificate_pem_ = embedded_cert;
      
      // Verify signature using embedded certificate
      result.signature_verified = VerifySamlSignature(saml_xml);
      LOG(INFO) << "SAML: Signature verification result: " << result.signature_verified;
    } else {
      LOG(ERROR) << "SAML: Failed to load embedded certificate";
    }
  } else {
    LOG(WARNING) << "SAML: No embedded certificate found, trying metadata fetch";
    
    // Step 2: Fallback to metadata fetching
    std::string issuer = ExtractSamlIssuer(saml_xml);
    if (!issuer.empty()) {
      // Check cache first
      auto cached_cert = certificate_cache_.find(issuer);
      if (cached_cert != certificate_cache_.end()) {
        LOG(INFO) << "SAML: Using cached certificate for issuer: " << issuer;
        if (LoadCertificateFromPem(cached_cert->second)) {
          okta_certificate_pem_ = cached_cert->second;
          result.signature_verified = VerifySamlSignature(saml_xml);
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
            metadata_url = "https://" + okta_domain + "/app/" + app_id + "/sso/saml/metadata";
          } else {
            // Try general metadata URL
            metadata_url = "https://" + okta_domain + "/.well-known/saml_metadata";
          }
        } else {
          // Fallback to old method
          metadata_url = ConstructMetadataUrl(issuer);
        }
        
        LOG(INFO) << "SAML: Fetching certificate from metadata: " << metadata_url;
        
        FetchSamlMetadata(metadata_url, 
          base::BindOnce(&SamlVerifier::OnMetadataFetched, 
                         base::Unretained(this), issuer, saml_xml, prefs, std::move(callback)));
        return; // Async operation continues in OnMetadataFetched
      }
    }
  }
  
  // If signature verification is required but failed, stop here
  if (signature_verification_enabled_ && !result.signature_verified) {
    result.success = false;
    result.error_message = "SAML signature verification failed with dynamic certificate";
    std::move(callback).Run(result);
    return;
  }
  
  // Step 3: Parse and process attributes
  result.attributes = ParseSamlAttributes(saml_xml);
  if (result.attributes.empty()) {
    result.success = false;
    result.error_message = "No SAML attributes found";
  } else {
    ProcessAttributesWithProcessors(result.attributes, prefs);
    result.success = true;
    LOG(INFO) << "SAML: Successfully processed with " << result.attributes.size() << " attributes";
  }
  
  std::move(callback).Run(result);
}

void SamlVerifier::OnMetadataFetched(const std::string& issuer,
                                     const std::string& saml_xml,
                                     PrefService* prefs,
                                     VerificationCallback callback,
                                     const std::string& metadata_xml) {
  LOG(INFO) << "SAML: Processing fetched metadata for issuer: " << issuer;
  
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
    LOG(ERROR) << "SAML: No certificate found in metadata for issuer: " << issuer;
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
  LOG(INFO) << "SAML: Successfully cached certificate for issuer: " << issuer;
  
  // Verify signature using the fetched certificate
  result.signature_verified = VerifySamlSignature(saml_xml);
  LOG(INFO) << "SAML: Signature verification result: " << result.signature_verified;
  
  // If signature verification is required but failed, stop here
  if (signature_verification_enabled_ && !result.signature_verified) {
    result.success = false;
    result.error_message = "SAML signature verification failed with metadata certificate";
    std::move(callback).Run(result);
    return;
  }
  
  // Parse and process attributes
  result.attributes = ParseSamlAttributes(saml_xml);
  if (result.attributes.empty()) {
    result.success = false;
    result.error_message = "No SAML attributes found";
  } else {
    ProcessAttributesWithProcessors(result.attributes, prefs);
    result.success = true;
    LOG(INFO) << "SAML: Successfully processed with " << result.attributes.size() << " attributes";
  }
  
  std::move(callback).Run(result);
}

}  // namespace saml_verifier 