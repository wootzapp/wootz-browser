// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/signin/bound_session_credentials/bound_session_refresh_cookie_fetcher_impl.h"

#include <memory>
#include <optional>

#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/metrics/histogram_functions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "base/timer/elapsed_timer.h"
#include "base/trace_event/typed_macros.h"
#include "chrome/browser/signin/bound_session_credentials/bound_session_params_util.h"
#include "chrome/browser/signin/bound_session_credentials/rotation_debug_info.pb.h"
#include "chrome/browser/signin/bound_session_credentials/session_binding_helper.h"
#include "components/variations/net/variations_http_headers.h"
#include "google_apis/gaia/gaia_urls.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "net/cookies/canonical_cookie.h"
#include "net/http/http_status_code.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/cookie_access_observer.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

namespace {
constexpr char kRotationChallengeHeader[] = "Sec-Session-Google-Challenge";
constexpr char kRotationChallengeResponseHeader[] =
    "Sec-Session-Google-Response";
constexpr char kRotationDebugHeader[] =
    "Sec-Session-Google-Rotation-Debug-Info";
constexpr char kChallengeItemKey[] = "challenge";
const size_t kMaxAssertionRequestsAllowed = 5;

bool IsExpectedCookie(
    const GURL& url,
    const std::string& cookie_name,
    const network::mojom::CookieOrLineWithAccessResultPtr& cookie_ptr) {
  if (cookie_ptr->access_result.status.IsInclude()) {
    CHECK(cookie_ptr->cookie_or_line->is_cookie());
    const net::CanonicalCookie& cookie =
        cookie_ptr->cookie_or_line->get_cookie();
    return (cookie.Name() == cookie_name) && cookie.IsDomainMatch(url.host());
  }
  return false;
}

std::string UpdateDebugInfoAndSerializeToHeader(
    bound_session_credentials::RotationDebugInfo& debug_info) {
  *debug_info.mutable_request_time() =
      bound_session_credentials::TimeToTimestamp(base::Time::Now());
  std::string serialized = debug_info.SerializeAsString();
  return base::Base64Encode(serialized);
}
}  // namespace

BoundSessionRefreshCookieFetcherImpl::BoundSessionRefreshCookieFetcherImpl(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    SessionBindingHelper& session_binding_helper,
    const GURL& refresh_url,
    const GURL& cookie_url,
    base::flat_set<std::string> cookie_names,
    bool is_off_the_record_profile,
    bound_session_credentials::RotationDebugInfo debug_info)
    : url_loader_factory_(std::move(url_loader_factory)),
      session_binding_helper_(session_binding_helper),
      refresh_url_(refresh_url),
      expected_cookie_domain_(cookie_url),
      expected_cookie_names_(std::move(cookie_names)),
      is_off_the_record_profile_(is_off_the_record_profile),
      debug_info_(std::move(debug_info)) {
  CHECK(refresh_url.is_empty() || refresh_url.is_valid());
}

BoundSessionRefreshCookieFetcherImpl::~BoundSessionRefreshCookieFetcherImpl() =
    default;

void BoundSessionRefreshCookieFetcherImpl::Start(
    RefreshCookieCompleteCallback callback) {
  TRACE_EVENT("browser", "BoundSessionRefreshCookieFetcherImpl::Start",
              perfetto::Flow::FromPointer(this), "url",
              expected_cookie_domain_);
  CHECK(!callback_);
  CHECK(callback);
  callback_ = std::move(callback);
  StartRefreshRequest(/*sec_session_challenge_response=*/std::nullopt);
}

bool BoundSessionRefreshCookieFetcherImpl::IsChallengeReceived() const {
  return assertion_requests_count_ > 0;
}

