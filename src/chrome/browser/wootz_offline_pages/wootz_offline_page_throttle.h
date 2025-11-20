// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_THROTTLE_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_THROTTLE_H_

#include <memory>

#include "content/public/browser/navigation_throttle.h"
#include "url/gurl.h"

namespace content {
class NavigationHandle;
}

class WootzOfflinePageThrottle : public content::NavigationThrottle {
 public:
  static std::unique_ptr<WootzOfflinePageThrottle> MaybeCreateThrottleFor(
      content::NavigationHandle* handle);

  explicit WootzOfflinePageThrottle(content::NavigationHandle* handle);
  ~WootzOfflinePageThrottle() override;

  // content::NavigationThrottle:
  ThrottleCheckResult WillStartRequest() override;
  ThrottleCheckResult WillRedirectRequest() override;
  const char* GetNameForLogging() override;
  
 private:
  ThrottleCheckResult CheckForOfflinePage();
};

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_THROTTLE_H_
