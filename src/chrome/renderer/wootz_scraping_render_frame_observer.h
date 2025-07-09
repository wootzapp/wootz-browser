/* Copyright (c) 2024 All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_RENDERER_WOOTZ_SCRAPING_RENDER_FRAME_OBSERVER_NEW_H_
#define CHROME_RENDERER_WOOTZ_SCRAPING_RENDER_FRAME_OBSERVER_NEW_H_

#include <optional>

#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "third_party/blink/public/web/web_navigation_type.h"
#include "url/gurl.h"

namespace wootz_scraping {

class WootzScrapingRenderFrameObserver : public content::RenderFrameObserver {
 public:
  explicit WootzScrapingRenderFrameObserver(content::RenderFrame* render_frame);
  ~WootzScrapingRenderFrameObserver() override;

  WootzScrapingRenderFrameObserver(const WootzScrapingRenderFrameObserver&) = delete;
  WootzScrapingRenderFrameObserver& operator=(const WootzScrapingRenderFrameObserver&) = delete;

  // RenderFrameObserver implementation.
  void DidStartNavigation(
      const GURL& url,
      std::optional<blink::WebNavigationType> navigation_type) override;
  void DidFinishLoad() override;
  void DidClearWindowObject() override;
  void OnDestruct() override;

 private:
  bool IsTwitterUrl(const GURL& url);
  void CreateTwitterInterceptorIfNeeded();

  GURL url_;
  bool interceptor_created_ = false;
};

}  // namespace wootz_scraping

#endif  // CHROME_RENDERER_WOOTZ_SCRAPING_RENDER_FRAME_OBSERVER_NEW_H_
