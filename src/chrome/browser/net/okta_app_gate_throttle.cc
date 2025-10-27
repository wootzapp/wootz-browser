// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/okta_app_gate_throttle.h"

#include <memory>
#include <vector>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "chrome/browser/net/okta_certificate_manager.h"
#include "chrome/browser/net/okta_gate_util.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "net/base/net_errors.h"
#include "net/http/http_request_headers.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "net/url_request/redirect_info.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "url/gurl.h"

OktaAppGateThrottle::OktaAppGateThrottle(content::BrowserContext* context)
    : context_(context),
      certificate_manager_(std::make_unique<OktaCertificateManager>(context)) {}

OktaAppGateThrottle::~OktaAppGateThrottle() = default;

void OktaAppGateThrottle::WillStartRequest(network::ResourceRequest* request,
                                           bool* defer) {
  // NEW FLOW: Handle eb.wootzapp.com visits - certificate stored via extension API
  if (IsFacebookUrl(request->url)) {
    LOG(INFO) << "Aaditesh_mtls -> eb.wootzapp.com URL detected: "
              << request->url;
    LOG(INFO) << "Aaditesh_mtls -> Certificate provisioning via extension API (chrome.wootz.mtlsCert)";
    
    // Check if we already have a valid certificate
    if (certificate_manager_->HasValidStoredCertificate()) {
      LOG(INFO) << "Aaditesh_mtls -> Valid certificate already exists, "
                   "allowing eb.wootzapp.com access";
    } else {
      LOG(INFO) << "Aaditesh_mtls -> No certificate yet - extension should call chrome.wootz.mtlsCert";
    }
    
    // Always allow eb.wootzapp.com access (certificate comes from extension API)
    return;
  }

  // NEW FLOW: Handle Okta URLs - check for valid certificate
  if (IsOktaAppUrl(request->url)) {
    LOG(INFO) << "Aaditesh_mtls -> Okta URL detected: " << request->url;

    // Check if we have a valid certificate
    if (certificate_manager_->HasValidStoredCertificate()) {
      LOG(INFO)
          << "Aaditesh_mtls -> Valid certificate found, allowing Okta access";
      return;  // Allow the request to proceed
    }

    LOG(INFO) << "Aaditesh_mtls -> No valid certificate found - ACCESS DENIED";
    LOG(INFO) << "Aaditesh_mtls -> User must visit eb.wootzapp.com first to "
                 "get certificate";

    // Block access and show error page
    ShowAccessDeniedPage();
    return;
  }

  // For all other URLs, allow access without any checks
}

void OktaAppGateThrottle::WillRedirectRequest(
    net::RedirectInfo* redirect_info,
    const network::mojom::URLResponseHead& response_head,
    bool* defer,
    std::vector<std::string>* to_be_removed_request_headers,
    net::HttpRequestHeaders* modified_request_headers,
    net::HttpRequestHeaders* modified_cors_exempt_request_headers) {
  // NEW FLOW: Handle eb.wootzapp.com redirects - certificate stored via extension API
  if (IsFacebookUrl(redirect_info->new_url)) {
    LOG(INFO) << "Aaditesh_mtls -> eb.wootzapp.com redirect detected: "
              << redirect_info->new_url;
    
    // Check if we already have a valid certificate
    if (certificate_manager_->HasValidStoredCertificate()) {
      LOG(INFO) << "Aaditesh_mtls -> Valid certificate already exists, "
                   "allowing eb.wootzapp.com redirect";
    } else {
      LOG(INFO) << "Aaditesh_mtls -> No certificate yet - extension should call chrome.wootz.mtlsCert";
    }
    
    // Always allow eb.wootzapp.com redirect (certificate comes from extension API)
    return;
  }

  // NEW FLOW: Handle Okta redirects - check for valid certificate
  if (IsOktaAppUrl(redirect_info->new_url)) {
    LOG(INFO) << "Aaditesh_mtls -> Okta redirect detected: "
              << redirect_info->new_url;

    // Check if we have a valid certificate
    if (certificate_manager_->HasValidStoredCertificate()) {
      LOG(INFO)
          << "Aaditesh_mtls -> Valid certificate found, allowing Okta redirect";
      return;  // Allow the redirect to proceed
    }

    LOG(INFO) << "Aaditesh_mtls -> No valid certificate found for redirect - "
                 "ACCESS DENIED";
    LOG(INFO) << "Aaditesh_mtls -> User must visit eb.wootzapp.com first to "
                 "get certificate";

    // Block redirect and show error page
    ShowAccessDeniedPage();
    return;
  }

  // For all other redirects, allow without any checks
}

void OktaAppGateThrottle::ShowAccessDeniedPage() {
  // Log the access denied event with detailed information
 

  // Use a custom error reason that will be visible in the browser's error page
  const std::string custom_error_message ="";

  // Cancel the request with a detailed custom reason
  delegate_->CancelWithError(net::ERR_ACCESS_DENIED, custom_error_message);
}
