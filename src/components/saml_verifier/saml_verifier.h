// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SAML_VERIFIER_SAML_VERIFIER_H_
#define COMPONENTS_SAML_VERIFIER_SAML_VERIFIER_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <set>

#include "base/functional/callback.h"
#include "base/time/time.h"
#include "crypto/signature_verifier.h"
#include "third_party/boringssl/src/include/openssl/x509.h"

class PrefService;

namespace saml_verifier {

// SAML attribute data structure for extensibility
struct SamlAttribute {
  SamlAttribute();
  SamlAttribute(const SamlAttribute& other);
  SamlAttribute& operator=(const SamlAttribute& other);
  SamlAttribute(SamlAttribute&& other) noexcept;
  SamlAttribute& operator=(SamlAttribute&& other) noexcept;
  ~SamlAttribute();
  
  std::string name;
  std::vector<std::string> values;
};

// Result of SAML verification and processing
struct VerificationResult {
  VerificationResult();
  ~VerificationResult();
  
  bool success = false;
  std::string error_message;
  bool signature_verified = false;
  std::vector<SamlAttribute> attributes;
  
  // Convenience accessors for common use cases
  std::vector<std::string> GetAttributeValues(const std::string& attribute_name) const;
  bool HasAttribute(const std::string& attribute_name) const;
};

// Callback for when verification is complete
using VerificationCallback = base::OnceCallback<void(const VerificationResult&)>;

// Processor interface for handling specific SAML attributes
class SamlAttributeProcessor {
 public:
  virtual ~SamlAttributeProcessor() = default;
  
  // Process specific attributes and update preferences
  virtual bool ProcessAttributes(const std::vector<SamlAttribute>& attributes,
                                 PrefService* prefs) = 0;
  
  // Get the attribute names this processor handles
  virtual std::vector<std::string> GetHandledAttributes() const = 0;
};

// Domain processor for handling blocked_domains attribute
class DomainAttributeProcessor : public SamlAttributeProcessor {
 public:
  DomainAttributeProcessor() = default;
  ~DomainAttributeProcessor() override = default;
  
  bool ProcessAttributes(const std::vector<SamlAttribute>& attributes,
                         PrefService* prefs) override;
  
  std::vector<std::string> GetHandledAttributes() const override;
};

// Content privacy processor for handling content_privacy attribute
class ContentPrivacyAttributeProcessor : public SamlAttributeProcessor {
 public:
  ContentPrivacyAttributeProcessor() = default;
  ~ContentPrivacyAttributeProcessor() override = default;
  
  bool ProcessAttributes(const std::vector<SamlAttribute>& attributes,
                         PrefService* prefs) override;
  
  std::vector<std::string> GetHandledAttributes() const override;
};

// Copy paste processor for handling copy_paste attribute
class CopyPasteAttributeProcessor : public SamlAttributeProcessor {
 public:
  CopyPasteAttributeProcessor() = default;
  ~CopyPasteAttributeProcessor() override = default;
  
  bool ProcessAttributes(const std::vector<SamlAttribute>& attributes,
                         PrefService* prefs) override;
  
  std::vector<std::string> GetHandledAttributes() const override;
};

// Download blocking processor for handling download_blocked_domains attribute
class DownloadBlockingAttributeProcessor : public SamlAttributeProcessor {
 public:
  DownloadBlockingAttributeProcessor() = default;
  ~DownloadBlockingAttributeProcessor() override = default;
  
  bool ProcessAttributes(const std::vector<SamlAttribute>& attributes,
                         PrefService* prefs) override;
  
  std::vector<std::string> GetHandledAttributes() const override;
};

// Upload blocking processor for handling upload_blocked_domains attribute
class UploadBlockingAttributeProcessor : public SamlAttributeProcessor {
 public:
  UploadBlockingAttributeProcessor() = default;
  ~UploadBlockingAttributeProcessor() override = default;
  
  bool ProcessAttributes(const std::vector<SamlAttribute>& attributes,
                         PrefService* prefs) override;
  
