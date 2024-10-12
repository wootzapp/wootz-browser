// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/google_url_loader_throttle.h"

#include <memory>
#include <string_view>

#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "base/time/time.h"
#include "build/buildflag.h"
#include "chrome/common/renderer_configuration.mojom.h"
#include "components/signin/public/base/signin_switches.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/url_loader_completion_status.h"
#include "services/network/public/mojom/fetch_api.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"

// This file only contains tests relevant to the bound session credentials
// feature.
#if BUILDFLAG(ENABLE_BOUND_SESSION_CREDENTIALS)
#include "chrome/common/bound_session_request_throttled_handler.h"

namespace {

enum class RequestAction { kWillStartRequest, kWillRedirectRequest };

class FakeBoundSessionRequestThrottledHandler
    : public BoundSessionRequestThrottledHandler {
 public:
  void HandleRequestBlockedOnCookie(
      ResumeOrCancelThrottledRequestCallback callback) override {
    EXPECT_FALSE(callback_);
    callback_ = std::move(callback);
  }

  void SimulateHandleRequestBlockedOnCookieCompleted(
      UnblockAction unblock_action,
      chrome::mojom::ResumeBlockedRequestsTrigger resume_trigger) {
    std::move(callback_).Run(unblock_action, resume_trigger);
  }

  bool IsRequestBlocked() { return !callback_.is_null(); }

 private:
  ResumeOrCancelThrottledRequestCallback callback_;
};

class MockThrottleDelegate : public blink::URLLoaderThrottle::Delegate {
 public:
  MockThrottleDelegate() = default;
  MOCK_METHOD(void, CancelWithError, (int, std::string_view), (override));
  MOCK_METHOD(void, Resume, (), (override));
};

class GoogleURLLoaderThrottleTest
    : public ::testing::Test,
      public ::testing::WithParamInterface<RequestAction> {
 public:
  const GURL kTestGoogleURL = GURL("https://google.com");
  const GURL kGoogleSubdomainURL = GURL("https://accounts.google.com");

  GoogleURLLoaderThrottleTest() = default;
  ~GoogleURLLoaderThrottleTest() override = default;

  void ConfigureBoundSessionThrottlerParams(const std::string& domain,
                                            const std::string& path,
                                            base::Time expiration_date) {
    bound_session_throttler_params_ =
        chrome::mojom::BoundSessionThrottlerParams::New(domain, path,
                                                        expiration_date);
  }

  void RunUntilIdle() { task_environment_.RunUntilIdle(); }

  FakeBoundSessionRequestThrottledHandler* bound_session_listener() {
    return bound_session_listener_.get();
  }

  GoogleURLLoaderThrottle* throttle() {
    if (!throttle_) {
      CreateThrottle();
    }
    return throttle_.get();
  }

  MockThrottleDelegate* delegate() { return delegate_.get(); }

  void CallThrottleAndVerifyDeferExpectation(bool expect_defer,
                                             const GURL& url) {
    // `WillStartRequest()` has to be occur before `WillRedirectRequest()`, so
    // call it unconditionally.
    bool defer = false;
    network::ResourceRequest request;
    request.url = url;
    throttle()->WillStartRequest(&request, &defer);
    if (GetParam() == RequestAction::kWillRedirectRequest) {
      // Undo effects of the initial request, if needed.
      if (defer == true) {
        UnblockRequestAndVerifyCallbackAction(
            BoundSessionRequestThrottledHandler::UnblockAction::kResume);
      }

      net::RedirectInfo redirect_info;
      redirect_info.new_url = url;
      network::mojom::URLResponseHead response_head;
      std::vector<std::string> to_be_removed_headers;
      net::HttpRequestHeaders modified_headers;
      net::HttpRequestHeaders modified_cors_exempt_headers;
      throttle()->WillRedirectRequest(&redirect_info, response_head, &defer,
                                      &to_be_removed_headers, &modified_headers,
                                      &modified_cors_exempt_headers);
    }
    EXPECT_EQ(expect_defer, defer);
    EXPECT_EQ(expect_defer, bound_session_listener()->IsRequestBlocked());
  }

  void UnblockRequestAndVerifyCallbackAction(
      BoundSessionRequestThrottledHandler::UnblockAction unblock_action,
      bool is_expected_navigation = false,
      chrome::mojom::ResumeBlockedRequestsTrigger resume_trigger =
          chrome::mojom::ResumeBlockedRequestsTrigger::kObservedFreshCookies) {
    switch (unblock_action) {
      case BoundSessionRequestThrottledHandler::UnblockAction::kResume:
        EXPECT_CALL(*delegate(), Resume());
        break;
      case BoundSessionRequestThrottledHandler::UnblockAction::kCancel:
        EXPECT_CALL(*delegate(), CancelWithError(net::ERR_ABORTED, testing::_));
        break;
    }

    bound_session_listener_->SimulateHandleRequestBlockedOnCookieCompleted(
        unblock_action, resume_trigger);

    RunUntilIdle();
    testing::Mock::VerifyAndClearExpectations(delegate());
    histogram_tester_->ExpectTotalCount(
        "Signin.BoundSessionCredentials.DeferredRequestDelay",
        /*expected_count=*/1);
    histogram_tester_->ExpectTotalCount(
        "Signin.BoundSessionCredentials.DeferredNavigationRequestDelay",
        /*expected_count=*/is_expected_navigation ? 1 : 0);
    histogram_tester_ = std::make_unique<base::HistogramTester>();
  }

 private:
  void CreateThrottle() {
    chrome::mojom::DynamicParamsPtr dynamic_params(
        chrome::mojom::DynamicParams::New());
    dynamic_params->bound_session_throttler_params =
        bound_session_throttler_params_.Clone();

    std::unique_ptr<FakeBoundSessionRequestThrottledHandler>
        bound_session_listener =
            std::make_unique<FakeBoundSessionRequestThrottledHandler>();
    bound_session_listener_ = bound_session_listener.get();
    delegate_ = std::make_unique<MockThrottleDelegate>();

    throttle_ = std::make_unique<GoogleURLLoaderThrottle>(
#if BUILDFLAG(IS_ANDROID)
        "",
#endif
        std::move(bound_session_listener), std::move(dynamic_params));
    throttle_->set_delegate(delegate_.get());
  }

  base::test::ScopedFeatureList feature_list_{
      switches::kEnableBoundSessionCredentials};
  base::test::TaskEnvironment task_environment_;
  raw_ptr<FakeBoundSessionRequestThrottledHandler, DanglingUntriaged>
      bound_session_listener_ = nullptr;
  std::unique_ptr<GoogleURLLoaderThrottle> throttle_;
  std::unique_ptr<MockThrottleDelegate> delegate_;
  chrome::mojom::BoundSessionThrottlerParamsPtr bound_session_throttler_params_;
  std::unique_ptr<base::HistogramTester> histogram_tester_ =
      std::make_unique<base::HistogramTester>();
};

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionNullParams) {
  EXPECT_FALSE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL, nullptr));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionEmptyParams) {
  EXPECT_FALSE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL,
      chrome::mojom::BoundSessionThrottlerParams::New("", "", base::Time::Now())
          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionCookieFresh) {
  EXPECT_FALSE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL,
      chrome::mojom::BoundSessionThrottlerParams::New(
          "google.com", "/", base::Time::Now() + base::Minutes(10))
          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionNotInBoundSession) {
  EXPECT_FALSE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      GURL("https://youtube.com"),
      chrome::mojom::BoundSessionThrottlerParams::New("google.com", "/",
                                                      base::Time::Min())
          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionCookieExpired) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL, chrome::mojom::BoundSessionThrottlerParams::New(
                          "google.com", "/", base::Time::Min())
                          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionCookieExpiresNow) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL, chrome::mojom::BoundSessionThrottlerParams::New(
                          "google.com", "/", base::Time::Now())
                          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionCookieExpiredDomainWithLeadingDot) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL, chrome::mojom::BoundSessionThrottlerParams::New(
                          ".google.com", "/", base::Time::Min())
                          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionSubdomainUrl) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kGoogleSubdomainURL, chrome::mojom::BoundSessionThrottlerParams::New(
                               "google.com", "/", base::Time::Min())
                               .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionParentDomainUrl) {
  EXPECT_FALSE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL, chrome::mojom::BoundSessionThrottlerParams::New(
                          "accounts.google.com", "/", base::Time::Min())
                          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionUrlWithPath) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      GURL("https://google.com/foo/bar.html"),
      chrome::mojom::BoundSessionThrottlerParams::New("google.com", "/",
                                                      base::Time::Now())
          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionPathEmpty) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL, chrome::mojom::BoundSessionThrottlerParams::New(
                          "google.com", "", base::Time::Now())
                          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionUrlNotOnBoundSessionPath) {
  EXPECT_FALSE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      kTestGoogleURL, chrome::mojom::BoundSessionThrottlerParams::New(
                          "google.com", "/test", base::Time::Now())
                          .get()));
}

