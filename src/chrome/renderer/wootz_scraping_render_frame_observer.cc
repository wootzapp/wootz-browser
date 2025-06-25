/* Copyright (c) 2024 All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/renderer/wootz_scraping_render_frame_observer.h"

#include <memory>
#include <optional>
#include <utility>

#include "base/logging.h"
#include "components/wootz_scraping/renderer/interceptor.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_navigation_type.h"
#include "url/gurl.h"

namespace wootz_scraping {

WootzScrapingRenderFrameObserver::WootzScrapingRenderFrameObserver(
    content::RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) {}

WootzScrapingRenderFrameObserver::~WootzScrapingRenderFrameObserver() = default;

bool WootzScrapingRenderFrameObserver::IsTwitterUrl(const GURL& url) {
  return url.host() == "twitter.com" || 
         url.host() == "x.com" || 
         url.host() == "www.twitter.com" || 
         url.host() == "www.x.com" ||
         url.host() == "mobile.twitter.com";
}

void WootzScrapingRenderFrameObserver::CreateTwitterInterceptorIfNeeded() {
  if (!interceptor_created_ && IsTwitterUrl(url_)) {
    LOG(INFO) << "Aaditesh -> ✅ TWITTER URL DETECTED! Creating TwitterInterceptor: " << url_.spec();
    TwitterInterceptor::CreateForFrame(render_frame());
    interceptor_created_ = true;
    LOG(INFO) << "Aaditesh -> TwitterInterceptor creation completed";
  }
}

void WootzScrapingRenderFrameObserver::DidStartNavigation(
    const GURL& url,
    std::optional<blink::WebNavigationType> navigation_type) {
  url_ = url;
  interceptor_created_ = false; // Reset for new navigation
  
  LOG(INFO) << "Aaditesh -> WootzScraping DidStartNavigation called for URL: " << url.spec();
  LOG(INFO) << "Aaditesh -> URL host: " << url.host();
  LOG(INFO) << "Aaditesh -> URL scheme: " << url.scheme();
  
  CreateTwitterInterceptorIfNeeded();
}

void WootzScrapingRenderFrameObserver::DidFinishLoad() {
  LOG(INFO) << "Aaditesh -> WootzScraping DidFinishLoad called for URL: " << url_.spec();
  CreateTwitterInterceptorIfNeeded();
}

void WootzScrapingRenderFrameObserver::DidClearWindowObject() {
  LOG(INFO) << "Aaditesh -> WootzScraping DidClearWindowObject called for URL: " << url_.spec();
  CreateTwitterInterceptorIfNeeded();
}

void WootzScrapingRenderFrameObserver::OnDestruct() {
  delete this;
}

}  // namespace wootz_scraping