  std::vector<std::string> GetHandledAttributes() const override;
};

// Synthetic keystrokes processor for handling synthetic_keystrokes attribute
class SyntheticKeystrokesAttributeProcessor : public SamlAttributeProcessor {
 public:
  SyntheticKeystrokesAttributeProcessor() = default;
  ~SyntheticKeystrokesAttributeProcessor() override = default;
  
  bool ProcessAttributes(const std::vector<SamlAttribute>& attributes,
                         PrefService* prefs) override;
  
  std::vector<std::string> GetHandledAttributes() const override;
};

class SamlVerifier {
 public:
  SamlVerifier();
  ~SamlVerifier();

  // Set Okta's certificate for signature verification
  // certificate_pem: Okta's public certificate in PEM format
  bool SetOktaCertificate(const std::string& certificate_pem);
  
  // Set Okta certificate from file path
  bool LoadOktaCertificateFromFile(const std::string& cert_file_path);

  // Main function: Process stored SAML response 
  // Reads from "wootz.saml_response" preference, verifies signature,
  // and processes attributes using registered processors
  void ProcessStoredSamlResponse(PrefService* prefs,
                                 VerificationCallback callback);

  // For testing: Process a specific SAML response string
  void ProcessSamlResponse(const std::string& saml_xml,
                           PrefService* prefs,
                           VerificationCallback callback);

  // Register attribute processors for different use cases
  void RegisterAttributeProcessor(std::unique_ptr<SamlAttributeProcessor> processor);
  
  // For convenience: Register the built-in domain processor
  void RegisterDomainProcessor();
  
  // For convenience: Register the built-in content privacy processor
  void RegisterContentPrivacyProcessor();
  
  // For convenience: Register the built-in copy paste processor
  void RegisterCopyPasteProcessor();
  
  // For convenience: Register the built-in download blocking processor
  void RegisterDownloadBlockingProcessor();
  
  // For convenience: Register the built-in upload blocking processor
  void RegisterUploadBlockingProcessor();
  
  // For convenience: Register the built-in synthetic keystrokes processor
  void RegisterSyntheticKeystrokesProcessor();
  
  // Enable/disable signature verification (default: enabled)
  void SetSignatureVerificationEnabled(bool enabled);
  
  // Enable dynamic certificate fetching from SAML metadata (default: enabled)
  void SetDynamicCertificateFetchingEnabled(bool enabled);
  
  // Set expected audience for SAML response validation
  void SetExpectedAudience(const std::string& audience);
  
  // Set maximum age for SAML responses (default: 5 minutes)
  void SetMaxResponseAge(base::TimeDelta max_age);
  
  // Enable development/trial mode for less strict certificate validation
  void SetDevelopmentMode(bool enabled);
  
  // Static convenience method: Create, configure, and process SAML automatically
  // This is the main entry point for external code
  static void ProcessNewSamlResponse(PrefService* prefs);

 private:
  // Extract issuer URL from SAML response for metadata fetching
  std::string ExtractSamlIssuer(const std::string& saml_xml);
  
  // Extract actual Okta domain from SAML response
  std::string ExtractOktaDomain(const std::string& saml_xml);
  
  // Construct Okta metadata URL from issuer
  std::string ConstructMetadataUrl(const std::string& issuer_url);
  
  // Fetch SAML metadata XML from Okta endpoint
  void FetchSamlMetadata(const std::string& metadata_url,
                         base::OnceCallback<void(const std::string&)> callback);
  
  // Extract X.509 certificate from SAML metadata XML
  std::string ExtractCertificateFromMetadata(const std::string& metadata_xml);
  
  // Extract embedded X.509 certificate from SAML response
  std::string ExtractEmbeddedCertificate(const std::string& saml_xml);
  
  // Process SAML with dynamic certificate fetching
  void ProcessSamlResponseWithDynamicCert(const std::string& saml_xml,
                                          PrefService* prefs,
                                          VerificationCallback callback);
  
