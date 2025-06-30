// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SAML_VERIFIER_SAML_VERIFIER_H_
#define COMPONENTS_SAML_VERIFIER_SAML_VERIFIER_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "base/functional/callback.h"
#include "crypto/signature_verifier.h"

class PrefService;

namespace saml_verifier {

// SAML attribute data structure for extensibility
struct SamlAttribute {
  SamlAttribute();
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
  
  // Enable/disable signature verification (default: enabled)
  void SetSignatureVerificationEnabled(bool enabled);
  
  // Enable dynamic certificate fetching from SAML metadata (default: enabled)
  void SetDynamicCertificateFetchingEnabled(bool enabled);
  
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
    std::string canonical_xml;
  };
  bool ExtractSignatureData(const std::string& saml_xml, SignatureData* sig_data);
  
  // Canonicalize XML according to C14N standard
  std::string CanonicalizeXml(const std::string& xml);
  
  // Extract Reference URI from SignedInfo for SAML signature verification
  std::string ExtractReferenceUri(const std::string& signed_info_xml);
  
  // Apply SAML signature transforms (enveloped signature + canonicalization)
  std::string ApplySamlSignatureTransforms(const std::string& saml_xml, 
                                           const std::string& reference_uri);
  
  // Apply enveloped signature transform (remove Signature element)
  std::string ApplyEnvelopedSignatureTransform(const std::string& xml);
  
  // Extract certificate from specific signature element
  std::string ExtractCertificateFromSignature(const std::string& saml_xml, 
                                              const std::string& signed_info_xml);
  
  // Verify RSA signature using Chromium crypto
  bool VerifyRsaSignature(const std::string& data, 
                          const std::string& signature_base64);

  bool signature_verification_enabled_ = true;
  bool dynamic_cert_fetching_enabled_ = true;
  std::string okta_certificate_pem_;
  std::map<std::string, std::string> certificate_cache_;  // issuer -> certificate
  std::vector<std::unique_ptr<SamlAttributeProcessor>> attribute_processors_;
  
  // Store parsed X.509 certificate for signature verification
  std::vector<uint8_t> certificate_der_;
  std::unique_ptr<crypto::SignatureVerifier> signature_verifier_;
};

}  // namespace saml_verifier

#endif  // COMPONENTS_SAML_VERIFIER_SAML_VERIFIER_H_ 