/* Copyright (c) 2021 The Dark Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/renderer/wootz_wallet_render_frame_observer.h"

#include <memory>
#include <optional>
#include <utility>

#include "base/logging.h"
#include "components/wootz_wallet/renderer/v8_helper.h"
// #include "components/wootz_wallet/renderer/interceptor.h"  // File not found
#include "build/buildflag.h"
#include "content/public/common/isolated_world_ids.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/platform/scheduler/web_agent_group_scheduler.h"

namespace wootz_wallet {

WootzWalletRenderFrameObserver::WootzWalletRenderFrameObserver(
    content::RenderFrame* render_frame,
    GetDynamicParamsCallback get_dynamic_params_callback)
    : RenderFrameObserver(render_frame),
      get_dynamic_params_callback_(std::move(get_dynamic_params_callback)) {}

WootzWalletRenderFrameObserver::~WootzWalletRenderFrameObserver() = default;

void WootzWalletRenderFrameObserver::DidStartNavigation(
    const GURL& url,
    std::optional<blink::WebNavigationType> navigation_type) {
  url_ = url;
  
  LOG(INFO) << "Aaditesh -> DidStartNavigation called for URL: " << url.spec();
  LOG(INFO) << "Aaditesh -> URL host: " << url.host();
  LOG(INFO) << "Aaditesh -> URL scheme: " << url.scheme();
  
  // Check if this is a Twitter URL and create interceptor
  if (url.host() == "twitter.com" || 
      url.host() == "x.com" || 
      url.host() == "www.twitter.com" || 
      url.host() == "www.x.com" ||
      url.host() == "mobile.twitter.com") {
    LOG(INFO) << "Aaditesh -> ✅ TWITTER URL DETECTED! TwitterInterceptor disabled (not available): " << url.spec();
    // TwitterInterceptor::CreateForFrame(render_frame());  // Disabled - file not found
    LOG(INFO) << "Aaditesh -> TwitterInterceptor creation skipped";
  } else {
    LOG(INFO) << "Aaditesh -> Not a Twitter URL, skipping interceptor creation";
  }
}

bool WootzWalletRenderFrameObserver::IsPageValid() {
  // There could be empty, invalid and "about:blank" URLs,
  // they should fallback to the main frame rules
  if (url_.is_empty() || !url_.is_valid() || url_.spec() == "about:blank") {
    url_ = url::Origin(render_frame()->GetWebFrame()->GetSecurityOrigin())
               .GetURL();
  }
    return url_.SchemeIsHTTPOrHTTPS() || url_.SchemeIs("chrome-extension");
}

bool WootzWalletRenderFrameObserver::CanCreateProvider() {
  // if (!IsPageValid()) {
  //   return false;
  // }

  // if (url_.SchemeIs("chrome-extension")) {
  //   return true;
  // }

  // // Wallet provider objects should only be created in secure contexts
  // if (!render_frame()->GetWebFrame()->GetDocument().IsSecureContext()) {
  //   return false;
  // }

  return true;
}

void WootzWalletRenderFrameObserver::DidFinishLoad() {
  LOG(INFO) << "Aaditesh -> DidFinishLoad called for URL: " << url_.spec();
  
#if !BUILDFLAG(IS_ANDROID)
  // Only record P3A for desktop and valid HTTP/HTTPS pages
  if (!IsPageValid()) {
    return;
  }

  auto dynamic_params = get_dynamic_params_callback_.Run();

  p3a_util_.ReportJSProviders(render_frame(), dynamic_params);
#endif

  // Check if this is a Twitter URL and ensure interceptor is created
  if (url_.host() == "twitter.com" || 
      url_.host() == "x.com" || 
      url_.host() == "www.twitter.com" || 
      url_.host() == "www.x.com" ||
      url_.host() == "mobile.twitter.com") {
    LOG(INFO) << "Aaditesh -> ✅ TWITTER URL in DidFinishLoad! TwitterInterceptor disabled: " << url_.spec();
    // TwitterInterceptor::CreateForFrame(render_frame());  // Disabled - file not found
    LOG(INFO) << "Aaditesh -> TwitterInterceptor creation/verification skipped in DidFinishLoad";
  }
}

void WootzWalletRenderFrameObserver::DidClearWindowObject() {
  // Twitter interception is now handled by TwitterInterceptor
  // No need to create Ethereum/Solana providers
  LOG(INFO) << "Aaditesh -> DidClearWindowObject called - Twitter interception handled by TwitterInterceptor";
}

void WootzWalletRenderFrameObserver::OnDestruct() {
  delete this;
}

}  // namespace wootz_wallet
