/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_SCRAPING_WOOTZ_SCRAPING_SERVICE_DELEGATE_IMPL_ANDROID_H_
#define CHROME_BROWSER_WOOTZ_SCRAPING_WOOTZ_SCRAPING_SERVICE_DELEGATE_IMPL_ANDROID_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/wootz_scraping/browser/wootz_scraping_service_delegate.h"

namespace content {
class BrowserContext;
class WebContents;
}

namespace net {
class CanonicalCookie;
}

class WootzScrapingServiceDelegateImpl : public WootzScrapingServiceDelegate {
 public:
  explicit WootzScrapingServiceDelegateImpl(content::BrowserContext* context);
  WootzScrapingServiceDelegateImpl(const WootzScrapingServiceDelegateImpl&) = delete;
  WootzScrapingServiceDelegateImpl& operator=(const WootzScrapingServiceDelegateImpl&) = delete;
  ~WootzScrapingServiceDelegateImpl() override;

  // WootzScrapingServiceDelegate implementation
  content::WebContents* GetActiveWebContents() override;
  scoped_refptr<network::SharedURLLoaderFactory> GetURLLoaderFactory() override;
  void GetTwitterCookies(
      base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)> callback) override;
  void HasTwitterCookies(base::OnceCallback<void(bool)> callback) override;
  void GetHttpOnlyCookies(
      base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)> callback) override;
  void SaveToStorage(const std::string& key,
                     const std::string& value,
                     base::OnceCallback<void(bool)> callback) override;
  void LoadFromStorage(
      const std::string& key,
      base::OnceCallback<void(const std::string&)> callback) override;

 private:
  raw_ptr<content::BrowserContext> context_;
  base::WeakPtrFactory<WootzScrapingServiceDelegateImpl> weak_ptr_factory_{this};
};

#endif  // CHROME_BROWSER_WOOTZ_SCRAPING_WOOTZ_SCRAPING_SERVICE_DELEGATE_IMPL_ANDROID_H_