void BoundSessionRefreshCookieFetcherImpl::StartRefreshRequest(
    std::optional<std::string> sec_session_challenge_response) {
  TRACE_EVENT("browser",
              "BoundSessionRefreshCookieFetcherImpl::StartRefreshRequest",
              perfetto::Flow::FromPointer(this), "has_challenge",
              sec_session_challenge_response.has_value());
  if (!cookie_refresh_duration_.has_value()) {
    cookie_refresh_duration_ = base::TimeTicks::Now();
  }

  // TODO(b/273920907): Update the `traffic_annotation` setting once a mechanism
  // allowing the user to disable the feature is implemented.
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("gaia_auth_rotate_bound_cookies",
                                          R"(
        semantics {
          sender: "Chrome - Google authentication API"
          description:
            "This request is used to rotate bound Google authentication "
            "cookies."
          trigger:
            "This request is triggered in a bound session when the bound Google"
            " authentication cookies are soon to expire."
          user_data {
            type: ACCESS_TOKEN
          }
          data: "Request includes cookies and a signed token proving that a"
                " request comes from the same device as was registered before."
          destination: GOOGLE_OWNED_SERVICE
          internal {
            contacts {
                email: "chrome-signin-team@google.com"
            }
          }
          last_reviewed: "2023-05-09"
        }
        policy {
          cookies_allowed: YES
          cookies_store: "user"
          setting:
            "This is a new feature being developed behind a flag that is"
            " disabled by default (kEnableBoundSessionCredentials). This"
            " request will only be sent if the feature is enabled and once"
            " a server requests it with a special header."

          policy_exception_justification:
            "Not implemented. "
            "If the feature is on, this request must be made to ensure the user"
            " maintains their signed in status on the web for Google owned"
            " domains."
        })");

  auto request = std::make_unique<network::ResourceRequest>();
  request->url = GetRefreshUrl();
  request->method = "GET";

  if (sec_session_challenge_response) {
    request->headers.SetHeader(kRotationChallengeResponseHeader,
                               *sec_session_challenge_response);
  }
  request->headers.SetHeader(kRotationDebugHeader,
                             UpdateDebugInfoAndSerializeToHeader(debug_info_));

  url::Origin origin = GaiaUrls::GetInstance()->gaia_origin();
  request->site_for_cookies = net::SiteForCookies::FromOrigin(origin);
  request->trusted_params = network::ResourceRequest::TrustedParams();
  request->trusted_params->isolation_info =
      net::IsolationInfo::CreateForInternalRequest(origin);

  mojo::PendingRemote<network::mojom::CookieAccessObserver> remote;
  cookie_observers_.Add(this, remote.InitWithNewPipeAndPassReceiver());
  request->trusted_params->cookie_observer = std::move(remote);

  url_loader_ =
      variations::CreateSimpleURLLoaderWithVariationsHeaderUnknownSignedIn(
          std::move(request),
          is_off_the_record_profile_ ? variations::InIncognito::kYes
                                     : variations::InIncognito::kNo,
          traffic_annotation);
  url_loader_->SetRetryOptions(
      3, network::SimpleURLLoader::RETRY_ON_NETWORK_CHANGE);
  // TODO(b/273920907): Download the response body to support in refresh DBSC
  // instructions update.
  // `base::Unretained(this)` is safe as `this` owns `url_loader_`.
  url_loader_->DownloadHeadersOnly(
      url_loader_factory_.get(),
      base::BindOnce(&BoundSessionRefreshCookieFetcherImpl::OnURLLoaderComplete,
                     base::Unretained(this)));
}

