// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>

#include "base/base64.h"
#include "base/strings/strcat.h"
#include "base/values.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/interstitials/security_interstitial_page_test_utils.h"
#include "chrome/browser/policy/policy_test_utils.h"
#include "chrome/common/buildflags.h"
#include "components/policy/core/common/policy_map.h"
#include "components/policy/policy_constants.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "net/base/features.h"
#include "net/cert/x509_util.h"
#include "net/net_buildflags.h"
#include "net/test/cert_test_util.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "testing/gtest/include/gtest/gtest.h"

#if BUILDFLAG(IS_LINUX)
#include "crypto/scoped_test_nss_db.h"
#include "net/cert/nss_cert_database.h"
#include "net/cert/scoped_nss_types.h"
#include "net/cert/x509_util_nss.h"
#endif

#if BUILDFLAG(IS_ANDROID)
#include "chrome/test/base/android/android_browser_test.h"
#else
#include "chrome/test/base/in_process_browser_test.h"
#endif

#if BUILDFLAG(CHROME_CERTIFICATE_POLICIES_SUPPORTED)
// Testing the CACertificates policy
class CertVerifierServiceCACertificatesPolicyTest
    : public policy::PolicyTest,
      public testing::WithParamInterface<bool> {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    if (add_cert_to_policy()) {
      scoped_refptr<net::X509Certificate> root_cert = net::ImportCertFromFile(
          net::EmbeddedTestServer::GetRootCertPemPath());
      ASSERT_TRUE(root_cert);

      std::string b64_cert = base::Base64Encode(
          net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
      base::Value certs_value(base::Value::Type::LIST);
      certs_value.GetList().Append(b64_cert);
      policy::PolicyMap policies;
      SetPolicy(&policies, policy::key::kCACertificates,
                std::make_optional(std::move(certs_value)));
      UpdateProviderPolicy(policies);
    }
  }

  bool add_cert_to_policy() const { return GetParam(); }
};

IN_PROC_BROWSER_TEST_P(CertVerifierServiceCACertificatesPolicyTest,
                       TestCACertificatesPolicy) {
  net::EmbeddedTestServer https_test_server(
      net::EmbeddedTestServer::TYPE_HTTPS);
  https_test_server.SetSSLConfig(
      net::test_server::EmbeddedTestServer::CERT_AUTO);
  https_test_server.ServeFilesFromSourceDirectory("chrome/test/data");
  ASSERT_TRUE(https_test_server.Start());

  // Clear test roots so that cert validation only happens with
  // what's in the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  ASSERT_TRUE(NavigateToUrl(https_test_server.GetURL("/simple.html"), this));
  EXPECT_NE(add_cert_to_policy(),
            chrome_browser_interstitials::IsShowingInterstitial(
                chrome_test_utils::GetActiveWebContents(this)));
}

INSTANTIATE_TEST_SUITE_P(All,
                         CertVerifierServiceCACertificatesPolicyTest,
                         ::testing::Bool());

// Test update of CACertificates policy after verifier is already
// created.
class CertVerifierServiceCACertificatesUpdatePolicyTest
    : public policy::PolicyTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};
};

IN_PROC_BROWSER_TEST_F(CertVerifierServiceCACertificatesUpdatePolicyTest,
                       TestCACertificatesUpdate) {
  // `net::EmbeddedTestServer` uses `net::TestRootCerts` to install a trusted
  // root. Clear test roots so that cert validation only happens with what's in
  // the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  scoped_refptr<net::X509Certificate> root_cert =
      net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
  ASSERT_TRUE(root_cert);

  {
    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // No policy set, root cert isn't trusted.
    EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }

  {
    // Update policy with root
    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    base::Value certs_value(base::Value::Type::LIST);
    certs_value.GetList().Append(std::move(b64_cert));
    policy::PolicyMap policies;
    SetPolicy(&policies, policy::key::kCACertificates,
              std::make_optional(std::move(certs_value)));
    UpdateProviderPolicy(policies);

    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // Updated with policy, root should be trusted.
    EXPECT_FALSE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }
}