TEST_F(GoogleURLLoaderThrottleTest,
       ShouldDeferRequestForBoundSessionUrlWithPathOnBoundSessionPath) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      GURL("https://google.com/test/foo/bar.html"),
      chrome::mojom::BoundSessionThrottlerParams::New("google.com", "/test",
                                                      base::Time::Now())
          .get()));
}

TEST_F(
    GoogleURLLoaderThrottleTest,
    ShouldDeferRequestForBoundSessionSubdomainUrlWithPathOnBoundSessionPath) {
  EXPECT_TRUE(GoogleURLLoaderThrottle::ShouldDeferRequestForBoundSession(
      GURL("https://accounts.google.com/test/foo/bar.html"),
      chrome::mojom::BoundSessionThrottlerParams::New("google.com", "/test",
                                                      base::Time::Now())
          .get()));
}

TEST_P(GoogleURLLoaderThrottleTest, NullBoundSessionThrottlerParams) {
  CallThrottleAndVerifyDeferExpectation(
      /*expect_defer=*/false, kTestGoogleURL);
}

TEST_P(GoogleURLLoaderThrottleTest, EmptyBoundSessionThrottlerParams) {
  ConfigureBoundSessionThrottlerParams("", "", base::Time::Now());
  CallThrottleAndVerifyDeferExpectation(
      /*expect_defer=*/false, kGoogleSubdomainURL);
}