void BoundSessionRefreshCookieFetcherImpl::OnURLLoaderComplete(
    scoped_refptr<net::HttpResponseHeaders> headers) {
  net::Error net_error = static_cast<net::Error>(url_loader_->NetError());
  TRACE_EVENT("browser",
              "BoundSessionRefreshCookieFetcherImpl::OnURLLoaderComplete",
              perfetto::Flow::FromPointer(this), "net_error", net_error);

  std::optional<std::string> challenge_header_value =
      GetChallengeIfBindingKeyAssertionRequired(headers);
  if (challenge_header_value) {
    HandleBindingKeyAssertionRequired(*challenge_header_value);
    return;
  }

  cookie_refresh_completed_ = true;
  result_ = GetResultFromNetErrorAndHttpStatusCode(
      net_error,
      headers ? std::optional<int>(headers->response_code()) : std::nullopt);

  if (result_ == Result::kConnectionError) {
    base::UmaHistogramSparse(
        "Signin.BoundSessionCredentials.CookieRotationNetError", -net_error);
  }

  if (result_ == Result::kSuccess && !reported_cookies_notified_) {
    // Normally, a cookie update notification should be sent before the request
    // is complete. Add some leeway in the case mojo messages are delivered out
    // of order.
    const base::TimeDelta kResponseCookiesNotifiedMaxDelay =
        base::Milliseconds(100);
    // `base::Unretained` is safe as `this` owns
    // `reported_cookies_notified_timer_`.
    reported_cookies_notified_timer_.Start(
        FROM_HERE, kResponseCookiesNotifiedMaxDelay,
        base::BindOnce(
            &BoundSessionRefreshCookieFetcherImpl::ReportRefreshResult,
            base::Unretained(this)));
  } else {
    ReportRefreshResult();
  }
}

BoundSessionRefreshCookieFetcher::Result
BoundSessionRefreshCookieFetcherImpl::GetResultFromNetErrorAndHttpStatusCode(
    net::Error net_error,
    std::optional<int> response_code) {
  if ((net_error != net::OK &&
       net_error != net::ERR_HTTP_RESPONSE_CODE_FAILURE) ||
      !response_code) {
    return Result::kConnectionError;
  }

  if (response_code == net::HTTP_OK) {
    return Result::kSuccess;
  }

  if (response_code >= net::HTTP_INTERNAL_SERVER_ERROR) {
    // Server error 5xx.
    return Result::kServerTransientError;
  }

  if (response_code >= net::HTTP_BAD_REQUEST) {
    // Server error 4xx.
    return Result::kServerPersistentError;
  }

  // Unexpected response code.
  return Result::kServerPersistentError;
}

void BoundSessionRefreshCookieFetcherImpl::ReportRefreshResult() {
  reported_cookies_notified_timer_.Stop();
  CHECK(cookie_refresh_completed_);
  if (result_ == Result::kSuccess && !expected_cookies_set_) {
    result_ = Result::kServerUnexepectedResponse;
  }
  TRACE_EVENT("browser",
              "BoundSessionRefreshCookieFetcherImpl::ReportRefreshResult",
              perfetto::TerminatingFlow::FromPointer(this), "result", result_);
  base::UmaHistogramEnumeration(
      "Signin.BoundSessionCredentials.CookieRotationResult", result_);

  CHECK(cookie_refresh_duration_.has_value());
  base::TimeDelta duration = base::TimeTicks::Now() - *cookie_refresh_duration_;
  cookie_refresh_duration_.reset();
  base::UmaHistogramMediumTimes(
      "Signin.BoundSessionCredentials.CookieRotationTotalDuration", duration);

  std::move(callback_).Run(result_);
}

std::optional<std::string>
BoundSessionRefreshCookieFetcherImpl::GetChallengeIfBindingKeyAssertionRequired(
    const scoped_refptr<net::HttpResponseHeaders>& headers) const {
  if (!headers || headers->response_code() != net::HTTP_UNAUTHORIZED ||
      !headers->HasHeader(kRotationChallengeHeader)) {
    return std::nullopt;
  }

  std::string challenge;
  headers->GetNormalizedHeader(kRotationChallengeHeader, &challenge);
  return challenge;
}

void BoundSessionRefreshCookieFetcherImpl::HandleBindingKeyAssertionRequired(
    const std::string& challenge_header_value) {
  if (assertion_requests_count_ >= kMaxAssertionRequestsAllowed) {
    CompleteRequestAndReportRefreshResult(
        Result::kChallengeRequiredLimitExceeded);
    return;
  }

  std::string challenge = ParseChallengeHeader(challenge_header_value);
  if (challenge.empty()) {
    CompleteRequestAndReportRefreshResult(
        Result::kChallengeRequiredUnexpectedFormat);
    return;
  }

  // Binding key assertion required.
  assertion_requests_count_++;
  RefreshWithChallenge(challenge);
}

