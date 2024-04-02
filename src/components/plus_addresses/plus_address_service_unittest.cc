// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/plus_addresses/plus_address_service.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "base/functional/callback_helpers.h"
#include "base/memory/scoped_refptr.h"
#include "base/metrics/field_trial_params.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/mock_callback.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "base/test/test_future.h"
#include "base/time/time.h"
#include "components/autofill/core/browser/ui/suggestion.h"
#include "components/autofill/core/browser/ui/suggestion_test_helpers.h"
#include "components/autofill/core/common/aliases.h"
#include "components/autofill/core/common/form_field_data.h"
#include "components/plus_addresses/features.h"
#include "components/plus_addresses/plus_address_http_client_impl.h"
#include "components/plus_addresses/plus_address_prefs.h"
#include "components/plus_addresses/plus_address_test_utils.h"
#include "components/plus_addresses/plus_address_types.h"
#include "components/plus_addresses/webdata/plus_address_table.h"
#include "components/plus_addresses/webdata/plus_address_webdata_service.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/testing_pref_service.h"
#include "components/signin/public/base/consent_level.h"
#include "components/signin/public/identity_manager/access_token_info.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "components/signin/public/identity_manager/identity_test_environment.h"
#include "components/webdata/common/web_database.h"
#include "components/webdata/common/web_database_service.h"
#include "net/http/http_status_code.h"
#include "services/data_decoder/public/cpp/test_support/in_process_data_decoder.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/network/test/test_url_loader_factory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace {

using SuggestionEvent = autofill::AutofillPlusAddressDelegate::SuggestionEvent;
using autofill::AutofillSuggestionTriggerSource;
using autofill::EqualsSuggestion;
using autofill::PopupItemId;
using autofill::Suggestion;
using ::testing::ElementsAre;
using ::testing::IsEmpty;

constexpr char kPlusAddress[] = "plus+remote@plus.plus";

auto IsSingleCreatePlusAddressSuggestion() {
  return ElementsAre(EqualsSuggestion(PopupItemId::kCreateNewPlusAddress));
}

auto IsSingleFillPlusAddressSuggestion(std::string_view address) {
  return ElementsAre(EqualsSuggestion(PopupItemId::kFillExistingPlusAddress,
                                      /*main_text=*/base::UTF8ToUTF16(address),
                                      Suggestion::Icon::kPlusAddress));
}

}  // namespace

namespace plus_addresses {

class PlusAddressServiceTest : public ::testing::Test {
 public:
  PlusAddressServiceTest()
      : test_shared_loader_factory_(
            base::MakeRefCounted<network::WeakWrapperSharedURLLoaderFactory>(
                &test_url_loader_factory_)) {
    InitService();
  }

 protected:
  // Constants that cannot be created at compile time:
  const url::Origin kNoSubdomainOrigin =
      url::Origin::Create(GURL("https://test.example"));
  const url::Origin kSubdomainOrigin =
      url::Origin::Create(GURL("https://asd.test.example"));

  signin::IdentityTestEnvironment& identity_env() { return identity_test_env_; }
  signin::IdentityManager* identity_manager() {
    return identity_test_env_.identity_manager();
  }
  PlusAddressService& service() { return *service_; }
  const scoped_refptr<network::SharedURLLoaderFactory>&
  shared_loader_factory() {
    return test_shared_loader_factory_;
  }
  base::test::TaskEnvironment& task_environment() { return task_environment_; }
  network::TestURLLoaderFactory& url_loader_factory() {
    return test_url_loader_factory_;
  }

  // Forces (re-)initialization of the `PlusAddressService`, which can be useful
  // when classes override feature parameters.
  void InitService() {
    service_.emplace(identity_manager(), /*pref_service=*/nullptr,
                     std::make_unique<PlusAddressHttpClientImpl>(
                         identity_manager(), shared_loader_factory()),
                     /*webdata_service=*/nullptr);
  }

 private:
  base::test::TaskEnvironment task_environment_{
      base::test::TaskEnvironment::TimeSource::MOCK_TIME};
  signin::IdentityTestEnvironment identity_test_env_;
  network::TestURLLoaderFactory test_url_loader_factory_;
  scoped_refptr<network::SharedURLLoaderFactory> test_shared_loader_factory_;
  data_decoder::test::InProcessDataDecoder decoder_;

  std::optional<PlusAddressService> service_;
};

TEST_F(PlusAddressServiceTest, BasicTest) {
  EXPECT_FALSE(service().IsPlusAddress(kPlusAddress));
  service().SavePlusAddress(kSubdomainOrigin, kPlusAddress);
  EXPECT_TRUE(service().IsPlusAddress(kPlusAddress));
  EXPECT_EQ(service().GetPlusAddress(kSubdomainOrigin), kPlusAddress);
  EXPECT_EQ(service().GetPlusAddress(url::Origin()), std::nullopt);
  EXPECT_EQ(service().GetPlusProfile(kSubdomainOrigin)->plus_address,
            kPlusAddress);
}

TEST_F(PlusAddressServiceTest, EnsureEtldPlusOneScope) {
  service().SavePlusAddress(kNoSubdomainOrigin, kPlusAddress);
  EXPECT_EQ(service().GetPlusAddress(kNoSubdomainOrigin), kPlusAddress);
  EXPECT_EQ(service().GetPlusAddress(kSubdomainOrigin), kPlusAddress);
  EXPECT_EQ(service().GetPlusProfile(kNoSubdomainOrigin)->plus_address,
            kPlusAddress);
  EXPECT_EQ(service().GetPlusProfile(kSubdomainOrigin)->plus_address,
            kPlusAddress);
}

TEST_F(PlusAddressServiceTest, EnsureEtldPlusOneScopeSubdomainAddedFirst) {
  service().SavePlusAddress(kSubdomainOrigin, kPlusAddress);
  EXPECT_EQ(service().GetPlusAddress(kNoSubdomainOrigin), kPlusAddress);
  EXPECT_EQ(service().GetPlusAddress(kNoSubdomainOrigin), kPlusAddress);
  EXPECT_EQ(service().GetPlusProfile(kNoSubdomainOrigin)->plus_address,
            kPlusAddress);
  EXPECT_EQ(service().GetPlusProfile(kSubdomainOrigin)->plus_address,
            kPlusAddress);
}

TEST_F(PlusAddressServiceTest, DefaultSupportsPlusAddressesState) {
  // By default, the `SupportsPlusAddresses` function should return `false`.
  EXPECT_FALSE(service().SupportsPlusAddresses(kNoSubdomainOrigin,
                                               /*is_off_the_record=*/false));
}

// Ensure `SupportsPlusAddresses` is false without a server URL.
TEST_F(PlusAddressServiceTest, SupportsPlusAddressNoServer) {
  // Enable the feature, but do not provide a server URL, which indicates no
  // suggestion should be shown.
  base::test::ScopedFeatureList scoped_feature_list{features::kFeature};
  InitService();
  EXPECT_FALSE(service().SupportsPlusAddresses(kNoSubdomainOrigin,
                                               /*is_off_the_record=*/false));
}

TEST_F(PlusAddressServiceTest, NoAccountPlusAddressCreation) {
  base::MockOnceCallback<void(const PlusProfileOrError&)> reserve_callback;
  base::MockOnceCallback<void(const PlusProfileOrError&)> confirm_callback;
  // Ensure that the lambdas aren't called since there is no signed-in account.
  EXPECT_CALL(reserve_callback, Run).Times(0);
  EXPECT_CALL(confirm_callback, Run).Times(0);
  service().ReservePlusAddress(kNoSubdomainOrigin, reserve_callback.Get());
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               confirm_callback.Get());
}