TEST_F(GoogleURLLoaderThrottleTest, NoInterceptRequestWithSendCookiesFalse) {
  ConfigureBoundSessionThrottlerParams("google.com", "/", base::Time::Min());
  bool defer = false;
  network::ResourceRequest request;
  request.url = kTestGoogleURL;
  request.credentials_mode = network::mojom::CredentialsMode::kOmit;
  throttle()->WillStartRequest(&request, &defer);
  EXPECT_FALSE(defer);
  EXPECT_FALSE(bound_session_listener()->IsRequestBlocked());

  // Subsequent redirects shouldn't be intercepted as well.
  net::RedirectInfo redirect_info;
  redirect_info.new_url = kTestGoogleURL;
  network::mojom::URLResponseHead response_head;
  std::vector<std::string> to_be_removed_headers;
  net::HttpRequestHeaders modified_headers;
  net::HttpRequestHeaders modified_cors_exempt_headers;
  throttle()->WillRedirectRequest(&redirect_info, response_head, &defer,
                                  &to_be_removed_headers, &modified_headers,
                                  &modified_cors_exempt_headers);
  EXPECT_FALSE(defer);
  EXPECT_FALSE(bound_session_listener()->IsRequestBlocked());
}

TEST_P(GoogleURLLoaderThrottleTest, InterceptBoundSessionCookieExpired) {
  ConfigureBoundSessionThrottlerParams("google.com", "/",
                                       base::Time::Now() - base::Minutes(10));
  CallThrottleAndVerifyDeferExpectation(
      /*expect_defer=*/true, GURL("https://accounts.google.com/test/bar.html"));
  UnblockRequestAndVerifyCallbackAction(
      BoundSessionRequestThrottledHandler::UnblockAction::kResume);
}

