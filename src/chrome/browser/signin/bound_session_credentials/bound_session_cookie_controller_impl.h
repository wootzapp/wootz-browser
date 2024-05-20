// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SIGNIN_BOUND_SESSION_CREDENTIALS_BOUND_SESSION_COOKIE_CONTROLLER_IMPL_H_
#define CHROME_BROWSER_SIGNIN_BOUND_SESSION_CREDENTIALS_BOUND_SESSION_COOKIE_CONTROLLER_IMPL_H_

#include "chrome/browser/signin/bound_session_credentials/bound_session_cookie_controller.h"

#include <memory>

#include "base/functional/callback_forward.h"
#include "base/scoped_observation.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "chrome/browser/signin/bound_session_credentials/bound_session_refresh_cookie_fetcher.h"
#include "chrome/browser/signin/bound_session_credentials/rotation_debug_info.pb.h"
#include "content/public/browser/storage_partition.h"
#include "services/network/public/cpp/network_connection_tracker.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "url/gurl.h"

namespace unexportable_keys {
class UnexportableKeyService;
}  // namespace unexportable_keys

namespace content {
class StoragePartition;
}

class BoundSessionCookieObserver;
class SessionBindingHelper;

class BoundSessionCookieControllerImpl
    : public BoundSessionCookieController,
      public network::NetworkConnectionTracker::NetworkConnectionObserver {
 public:
  BoundSessionCookieControllerImpl(
      unexportable_keys::UnexportableKeyService& key_service,
      content::StoragePartition* storage_partition,
      network::NetworkConnectionTracker* network_connection_tracker,
      const bound_session_credentials::BoundSessionParams& bound_session_params,
      Delegate* delegate,
      bool is_off_the_record_profile);

  ~BoundSessionCookieControllerImpl() override;

  BoundSessionCookieControllerImpl(const BoundSessionCookieControllerImpl&) =
      delete;
  BoundSessionCookieControllerImpl& operator=(
      const BoundSessionCookieControllerImpl&) = delete;

  // BoundSessionCookieController:
  void Initialize() override;

  void HandleRequestBlockedOnCookie(
      chrome::mojom::BoundSessionRequestThrottledHandler::
          HandleRequestBlockedOnCookieCallback resume_blocked_request) override;

  // network::NetworkConnectionTracker::NetworkConnectionObserver:
  void OnConnectionChanged(network::mojom::ConnectionType type) override;

 private:
  friend class BoundSessionCookieControllerImplTest;

  // Used by tests to provide their own implementation of the
  // `BoundSessionRefreshCookieFetcher`.
  using RefreshCookieFetcherFactoryForTesting =
      base::RepeatingCallback<std::unique_ptr<BoundSessionRefreshCookieFetcher>(
          network::mojom::CookieManager* cookie_manager,
          const GURL& url,
          base::flat_set<std::string> cookie_names)>;

  std::unique_ptr<BoundSessionRefreshCookieFetcher> CreateRefreshCookieFetcher()
      const;
  void CreateBoundCookiesObservers();

  bool AreAllCookiesFresh();
  void MaybeRefreshCookie();
  void SetCookieExpirationTimeAndNotify(const std::string& cookie_name,
                                        base::Time expiration_time);
  void OnCookieRefreshFetched(BoundSessionRefreshCookieFetcher::Result result);
  void MaybeScheduleCookieRotation();
  void ResumeBlockedRequests(
      chrome::mojom::ResumeBlockedRequestsTrigger trigger);
  void OnResumeBlockedRequestsTimeout();

  // Added for manual testing, should be inlined after
  // `artificial_cookie_rotation_delay_` is removed.
  void StartCookieRefresh();

  void set_refresh_cookie_fetcher_factory_for_testing(
      RefreshCookieFetcherFactoryForTesting
          refresh_cookie_fetcher_factory_for_testing) {
    refresh_cookie_fetcher_factory_for_testing_ =
        refresh_cookie_fetcher_factory_for_testing;
  }

  const raw_ref<unexportable_keys::UnexportableKeyService> key_service_;
  const raw_ptr<content::StoragePartition> storage_partition_;
  const raw_ptr<network::NetworkConnectionTracker> network_connection_tracker_;
  const bool is_off_the_record_profile_;

  std::vector<std::unique_ptr<BoundSessionCookieObserver>>
      bound_cookies_observers_;

  base::ScopedObservation<
      network::NetworkConnectionTracker,
      network::NetworkConnectionTracker::NetworkConnectionObserver>
      network_connection_observer_{this};
  // Also `true` while the initial connection state is unknown.
  bool is_offline_ = true;

  std::unique_ptr<SessionBindingHelper> session_binding_helper_;
  std::unique_ptr<BoundSessionRefreshCookieFetcher> refresh_cookie_fetcher_;

  std::vector<chrome::mojom::BoundSessionRequestThrottledHandler::
                  HandleRequestBlockedOnCookieCallback>
      resume_blocked_requests_;
  // Used to schedule preemptive cookie refresh.
  base::OneShotTimer preemptive_cookie_refresh_timer_;
  // Used to release blocked requests after a timeout.
  base::OneShotTimer resume_blocked_requests_timer_;
  size_t successive_timeout_ = 0;

  RefreshCookieFetcherFactoryForTesting
      refresh_cookie_fetcher_factory_for_testing_;

  bound_session_credentials::RotationDebugInfo debug_info_;

  // Added for testing purposes.
  std::unique_ptr<base::RetainingOneShotTimer> artifical_cookie_rotation_delay_;
  std::optional<BoundSessionRefreshCookieFetcher::Result>
      artificial_cookie_rotation_result_;
};

#endif  // CHROME_BROWSER_SIGNIN_BOUND_SESSION_CREDENTIALS_BOUND_SESSION_COOKIE_CONTROLLER_IMPL_H_