TEST_F(PlusAddressServiceTest, AbortPlusAddressCreation) {
  const std::string invalid_email = "plus";
  identity_env().MakeAccountAvailable(invalid_email,
                                      {signin::ConsentLevel::kSignin});
  InitService();

  base::MockOnceCallback<void(const PlusProfileOrError&)> reserve_callback;
  base::MockOnceCallback<void(const PlusProfileOrError&)> confirm_callback;
  // Ensure that the lambdas aren't called since there is no signed-in account.
  EXPECT_CALL(reserve_callback, Run).Times(0);
  EXPECT_CALL(confirm_callback, Run).Times(0);
  service().ReservePlusAddress(kNoSubdomainOrigin, reserve_callback.Get());
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               confirm_callback.Get());
}

// Tests that GetPlusProfiles returns all cached plus profiles.
TEST_F(PlusAddressServiceTest, GetPlusProfiles) {
  service().SavePlusAddress(url::Origin::Create(GURL("https://foo.com")),
                            "plus+foo@plus.plus");
  service().SavePlusAddress(url::Origin::Create(GURL("https://bar.com")),
                            "plus+bar@plus.plus");

  EXPECT_THAT(service().GetPlusProfiles(),
              testing::UnorderedElementsAre(
                  PlusProfile{.facet = "foo.com",
                              .plus_address = "plus+foo@plus.plus",
                              .is_confirmed = true},
                  PlusProfile{.facet = "bar.com",
                              .plus_address = "plus+bar@plus.plus",
                              .is_confirmed = true}));
}

// Tests the PlusAddressService ability to make network requests.
class PlusAddressServiceRequestsTest : public PlusAddressServiceTest {
 public:
  explicit PlusAddressServiceRequestsTest()
      : kPlusProfilesEndpoint(
            kServerUrl.Resolve(kServerPlusProfileEndpoint).spec()),
        kReservePlusAddressEndpoint(
            kServerUrl.Resolve(kServerReservePlusAddressEndpoint).spec()),
        kCreatePlusAddressEndpoint(
            kServerUrl.Resolve(kServerCreatePlusAddressEndpoint).spec()) {
    scoped_feature_list_.InitAndEnableFeatureWithParameters(
        features::kFeature, GetFieldTrialParams());
    identity_env().MakeAccountAvailable(kSigninAccount,
                                        {signin::ConsentLevel::kSignin});
    identity_env().SetAutomaticIssueOfAccessTokens(true);
    InitService();
  }

 protected:
  static constexpr std::string_view kSigninAccount = "plus@plus.plus";

  // Constants that cannot be created at compile time:
  const GURL kServerUrl = GURL("https://server.example");
  const std::string kPlusProfilesEndpoint;
  const std::string kReservePlusAddressEndpoint;
  const std::string kCreatePlusAddressEndpoint;

  // The eTLD+1 of `kNoSubdomainOrigin`.
  static constexpr char kSite[] = "test.example";

  base::FieldTrialParams GetFieldTrialParams() const {
    return {{"server-url", kServerUrl.spec()},
            {"oauth-scope", "scope.example"}};
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_F(PlusAddressServiceRequestsTest, ReservePlusAddress_ReturnsUnconfirmed) {
  base::test::TestFuture<const PlusProfileOrError&> future;
  service().ReservePlusAddress(kNoSubdomainOrigin, future.GetCallback());

  // Check that the future callback is still blocked, and unblock it.
  ASSERT_FALSE(future.IsReady());
  url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint,
      test::MakeCreationResponse(PlusProfile({.facet = kSite,
                                              .plus_address = kPlusAddress,
                                              .is_confirmed = false})));
  ASSERT_TRUE(future.IsReady());
  EXPECT_EQ(future.Get()->plus_address, kPlusAddress);

  // The service should not save plus_address if it hasn't been confirmed yet.
  EXPECT_FALSE(service().IsPlusAddress(kPlusAddress));
}

TEST_F(PlusAddressServiceRequestsTest, ReservePlusAddress_ReturnsConfirmed) {
  base::test::TestFuture<const PlusProfileOrError&> future;
  service().ReservePlusAddress(kNoSubdomainOrigin, future.GetCallback());

  // Check that the future callback is still blocked, and unblock it.
  ASSERT_FALSE(future.IsReady());
  url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint,
      test::MakeCreationResponse(PlusProfile({.facet = kSite,
                                              .plus_address = kPlusAddress,
                                              .is_confirmed = true})));
  ASSERT_TRUE(future.IsReady());
  EXPECT_EQ(future.Get()->plus_address, kPlusAddress);

  // The service should save kPlusAddress if it has already been confirmed.
  EXPECT_TRUE(service().IsPlusAddress(kPlusAddress));
}