// Testing the CADistrutedCertificates policy
class CertVerifierServiceCADistrustedCertificatesPolicyTest
    : public policy::PolicyTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    scoped_refptr<net::X509Certificate> root_cert =
        net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
    ASSERT_TRUE(root_cert);

    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    base::Value certs_value(base::Value::Type::LIST);
    certs_value.GetList().Append(b64_cert);
    policy::PolicyMap policies;
    // Distrust the test server certificate
    SetPolicy(&policies, policy::key::kCADistrustedCertificates,
              std::make_optional(std::move(certs_value)));
    UpdateProviderPolicy(policies);
  }
};

IN_PROC_BROWSER_TEST_F(CertVerifierServiceCADistrustedCertificatesPolicyTest,
                       TestPolicy) {
  net::EmbeddedTestServer https_test_server(
      net::EmbeddedTestServer::TYPE_HTTPS);
  https_test_server.SetSSLConfig(
      net::test_server::EmbeddedTestServer::CERT_AUTO);
  https_test_server.ServeFilesFromSourceDirectory("chrome/test/data");
  ASSERT_TRUE(https_test_server.Start());

  // We don't clear the test roots but the cert should still be distrusted based
  // on the enterprise policy.

  ASSERT_TRUE(NavigateToUrl(https_test_server.GetURL("/simple.html"), this));

  EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
      chrome_test_utils::GetActiveWebContents(this)));
}

class CertVerifierServiceCATrustedDistrustedCertificatesPolicyTest
    : public policy::PolicyTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    scoped_refptr<net::X509Certificate> root_cert =
        net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
    ASSERT_TRUE(root_cert);

    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    policy::PolicyMap policies;
    // Distrust the test server certificate
    {
      base::Value certs_value(base::Value::Type::LIST);
      certs_value.GetList().Append(b64_cert);
      SetPolicy(&policies, policy::key::kCADistrustedCertificates,
                std::make_optional(std::move(certs_value)));
    }
    // Trust the test server certificate
    {
      base::Value certs_value(base::Value::Type::LIST);
      certs_value.GetList().Append(b64_cert);
      SetPolicy(&policies, policy::key::kCACertificates,
                std::make_optional(std::move(certs_value)));
    }
    UpdateProviderPolicy(policies);
  }
};

IN_PROC_BROWSER_TEST_F(
    CertVerifierServiceCATrustedDistrustedCertificatesPolicyTest,
    TestDistrustOverridesTrust) {
  net::EmbeddedTestServer https_test_server(
      net::EmbeddedTestServer::TYPE_HTTPS);
  https_test_server.SetSSLConfig(
      net::test_server::EmbeddedTestServer::CERT_AUTO);
  https_test_server.ServeFilesFromSourceDirectory("chrome/test/data");
  ASSERT_TRUE(https_test_server.Start());

  // We don't clear the test roots but the cert should still be distrusted based
  // on the enterprise policy.

  ASSERT_TRUE(NavigateToUrl(https_test_server.GetURL("/simple.html"), this));

  EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
      chrome_test_utils::GetActiveWebContents(this)));
}

// Test update of CADistrustedCertificates policy after verifier is already
// created.
class CertVerifierServiceCADistrustedCertificatesUpdatePolicyTest
    : public policy::PolicyTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};
};

IN_PROC_BROWSER_TEST_F(
    CertVerifierServiceCADistrustedCertificatesUpdatePolicyTest,
    TestCADistrustedCertificatesUpdate) {
  scoped_refptr<net::X509Certificate> root_cert =
      net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
  ASSERT_TRUE(root_cert);

  {
    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // Cert is trusted by default.
    EXPECT_FALSE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }

  {
    // Update policy to distrust the test server certificate
    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    policy::PolicyMap policies;
    base::Value certs_value(base::Value::Type::LIST);
    certs_value.GetList().Append(std::move(b64_cert));
    SetPolicy(&policies, policy::key::kCADistrustedCertificates,
              std::make_optional(std::move(certs_value)));
    UpdateProviderPolicy(policies);

    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // Updated with policy, root should no longer be trusted.
    EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }
}

