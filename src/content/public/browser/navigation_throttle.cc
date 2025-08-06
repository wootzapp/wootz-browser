// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/navigation_throttle.h"

#include "base/functional/bind.h"
#include "base/task/single_thread_task_runner.h"
#include "content/browser/renderer_host/navigation_request.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_errors.h"
#include "ui/base/page_transition_types.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "content/public/browser/domain_block_checker.h"
#include "content/public/browser/blocked_domains_prefs.h"
#include "url/gurl.h"

namespace content {

namespace {

net::Error DefaultNetErrorCode(NavigationThrottle::ThrottleAction action) {
  switch (action) {
    case NavigationThrottle::PROCEED:
    case NavigationThrottle::DEFER:
      return net::OK;
    case NavigationThrottle::CANCEL:
    case NavigationThrottle::CANCEL_AND_IGNORE:
      return net::ERR_ABORTED;
    case NavigationThrottle::BLOCK_REQUEST:
    case NavigationThrottle::BLOCK_REQUEST_AND_COLLAPSE:
      return net::ERR_BLOCKED_BY_CLIENT;
    case NavigationThrottle::BLOCK_RESPONSE:
      return net::ERR_BLOCKED_BY_RESPONSE;
    default:
      NOTREACHED_IN_MIGRATION();
      return net::ERR_UNEXPECTED;
  }
}

}  // namespace

NavigationThrottle::ThrottleCheckResult::ThrottleCheckResult(
    NavigationThrottle::ThrottleAction action)
    : NavigationThrottle::ThrottleCheckResult(action,
                                              DefaultNetErrorCode(action),
                                              std::nullopt) {}

NavigationThrottle::ThrottleCheckResult::ThrottleCheckResult(
    NavigationThrottle::ThrottleAction action,
    net::Error net_error_code)
    : NavigationThrottle::ThrottleCheckResult(action,
                                              net_error_code,
                                              std::nullopt) {}

NavigationThrottle::ThrottleCheckResult::ThrottleCheckResult(
    NavigationThrottle::ThrottleAction action,
    net::Error net_error_code,
    std::optional<std::string> error_page_content)
    : action_(action),
      net_error_code_(net_error_code),
      error_page_content_(error_page_content) {}

NavigationThrottle::ThrottleCheckResult::ThrottleCheckResult(
    const ThrottleCheckResult& other) = default;

NavigationThrottle::ThrottleCheckResult::~ThrottleCheckResult() {}

NavigationThrottle::NavigationThrottle(NavigationHandle* navigation_handle)
    : navigation_handle_(navigation_handle) {}

NavigationThrottle::~NavigationThrottle() {}

NavigationThrottle::ThrottleCheckResult NavigationThrottle::WillStartRequest() {
  const GURL& url = navigation_handle()->GetURL();

  if (DomainBlockChecker::GetInstance().IsUrlBlocked(url, navigation_handle())) {
    VLOG(1) << "[DomainBlocker] BLOCKED: " << url.host();
    
    return NavigationThrottle::ThrottleCheckResult(
        NavigationThrottle::BLOCK_REQUEST,
        net::ERR_BLOCKED_BY_CLIENT,
        blocked_domains::prefs::GetBlockedDomainErrorPage());
  }

  // Check if the URL is chromewebstore.google.com or chrome.google.com/webstore
  if (url.host() == "chromewebstore.google.com" ||
      (url.host() == "chrome.google.com" && 
       url.path().find("/webstore") == 0)) {
    // Get the WebContents from the navigation handle
    WebContents* web_contents = navigation_handle()->GetWebContents();
    if (!web_contents) {
      // If WebContents is null, just proceed with the original navigation
      return NavigationThrottle::PROCEED;
    }

    // Create the new URL
    GURL redirect_url("wootzapp://flow-store");

    // Use base::PostTask to schedule the navigation after the current task completes
    // This helps avoid race conditions
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE,
        base::BindOnce([](WebContents* contents, const GURL& url) {
          if (contents && !contents->IsBeingDestroyed()) {
            contents->GetController().LoadURL(
                url, 
                content::Referrer(), 
                ui::PAGE_TRANSITION_CLIENT_REDIRECT, 
                std::string());
          }
        }, web_contents, redirect_url));

    // Cancel the current navigation
    return NavigationThrottle::CANCEL;
  }

  return NavigationThrottle::PROCEED;
}

NavigationThrottle::ThrottleCheckResult
NavigationThrottle::WillRedirectRequest() {
  return NavigationThrottle::PROCEED;
}

NavigationThrottle::ThrottleCheckResult NavigationThrottle::WillFailRequest() {
  return NavigationThrottle::PROCEED;
}

NavigationThrottle::ThrottleCheckResult
NavigationThrottle::WillProcessResponse() {
  return NavigationThrottle::PROCEED;
}

NavigationThrottle::ThrottleCheckResult
NavigationThrottle::WillCommitWithoutUrlLoader() {
  return NavigationThrottle::PROCEED;
}

void NavigationThrottle::Resume() {
  if (resume_callback_) {
    resume_callback_.Run();
    return;
  }
  NavigationRequest::From(navigation_handle_)->Resume(this);
}

void NavigationThrottle::CancelDeferredNavigation(
    NavigationThrottle::ThrottleCheckResult result) {
  if (cancel_deferred_navigation_callback_) {
    cancel_deferred_navigation_callback_.Run(result);
    return;
  }
  NavigationRequest::From(navigation_handle_)
      ->CancelDeferredNavigation(this, result);
}

}  // namespace content