TEST_F(PlusAddressServiceRequestsTest, ReservePlusAddress_Fails) {
  base::test::TestFuture<const PlusProfileOrError&> future;
  service().ReservePlusAddress(kNoSubdomainOrigin, future.GetCallback());

  // Check that the future callback is still blocked, and unblock it.
  ASSERT_FALSE(future.IsReady());
  url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint, "", net::HTTP_BAD_REQUEST);
  ASSERT_TRUE(future.IsReady());
  EXPECT_FALSE(future.Get().has_value());
}

TEST_F(PlusAddressServiceRequestsTest, ConfirmPlusAddress_Successful) {
  base::test::TestFuture<const PlusProfileOrError&> future;
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               future.GetCallback());

  // Check that the future callback is still blocked, and unblock it.
  ASSERT_FALSE(future.IsReady());
  url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint,
      test::MakeCreationResponse(PlusProfile({.facet = kSite,
                                              .plus_address = kPlusAddress,
                                              .is_confirmed = true})));
  ASSERT_TRUE(future.IsReady());
  EXPECT_EQ(future.Get()->plus_address, kPlusAddress);
  // Verify that the kPlusAddress is saved when confirmation is successful.
  EXPECT_TRUE(service().IsPlusAddress(kPlusAddress));

  // Assert that ensuing calls to the same facet do not make a network request.
  base::test::TestFuture<const PlusProfileOrError&> second_future;
  service().ConfirmPlusAddress(kSubdomainOrigin, kPlusAddress,
                               second_future.GetCallback());
  ASSERT_TRUE(second_future.IsReady());
  EXPECT_EQ(second_future.Get()->plus_address, kPlusAddress);
}

TEST_F(PlusAddressServiceRequestsTest, ConfirmPlusAddress_Fails) {
  ASSERT_FALSE(service().IsPlusAddress(kPlusAddress));

  base::test::TestFuture<const PlusProfileOrError&> future;
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               future.GetCallback());

  // Check that the future callback is still blocked, and unblock it.
  ASSERT_FALSE(future.IsReady());
  url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint, "", net::HTTP_BAD_REQUEST);
  ASSERT_TRUE(future.IsReady());

  // An error is propagated from the callback and kPlusAddress is not saved.
  EXPECT_FALSE(future.Get().has_value());
  EXPECT_FALSE(service().IsPlusAddress(kPlusAddress));
}

// Doesn't run on ChromeOS since ClearPrimaryAccount() doesn't exist for it.
#if !BUILDFLAG(IS_CHROMEOS_ASH)
TEST_F(PlusAddressServiceRequestsTest,
       PrimaryAccountCleared_TogglesPlusAddressCreationOff) {
  // Toggle creation off by removing the primary account.
  identity_env().ClearPrimaryAccount();

  // Verify that Plus Address creation doesn't occur.
  service().ReservePlusAddress(kNoSubdomainOrigin, base::DoNothing());
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  EXPECT_EQ(url_loader_factory().NumPending(), 0);

  // Toggle creation back on by signing in again.
  identity_env().MakePrimaryAccountAvailable("plus@plus.plus",
                                             signin::ConsentLevel::kSignin);

  // Verify that Plus Address creation occurs and makes a network request.
  base::test::TestFuture<const PlusProfileOrError&> reserve;
  service().ReservePlusAddress(kNoSubdomainOrigin, reserve.GetCallback());
  EXPECT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint,
      test::MakeCreationResponse(
          PlusProfile({.facet = kSite, .plus_address = kPlusAddress})));
  EXPECT_EQ(reserve.Get()->plus_address, kPlusAddress);

  base::test::TestFuture<const PlusProfileOrError&> confirm;
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               confirm.GetCallback());
  EXPECT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint,
      test::MakeCreationResponse(
          PlusProfile({.facet = kSite, .plus_address = kPlusAddress})));
  EXPECT_EQ(confirm.Get()->plus_address, kPlusAddress);
}
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

TEST_F(PlusAddressServiceRequestsTest,
       PrimaryRefreshTokenError_TogglesPlusAddressCreationOff) {
  CoreAccountInfo primary_account =
      identity_manager()->GetPrimaryAccountInfo(signin::ConsentLevel::kSignin);

  // Toggle creation off by triggering an error for the primary refresh token.
  identity_env().UpdatePersistentErrorOfRefreshTokenForAccount(
      primary_account.account_id,
      GoogleServiceAuthError(GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS));

  // Verify that Plus Address creation doesn't occur.
  service().ReservePlusAddress(kNoSubdomainOrigin, base::DoNothing());
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  EXPECT_EQ(url_loader_factory().NumPending(), 0);

  // Toggle creation back on by removing the error.
  identity_env().UpdatePersistentErrorOfRefreshTokenForAccount(
      primary_account.account_id,
      GoogleServiceAuthError(GoogleServiceAuthError::NONE));

  // Verify that Plus Address creation occurs and makes a network request.
  base::test::TestFuture<const PlusProfileOrError&> reserve;
  service().ReservePlusAddress(kNoSubdomainOrigin, reserve.GetCallback());
  EXPECT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint,
      test::MakeCreationResponse(
          PlusProfile({.facet = kSite, .plus_address = kPlusAddress})));
  EXPECT_EQ(reserve.Get()->plus_address, kPlusAddress);

  base::test::TestFuture<const PlusProfileOrError&> confirm;
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               confirm.GetCallback());
  EXPECT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint,
      test::MakeCreationResponse(
          PlusProfile({.facet = kSite, .plus_address = kPlusAddress})));
  EXPECT_EQ(confirm.Get()->plus_address, kPlusAddress);
}

// Tests that ongoing network requests are cancelled on signout.
#if !BUILDFLAG(IS_CHROMEOS_ASH)
TEST_F(PlusAddressServiceRequestsTest, OngoingRequestsCancelledOnSignout) {
  base::test::TestFuture<const PlusProfileOrError&> future;
  service().ReservePlusAddress(kNoSubdomainOrigin, future.GetCallback());
  EXPECT_FALSE(future.IsReady());

  EXPECT_EQ(url_loader_factory().NumPending(), 1);
  identity_env().ClearPrimaryAccount();
  EXPECT_EQ(url_loader_factory().NumPending(), 0);
  ASSERT_TRUE(future.IsReady());
  EXPECT_EQ(future.Get(), base::unexpected(PlusAddressRequestError(
                              PlusAddressRequestErrorType::kUserSignedOut)));
}
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