// static
std::string BoundSessionRefreshCookieFetcherImpl::ParseChallengeHeader(
    const std::string& header) {
  base::StringPairs items;
  base::SplitStringIntoKeyValuePairs(header, '=', ';', &items);
  std::string challenge;
  for (const auto& [key, value] : items) {
    // TODO(b/293838716): Check `session_id` matches the current session's id.
    if (base::EqualsCaseInsensitiveASCII(key, kChallengeItemKey)) {
      challenge = value;
    }
  }

  if (!base::IsStringUTF8AllowingNoncharacters(challenge)) {
    DVLOG(1) << "Server-side challenge has non-UTF8 characters.";
    return std::string();
  }
  return challenge;
}

void BoundSessionRefreshCookieFetcherImpl::
    CompleteRequestAndReportRefreshResult(Result result) {
  cookie_refresh_completed_ = true;
  result_ = result;
  ReportRefreshResult();
}

void BoundSessionRefreshCookieFetcherImpl::RefreshWithChallenge(
    const std::string& challenge) {
  TRACE_EVENT("browser",
              "BoundSessionRefreshCookieFetcherImpl::RefreshWithChallenge",
              perfetto::Flow::FromPointer(this));
  session_binding_helper_->GenerateBindingKeyAssertion(
      challenge, GetRefreshUrl(),
      base::BindOnce(
          &BoundSessionRefreshCookieFetcherImpl::OnGenerateBindingKeyAssertion,
          weak_ptr_factory_.GetWeakPtr(), base::ElapsedTimer()));
}

void BoundSessionRefreshCookieFetcherImpl::OnGenerateBindingKeyAssertion(
    base::ElapsedTimer generate_assertion_timer,
    std::string assertion) {
  base::UmaHistogramMediumTimes(
      "Signin.BoundSessionCredentials.CookieRotationGenerateAssertionDuration",
      generate_assertion_timer.Elapsed());
  TRACE_EVENT(
      "browser",
      "BoundSessionRefreshCookieFetcherImpl::OnGenerateBindingKeyAssertion",
      perfetto::Flow::FromPointer(this), "success", !assertion.empty());

  if (assertion.empty()) {
    CompleteRequestAndReportRefreshResult(Result::kSignChallengeFailed);
    return;
  }

  StartRefreshRequest(std::move(assertion));
}

void BoundSessionRefreshCookieFetcherImpl::OnCookiesAccessed(
    std::vector<network::mojom::CookieAccessDetailsPtr> details_vector) {
  // TODO(b/296999000): record a trace event.
  for (const auto& cookie_details : details_vector) {
    if (cookie_details->type !=
        network::mojom::CookieAccessDetails::Type::kChange) {
      continue;
    }

    reported_cookies_notified_ = true;
    bool all_cookies_set = true;
    for (const std::string& expected_cookie_name : expected_cookie_names_) {
      auto it = base::ranges::find_if(
          cookie_details->cookie_list,
          [this, &expected_cookie_name](
              const network::mojom::CookieOrLineWithAccessResultPtr& cookie) {
            return IsExpectedCookie(expected_cookie_domain_,
                                    expected_cookie_name, cookie);
          });
      if (it == cookie_details->cookie_list.end()) {
        all_cookies_set = false;
        break;
      }
    }
    expected_cookies_set_ = expected_cookies_set_ || all_cookies_set;
  }

  if (cookie_refresh_completed_ && reported_cookies_notified_) {
    ReportRefreshResult();
  }
}

void BoundSessionRefreshCookieFetcherImpl::Clone(
    mojo::PendingReceiver<network::mojom::CookieAccessObserver> observer) {
  cookie_observers_.Add(this, std::move(observer));
}

const GURL& BoundSessionRefreshCookieFetcherImpl::GetRefreshUrl() {
  return !refresh_url_.is_empty()
             ? refresh_url_
             : GaiaUrls::GetInstance()->rotate_bound_cookies_url();
}