  // Callback for when metadata fetching completes
  void OnMetadataFetched(const std::string& issuer,
                         const std::string& saml_xml,
                         PrefService* prefs,
                         VerificationCallback callback,
                         const std::string& metadata_xml);

  // Load and parse X.509 certificate using Chromium crypto
  bool LoadCertificateFromPem(const std::string& certificate_pem);

  // Verify SAML signature using Chromium's crypto libraries
  bool VerifySamlSignature(const std::string& saml_xml);

  // Parse all SAML attributes from the response
  std::vector<SamlAttribute> ParseSamlAttributes(const std::string& saml_xml);

  // Process attributes using registered processors
  void ProcessAttributesWithProcessors(const std::vector<SamlAttribute>& attributes,
                                       PrefService* prefs);

  // Extract signature data from XML for verification
  struct SignatureData {
    SignatureData();
    ~SignatureData();
    
    std::string signed_info_xml;
    std::string signature_value;
    std::string digest_value;
  };
  bool ExtractSignatureData(const std::string& saml_xml, SignatureData* sig_data);
  
  // Canonicalize XML according to C14N standard
  std::string CanonicalizeXml(const std::string& xml);
  
  // Canonicalize a specific XML element subset using C14N
  std::string CanonicalizeXmlSubset(const std::string& xml, const std::string& element_id);
  
  // Extract Reference URI from SignedInfo for SAML signature verification
  std::string ExtractReferenceUri(const std::string& signed_info_xml);
  
  // Extract signature method algorithm from SignedInfo
  std::string ExtractSignatureMethod(const std::string& signed_info_xml);
  
  // Apply SAML signature transforms (enveloped signature + canonicalization)
  std::string ApplySamlSignatureTransforms(const std::string& saml_xml, 
                                           const std::string& reference_uri);
  
  // Apply enveloped signature transform (remove Signature element)
  std::string ApplyEnvelopedSignatureTransform(const std::string& xml);
  
  // Verify RSA signature using Chromium crypto
  bool VerifyRsaSignature(const std::string& data, 
                          const std::string& signature_base64);
  
  // Certificate validation methods
  bool ValidateCertificate();
  bool ValidateEmbeddedCertificate(const std::string& certificate_pem);
  bool ValidateOktaCertificate(const std::string& certificate_pem);
  bool ValidateCertificatePurpose(X509* cert);
  bool ValidateCertificateIssuer(X509* cert);
  bool ValidateOktaDomain(X509* cert);
  bool ValidateCertificateChain(X509* cert);
  
  // Digest verification for referenced elements
  bool VerifyDigestValue(const std::string& canonical_data, const std::string& signed_info_xml);
  
  // SAML-specific validations
  bool ValidateSamlTimestamps(const std::string& saml_xml);
  bool ValidateSamlAudience(const std::string& saml_xml);
  bool ValidateSamlConditions(const std::string& saml_xml);
  bool CheckReplayAttack(const std::string& saml_xml);

  bool signature_verification_enabled_ = true;
  bool dynamic_cert_fetching_enabled_ = true;
  std::string okta_certificate_pem_;
  std::map<std::string, std::string> certificate_cache_;  // issuer -> certificate
  std::vector<std::unique_ptr<SamlAttributeProcessor>> attribute_processors_;
  
  // Store parsed X.509 certificate for signature verification
  std::vector<uint8_t> certificate_der_;
  std::unique_ptr<crypto::SignatureVerifier> signature_verifier_;
  
  // SAML validation settings and state
  std::string expected_audience_;  // Expected audience for SAML responses
  std::set<std::string> processed_response_ids_;  // For replay attack prevention
  base::TimeDelta max_response_age_ = base::Minutes(5);  // Maximum age for SAML response
  bool development_mode_ = false;  // Allow less strict validation for development/trial
};

}  // namespace saml_verifier

#endif  // COMPONENTS_SAML_VERIFIER_SAML_VERIFIER_H_ 