class PlusAddressHttpForbiddenResponseTest
    : public PlusAddressServiceRequestsTest {
 public:
  PlusAddressHttpForbiddenResponseTest() {
    base::FieldTrialParams params =
        PlusAddressServiceRequestsTest::GetFieldTrialParams();
    params[features::kDisableForForbiddenUsers.name] = "true";
    features_.InitAndEnableFeatureWithParameters(features::kFeature, params);
  }

 private:
  base::test::ScopedFeatureList features_;
};

// Tests that two `HTTP_FORBIDDEN` responses and no successful network request
// lead to a disabled service.
TEST_F(PlusAddressHttpForbiddenResponseTest, RepeatedHttpForbiddenFromConfirm) {
  const std::string kPlusAddress = "plus+remote@plus.plus";
  ASSERT_FALSE(service().IsPlusAddress(kPlusAddress));

  // The service remains enabled after a single `HTTP_FORBIDDEN` response.
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
  EXPECT_TRUE(service().is_enabled());

  // A second `HTTP_FORBIDDEN` responses disables it.
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
  EXPECT_FALSE(service().is_enabled());
}

// Tests that two `HTTP_FORBIDDEN` responses and no successful network request
// do not lead to a disabled service unless the feature param is set.
TEST_F(PlusAddressHttpForbiddenResponseTest,
       RepeatedHttpForbiddenFromConfirmWithDisabledParam) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndEnableFeatureWithParameters(
      features::kFeature,
      PlusAddressServiceRequestsTest::GetFieldTrialParams());

  const std::string kPlusAddress = "plus+remote@plus.plus";
  ASSERT_FALSE(service().IsPlusAddress(kPlusAddress));

  // The service remains enabled after a single `HTTP_FORBIDDEN` response.
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
  EXPECT_TRUE(service().is_enabled());

  // A second `HTTP_FORBIDDEN` responses disables it.
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
  EXPECT_TRUE(service().is_enabled());
}

// Tests that two `HTTP_FORBIDDEN` responses and no successful network request
// lead to a disabled service and that other network errors do not have an
// impact.
TEST_F(PlusAddressHttpForbiddenResponseTest, OtherErrorsHaveNoEffect) {
  const std::string kPlusAddress = "plus+remote@plus.plus";
  ASSERT_FALSE(service().IsPlusAddress(kPlusAddress));

  // The service remains enabled after a single `HTTP_FORBIDDEN` response.
  service().ReservePlusAddress(kNoSubdomainOrigin, base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
  EXPECT_TRUE(service().is_enabled());

  // A failure that is not `HTTP_FORBIDDEN` does not disable the service.
  service().ReservePlusAddress(kNoSubdomainOrigin, base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint, "", net::HTTP_REQUEST_TIMEOUT));
  EXPECT_TRUE(service().is_enabled());

  // But a second `HTTP_FORBIDDEN` does.
  service().ReservePlusAddress(kNoSubdomainOrigin, base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kReservePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
  EXPECT_FALSE(service().is_enabled());
}

// Tests a single successful response prevents later `HTTP_FORBIDDEN` responses
// from disabling the service.
TEST_F(PlusAddressHttpForbiddenResponseTest, NoDisablingAfterSuccess) {
  const std::string kPlusAddress = "plus+remote@plus.plus";
  const std::string kOtherPlusAddress = "plusplus+remote@plus.plus";
  ASSERT_FALSE(service().IsPlusAddress(kPlusAddress));
  const url::Origin kOtherOrigin =
      url::Origin::Create(GURL("https://test.example.com"));

  // The service remains enabled after a single `HTTP_FORBIDDEN` response.
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
  EXPECT_TRUE(service().is_enabled());

  // After a single successful call ...
  service().ConfirmPlusAddress(kNoSubdomainOrigin, kPlusAddress,
                               base::DoNothing());
  ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
      kCreatePlusAddressEndpoint,
      test::MakeCreationResponse(PlusProfile({.facet = "test.example",
                                              .plus_address = kPlusAddress,
                                              .is_confirmed = true}))));
  EXPECT_TRUE(service().IsPlusAddress(kPlusAddress));

  // ... even repeated `HTTP_FORBIDDEN` responses do not disable the service.
  for (int i = 0; i < 5; ++i) {
    SCOPED_TRACE(::testing::Message() << "Iteration #" << 1);
    // But a second `HTTP_FORBIDDEN` does.
    service().ConfirmPlusAddress(kOtherOrigin, kOtherPlusAddress,
                                 base::DoNothing());
    ASSERT_TRUE(url_loader_factory().SimulateResponseForPendingRequest(
        kCreatePlusAddressEndpoint, "", net::HTTP_FORBIDDEN));
    EXPECT_TRUE(service().is_enabled());
  }
}

// Tests the PlusAddressService ability to make network requests.
class PlusAddressServicePolling : public PlusAddressServiceRequestsTest {
 public:
  PlusAddressServicePolling() {
    base::FieldTrialParams params =
        PlusAddressServiceRequestsTest::GetFieldTrialParams();
    params["sync-with-server"] = "true";
    feature_list_.InitAndEnableFeatureWithParameters(features::kFeature,
                                                     params);
    InitService();
  }

 private:
  base::test::ScopedFeatureList feature_list_;
};