// Testing the CAHintCertificate policy
class CertVerifierServiceCAHintCertificatesPolicyTest
    : public policy::PolicyTest,
      public testing::WithParamInterface<bool> {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    // Don't serve the intermediate either via AIA or as part of the handshake.
    net::EmbeddedTestServer::ServerCertificateConfig cert_config;
    cert_config.intermediate =
        net::EmbeddedTestServer::IntermediateType::kMissing;
    https_test_server_.SetSSLConfig(cert_config);
    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());

    if (add_cert_to_policy()) {
      // Add the intermediate as a hint.
      scoped_refptr<net::X509Certificate> intermediate_cert =
          https_test_server_.GetGeneratedIntermediate();
      ASSERT_TRUE(intermediate_cert);

      std::string b64_cert =
          base::Base64Encode(net::x509_util::CryptoBufferAsStringPiece(
              intermediate_cert->cert_buffer()));
      base::Value certs_value(base::Value::Type::LIST);
      certs_value.GetList().Append(b64_cert);
      policy::PolicyMap policies;
      SetPolicy(&policies, policy::key::kCAHintCertificates,
                std::make_optional(std::move(certs_value)));
      UpdateProviderPolicy(policies);
    }
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};

  bool add_cert_to_policy() const { return GetParam(); }
};

IN_PROC_BROWSER_TEST_P(CertVerifierServiceCAHintCertificatesPolicyTest,
                       TestPolicy) {
  ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));

  EXPECT_NE(add_cert_to_policy(),
            chrome_browser_interstitials::IsShowingInterstitial(
                chrome_test_utils::GetActiveWebContents(this)));
}

INSTANTIATE_TEST_SUITE_P(All,
                         CertVerifierServiceCAHintCertificatesPolicyTest,
                         ::testing::Bool());

// Test update of CAHintCertificates policy after verifier is already
// created.
class CertVerifierServiceCAHintCertificatesUpdatePolicyTest
    : public policy::PolicyTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    net::EmbeddedTestServer::ServerCertificateConfig cert_config;
    cert_config.intermediate =
        net::EmbeddedTestServer::IntermediateType::kMissing;
    https_test_server_.SetSSLConfig(cert_config);
    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};
};

IN_PROC_BROWSER_TEST_F(CertVerifierServiceCAHintCertificatesUpdatePolicyTest,
                       TestCAHintCertificatesUpdate) {
  {
    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // Intermediate isn't found, so can't build the chain
    EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }

  {
    // Update policy to add the intermediate as a hint by policy
    scoped_refptr<net::X509Certificate> intermediate_cert =
        https_test_server_.GetGeneratedIntermediate();
    ASSERT_TRUE(intermediate_cert);

    std::string b64_cert =
        base::Base64Encode(net::x509_util::CryptoBufferAsStringPiece(
            intermediate_cert->cert_buffer()));
    base::Value certs_value(base::Value::Type::LIST);
    certs_value.GetList().Append(std::move(b64_cert));
    policy::PolicyMap policies;
    SetPolicy(&policies, policy::key::kCAHintCertificates,
              std::make_optional(std::move(certs_value)));
    UpdateProviderPolicy(policies);

    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // Updated with policy, intermediate is used so chain can be built.
    EXPECT_FALSE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }
}

#if BUILDFLAG(IS_LINUX)
// Test the CAPlatformIntegrationEnabled policy.
//
// Ideally we'd have this set up for every platform where this policy is
// supported, but on most platforms its really hard to modify the OS root
// store in an integration test without possibly messing up other tests.
// Except on Linux.
class CertVerifierServiceCAPlatformIntegrationPolicyBaseTest
    : public policy::PolicyTest {
 public:
  void SetUpOnMainThread() override {
    policy::PolicyTest::SetUpOnMainThread();

    // Set up test NSS DB
    nss_db_ = std::make_unique<crypto::ScopedTestNSSDB>();
    cert_db_ = std::make_unique<net::NSSCertDatabase>(
        crypto::ScopedPK11Slot(PK11_ReferenceSlot(nss_db_->slot())),
        crypto::ScopedPK11Slot(PK11_ReferenceSlot(nss_db_->slot())));
    ASSERT_TRUE(nss_db_->is_open());

    // Add root cert to test NSS DB.
    scoped_refptr<net::X509Certificate> root_cert =
        net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
    ASSERT_TRUE(root_cert);
    net::ScopedCERTCertificateList nss_certs;
    net::ScopedCERTCertificate nss_cert =
        net::x509_util::CreateCERTCertificateFromX509Certificate(
            root_cert.get());
    ASSERT_TRUE(nss_cert);
    nss_certs.push_back(std::move(nss_cert));

    net::NSSCertDatabase::ImportCertFailureList failure_list;
    cert_db_->ImportCACerts(nss_certs,
                            /*trust_bits=*/net::NSSCertDatabase::TRUSTED_SSL,
                            &failure_list);
    ASSERT_TRUE(failure_list.empty());
  }

 private:
  std::unique_ptr<crypto::ScopedTestNSSDB> nss_db_;
  std::unique_ptr<net::NSSCertDatabase> cert_db_;
};

