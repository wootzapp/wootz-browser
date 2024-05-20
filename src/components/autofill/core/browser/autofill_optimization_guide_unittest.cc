// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/autofill_optimization_guide.h"

#include <memory>

#include "base/ranges/algorithm.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "components/autofill/core/browser/autofill_test_utils.h"
#include "components/autofill/core/browser/country_type.h"
#include "components/autofill/core/browser/data_model/credit_card.h"
#include "components/autofill/core/browser/data_model/credit_card_benefit.h"
#include "components/autofill/core/browser/data_model/credit_card_test_api.h"
#include "components/autofill/core/browser/form_structure.h"
#include "components/autofill/core/browser/form_structure_test_api.h"
#include "components/autofill/core/browser/payments/constants.h"
#include "components/autofill/core/browser/payments_data_manager.h"
#include "components/autofill/core/browser/test_personal_data_manager.h"
#include "components/autofill/core/common/autofill_payments_features.h"
#include "components/autofill/core/common/autofill_test_utils.h"
#include "components/autofill/core/common/form_data.h"
#include "components/optimization_guide/core/optimization_guide_decider.h"
#include "components/optimization_guide/core/optimization_guide_decision.h"
#include "components/optimization_guide/core/optimization_metadata.h"
#include "components/prefs/pref_service.h"
#include "components/sync/test/test_sync_service.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace autofill {

using test::CreateTestCreditCardFormData;
using test::CreateTestIbanFormData;

class MockOptimizationGuideDecider
    : public optimization_guide::OptimizationGuideDecider {
 public:
  MOCK_METHOD(void,
              RegisterOptimizationTypes,
              (const std::vector<optimization_guide::proto::OptimizationType>&),
              (override));
  MOCK_METHOD(void,
              CanApplyOptimization,
              (const GURL&,
               optimization_guide::proto::OptimizationType,
               optimization_guide::OptimizationGuideDecisionCallback),
              (override));
  MOCK_METHOD(optimization_guide::OptimizationGuideDecision,
              CanApplyOptimization,
              (const GURL&,
               optimization_guide::proto::OptimizationType,
               optimization_guide::OptimizationMetadata*),
              (override));
  MOCK_METHOD(
      void,
      CanApplyOptimizationOnDemand,
      (const std::vector<GURL>&,
       const base::flat_set<optimization_guide::proto::OptimizationType>&,
       optimization_guide::proto::RequestContext,
       optimization_guide::OnDemandOptimizationGuideDecisionRepeatingCallback,
       std::optional<optimization_guide::proto::RequestContextMetadata>
           request_context_metadata),
      (override));
};

class AutofillOptimizationGuideTest : public testing::Test {
 public:
  AutofillOptimizationGuideTest()
      : pref_service_(test::PrefServiceForTesting()),
        decider_(std::make_unique<MockOptimizationGuideDecider>()),
        personal_data_manager_(std::make_unique<TestPersonalDataManager>()),
        autofill_optimization_guide_(
            std::make_unique<AutofillOptimizationGuide>(decider_.get())) {
    // TODO(crbug.com/41492641): Cleanup default credit card creation in
    // Autofill Optimization Guide unittests by defining the credit card in each
    // individual test.
    CreditCard card = test::GetVirtualCard();
    test_api(card).set_network_for_virtual_card(kVisaCard);
    card.set_virtual_card_enrollment_type(
        CreditCard::VirtualCardEnrollmentType::kNetwork);
    personal_data_manager_->SetPrefService(pref_service_.get());
    personal_data_manager_->SetSyncServiceForTest(&sync_service_);
    personal_data_manager_->AddServerCreditCard(card);
  }

  void MockCapitalOneCreditCardBenefitsBlockedDecisionForUrl(
      const GURL& url,
      optimization_guide::OptimizationGuideDecision decision) {
    ON_CALL(*decider_,
            CanApplyOptimization(
                testing::Eq(url),
                testing::Eq(optimization_guide::proto::
                                CAPITAL_ONE_CREDIT_CARD_BENEFITS_BLOCKED),
                testing::Matcher<optimization_guide::OptimizationMetadata*>(
                    testing::Eq(nullptr))))
        .WillByDefault(testing::Return(decision));
  }

