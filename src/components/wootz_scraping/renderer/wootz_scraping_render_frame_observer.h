/* Copyright (c) 2024 All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_SCRAPING_RENDERER_WOOTZ_SCRAPING_RENDER_FRAME_OBSERVER_H_
#define COMPONENTS_WOOTZ_SCRAPING_RENDERER_WOOTZ_SCRAPING_RENDER_FRAME_OBSERVER_H_

#include <memory>
#include <optional>

#include "base/memory/weak_ptr.h"
#include "content/public/renderer/render_frame_observer.h"
#include "url/gurl.h"

namespace content {
class RenderFrame;
}

namespace wootz_scraping {

class TwitterInterceptor;

class WootzScrapingRenderFrameObserver : public content::RenderFrameObserver {
 public:
  explicit WootzScrapingRenderFrameObserver(content::RenderFrame* render_frame);
  
  WootzScrapingRenderFrameObserver(const WootzScrapingRenderFrameObserver&) = delete;
  WootzScrapingRenderFrameObserver& operator=(const WootzScrapingRenderFrameObserver&) = delete;
  
  ~WootzScrapingRenderFrameObserver() override;

  // content::RenderFrameObserver:
  void DidStartNavigation(const GURL& url,
                         std::optional<blink::WebNavigationType> navigation_type) override;
  void DidFinishLoad() override;
  void DidClearWindowObject() override;
  void OnDestruct() override;

 private:
  bool IsTwitterUrl(const GURL& url) const;
  void CreateTwitterInterceptorIfNeeded();

  GURL url_;
  
  base::WeakPtrFactory<WootzScrapingRenderFrameObserver> weak_ptr_factory_{this};
};

}  // namespace wootz_scraping

#endif  // COMPONENTS_WOOTZ_SCRAPING_RENDERER_WOOTZ_SCRAPING_RENDER_FRAME_OBSERVER_H_