class CertVerifierServiceCAPlatformIntegrationPolicyTest
    : public CertVerifierServiceCAPlatformIntegrationPolicyBaseTest,
      public testing::WithParamInterface<bool> {
 public:
  bool platform_root_store_enabled() const { return GetParam(); }
};

IN_PROC_BROWSER_TEST_P(CertVerifierServiceCAPlatformIntegrationPolicyTest,
                       TestCAPlatformIntegrationPolicy) {
  policy::PolicyMap policies;
  SetPolicy(&policies, policy::key::kCAPlatformIntegrationEnabled,
            std::optional<base::Value>(platform_root_store_enabled()));
  UpdateProviderPolicy(policies);

  net::EmbeddedTestServer https_test_server(
      net::EmbeddedTestServer::TYPE_HTTPS);
  https_test_server.SetSSLConfig(
      net::test_server::EmbeddedTestServer::CERT_AUTO);
  https_test_server.ServeFilesFromSourceDirectory("chrome/test/data");
  ASSERT_TRUE(https_test_server.Start());

  // `net::EmbeddedTestServer` uses `net::TestRootCerts` to install a trusted
  // root.
  // Clear test roots so that cert validation only happens with
  // what's in the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  ASSERT_TRUE(NavigateToUrl(https_test_server.GetURL("/simple.html"), this));
  EXPECT_NE(platform_root_store_enabled(),
            chrome_browser_interstitials::IsShowingInterstitial(
                chrome_test_utils::GetActiveWebContents(this)));
}

INSTANTIATE_TEST_SUITE_P(All,
                         CertVerifierServiceCAPlatformIntegrationPolicyTest,
                         ::testing::Bool());

// Test update of CAPlatformIntegrationEnabled policy after verifier is created.
IN_PROC_BROWSER_TEST_F(CertVerifierServiceCAPlatformIntegrationPolicyBaseTest,
                       TestCAPlatformIntegrationUpdatePolicy) {
  net::EmbeddedTestServer https_test_server(
      net::EmbeddedTestServer::TYPE_HTTPS);
  https_test_server.ServeFilesFromSourceDirectory("chrome/test/data");
  ASSERT_TRUE(https_test_server.Start());

  // `net::EmbeddedTestServer` uses `net::TestRootCerts` to install a trusted
  // root.
  // Clear test roots so that cert validation only happens with
  // what's in the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  ASSERT_TRUE(NavigateToUrl(https_test_server.GetURL("/simple.html"), this));
  // Platform integration is on by default, request should succeed.
  EXPECT_FALSE(chrome_browser_interstitials::IsShowingInterstitial(
      chrome_test_utils::GetActiveWebContents(this)));

  // Turn platform integration off
  policy::PolicyMap policies;
  SetPolicy(&policies, policy::key::kCAPlatformIntegrationEnabled,
            std::optional<base::Value>(false));
  UpdateProviderPolicy(policies);

  ASSERT_TRUE(NavigateToUrl(https_test_server.GetURL("/simple.html"), this));
  // Platform integration is false, request should fail to verify cert.
  EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
      chrome_test_utils::GetActiveWebContents(this)));
}

#endif  // BUILDFLAG(IS_LINUX)

