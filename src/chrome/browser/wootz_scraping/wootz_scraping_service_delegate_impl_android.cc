/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_scraping/wootz_scraping_service_delegate_impl_android.h"

#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_options.h"
#include "net/cookies/cookie_partition_key_collection.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "url/gurl.h"

WootzScrapingServiceDelegateImpl::WootzScrapingServiceDelegateImpl(
    content::BrowserContext* context)
    : context_(context) {
  DCHECK(context_);
}

WootzScrapingServiceDelegateImpl::~WootzScrapingServiceDelegateImpl() = default;

content::WebContents* WootzScrapingServiceDelegateImpl::GetActiveWebContents() {
  // Get the active tab from TabModelList for Android
  TabModel* tab_model = TabModelList::GetTabModelForWebContents(nullptr);
  if (!tab_model) {
    // Try to get any available tab model
    const TabModelList::TabModelVector& models = TabModelList::models();
    if (!models.empty()) {
      tab_model = models[0].get();
    }
  }

  if (!tab_model) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: No tab model found";
    return nullptr;
  }

  content::WebContents* web_contents = tab_model->GetActiveWebContents();
  if (!web_contents) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: No active web contents found";
    return nullptr;
  }

  LOG(INFO) << "WootzScrapingServiceDelegateImpl: Successfully retrieved active WebContents";
  return web_contents;
}

scoped_refptr<network::SharedURLLoaderFactory> 
WootzScrapingServiceDelegateImpl::GetURLLoaderFactory() {
  if (!context_) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: No browser context available";
    return nullptr;
  }

  return context_->GetDefaultStoragePartition()
      ->GetURLLoaderFactoryForBrowserProcess();
}

void WootzScrapingServiceDelegateImpl::GetTwitterCookies(
    base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)> callback) {
  
  LOG(INFO) << "WootzScrapingServiceDelegateImpl: GetTwitterCookies called";

  if (!context_) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: No browser context for cookie retrieval";
    std::move(callback).Run({});
    return;
  }

  auto* storage_partition = context_->GetDefaultStoragePartition();
  auto* cookie_manager = storage_partition->GetCookieManagerForBrowserProcess();

  // Get cookies for Twitter domains
  std::vector<GURL> twitter_urls = {
      GURL("https://twitter.com"),
      GURL("https://x.com"),
      GURL("https://api.twitter.com"),
      GURL("https://mobile.twitter.com")
  };

  auto cookies_result = std::make_shared<std::vector<net::CanonicalCookie>>();
  auto remaining_count = std::make_shared<int>(twitter_urls.size());
  auto shared_callback = std::make_shared<base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)>>(std::move(callback));

  // Request cookies for each Twitter domain
  for (const auto& url : twitter_urls) {
    net::CookieOptions options;
    options.set_include_httponly();
    options.set_same_site_cookie_context(
        net::CookieOptions::SameSiteCookieContext::MakeInclusive());
    
    cookie_manager->GetCookieList(
        url, options, net::CookiePartitionKeyCollection(),
        base::BindOnce(
            [](std::shared_ptr<base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)>> shared_callback,
               std::shared_ptr<std::vector<net::CanonicalCookie>> cookies_result,
               std::shared_ptr<int> remaining_count,
               const net::CookieAccessResultList& cookies,
               const net::CookieAccessResultList& excluded_cookies) {
              
              for (const auto& cookie_result : cookies) {
                if (cookie_result.access_result.status.IsInclude()) {
                  cookies_result->push_back(cookie_result.cookie);
                }
              }

              (*remaining_count)--;
              if (*remaining_count == 0) {
                LOG(INFO) << "WootzScrapingServiceDelegateImpl: Retrieved " 
                          << cookies_result->size() << " Twitter cookies";
                std::move(*shared_callback).Run(*cookies_result);
              }
            },
            shared_callback, cookies_result, remaining_count));
  }
}

void WootzScrapingServiceDelegateImpl::HasTwitterCookies(
    base::OnceCallback<void(bool)> callback) {
  
  LOG(INFO) << "WootzScrapingServiceDelegateImpl: HasTwitterCookies called";

  GetTwitterCookies(
      base::BindOnce([](base::OnceCallback<void(bool)> callback,
                        const std::vector<net::CanonicalCookie>& cookies) {
        bool has_cookies = !cookies.empty();
        LOG(INFO) << "WootzScrapingServiceDelegateImpl: Has Twitter cookies: " 
                  << (has_cookies ? "YES" : "NO");
        std::move(callback).Run(has_cookies);
      }, std::move(callback)));
}