 protected:
  base::test::TaskEnvironment task_environment_;
  test::AutofillUnitTestEnvironment autofill_test_environment_;
  std::unique_ptr<PrefService> pref_service_;
  syncer::TestSyncService sync_service_;
  std::unique_ptr<MockOptimizationGuideDecider> decider_;
  std::unique_ptr<TestPersonalDataManager> personal_data_manager_;
  std::unique_ptr<AutofillOptimizationGuide> autofill_optimization_guide_;
};

TEST_F(AutofillOptimizationGuideTest, EnsureIntegratorInitializedCorrectly) {
  EXPECT_TRUE(autofill_optimization_guide_
                  ->GetOptimizationGuideKeyedServiceForTesting() ==
              decider_.get());
}

// Test that the `IBAN_AUTOFILL_BLOCKED` optimization type is registered when we
// have seen an IBAN form.
TEST_F(AutofillOptimizationGuideTest, IbanFieldFound_IbanAutofillBlocked) {
  FormStructure form_structure{CreateTestIbanFormData()};
  test_api(form_structure).SetFieldTypes({IBAN_VALUE}, {IBAN_VALUE});

  EXPECT_CALL(*decider_,
              RegisterOptimizationTypes(testing::ElementsAre(
                  optimization_guide::proto::IBAN_AUTOFILL_BLOCKED)));

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that the `VCN_MERCHANT_OPT_OUT_VISA` optimization type is registered
// when we have seen a credit card form, and meet all of the pre-requisites for
// the Visa merchant opt-out use-case.
TEST_F(AutofillOptimizationGuideTest, CreditCardFormFound_VcnMerchantOptOut) {
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  form_structure.DetermineHeuristicTypes(
      GeoIpCountryCode(""),
      /*form_interactions_ukm_logger=*/nullptr, /*log_manager=*/nullptr);

  EXPECT_CALL(*decider_,
              RegisterOptimizationTypes(testing::ElementsAre(
                  optimization_guide::proto::VCN_MERCHANT_OPT_OUT_VISA)));

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that the `VCN_MERCHANT_OPT_OUT_VISA` optimization type is not registered
// when we have seen a credit card form, but the network is not Visa.
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_VcnMerchantOptOut_NotVisaNetwork) {
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  form_structure.DetermineHeuristicTypes(
      GeoIpCountryCode(""),
      /*form_interactions_ukm_logger=*/nullptr, /*log_manager=*/nullptr);
  test_api(*personal_data_manager_->payments_data_manager().GetCreditCards()[0])
      .set_network_for_virtual_card(kMasterCard);

  EXPECT_CALL(*decider_, RegisterOptimizationTypes).Times(0);

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that the `VCN_MERCHANT_OPT_OUT_VISA` optimization type is not registered
// when we have seen a credit card form, but the virtual card is an issuer-level
// enrollment
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_VcnMerchantOptOut_IssuerEnrollment) {
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  form_structure.DetermineHeuristicTypes(
      GeoIpCountryCode(""),
      /*form_interactions_ukm_logger=*/nullptr, /*log_manager=*/nullptr);
  personal_data_manager_->payments_data_manager()
      .GetCreditCards()[0]
      ->set_virtual_card_enrollment_type(
          CreditCard::VirtualCardEnrollmentType::kIssuer);

  EXPECT_CALL(*decider_, RegisterOptimizationTypes).Times(0);

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that the `VCN_MERCHANT_OPT_OUT_VISA` optimization type is not registered
// when we have seen a credit card form, but we do not have a virtual card on
// the account.
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_VcnMerchantOptOut_NotEnrolledInVirtualCard) {
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  form_structure.DetermineHeuristicTypes(
      GeoIpCountryCode(""),
      /*form_interactions_ukm_logger=*/nullptr, /*log_manager=*/nullptr);
  personal_data_manager_->payments_data_manager()
      .GetCreditCards()[0]
      ->set_virtual_card_enrollment_state(
          CreditCard::VirtualCardEnrollmentState::kUnenrolledAndEligible);

  EXPECT_CALL(*decider_, RegisterOptimizationTypes).Times(0);

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that no optimization type is registered when we have seen a credit card
// form, and meet all of the pre-requisites for the Visa merchant opt-out
// use-case, but there is no personal data manager present.
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_VcnMerchantOptOut_NoPersonalDataManager) {
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  form_structure.DetermineHeuristicTypes(
      GeoIpCountryCode(""),
      /*form_interactions_ukm_logger=*/nullptr, /*log_manager=*/nullptr);
  personal_data_manager_.reset();

  EXPECT_CALL(*decider_, RegisterOptimizationTypes).Times(0);

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that if the field type does not correlate to any optimization type we
// have, that no optimization type is registered.
TEST_F(AutofillOptimizationGuideTest, OptimizationTypeToRegisterNotFound) {
  AutofillField field;
  FormData form_data;
  form_data.fields = {field};
  FormStructure form_structure{form_data};
  test_api(form_structure)
      .SetFieldTypes({MERCHANT_PROMO_CODE}, {MERCHANT_PROMO_CODE});

  EXPECT_CALL(*decider_, RegisterOptimizationTypes).Times(0);

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that if the form denotes that we need to register multiple optimization
// types, all of the optimization types that we need to register will be
// registered.
TEST_F(AutofillOptimizationGuideTest,
       FormWithMultipleOptimizationTypesToRegisterFound) {
  FormData form_data = CreateTestCreditCardFormData(/*is_https=*/true,
                                                    /*use_month_type=*/false);
  base::ranges::move(CreateTestIbanFormData().fields,
                     std::back_inserter(form_data.fields));
  FormStructure form_structure{form_data};
  const std::vector<FieldType> field_types = {
      CREDIT_CARD_NAME_FIRST, CREDIT_CARD_NAME_LAST,        CREDIT_CARD_NUMBER,
      CREDIT_CARD_EXP_MONTH,  CREDIT_CARD_EXP_4_DIGIT_YEAR, IBAN_VALUE};
  test_api(form_structure).SetFieldTypes(field_types, field_types);

  EXPECT_CALL(*decider_,
              RegisterOptimizationTypes(testing::ElementsAre(
                  optimization_guide::proto::IBAN_AUTOFILL_BLOCKED,
                  optimization_guide::proto::VCN_MERCHANT_OPT_OUT_VISA)));

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that single field suggestions are blocked when we are about to display
// suggestions for an IBAN field but the OptimizationGuideDecider denotes that
// displaying the suggestion is not allowed for the `IBAN_AUTOFILL_BLOCKED`
// optimization type.
TEST_F(AutofillOptimizationGuideTest,
       ShouldBlockSingleFieldSuggestions_IbanAutofillBlocked) {
  FormStructure form_structure{CreateTestIbanFormData()};
  test_api(form_structure).SetFieldTypes({IBAN_VALUE}, {IBAN_VALUE});
  GURL url("https://example.com/");
  ON_CALL(*decider_,
          CanApplyOptimization(
              testing::Eq(url),
              testing::Eq(optimization_guide::proto::IBAN_AUTOFILL_BLOCKED),
              testing::Matcher<optimization_guide::OptimizationMetadata*>(
                  testing::Eq(nullptr))))
      .WillByDefault(testing::Return(
          optimization_guide::OptimizationGuideDecision::kFalse));

  EXPECT_TRUE(autofill_optimization_guide_->ShouldBlockSingleFieldSuggestions(
      url, form_structure.field(0)));
}

// Test that single field suggestions are not blocked when we are about to
// display suggestions for an IBAN field and OptimizationGuideDecider denotes
// that displaying the suggestion is allowed for the `IBAN_AUTOFILL_BLOCKED`
// use-case.
TEST_F(AutofillOptimizationGuideTest,
       ShouldNotBlockSingleFieldSuggestions_IbanAutofillBlocked) {
  FormStructure form_structure{CreateTestIbanFormData()};
  test_api(form_structure).SetFieldTypes({IBAN_VALUE}, {IBAN_VALUE});
  GURL url("https://example.com/");
  ON_CALL(*decider_,
          CanApplyOptimization(
              testing::Eq(url),
              testing::Eq(optimization_guide::proto::IBAN_AUTOFILL_BLOCKED),
              testing::Matcher<optimization_guide::OptimizationMetadata*>(
                  testing::Eq(nullptr))))
      .WillByDefault(testing::Return(
          optimization_guide::OptimizationGuideDecision::kTrue));

  EXPECT_FALSE(autofill_optimization_guide_->ShouldBlockSingleFieldSuggestions(
      url, form_structure.field(0)));
}

// Test that single field suggestions are not blocked for the
// `IBAN_AUTOFILL_BLOCKED` use-case when the field is not an IBAN field.
TEST_F(
    AutofillOptimizationGuideTest,
    ShouldNotBlockSingleFieldSuggestions_IbanAutofillBlocked_FieldTypeForBlockingNotFound) {
  FormStructure form_structure{CreateTestIbanFormData()};
  GURL url("https://example.com/");
  EXPECT_CALL(*decider_,
              CanApplyOptimization(
                  testing::Eq(url),
                  testing::Eq(optimization_guide::proto::IBAN_AUTOFILL_BLOCKED),
                  testing::Matcher<optimization_guide::OptimizationMetadata*>(
                      testing::Eq(nullptr))))
      .Times(0);

  EXPECT_FALSE(autofill_optimization_guide_->ShouldBlockSingleFieldSuggestions(
      url, form_structure.field(0)));
}

// Test that blocking a virtual card suggestion works correctly in the VCN
// merchant opt-out use-case.
TEST_F(AutofillOptimizationGuideTest,
       ShouldBlockFormFieldSuggestion_VcnMerchantOptOut) {
  GURL url("https://example.com/");
  CreditCard virtual_card = test::GetVirtualCard();
  virtual_card.set_virtual_card_enrollment_type(
      CreditCard::VirtualCardEnrollmentType::kNetwork);
  test_api(virtual_card).set_network_for_virtual_card(kVisaCard);

  ON_CALL(*decider_,
          CanApplyOptimization(
              testing::Eq(url),
              testing::Eq(optimization_guide::proto::VCN_MERCHANT_OPT_OUT_VISA),
              testing::Matcher<optimization_guide::OptimizationMetadata*>(
                  testing::Eq(nullptr))))
      .WillByDefault(testing::Return(
          optimization_guide::OptimizationGuideDecision::kFalse));

  EXPECT_TRUE(autofill_optimization_guide_->ShouldBlockFormFieldSuggestion(
      url, virtual_card));
}

// Test that if the URL is not blocklisted, we do not block a virtual card
// suggestion in the VCN merchant opt-out use-case.
TEST_F(AutofillOptimizationGuideTest,
       ShouldNotBlockFormFieldSuggestion_VcnMerchantOptOut_UrlNotBlocked) {
  GURL url("https://example.com/");
  CreditCard virtual_card = test::GetVirtualCard();
  virtual_card.set_virtual_card_enrollment_type(
      CreditCard::VirtualCardEnrollmentType::kNetwork);
  test_api(virtual_card).set_network_for_virtual_card(kVisaCard);

  ON_CALL(*decider_,
          CanApplyOptimization(
              testing::Eq(url),
              testing::Eq(optimization_guide::proto::VCN_MERCHANT_OPT_OUT_VISA),
              testing::Matcher<optimization_guide::OptimizationMetadata*>(
                  testing::Eq(nullptr))))
      .WillByDefault(testing::Return(
          optimization_guide::OptimizationGuideDecision::kTrue));

  EXPECT_FALSE(autofill_optimization_guide_->ShouldBlockFormFieldSuggestion(
      url, virtual_card));
}

// Test that we do not block virtual card suggestions in the VCN merchant
// opt-out use-case if the card is an issuer-level enrollment.
TEST_F(AutofillOptimizationGuideTest,
       ShouldNotBlockFormFieldSuggestion_VcnMerchantOptOut_IssuerEnrollment) {
  GURL url("https://example.com/");
  CreditCard virtual_card = test::GetVirtualCard();
  virtual_card.set_virtual_card_enrollment_type(
      CreditCard::VirtualCardEnrollmentType::kIssuer);
  test_api(virtual_card).set_network_for_virtual_card(kVisaCard);

  EXPECT_CALL(
      *decider_,
      CanApplyOptimization(
          testing::Eq(url),
          testing::Eq(optimization_guide::proto::VCN_MERCHANT_OPT_OUT_VISA),
          testing::Matcher<optimization_guide::OptimizationMetadata*>(
              testing::Eq(nullptr))))
      .Times(0);

  EXPECT_FALSE(autofill_optimization_guide_->ShouldBlockFormFieldSuggestion(
      url, virtual_card));
}

// Test that we do not block the virtual card suggestion from being shown in the
// VCN merchant opt-out use-case if the network does not have a VCN merchant
// opt-out blocklist.
TEST_F(
    AutofillOptimizationGuideTest,
    ShouldNotBlockFormFieldSuggestion_VcnMerchantOptOut_NetworkDoesNotHaveBlocklist) {
  GURL url("https://example.com/");
  CreditCard virtual_card = test::GetVirtualCard();
  virtual_card.set_virtual_card_enrollment_type(
      CreditCard::VirtualCardEnrollmentType::kNetwork);
  test_api(virtual_card).set_network_for_virtual_card(kMasterCard);

  EXPECT_CALL(
      *decider_,
      CanApplyOptimization(
          testing::Eq(url),
          testing::Eq(optimization_guide::proto::VCN_MERCHANT_OPT_OUT_VISA),
          testing::Matcher<optimization_guide::OptimizationMetadata*>(
              testing::Eq(nullptr))))
      .Times(0);

  EXPECT_FALSE(autofill_optimization_guide_->ShouldBlockFormFieldSuggestion(
      url, virtual_card));
}

// Test that we block benefits suggestions for Capital One cards on blocked
// URLs.
TEST_F(AutofillOptimizationGuideTest,
       ShouldBlockBenefitSuggestionLabelsForCardAndUrl_CapitalOne_BlockedUrl) {
  GURL url("https://example.com/");
  CreditCard* card =
      personal_data_manager_->payments_data_manager().GetCreditCards()[0];
  test_api(*card).set_issuer_id_for_card(kCapitalOneCardIssuerId);

  MockCapitalOneCreditCardBenefitsBlockedDecisionForUrl(
      url, optimization_guide::OptimizationGuideDecision::kFalse);

  EXPECT_TRUE(
      autofill_optimization_guide_
          ->ShouldBlockBenefitSuggestionLabelsForCardAndUrl(*card, url));
}

// Test that we do not block benefits suggestions for Capital One cards on
// unblocked URLs.
TEST_F(
    AutofillOptimizationGuideTest,
    ShouldNotBlockBenefitSuggestionLabelsForCardAndUrl_CapitalOne_UnblockedUrl) {
  GURL url("https://example.com/");
  CreditCard* card =
      personal_data_manager_->payments_data_manager().GetCreditCards()[0];
  test_api(*card).set_issuer_id_for_card(kCapitalOneCardIssuerId);

  MockCapitalOneCreditCardBenefitsBlockedDecisionForUrl(
      url, optimization_guide::OptimizationGuideDecision::kTrue);

  EXPECT_FALSE(
      autofill_optimization_guide_
          ->ShouldBlockBenefitSuggestionLabelsForCardAndUrl(*card, url));
}

// Test that we do not block benefits suggestions when a kUnknown decision is
// returned.
TEST_F(
    AutofillOptimizationGuideTest,
    ShouldNotBlockBenefitSuggestionLabelsForCardAndUrl_CapitalOne_UnknownDecision) {
  GURL url("https://example.com/");
  CreditCard* card =
      personal_data_manager_->payments_data_manager().GetCreditCards()[0];
  test_api(*card).set_issuer_id_for_card(kCapitalOneCardIssuerId);

  MockCapitalOneCreditCardBenefitsBlockedDecisionForUrl(
      url, optimization_guide::OptimizationGuideDecision::kUnknown);

  EXPECT_FALSE(
      autofill_optimization_guide_
          ->ShouldBlockBenefitSuggestionLabelsForCardAndUrl(*card, url));
}

// Test that we do not block benefits suggestions for non-Capital One cards on
// blocked URLs.
TEST_F(
    AutofillOptimizationGuideTest,
    ShouldNotBlockBenefitSuggestionLabelsForCardAndUrl_NonCapitalOne_BlockedUrl) {
  GURL url("https://example.com/");
  CreditCard* card =
      personal_data_manager_->payments_data_manager().GetCreditCards()[0];
  test_api(*card).set_issuer_id_for_card(kAmexCardIssuerId);

  MockCapitalOneCreditCardBenefitsBlockedDecisionForUrl(
      url, optimization_guide::OptimizationGuideDecision::kFalse);

  EXPECT_FALSE(
      autofill_optimization_guide_
          ->ShouldBlockBenefitSuggestionLabelsForCardAndUrl(*card, url));
}

// Test that we do not block benefits suggestions for non-Capital One cards on
// unblocked URLs.
TEST_F(
    AutofillOptimizationGuideTest,
    ShouldNotBlockBenefitSuggestionLabelsForCardAndUrl_NonCapitalOne_UnblockedUrl) {
  GURL url("https://example.com/");
  CreditCard* card =
      personal_data_manager_->payments_data_manager().GetCreditCards()[0];
  test_api(*card).set_issuer_id_for_card(kAmexCardIssuerId);

  MockCapitalOneCreditCardBenefitsBlockedDecisionForUrl(
      url, optimization_guide::OptimizationGuideDecision::kTrue);

  EXPECT_FALSE(
      autofill_optimization_guide_
          ->ShouldBlockBenefitSuggestionLabelsForCardAndUrl(*card, url));
}

// Test that the Amex category-benefit optimization types are registered when we
// have seen a credit card form and the user has an Amex card.
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_AmexCategoryBenefits) {
  base::test::ScopedFeatureList feature_list{
      features::kAutofillEnableCardBenefitsForAmericanExpress};
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  test_api(form_structure)
      .SetFieldTypes({CREDIT_CARD_NAME_FULL, CREDIT_CARD_NUMBER,
                      CREDIT_CARD_EXP_MONTH, CREDIT_CARD_VERIFICATION_CODE});
  test_api(*personal_data_manager_->payments_data_manager().GetCreditCards()[0])
      .set_network_for_virtual_card(kAmericanExpressCard);
  test_api(*personal_data_manager_->payments_data_manager().GetCreditCards()[0])
      .set_issuer_id_for_card(kAmexCardIssuerId);

  EXPECT_CALL(*decider_,
              RegisterOptimizationTypes(testing::UnorderedElementsAre(
                  optimization_guide::proto::
                      AMERICAN_EXPRESS_CREDIT_CARD_FLIGHT_BENEFITS,
                  optimization_guide::proto::
                      AMERICAN_EXPRESS_CREDIT_CARD_SUBSCRIPTION_BENEFITS)));

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that the Capital One category-benefit optimization types are registered
// when we have seen a credit card form and the user has a Capital One card.
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_CapitalOneCategoryBenefits) {
  base::test::ScopedFeatureList feature_list{
      features::kAutofillEnableCardBenefitsForCapitalOne};
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  test_api(form_structure)
      .SetFieldTypes({CREDIT_CARD_NAME_FULL, CREDIT_CARD_NUMBER,
                      CREDIT_CARD_EXP_MONTH, CREDIT_CARD_VERIFICATION_CODE});
  CreditCard* card =
      personal_data_manager_->payments_data_manager().GetCreditCards()[0];
  test_api(*card).set_network_for_virtual_card(kMasterCard);
  test_api(*card).set_issuer_id_for_card(kCapitalOneCardIssuerId);

  EXPECT_CALL(
      *decider_,
      RegisterOptimizationTypes(testing::UnorderedElementsAre(
          optimization_guide::proto::CAPITAL_ONE_CREDIT_CARD_DINING_BENEFITS,
          optimization_guide::proto::CAPITAL_ONE_CREDIT_CARD_GROCERY_BENEFITS,
          optimization_guide::proto::
              CAPITAL_ONE_CREDIT_CARD_ENTERTAINMENT_BENEFITS,
          optimization_guide::proto::CAPITAL_ONE_CREDIT_CARD_STREAMING_BENEFITS,
          optimization_guide::proto::
              CAPITAL_ONE_CREDIT_CARD_BENEFITS_BLOCKED)));

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that the Amex category-benefit optimization types are not registered
// when the kAutofillEnableCardBenefitsForAmericanExpress experiment is
// disabled.
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_AmexCategoryBenefits_ExperimentDisabled) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndDisableFeature(
      features::kAutofillEnableCardBenefitsForAmericanExpress);
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  test_api(form_structure)
      .SetFieldTypes({CREDIT_CARD_NAME_FULL, CREDIT_CARD_NUMBER,
                      CREDIT_CARD_EXP_MONTH, CREDIT_CARD_VERIFICATION_CODE});
  test_api(*personal_data_manager_->payments_data_manager().GetCreditCards()[0])
      .set_network_for_virtual_card(kAmericanExpressCard);
  test_api(*personal_data_manager_->payments_data_manager().GetCreditCards()[0])
      .set_issuer_id_for_card(kAmexCardIssuerId);

  EXPECT_CALL(*decider_,
              RegisterOptimizationTypes(testing::UnorderedElementsAre(
                  optimization_guide::proto::
                      AMERICAN_EXPRESS_CREDIT_CARD_FLIGHT_BENEFITS,
                  optimization_guide::proto::
                      AMERICAN_EXPRESS_CREDIT_CARD_SUBSCRIPTION_BENEFITS)))
      .Times(0);

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

// Test that the Capital One category-benefit optimization types are not
// registered when the kAutofillEnableCardBenefitsForCapitalOne experiment is
// disabled.
TEST_F(AutofillOptimizationGuideTest,
       CreditCardFormFound_CapitalOneCategoryBenefits_ExperimentDisabled) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndDisableFeature(
      features::kAutofillEnableCardBenefitsForCapitalOne);
  FormStructure form_structure{
      CreateTestCreditCardFormData(/*is_https=*/true,
                                   /*use_month_type=*/true)};
  test_api(form_structure)
      .SetFieldTypes({CREDIT_CARD_NAME_FULL, CREDIT_CARD_NUMBER,
                      CREDIT_CARD_EXP_MONTH, CREDIT_CARD_VERIFICATION_CODE});
  CreditCard* card =
      personal_data_manager_->payments_data_manager().GetCreditCards()[0];
  test_api(*card).set_network_for_virtual_card(kMasterCard);
  test_api(*card).set_issuer_id_for_card(kCapitalOneCardIssuerId);

  EXPECT_CALL(
      *decider_,
      RegisterOptimizationTypes(testing::UnorderedElementsAre(
          optimization_guide::proto::CAPITAL_ONE_CREDIT_CARD_DINING_BENEFITS,
          optimization_guide::proto::CAPITAL_ONE_CREDIT_CARD_GROCERY_BENEFITS,
          optimization_guide::proto::
              CAPITAL_ONE_CREDIT_CARD_ENTERTAINMENT_BENEFITS,
          optimization_guide::proto::
              CAPITAL_ONE_CREDIT_CARD_STREAMING_BENEFITS)))
      .Times(0);

  autofill_optimization_guide_->OnDidParseForm(form_structure,
                                               personal_data_manager_.get());
}

struct BenefitOptimizationToBenefitCategoryTestCase {
  const std::string issuer_id;
  const optimization_guide::proto::OptimizationType optimization_type;
  const CreditCardCategoryBenefit::BenefitCategory benefit_category;
};

class BenefitOptimizationToBenefitCategoryTest
    : public AutofillOptimizationGuideTest,
      public testing::WithParamInterface<
          BenefitOptimizationToBenefitCategoryTestCase> {
 public:
  BenefitOptimizationToBenefitCategoryTest() = default;
  ~BenefitOptimizationToBenefitCategoryTest() override = default;

  optimization_guide::proto::OptimizationType expected_benefit_optimization()
      const {
    return GetParam().optimization_type;
  }
  CreditCardCategoryBenefit::BenefitCategory expected_benefit_category() const {
    return GetParam().benefit_category;
  }

  const CreditCard& credit_card() const { return card_; }

  void SetUp() override {
    AutofillOptimizationGuideTest::SetUp();
    card_ = test::GetMaskedServerCard();
    card_.set_issuer_id(GetParam().issuer_id);
    personal_data_manager_->AddServerCreditCard(card_);
  }

 private:
  CreditCard card_;
};

// Tests that the correct benefit category is returned when a benefit
// optimization is found for a particular credit card issuer and url.
TEST_P(BenefitOptimizationToBenefitCategoryTest,
       GetBenefitCategoryForOptimizationType) {
  GURL url = GURL("https://example.com/");
  ON_CALL(*decider_,
          CanApplyOptimization(
              testing::Eq(url), testing::Eq(expected_benefit_optimization()),
              testing::Matcher<optimization_guide::OptimizationMetadata*>(
                  testing::Eq(nullptr))))
      .WillByDefault(testing::Return(
          optimization_guide::OptimizationGuideDecision::kTrue));

  EXPECT_EQ(autofill_optimization_guide_
                ->AttemptToGetEligibleCreditCardBenefitCategory(
                    credit_card().issuer_id(), url),
            expected_benefit_category());
}

INSTANTIATE_TEST_SUITE_P(
    All,
    BenefitOptimizationToBenefitCategoryTest,
    testing::Values(
        BenefitOptimizationToBenefitCategoryTestCase{
            "amex",
            optimization_guide::proto::
                AMERICAN_EXPRESS_CREDIT_CARD_FLIGHT_BENEFITS,
            CreditCardCategoryBenefit::BenefitCategory::kFlights},
        BenefitOptimizationToBenefitCategoryTestCase{
            "amex",
            optimization_guide::proto::
                AMERICAN_EXPRESS_CREDIT_CARD_SUBSCRIPTION_BENEFITS,
            CreditCardCategoryBenefit::BenefitCategory::kSubscription},
        BenefitOptimizationToBenefitCategoryTestCase{
            "capitalone",
            optimization_guide::proto::CAPITAL_ONE_CREDIT_CARD_DINING_BENEFITS,
            CreditCardCategoryBenefit::BenefitCategory::kDining},
        BenefitOptimizationToBenefitCategoryTestCase{
            "capitalone",
            optimization_guide::proto::CAPITAL_ONE_CREDIT_CARD_GROCERY_BENEFITS,
            CreditCardCategoryBenefit::BenefitCategory::kGroceryStores},
        BenefitOptimizationToBenefitCategoryTestCase{
            "capitalone",
            optimization_guide::proto::
                CAPITAL_ONE_CREDIT_CARD_ENTERTAINMENT_BENEFITS,
            CreditCardCategoryBenefit::BenefitCategory::kEntertainment},
        BenefitOptimizationToBenefitCategoryTestCase{
            "capitalone",
            optimization_guide::proto::
                CAPITAL_ONE_CREDIT_CARD_STREAMING_BENEFITS,
            CreditCardCategoryBenefit::BenefitCategory::kStreaming}));

}  // namespace autofill