// Test the CACertificatesWithConstraints policy
class CertVerifierServiceCACertsWithConstraintsPolicyTest
    : public policy::PolicyTest,
      public testing::WithParamInterface<bool> {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    // Use a certificate valid for the dns name localhost rather than an IP.
    https_test_server_.SetSSLConfig(
        net::EmbeddedTestServer::CERT_COMMON_NAME_IS_DOMAIN);
    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());

    scoped_refptr<net::X509Certificate> root_cert =
        net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
    ASSERT_TRUE(root_cert);

    // Set policy. DNS constraint only matches when
    // set_proper_dns_name_constraint() = true.
    std::string dns_name_constraint =
        set_proper_dns_name_constraint() ? "localhost" : "cruddyhost";
    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    base::Value::List certs_with_constraints_value = base::Value::List().Append(
        base::Value::Dict()
            .Set("certificate", b64_cert)
            .Set("constraints",
                 base::Value::Dict().Set(
                     "permitted_dns_names",
                     base::Value::List().Append(dns_name_constraint))));
    policy::PolicyMap policies;
    SetPolicy(&policies, policy::key::kCACertificatesWithConstraints,
              std::make_optional(
                  base::Value(std::move(certs_with_constraints_value))));
    UpdateProviderPolicy(policies);
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};

  bool set_proper_dns_name_constraint() const { return GetParam(); }
};

IN_PROC_BROWSER_TEST_P(CertVerifierServiceCACertsWithConstraintsPolicyTest,
                       TestCACertsWithConstraintsPolicy) {
  // `net::EmbeddedTestServer` uses `net::TestRootCerts` to install a trusted
  // root.
  // Clear test roots so that cert validation only happens with
  // what's in the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
  EXPECT_NE(set_proper_dns_name_constraint(),
            chrome_browser_interstitials::IsShowingInterstitial(
                chrome_test_utils::GetActiveWebContents(this)));
}

INSTANTIATE_TEST_SUITE_P(All,
                         CertVerifierServiceCACertsWithConstraintsPolicyTest,
                         ::testing::Bool());

class CertVerifierServiceCACertsWithCIDRConstraintsPolicyTest
    : public policy::PolicyTest,
      public testing::WithParamInterface<bool> {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());

    scoped_refptr<net::X509Certificate> root_cert =
        net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
    ASSERT_TRUE(root_cert);

    // Set policy. CIDR constraint only matches when
    // set_proper_cidr_name_constraint() = true.
    std::string cidr_name_constraint =
        set_proper_cidr_name_constraint() ? "127.127.0.1/8" : "127.127.0.1/16";
    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    base::Value::List certs_with_constraints_value = base::Value::List().Append(
        base::Value::Dict()
            .Set("certificate", b64_cert)
            .Set("constraints",
                 base::Value::Dict().Set(
                     "permitted_cidrs",
                     base::Value::List().Append(cidr_name_constraint))));
    policy::PolicyMap policies;
    SetPolicy(&policies, policy::key::kCACertificatesWithConstraints,
              std::make_optional(
                  base::Value(std::move(certs_with_constraints_value))));
    UpdateProviderPolicy(policies);
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};

  bool set_proper_cidr_name_constraint() const { return GetParam(); }
};

IN_PROC_BROWSER_TEST_P(CertVerifierServiceCACertsWithCIDRConstraintsPolicyTest,
                       TestCACertsWithCIDRConstraintsPolicy) {
  // `net::EmbeddedTestServer` uses `net::TestRootCerts` to install a trusted
  // root.
  // Clear test roots so that cert validation only happens with
  // what's in the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
  EXPECT_NE(set_proper_cidr_name_constraint(),
            chrome_browser_interstitials::IsShowingInterstitial(
                chrome_test_utils::GetActiveWebContents(this)));
}

INSTANTIATE_TEST_SUITE_P(
    All,
    CertVerifierServiceCACertsWithCIDRConstraintsPolicyTest,
    ::testing::Bool());

