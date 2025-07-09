/* Copyright (c) 2024 All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_scraping/renderer/wootz_scraping_render_frame_observer.h"

#include <memory>
#include <optional>
#include <utility>

#include "base/logging.h"
#include "components/wootz_scraping/renderer/interceptor.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"

namespace wootz_scraping {

WootzScrapingRenderFrameObserver::WootzScrapingRenderFrameObserver(
    content::RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) {}

WootzScrapingRenderFrameObserver::~WootzScrapingRenderFrameObserver() = default;

void WootzScrapingRenderFrameObserver::DidStartNavigation(
    const GURL& url,
    std::optional<blink::WebNavigationType> navigation_type) {
  url_ = url;
  
  LOG(INFO) << "WootzScraping -> DidStartNavigation called for URL: " << url.spec();
  LOG(INFO) << "WootzScraping -> URL host: " << url.host();
  LOG(INFO) << "WootzScraping -> URL scheme: " << url.scheme();
  
  // Check if this is a Twitter URL and create interceptor
  if (IsTwitterUrl(url)) {
    LOG(INFO) << "WootzScraping -> ✅ TWITTER URL DETECTED! Creating TwitterInterceptor: " << url.spec();
    CreateTwitterInterceptorIfNeeded();
    LOG(INFO) << "WootzScraping -> TwitterInterceptor creation completed in DidStartNavigation";
  } else {
    LOG(INFO) << "WootzScraping -> Not a Twitter URL, skipping interceptor creation";
  }
}

void WootzScrapingRenderFrameObserver::DidFinishLoad() {
  LOG(INFO) << "WootzScraping -> DidFinishLoad called for URL: " << url_.spec();
  
  // Check if this is a Twitter URL and ensure interceptor is created
  if (IsTwitterUrl(url_)) {
    LOG(INFO) << "WootzScraping -> ✅ TWITTER URL in DidFinishLoad! TwitterInterceptor already created in DidStartNavigation: " << url_.spec();
    // Don't create again - already created in DidStartNavigation to avoid multiple instances
    LOG(INFO) << "WootzScraping -> TwitterInterceptor already exists - skipping creation in DidFinishLoad";
  }
}

void WootzScrapingRenderFrameObserver::DidClearWindowObject() {
  LOG(INFO) << "WootzScraping -> DidClearWindowObject called - Twitter interception handled by TwitterInterceptor";
  
  // Ensure TwitterInterceptor is active for Twitter URLs
  if (IsTwitterUrl(url_)) {
    LOG(INFO) << "WootzScraping -> ✅ TWITTER URL in DidClearWindowObject! TwitterInterceptor already created in DidStartNavigation: " << url_.spec();
    // Don't create again - already created in DidStartNavigation to avoid multiple instances
    LOG(INFO) << "WootzScraping -> TwitterInterceptor already exists - skipping creation in DidClearWindowObject";
  }
}

void WootzScrapingRenderFrameObserver::OnDestruct() {
  delete this;
}

bool WootzScrapingRenderFrameObserver::IsTwitterUrl(const GURL& url) const {
  return url.host() == "twitter.com" || 
         url.host() == "x.com" || 
         url.host() == "www.twitter.com" || 
         url.host() == "www.x.com" ||
         url.host() == "mobile.twitter.com";
}

void WootzScrapingRenderFrameObserver::CreateTwitterInterceptorIfNeeded() {
  // Create TwitterInterceptor once per navigation to avoid multiple instances
  TwitterInterceptor::CreateForFrame(render_frame());
}

}  // namespace wootz_scraping
