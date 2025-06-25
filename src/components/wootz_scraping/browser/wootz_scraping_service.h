/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_SERVICE_H_
#define COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_SERVICE_H_

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/containers/circular_deque.h"
#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/types/expected.h"
#include "base/values.h"
#include "base/time/time.h"
#include "components/api_request_helper/api_request_helper.h"
#include "components/wootz_scraping/browser/wootz_scraping_service_delegate.h"
#include "components/wootz_scraping/common/wootz_scraping.mojom.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "url/origin.h"

// Forward declarations
namespace content {
class WebContents;
}

namespace net {
class CanonicalCookie;
}

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
}

class PrefService;

namespace wootz_scraping {

class WootzScrapingService : public KeyedService,
                           public mojom::WootzScrapingService {
 public:
  using APIRequestHelper = api_request_helper::APIRequestHelper;

  WootzScrapingService();
  WootzScrapingService(
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
      std::unique_ptr<WootzScrapingServiceDelegate> delegate,
      PrefService* profile_prefs,
      PrefService* local_state);

  ~WootzScrapingService() override;

  WootzScrapingService(const WootzScrapingService&) = delete;
  WootzScrapingService& operator=(const WootzScrapingService&) = delete;

  template <class T>
  void Bind(mojo::PendingReceiver<T> receiver);

  // mojom::WootzScrapingService:
  void GetTwitterCookies(mojom::WootzScrapingService::GetTwitterCookiesCallback callback) override;
  void HasTwitterCookies(mojom::WootzScrapingService::HasTwitterCookiesCallback callback) override;
  void ScrapeHttpOnlyTokens(mojom::WootzScrapingService::ScrapeHttpOnlyTokensCallback callback) override;
  void MakeTwitterGraphQLCall(const std::string& url,
                             const std::string& method,
                             const std::string& headers,
                             const std::string& body,
                             mojom::WootzScrapingService::MakeTwitterGraphQLCallCallback callback) override;
  void MakeAPICallWithStoredCredentials() override;
  
  // NEW: Store Twitter API credentials in Chrome Storage after successful call
  void StoreTwitterAPICredentials(const std::string& url, const std::string& method,
                                 const std::string& headers, const std::string& body);
  
  // NEW: Browser-side Twitter API call using stored credentials (call externally)
  void MakeBrowserSideTwitterAPICall(const std::string& callback_identifier = "");
  
  // NEW: Check if Twitter API credentials are stored
  bool HasStoredTwitterAPICredentials() const;
  
  // NEW: Clear expired Twitter API credentials from storage
  void ClearExpiredCredentials();
  
  // NEW: Debug method to test credential storage/retrieval
  void DebugTestCredentialStorage();
  
  // NEW: Initialize credential system after service construction
  void PostConstructionInitialize();

  WootzScrapingServiceDelegate* GetDelegate();
  
  // Helper methods for token scraping
  void OnLoginDetectionComplete(ScrapeHttpOnlyTokensCallback callback,
                               base::Value result);
  void OnHttpOnlyCookiesRetrieved(ScrapeHttpOnlyTokensCallback callback,
                                 const std::vector<net::CanonicalCookie>& cookies);
  
  // Browser-side Twitter GraphQL API response callback
  void OnTwitterGraphQLAPIResponse(MakeTwitterGraphQLCallCallback callback,
                                  std::unique_ptr<network::SimpleURLLoader> url_loader,
                                  std::optional<std::string> response_body,
                                  const std::string& url, const std::string& method,
                                  const std::string& headers, const std::string& body);
                                   
  content::WebContents* GetActiveWebContents();

 protected:
  // For tests - this is already public above

 private:
  friend class EthereumProviderImplUnitTest;
  friend class SolanaProviderImplUnitTest;
  friend class WootzScrapingServiceUnitTest;

  FRIEND_TEST_ALL_PREFIXES(WootzScrapingServiceUnitTest, ImportFromMetaMask);
  FRIEND_TEST_ALL_PREFIXES(WootzScrapingServiceUnitTest, Reset);
  
  // ============================================================================
  // STORED TWITTER API CREDENTIALS - Chrome Storage Integration
  // ============================================================================
  
  // Stored Twitter API credentials for browser-side calls
  struct StoredTwitterAPICredentials {
    StoredTwitterAPICredentials();
    ~StoredTwitterAPICredentials();
    
    std::string url;
    std::string method;
    std::string headers;
    std::string body;
    base::Time stored_time;
    bool is_valid = false;
    
    void Clear();
  };
  
  StoredTwitterAPICredentials stored_twitter_credentials_;
  
  // Helper methods for credential storage
  void StoreCredentialsAsync(const std::string& url, const std::string& method,
                           const std::string& headers, const std::string& body);
  void SaveCredentialsToStorage();
  void LoadCredentialsFromStorage();
  void OnBrowserSideTwitterAPIResponse(
      std::unique_ptr<network::SimpleURLLoader> url_loader,
      const std::string& callback_identifier,
      std::optional<std::string> response_body);
  void OnCredentialsSaved(bool success);
  void OnCredentialsLoaded(const std::string& stored_data);
  
  // Validate stored credentials 
  bool ValidateStoredCredentials() const {
    if (!stored_twitter_credentials_.is_valid) {
      return false;
    }
    
    if (stored_twitter_credentials_.url.empty() || 
        stored_twitter_credentials_.method.empty() || 
        stored_twitter_credentials_.headers.empty()) {
      return false;
    }
    
    // Check if credentials are not too old (24 hours max)
    base::Time now = base::Time::Now();
    base::TimeDelta age = now - stored_twitter_credentials_.stored_time;
    if (age > base::Hours(24)) {
      LOG(WARNING) << "Aaditesh -> Stored credentials are too old (" << age.InHours() << " hours)";
      return false;
    }
    
    return true;
  }
  
  // ============================================================================
  // END STORED TWITTER API CREDENTIALS
  // ============================================================================
  
  std::unique_ptr<WootzScrapingServiceDelegate> delegate_;
  raw_ptr<PrefService> profile_prefs_ = nullptr;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  mojo::ReceiverSet<mojom::WootzScrapingService> receivers_;
  PrefChangeRegistrar pref_change_registrar_;
  base::WeakPtrFactory<WootzScrapingService> weak_ptr_factory_;
};

}  // namespace wootz_scraping

#endif  // COMPONENTS_WOOTZ_SCRAPING_BROWSER_WOOTZ_SCRAPING_SERVICE_H_