TEST_F(PlusAddressServicePolling, CallsGetAllPlusAddresses) {
  // The service starts the timer on construction and issues a request to
  // poll. Unblock the initial polling request.
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint, test::MakeListResponse({}));

  EXPECT_FALSE(service().IsPlusAddress("plus+foo@plus.plus"));
  EXPECT_FALSE(service().IsPlusAddress("plus+bar@plus.plus"));

  task_environment().FastForwardBy(
      features::kEnterprisePlusAddressTimerDelay.Get() + base::Seconds(1));
  EXPECT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint,
      test::MakeListResponse(
          {PlusProfile{.facet = "foo.com",
                       .plus_address = "plus+foo@plus.plus"},
           PlusProfile{.facet = "bar.com",
                       .plus_address = "plus+bar@plus.plus"}}));

  // The service's mapping should be updated now.
  url::Origin foo_origin = url::Origin::Create(GURL("https://foo.com"));
  ASSERT_TRUE(service().GetPlusAddress(foo_origin).has_value());
  EXPECT_EQ(service().GetPlusAddress(foo_origin).value(), "plus+foo@plus.plus");
  EXPECT_TRUE(service().IsPlusAddress("plus+foo@plus.plus"));

  url::Origin bar_origin = url::Origin::Create(GURL("https://bar.com"));
  ASSERT_TRUE(service().GetPlusAddress(bar_origin).has_value());
  EXPECT_EQ(service().GetPlusAddress(bar_origin).value(), "plus+bar@plus.plus");
  EXPECT_TRUE(service().IsPlusAddress("plus+bar@plus.plus"));
}

TEST_F(PlusAddressServicePolling,
       DisableForForbiddenUsers_Enabled_404sDontDisableFeature) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndEnableFeatureWithParameters(
      features::kFeature, {
                              {"server-url", kServerUrl.spec()},
                              {"oauth-scope", "scope.example"},
                              {"sync-with-server", "true"},
                              {"disable-for-forbidden-users", "true"},
                          });
  InitService();

  EXPECT_TRUE(service().is_enabled());
  // Unblock the initial polling request.
  ASSERT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint, "", net::HTTP_NOT_FOUND);
  EXPECT_TRUE(service().is_enabled());
}

TEST_F(PlusAddressServicePolling,
       DisableForForbiddenUsers_Enabled_403sDisableFeature) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndEnableFeatureWithParameters(
      features::kFeature, {
                              {"server-url", kServerUrl.spec()},
                              {"oauth-scope", "scope.example"},
                              {"sync-with-server", "true"},
                              {"disable-for-forbidden-users", "true"},
                          });
  InitService();

  EXPECT_TRUE(service().is_enabled());
  // Unblock the initial polling request.
  ASSERT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint, "", net::HTTP_FORBIDDEN);
  // Simulate failed responses for the successive retry requests
  for (int i = 0; i < PlusAddressService::kMaxHttpForbiddenResponses; i++) {
    EXPECT_TRUE(service().is_enabled());
    ASSERT_EQ(url_loader_factory().NumPending(), 1);
    url_loader_factory().SimulateResponseForPendingRequest(
        kPlusProfilesEndpoint, "", net::HTTP_FORBIDDEN);
  }
  // Service is finally disabled once retries are exhausted.
  EXPECT_FALSE(service().is_enabled());
}

TEST_F(PlusAddressServicePolling,
       DisableForForbiddenUsers_Disabled_403DoesntRetryOrDisableFeature) {
  EXPECT_TRUE(service().is_enabled());
  // Unblock the initial polling request.
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint, "", net::HTTP_FORBIDDEN);
  EXPECT_EQ(url_loader_factory().NumPending(), 0);
  EXPECT_TRUE(service().is_enabled());
}

// Doesn't run on ChromeOS since ClearPrimaryAccount() doesn't exist for it.
#if !BUILDFLAG(IS_CHROMEOS_ASH)
TEST_F(PlusAddressServicePolling, PrimaryAccountCleared_TogglesPollingOff) {
  // Unblock initial poll.
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint, test::MakeListResponse({}));
  identity_env().ClearPrimaryAccount();

  // Toggle polling back on by signing into a primary account.
  identity_env().MakePrimaryAccountAvailable("plus2@plus.plus",
                                             signin::ConsentLevel::kSignin);
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint,
      test::MakeListResponse({PlusProfile{
          .facet = "foo.com", .plus_address = "plus+foo@plus.plus"}}));
  url::Origin foo_origin = url::Origin::Create(GURL("https://foo.com"));
  ASSERT_TRUE(service().GetPlusAddress(foo_origin).has_value());
  EXPECT_EQ(service().GetPlusAddress(foo_origin).value(), "plus+foo@plus.plus");
  EXPECT_TRUE(service().IsPlusAddress("plus+foo@plus.plus"));
}
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

TEST_F(PlusAddressServicePolling, PrimaryRefreshTokenError_TogglesPollingOff) {
  CoreAccountInfo primary_account =
      identity_manager()->GetPrimaryAccountInfo(signin::ConsentLevel::kSignin);
  // Unblock initial poll.
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint, test::MakeListResponse({}));

  // Toggle creation off by triggering an error for the primary refresh token.
  identity_env().UpdatePersistentErrorOfRefreshTokenForAccount(
      primary_account.account_id,
      GoogleServiceAuthError(GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS));
  task_environment().RunUntilIdle();
  // No network requests are expected because the service is not enabled.
  EXPECT_EQ(url_loader_factory().NumPending(), 0);

  // Toggle creation back on by removing the error.
  identity_env().UpdatePersistentErrorOfRefreshTokenForAccount(
      primary_account.account_id,
      GoogleServiceAuthError(GoogleServiceAuthError::NONE));
  task_environment().RunUntilIdle();

  EXPECT_EQ(url_loader_factory().NumPending(), 1);
  url_loader_factory().SimulateResponseForPendingRequest(
      kPlusProfilesEndpoint,
      test::MakeListResponse({PlusProfile{
          .facet = "foo.com", .plus_address = "plus+foo@plus.plus"}}));
  url::Origin foo_origin = url::Origin::Create(GURL("https://foo.com"));
  ASSERT_TRUE(service().GetPlusAddress(foo_origin).has_value());
  EXPECT_EQ(service().GetPlusAddress(foo_origin).value(), "plus+foo@plus.plus");
  EXPECT_TRUE(service().IsPlusAddress("plus+foo@plus.plus"));
}