TEST_P(GoogleURLLoaderThrottleTest,
       InterceptNavigationBoundSessionCookieExpired) {
  ConfigureBoundSessionThrottlerParams("google.com", "/",
                                       base::Time::Now() - base::Minutes(10));
  bool defer = false;
  network::ResourceRequest request;
  request.url = GURL("https://accounts.google.com/test/bar.html");
  // Make `request` look like a main frame navigation request.
  request.is_outermost_main_frame = true;
  request.destination = network::mojom::RequestDestination::kDocument;
  throttle()->WillStartRequest(&request, &defer);
  EXPECT_TRUE(defer);
  EXPECT_TRUE(bound_session_listener()->IsRequestBlocked());
  UnblockRequestAndVerifyCallbackAction(
      BoundSessionRequestThrottledHandler::UnblockAction::kResume,
      /*is_expected_navigation=*/true);
}

TEST_P(GoogleURLLoaderThrottleTest, InterceptAndCancelRequest) {
  ConfigureBoundSessionThrottlerParams("google.com", "/", base::Time::Now());

  CallThrottleAndVerifyDeferExpectation(
      /*expect_defer=*/true, kGoogleSubdomainURL);
  UnblockRequestAndVerifyCallbackAction(
      BoundSessionRequestThrottledHandler::UnblockAction::kCancel);
}

TEST_P(GoogleURLLoaderThrottleTest,
       RecordDeferredRequestUnblockTriggerSuccess) {
  ConfigureBoundSessionThrottlerParams("google.com", "/", base::Time::Now());

  CallThrottleAndVerifyDeferExpectation(
      /*expect_defer=*/true, kGoogleSubdomainURL);
  const auto kResumeTrigger =
      chrome::mojom::ResumeBlockedRequestsTrigger::kCookieRefreshFetchFailure;
  UnblockRequestAndVerifyCallbackAction(
      BoundSessionRequestThrottledHandler::UnblockAction::kResume,
      /*is_expected_navigation=*/false, kResumeTrigger);
  base::HistogramTester histogram_tester;
  throttle()->WillProcessResponse(kGoogleSubdomainURL, nullptr, nullptr);
  histogram_tester.ExpectBucketCount(
      "Signin.BoundSessionCredentials.DeferredRequestUnblockTrigger.Success",
      static_cast<int>(kResumeTrigger),
      /*expected_count=*/1);
}

TEST_P(GoogleURLLoaderThrottleTest,
       RecordDeferredRequestUnblockTriggerFailure) {
  ConfigureBoundSessionThrottlerParams("google.com", "/", base::Time::Now());

  CallThrottleAndVerifyDeferExpectation(
      /*expect_defer=*/true, kGoogleSubdomainURL);
  const auto kResumeTrigger =
      chrome::mojom::ResumeBlockedRequestsTrigger::kCookieRefreshFetchFailure;
  UnblockRequestAndVerifyCallbackAction(
      BoundSessionRequestThrottledHandler::UnblockAction::kResume,
      /*is_expected_navigation=*/false, kResumeTrigger);
  base::HistogramTester histogram_tester;
  throttle()->WillOnCompleteWithError(network::URLLoaderCompletionStatus());
  histogram_tester.ExpectBucketCount(
      "Signin.BoundSessionCredentials.DeferredRequestUnblockTrigger.Failure",
      static_cast<int>(kResumeTrigger),
      /*expected_count=*/1);
}

TEST_P(GoogleURLLoaderThrottleTest,
       NoRecordDeferredRequestUnblockTriggerNotDeferred) {
  ConfigureBoundSessionThrottlerParams("google.com", "/",
                                       base::Time::Now() + base::Minutes(10));

  CallThrottleAndVerifyDeferExpectation(
      /*expect_defer=*/false, kGoogleSubdomainURL);
  base::HistogramTester histogram_tester;
  throttle()->WillProcessResponse(kGoogleSubdomainURL, nullptr, nullptr);
  EXPECT_THAT(
      histogram_tester.GetTotalCountsForPrefix(
          "Signin.BoundSessionCredentials.DeferredRequestUnblockTrigger."),
      testing::IsEmpty());
}

INSTANTIATE_TEST_SUITE_P(WillStartRequest,
                         GoogleURLLoaderThrottleTest,
                         ::testing::Values(RequestAction::kWillStartRequest));

INSTANTIATE_TEST_SUITE_P(
    WillRedirectRequest,
    GoogleURLLoaderThrottleTest,
    ::testing::Values(RequestAction::kWillRedirectRequest));

}  // namespace

#endif  // BUILDFLAG(ENABLE_BOUND_SESSION_CREDENTIALS)