void WootzScrapingServiceDelegateImpl::GetHttpOnlyCookies(
    base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)> callback) {
  
  LOG(INFO) << "WootzScrapingServiceDelegateImpl: GetHttpOnlyCookies called";

  if (!context_) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: No browser context for HttpOnly cookie retrieval";
    std::move(callback).Run({});
    return;
  }

  auto* storage_partition = context_->GetDefaultStoragePartition();
  auto* cookie_manager = storage_partition->GetCookieManagerForBrowserProcess();

  // Get HttpOnly cookies specifically for Twitter domains
  std::vector<GURL> twitter_urls = {
      GURL("https://twitter.com"),
      GURL("https://x.com"),
      GURL("https://api.twitter.com"),
      GURL("https://mobile.twitter.com")
  };

  auto httponly_cookies = std::make_shared<std::vector<net::CanonicalCookie>>();
  auto remaining_count = std::make_shared<int>(twitter_urls.size());
  auto shared_callback = std::make_shared<base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)>>(std::move(callback));

  // Request HttpOnly cookies for each Twitter domain
  for (const auto& url : twitter_urls) {
    net::CookieOptions options;
    options.set_include_httponly();  // This is key for HttpOnly cookies
    options.set_same_site_cookie_context(
        net::CookieOptions::SameSiteCookieContext::MakeInclusive());
    
    cookie_manager->GetCookieList(
        url, options, net::CookiePartitionKeyCollection(),
        base::BindOnce(
            [](std::shared_ptr<base::OnceCallback<void(const std::vector<net::CanonicalCookie>&)>> shared_callback,
               std::shared_ptr<std::vector<net::CanonicalCookie>> httponly_cookies,
               std::shared_ptr<int> remaining_count,
               const net::CookieAccessResultList& cookies,
               const net::CookieAccessResultList& excluded_cookies) {
              
              // Filter for HttpOnly cookies only
              for (const auto& cookie_result : cookies) {
                if (cookie_result.access_result.status.IsInclude() && 
                    cookie_result.cookie.IsHttpOnly()) {
                  httponly_cookies->push_back(cookie_result.cookie);
                }
              }

              (*remaining_count)--;
              if (*remaining_count == 0) {
                LOG(INFO) << "WootzScrapingServiceDelegateImpl: Retrieved " 
                          << httponly_cookies->size() << " HttpOnly Twitter cookies";
                std::move(*shared_callback).Run(*httponly_cookies);
              }
            },
            shared_callback, httponly_cookies, remaining_count));
  }
}

void WootzScrapingServiceDelegateImpl::SaveToStorage(
    const std::string& key,
    const std::string& value,
    base::OnceCallback<void(bool)> callback) {
  
  LOG(INFO) << "WootzScrapingServiceDelegateImpl: SaveToStorage called for key: " << key;

  if (!context_) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: No browser context for storage";
    std::move(callback).Run(false);
    return;
  }

  // For Android, we'll use Profile preferences
  Profile* profile = Profile::FromBrowserContext(context_);
  if (!profile) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: Failed to get profile";
    std::move(callback).Run(false);
    return;
  }

  // Store in preferences with a wootz_scraping prefix
  std::string pref_key = "wootz_scraping." + key;
  profile->GetPrefs()->SetString(pref_key, value);
  
  LOG(INFO) << "WootzScrapingServiceDelegateImpl: Successfully saved to storage";
  std::move(callback).Run(true);
}

void WootzScrapingServiceDelegateImpl::LoadFromStorage(
    const std::string& key,
    base::OnceCallback<void(const std::string&)> callback) {
  
  LOG(INFO) << "WootzScrapingServiceDelegateImpl: LoadFromStorage called for key: " << key;

  if (!context_) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: No browser context for storage";
    std::move(callback).Run("");
    return;
  }

  // For Android, we'll use Profile preferences
  Profile* profile = Profile::FromBrowserContext(context_);
  if (!profile) {
    LOG(ERROR) << "WootzScrapingServiceDelegateImpl: Failed to get profile";
    std::move(callback).Run("");
    return;
  }

  // Load from preferences with a wootz_scraping prefix
  std::string pref_key = "wootz_scraping." + key;
  std::string value = profile->GetPrefs()->GetString(pref_key);
  
  LOG(INFO) << "WootzScrapingServiceDelegateImpl: Successfully loaded from storage, value length: " << value.length();
  std::move(callback).Run(value);
}