// Tests that communication with `PlusAddressTable` works.
class PlusAddressServiceWebDataTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create an in-memory PlusAddressTable fully operating on the UI sequence.
    webdatabase_service_ = base::MakeRefCounted<WebDatabaseService>(
        base::FilePath(WebDatabase::kInMemoryPath),
        base::SingleThreadTaskRunner::GetCurrentDefault(),
        base::SingleThreadTaskRunner::GetCurrentDefault());
    webdatabase_service_->AddTable(std::make_unique<PlusAddressTable>());
    webdatabase_service_->LoadDatabase();
    plus_webdata_service_ = base::MakeRefCounted<PlusAddressWebDataService>(
        webdatabase_service_, base::SingleThreadTaskRunner::GetCurrentDefault(),
        base::SingleThreadTaskRunner::GetCurrentDefault());
    plus_webdata_service_->Init(base::DoNothing());
  }

  base::test::TaskEnvironment task_environment_;
  scoped_refptr<WebDatabaseService> webdatabase_service_;
  scoped_refptr<PlusAddressWebDataService> plus_webdata_service_;
};

// Tests that when plus addresses are received from the backend, they are
// persisted in the database and afterwards available through
// PlusAddressService.
TEST_F(PlusAddressServiceWebDataTest, DatabaseRoundTrip) {
  PlusAddressService service(
      /*identity_manager=*/nullptr, /*pref_service=*/nullptr,
      std::make_unique<PlusAddressHttpClientImpl>(
          /*identity_manager=*/nullptr,
          /*url_loader_factory=*/nullptr),
      plus_webdata_service_);
  // Simulate receiving an address from the backend.
  // TODO(b/322147254): Update once sync integration exists.
  url::Origin foo_origin = url::Origin::Create(GURL("https://foo.com"));
  service.UpdatePlusAddressMap(
      {{foo_origin.GetURL().host(), "plus+foo@plus.plus"}});

  // Expect that it is not available through the `service` yet, since the DB
  // task is still pending.
  EXPECT_FALSE(service.GetPlusAddress(foo_origin).has_value());

  // Wait for the DB tasks to finish and expect that the address is available.
  task_environment_.RunUntilIdle();
  EXPECT_TRUE(service.GetPlusAddress(foo_origin).has_value());
}

