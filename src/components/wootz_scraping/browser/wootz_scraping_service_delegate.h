/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_SERVICE_DELEGATE_H_
#define COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_SERVICE_DELEGATE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "net/cookies/canonical_cookie.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace content {
class WebContents;
}

class WootzScrapingServiceDelegate {
 public:
  virtual ~WootzScrapingServiceDelegate() = default;

  virtual content::WebContents* GetActiveWebContents() = 0;
  virtual scoped_refptr<network::SharedURLLoaderFactory> GetURLLoaderFactory() = 0;
  virtual void GetTwitterCookies(
      base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)> callback) = 0;
  virtual void HasTwitterCookies(base::OnceCallback<void(bool)> callback) = 0;
  virtual void GetHttpOnlyCookies(
      base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)> callback) = 0;
  virtual void SaveToStorage(const std::string& key,
                           const std::string& value,
                           base::OnceCallback<void(bool)> callback) = 0;
  virtual void LoadFromStorage(
      const std::string& key,
      base::OnceCallback<void(const std::string&)> callback) = 0;
};

#endif  // COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_SERVICE_DELEGATE_H_