class CertVerifierServiceCACertsWithInvalidCIDRConstraintsPolicyTest
    : public policy::PolicyTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    https_test_server_.SetSSLConfig(
        net::test_server::EmbeddedTestServer::CERT_AUTO);
    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());

    scoped_refptr<net::X509Certificate> root_cert =
        net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
    ASSERT_TRUE(root_cert);

    // Set invalid policy.
    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    base::Value::List certs_with_constraints_value = base::Value::List().Append(
        base::Value::Dict()
            .Set("certificate", b64_cert)
            .Set("constraints",
                 base::Value::Dict().Set(
                     "permitted_cidrs",
                     base::Value::List().Append("invalidcidr"))));
    policy::PolicyMap policies;
    SetPolicy(&policies, policy::key::kCACertificatesWithConstraints,
              std::make_optional(
                  base::Value(std::move(certs_with_constraints_value))));
    UpdateProviderPolicy(policies);
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};
};

IN_PROC_BROWSER_TEST_F(
    CertVerifierServiceCACertsWithInvalidCIDRConstraintsPolicyTest,
    TestCACertsWithCIDRConstraintsPolicy) {
  // `net::EmbeddedTestServer` uses `net::TestRootCerts` to install a trusted
  // root.
  // Clear test roots so that cert validation only happens with
  // what's in the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
  // invalid CIDR constraint means the root cert isn't trusted.
  EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
      chrome_test_utils::GetActiveWebContents(this)));
}

class CertVerifierServiceCACertsWithConstraintsUpdatePolicyTest
    : public policy::PolicyTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    policy::PolicyTest::SetUpInProcessBrowserTestFixture();

    https_test_server_.ServeFilesFromSourceDirectory("chrome/test/data");
    ASSERT_TRUE(https_test_server_.Start());
  }

  net::EmbeddedTestServer https_test_server_{
      net::EmbeddedTestServer::TYPE_HTTPS};
};

// Test update of CACertsWithConstraints policy after verifier is already
// created.
IN_PROC_BROWSER_TEST_F(
    CertVerifierServiceCACertsWithConstraintsUpdatePolicyTest,
    TestCACertsWithCIDRConstraintsPolicy) {
  // `net::EmbeddedTestServer` uses `net::TestRootCerts` to install a trusted
  // root.
  // Clear test roots so that cert validation only happens with
  // what's in the relevant root store + policies.
  net::TestRootCerts::GetInstance()->Clear();

  scoped_refptr<net::X509Certificate> root_cert =
      net::ImportCertFromFile(net::EmbeddedTestServer::GetRootCertPemPath());
  ASSERT_TRUE(root_cert);

  {
    // Set invalid policy.
    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    base::Value::List certs_with_constraints_value = base::Value::List().Append(
        base::Value::Dict()
            .Set("certificate", b64_cert)
            .Set("constraints",
                 base::Value::Dict().Set(
                     "permitted_cidrs",
                     base::Value::List().Append("invalidcidr"))));
    policy::PolicyMap policies;
    SetPolicy(&policies, policy::key::kCACertificatesWithConstraints,
              std::make_optional(
                  base::Value(std::move(certs_with_constraints_value))));
    UpdateProviderPolicy(policies);

    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // invalid CIDR constraint means the root cert isn't trusted.
    EXPECT_TRUE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }

  {
    // Update with valid policy
    std::string b64_cert = base::Base64Encode(
        net::x509_util::CryptoBufferAsStringPiece(root_cert->cert_buffer()));
    base::Value::List certs_with_constraints_value = base::Value::List().Append(
        base::Value::Dict()
            .Set("certificate", b64_cert)
            .Set("constraints",
                 base::Value::Dict().Set(
                     "permitted_cidrs",
                     base::Value::List().Append("127.127.0.1/8"))));
    policy::PolicyMap policies;
    SetPolicy(&policies, policy::key::kCACertificatesWithConstraints,
              std::make_optional(
                  base::Value(std::move(certs_with_constraints_value))));
    UpdateProviderPolicy(policies);

    ASSERT_TRUE(NavigateToUrl(https_test_server_.GetURL("/simple.html"), this));
    // Updated with a valid CIDR constraint, cert should be trusted.
    EXPECT_FALSE(chrome_browser_interstitials::IsShowingInterstitial(
        chrome_test_utils::GetActiveWebContents(this)));
  }
}

#endif  // BUILDFLAG(CHROME_CERTIFICATE_POLICIES_SUPPORTED)