class PlusAddressServiceDisabledTest : public PlusAddressServiceTest {
 protected:
  PlusAddressServiceDisabledTest() {
    scoped_feature_list_.InitAndDisableFeature(features::kFeature);
    InitService();
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_F(PlusAddressServiceDisabledTest, FeatureExplicitlyDisabled) {
  // `SupportsPlusAddresses` should return `false`, even if there's a
  // signed-in user.
  identity_env().MakeAccountAvailable("plus@plus.plus",
                                      {signin::ConsentLevel::kSignin});
  InitService();
  EXPECT_FALSE(service().SupportsPlusAddresses(
      url::Origin::Create(GURL("https://test.example")),
      /*is_off_the_record=*/false));
}

class PlusAddressServiceEnabledTest : public PlusAddressServiceTest {
 public:
  PlusAddressServiceEnabledTest() {
    scoped_feature_list_.InitAndEnableFeatureWithParameters(
        features::kFeature,
        {{features::kEnterprisePlusAddressServerUrl.name, "mattwashere"}});
    InitService();
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_F(PlusAddressServiceEnabledTest, NoSignedInUser) {
  // Without a signed in user, the `SupportsPlusAddresses` should return
  // `false`.
  EXPECT_FALSE(service().SupportsPlusAddresses(kNoSubdomainOrigin,
                                               /*is_off_the_record=*/false));
}

TEST_F(PlusAddressServiceEnabledTest, FullySupported) {
  // With a signed in user, the `SupportsPlusAddresses` function should return
  // `true`.
  identity_env().MakeAccountAvailable("plus@plus.plus",
                                      {signin::ConsentLevel::kSignin});
  InitService();
  EXPECT_TRUE(service().SupportsPlusAddresses(kNoSubdomainOrigin,
                                              /*is_off_the_record=*/false));
}

// `SupportsPlusAddresses` returns false when `origin` is included on
// `kPlusAddressExcludedSites` and true otherwise.
TEST_F(PlusAddressServiceEnabledTest, ExcludedSitesAreNotSupported) {
  identity_env().MakeAccountAvailable("plus@plus.plus",
                                      {signin::ConsentLevel::kSignin});
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndEnableFeatureWithParameters(
      features::kFeature,
      {{features::kEnterprisePlusAddressServerUrl.name, "mattwashere"},
       {features::kPlusAddressExcludedSites.name,
        "exclude.co.th,forbidden.com"}});
  InitService();

  // Verify that url not on the excluded site continues to work.
  EXPECT_TRUE(service().SupportsPlusAddresses(
      url::Origin::Create(GURL("https://test.example")),
      /*is_off_the_record=*/false));

  // Sites on excluded list are not supported.
  EXPECT_FALSE(service().SupportsPlusAddresses(
      url::Origin::Create(GURL("https://www.forbidden.com")),
      /*is_off_the_record=*/false));
  EXPECT_FALSE(service().SupportsPlusAddresses(
      url::Origin::Create(GURL("https://www.exclude.co.th")),
      /*is_off_the_record=*/false));

  // Excluded site with different subdomain are also not supported.
  EXPECT_FALSE(service().SupportsPlusAddresses(
      url::Origin::Create(GURL("https://myaccount.forbidden.com")),
      /*is_off_the_record=*/false));
}

// `SupportsPlusAddresses` returns false when `origin` scheme is not http or
// https.
TEST_F(PlusAddressServiceEnabledTest, NonHTTPSchemesAreNotSupported) {
  identity_env().MakeAccountAvailable("plus@plus.plus",
                                      {signin::ConsentLevel::kSignin});
  InitService();
  EXPECT_TRUE(service().SupportsPlusAddresses(kNoSubdomainOrigin,
                                              /*is_off_the_record=*/false));
  EXPECT_FALSE(service().SupportsPlusAddresses(
      url::Origin::Create(GURL("other://hello")),
      /*is_off_the_record=*/false));
}

// `SupportsPlusAddresses` returns false when `origin` is opaque.
TEST_F(PlusAddressServiceEnabledTest, OpaqueOriginIsNotSupported) {
  identity_env().MakeAccountAvailable("plus@plus.plus",
                                      {signin::ConsentLevel::kSignin});
  InitService();
  EXPECT_FALSE(service().SupportsPlusAddresses(url::Origin(), false));
}

TEST_F(PlusAddressServiceEnabledTest, OTRWithNoExistingAddress) {
  // With a signed in user, an off-the-record session, and no existing address,
  // the `SupportsPlusAddresses` function should return `false`.
  identity_env().MakeAccountAvailable("plus@plus.plus",
                                      {signin::ConsentLevel::kSignin});
  InitService();
  EXPECT_FALSE(service().SupportsPlusAddresses(kNoSubdomainOrigin,
                                               /*is_off_the_record=*/true));
}

TEST_F(PlusAddressServiceEnabledTest, OTRWithExistingAddress) {
  // With a signed in user, an off-the-record session, and an existing address,
  // the `SupportsPlusAddresses` function should return `true`.
  identity_env().MakeAccountAvailable("plus@plus.plus",
                                      {signin::ConsentLevel::kSignin});
  InitService();

  service().SavePlusAddress(kNoSubdomainOrigin, "plus@plus.plus");
  EXPECT_TRUE(service().SupportsPlusAddresses(kNoSubdomainOrigin,
                                              /*is_off_the_record=*/true));
}

TEST_F(PlusAddressServiceEnabledTest, SignedOutGetEmail) {
  EXPECT_EQ(service().GetPrimaryEmail(), std::nullopt);
}

TEST_F(PlusAddressServiceEnabledTest, SignedInGetEmail) {
  constexpr std::string_view expected_email = "plus@plus.plus";
  identity_env().MakeAccountAvailable(expected_email,
                                      {signin::ConsentLevel::kSignin});
  InitService();

  EXPECT_EQ(service().GetPrimaryEmail(), expected_email);
}

// Tests that PlusAddresses is "disabled" in the following states:
// - When a primary account is unset after login.
// - When a primary account's refresh token has an auth error.
//
// If PlusAddressService is "disabled" it should stop offering the feature,
// clear any local storage, and not issue network requests.
class PlusAddressServiceSignoutTest : public PlusAddressServiceTest {
 public:
  PlusAddressServiceSignoutTest() {
    scoped_feature_list_.InitAndEnableFeatureWithParameters(
        features::kFeature,
        {{features::kEnterprisePlusAddressServerUrl.name, "mattwashere"},
         {features::kEnterprisePlusAddressOAuthScope.name, "scope.example"}});
    secondary_account_ = identity_env().MakeAccountAvailable(
        "beta@plus.plus", {signin::ConsentLevel::kSignin});
    primary_account_ = identity_env().MakePrimaryAccountAvailable(
        "alpha@plus.plus", signin::ConsentLevel::kSignin);
    InitService();
  }

  const CoreAccountInfo& primary_account() const { return primary_account_; }
  const AccountInfo& secondary_account() const { return secondary_account_; }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
  CoreAccountInfo primary_account_;
  AccountInfo secondary_account_;
};

// Doesn't run on ChromeOS since ClearPrimaryAccount() doesn't exist for it.
#if !BUILDFLAG(IS_CHROMEOS_ASH)
TEST_F(PlusAddressServiceSignoutTest, PrimaryAccountCleared_TogglesIsEnabled) {
  ASSERT_TRUE(service().is_enabled());

  // Verify behaviors expected when service is enabled.
  url::Origin site = url::Origin::Create(GURL("https://foo.com"));
  service().SavePlusAddress(site, "plus@plus.plus");
  EXPECT_TRUE(
      service().SupportsPlusAddresses(site, /*is_off_the_record=*/false));
  EXPECT_TRUE(service().GetPlusAddress(site));
  EXPECT_EQ(service().GetPlusAddress(site).value(), "plus@plus.plus");
  EXPECT_TRUE(service().IsPlusAddress("plus@plus.plus"));

  identity_env().ClearPrimaryAccount();
  EXPECT_FALSE(service().is_enabled());

  // Ensure that the local data is cleared on disabling.
  EXPECT_FALSE(
      service().SupportsPlusAddresses(site, /*is_off_the_record=*/false));
  EXPECT_FALSE(service().IsPlusAddress("plus@plus.plus"));
}
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

TEST_F(PlusAddressServiceSignoutTest,
       PrimaryRefreshTokenError_TogglesIsEnabled) {
  ASSERT_TRUE(service().is_enabled());

  // Verify behaviors expected when service is enabled.
  url::Origin site = url::Origin::Create(GURL("https://foo.com"));
  service().SavePlusAddress(site, "plus@plus.plus");
  EXPECT_TRUE(
      service().SupportsPlusAddresses(site, /*is_off_the_record=*/false));
  EXPECT_TRUE(service().GetPlusAddress(site));
  EXPECT_EQ(service().GetPlusAddress(site).value(), "plus@plus.plus");
  EXPECT_TRUE(service().IsPlusAddress("plus@plus.plus"));

  // Setting to NONE doesn't disable the service.
  identity_env().UpdatePersistentErrorOfRefreshTokenForAccount(
      primary_account().account_id,
      GoogleServiceAuthError(GoogleServiceAuthError::NONE));
  EXPECT_TRUE(service().is_enabled());

  // The PlusAddressService isn't disabled for secondary account auth errors.
  identity_env().UpdatePersistentErrorOfRefreshTokenForAccount(
      secondary_account().account_id,
      GoogleServiceAuthError(GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS));
  EXPECT_TRUE(service().is_enabled());

  // Being in the "sync-paused" state results in this error.
  identity_env().UpdatePersistentErrorOfRefreshTokenForAccount(
      primary_account().account_id,
      GoogleServiceAuthError(GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS));
  EXPECT_FALSE(service().is_enabled());

  // Ensure that the local data is cleared on disabling.
  EXPECT_FALSE(
      service().SupportsPlusAddresses(site, /*is_off_the_record=*/false));
  EXPECT_FALSE(service().IsPlusAddress("plus@plus.plus"));
}

// A test fixture with a `PlusAddressService` that is enabled to allow testing
// suggestion generation.
class PlusAddressSuggestionsTest : public PlusAddressServiceTest {
 public:
  PlusAddressSuggestionsTest() {
    scoped_feature_list_.InitAndEnableFeatureWithParameters(
        features::kFeature, {{"server-url", "https://server.example"},
                             {"oauth-scope", "scope.example"}});
    identity_env().MakePrimaryAccountAvailable("plus@plus.plus",
                                               signin::ConsentLevel::kSignin);
    identity_env().SetAutomaticIssueOfAccessTokens(true);
    InitService();
  }

 protected:
  static constexpr std::string_view kPlusAddressSuggestionMetric =
      "Autofill.PlusAddresses.Suggestion.Events";

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

// Tests that fill plus address suggestions are offered iff the value in the
// focused field matches the prefix of an existing plus address.
TEST_F(PlusAddressSuggestionsTest, SuggestionsForExistingPlusAddress) {
  base::HistogramTester histogram_tester;
  const auto origin = url::Origin::Create(GURL("https://foo.coom"));
  service().SavePlusAddress(origin, kPlusAddress);

  // We offer filling if the field is empty.
  EXPECT_THAT(service().GetSuggestions(
                  origin, /*is_off_the_record=*/false,
                  /*focused_field_value=*/u"",
                  AutofillSuggestionTriggerSource::kFormControlElementClicked),
              IsSingleFillPlusAddressSuggestion(kPlusAddress));
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kExistingPlusAddressSuggested, 1);

  // If the user types a letter and it matches the plus address (after
  // normalization), the plus address continues to be offered.
  EXPECT_THAT(service().GetSuggestions(
                  origin, /*is_off_the_record=*/false,
                  /*focused_field_value=*/u"P",
                  AutofillSuggestionTriggerSource::kFormControlElementClicked),
              IsSingleFillPlusAddressSuggestion(kPlusAddress));
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kExistingPlusAddressSuggested, 2);

  // If the value does not match the prefix of the plus address, nothing is
  // shown.
  EXPECT_THAT(service().GetSuggestions(
                  origin, /*is_off_the_record=*/false,
                  /*focused_field_value=*/u"pp",
                  AutofillSuggestionTriggerSource::kFormControlElementClicked),
              IsEmpty());
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kExistingPlusAddressSuggested, 2);
}

// Tests that fill plus address suggestions regardless of whether there is
// already text in the field if the trigger source was manual fallback.
TEST_F(PlusAddressSuggestionsTest,
       SuggestionsForExistingPlusAddressWithManualFallback) {
  base::HistogramTester histogram_tester;
  const auto origin = url::Origin::Create(GURL("https://foo.coom"));
  service().SavePlusAddress(origin, kPlusAddress);

  // We offer filling if the field is empty.
  EXPECT_THAT(
      service().GetSuggestions(
          origin, /*is_off_the_record=*/false,
          /*focused_field_value=*/u"",
          AutofillSuggestionTriggerSource::kManualFallbackPlusAddresses),
      IsSingleFillPlusAddressSuggestion(kPlusAddress));
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kExistingPlusAddressSuggested, 1);

  // We also offer filling if the field is not empty and the prefix does not
  // match the address.
  EXPECT_THAT(
      service().GetSuggestions(
          origin, /*is_off_the_record=*/false,
          /*focused_field_value=*/u"pp",
          AutofillSuggestionTriggerSource::kManualFallbackPlusAddresses),
      IsSingleFillPlusAddressSuggestion(kPlusAddress));
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kExistingPlusAddressSuggested, 2);
}

// Tests that a create plus address suggestion is offered if there is no
// existing plus address for the domain and the field value is empty.
TEST_F(PlusAddressSuggestionsTest, SuggestionsForCreateNewPlusAddress) {
  base::HistogramTester histogram_tester;
  const auto origin = url::Origin::Create(GURL("https://foo.coom"));

  // We offer creation if the field is empty.
  EXPECT_THAT(service().GetSuggestions(
                  origin, /*is_off_the_record=*/false,
                  /*focused_field_value=*/u"",
                  AutofillSuggestionTriggerSource::kFormControlElementClicked),
              IsSingleCreatePlusAddressSuggestion());
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kCreateNewPlusAddressSuggested, 1);

  // If the field value is not empty, nothing is shown.
  EXPECT_THAT(service().GetSuggestions(
                  origin, /*is_off_the_record=*/false,
                  /*focused_field_value=*/u"some text",
                  AutofillSuggestionTriggerSource::kFormControlElementClicked),
              IsEmpty());
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kCreateNewPlusAddressSuggested, 1);
}

// Tests that a create plus address suggestion is offered regardless of the
// field's value if there is no existing plus address for the domain and the
// trigger source is a manual fallback.
TEST_F(PlusAddressSuggestionsTest,
       SuggestionsForCreateNewPlusAddressWithManualFallback) {
  base::HistogramTester histogram_tester;
  const auto origin = url::Origin::Create(GURL("https://foo.coom"));

  EXPECT_THAT(
      service().GetSuggestions(
          origin, /*is_off_the_record=*/false,
          /*focused_field_value=*/u"",
          AutofillSuggestionTriggerSource::kManualFallbackPlusAddresses),
      IsSingleCreatePlusAddressSuggestion());
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kCreateNewPlusAddressSuggested, 1);

  EXPECT_THAT(
      service().GetSuggestions(
          origin, /*is_off_the_record=*/false,
          /*focused_field_value=*/u"some text",
          AutofillSuggestionTriggerSource::kManualFallbackPlusAddresses),
      IsSingleCreatePlusAddressSuggestion());
  histogram_tester.ExpectUniqueSample(
      kPlusAddressSuggestionMetric,
      SuggestionEvent::kCreateNewPlusAddressSuggested, 2);
}

// Tests that no suggestions are returned when plus address are disabled.
TEST_F(PlusAddressSuggestionsTest, NoSuggestionsWhenDisabled) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndDisableFeature(features::kFeature);

  EXPECT_THAT(service().GetSuggestions(
                  url::Origin::Create(GURL("https://foo.coom")),
                  /*is_off_the_record=*/false, /*focused_field_value=*/u"",
                  AutofillSuggestionTriggerSource::kFormControlElementClicked),
              IsEmpty());
}

}  // namespace plus_addresses
