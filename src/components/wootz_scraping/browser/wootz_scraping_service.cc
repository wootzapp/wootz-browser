/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_scraping/browser/wootz_scraping_service.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "base/containers/contains.h"
#include "base/notreached.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/json/json_writer.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "components/wootz_scraping/browser/wootz_scraping_service_delegate.h"
#include "components/wootz_scraping/browser/pref_names.h"
#include "components/country_codes/country_codes.h"
#include "components/grit/wootz_components_strings.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "net/traffic_annotation/network_traffic_annotation.h"

#if BUILDFLAG(IS_ANDROID)
#include <android/log.h>
#endif
#include "ui/base/l10n/l10n_util.h"
#include "url/origin.h"
#include "url/gurl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_frame_host.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#else
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#endif
#include "content/public/browser/web_contents_observer.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_access_result.h"
#include "content/public/common/isolated_world_ids.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "services/network/public/mojom/restricted_cookie_manager.mojom.h"
#include "net/cookies/cookie_access_result.h"
#include "net/cookies/cookie_util.h"
#include "net/cookies/cookie_partition_key.h"
#include "net/cookies/cookie_partition_key_collection.h"
#include "net/cookies/cookie_options.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"

namespace wootz_scraping {

WootzScrapingService::WootzScrapingService(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    std::unique_ptr<WootzScrapingServiceDelegate> delegate,
    PrefService* profile_prefs,
    PrefService* local_state)
    : delegate_(std::move(delegate)),
      profile_prefs_(profile_prefs),
      url_loader_factory_(url_loader_factory),
      weak_ptr_factory_(this) {
  
  LOG(INFO) << "WootzScrapingService: Constructor called";
  
  // Defensive null checks to prevent crashes - log errors instead of crashing
  if (!delegate_) {
    LOG(ERROR) << "WootzScrapingService: delegate is null, service may not function properly";
    return;
  }
  
  if (!profile_prefs_) {
    LOG(ERROR) << "WootzScrapingService: profile_prefs is null, service may not function properly";
    return;
  }

  if (!url_loader_factory_) {
    LOG(ERROR) << "WootzScrapingService: url_loader_factory is null, service may not function properly";
    return;
  }

  LOG(INFO) << "WootzScrapingService: All dependencies validated successfully";

  // Initialize pref change registrar only if profile_prefs is valid
  // This might be causing the crash, so let's make it extra safe
  if (profile_prefs_) {
    LOG(INFO) << "WootzScrapingService: Initializing pref change registrar";
    pref_change_registrar_.Init(profile_prefs_);
    LOG(INFO) << "WootzScrapingService: Pref change registrar initialized successfully";
  }
  
  LOG(INFO) << "WootzScrapingService: Basic initialization complete, deferring credential operations";
  
  // NOTE: Defer credential loading and cleanup to prevent initialization crashes
  // These operations will be performed when actually needed
  // LoadCredentialsFromStorage();
  // ClearExpiredCredentials();
  
  LOG(INFO) << "WootzScrapingService: Constructor completed successfully";
}

WootzScrapingService::WootzScrapingService() : weak_ptr_factory_(this) {}

WootzScrapingService::~WootzScrapingService() = default;

template <class T>
void WootzScrapingService::Bind(mojo::PendingReceiver<T> receiver) {
  static_assert(std::is_same_v<T, mojom::WootzScrapingService>,
                "Only WootzScrapingService interface is supported");
  receivers_.Add(this, std::move(receiver));
}

// Explicit template instantiation
template void WootzScrapingService::Bind<mojom::WootzScrapingService>(
    mojo::PendingReceiver<mojom::WootzScrapingService> receiver);

void WootzScrapingService::GetTwitterCookies(mojom::WootzScrapingService::GetTwitterCookiesCallback callback) {
  // Use delegate's GetTwitterCookies method instead
  delegate_->GetTwitterCookies(base::BindOnce(
      [](mojom::WootzScrapingService::GetTwitterCookiesCallback callback, 
         const std::vector<net::CanonicalCookie>& cookies) {
        // Process cookies and extract auth tokens
        std::string auth_token, twitter_sess;
        for (const auto& cookie : cookies) {
          if (cookie.Name() == "auth_token") {
            auth_token = cookie.Value();
          } else if (cookie.Name() == "twitter_sess") {
            twitter_sess = cookie.Value();
          }
        }
        bool success = !auth_token.empty() || !twitter_sess.empty();
        std::move(callback).Run(success, 
                               auth_token.empty() ? std::nullopt : std::make_optional(auth_token),
                               twitter_sess.empty() ? std::nullopt : std::make_optional(twitter_sess),
                               success ? std::nullopt : std::make_optional("No valid tokens found"));
      }, std::move(callback)));
}
void WootzScrapingService::HasTwitterCookies(mojom::WootzScrapingService::HasTwitterCookiesCallback callback) {
  // Use delegate's HasTwitterCookies method
  delegate_->HasTwitterCookies(std::move(callback));
}

// Token scraping functionality - detects login and scrapes HttpOnly cookies
void WootzScrapingService::ScrapeHttpOnlyTokens(mojom::WootzScrapingService::ScrapeHttpOnlyTokensCallback callback) {
  LOG(INFO) << "Aadi_Kadu -> ===== Browser HttpCookies -> ScrapeHttpOnlyTokens() START =====";
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> ScrapeHttpOnlyTokens called via Mojom from renderer";
  
  auto* delegate = GetDelegate();
  if (!delegate) {
    LOG(ERROR) << "Aadi_Kadu -> Browser HttpCookies -> CRITICAL: No delegate available";
    std::move(callback).Run(false, std::nullopt, "No delegate available");
    return;
  }
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> ✓ Delegate validation passed";

  // Use delegate's GetHttpOnlyCookies method instead of direct browser context access
  delegate->GetHttpOnlyCookies(base::BindOnce(
      [](mojom::WootzScrapingService::ScrapeHttpOnlyTokensCallback callback, 
         const std::vector<net::CanonicalCookie>& cookies) {
        // Process cookies and extract tokens
        std::string tokens;
        for (const auto& cookie : cookies) {
          if (cookie.IsHttpOnly()) {
            tokens += cookie.Name() + "=" + cookie.Value() + "; ";
          }
        }
        bool success = !tokens.empty();
        std::move(callback).Run(success, 
                               success ? std::make_optional(tokens) : std::nullopt,
                               success ? std::nullopt : std::make_optional("No HttpOnly tokens found"));
      }, std::move(callback)));
}

void WootzScrapingService::OnHttpOnlyCookiesRetrieved(
    mojom::WootzScrapingService::ScrapeHttpOnlyTokensCallback callback,
    const std::vector<net::CanonicalCookie>& cookies) {
  
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Retrieved " << cookies.size() << " total cookies from cookie manager";
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Starting HttpOnly authentication cookie filtering...";
  
  // First, log ALL cookies for debugging
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> ===== ALL COOKIES DEBUG =====";
  int httponly_count = 0;
  int regular_count = 0;
  
  for (const auto& cookie : cookies) {
    if (cookie.IsHttpOnly()) {
      httponly_count++;
      LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> HttpOnly Cookie: " 
                << cookie.Name() << " = " << cookie.Value() << " (Domain: " << cookie.Domain() << ")";
    } else {
      regular_count++;
      LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Regular Cookie: " 
                << cookie.Name() << " = " << cookie.Value() << " (Domain: " << cookie.Domain() << ")";
    }
  }
  
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Cookie Summary: " 
            << httponly_count << " HttpOnly, " 
            << regular_count << " Regular, " 
            << cookies.size() << " Total";
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> ===== END ALL COOKIES DEBUG =====";
  
  std::vector<std::string> httponly_tokens;
  
  // Filter for HttpOnly cookies and extract authentication-related ones
  for (const auto& cookie : cookies) {
    if (cookie.IsHttpOnly()) {
      // Check for authentication-related cookie names
      if (cookie.Name() == "auth_token" || 
          cookie.Name() == "_twitter_sess" ||
          cookie.Name() == "twid" ||
          cookie.Name() == "ct0" ||
          cookie.Name() == "session" ||
          cookie.Name() == "csrf_token") {
        
        std::string cookie_string = cookie.Name() + "=" + cookie.Value();
        httponly_tokens.push_back(cookie_string);
        
        LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> ✅ Found authentication cookie: " 
                  << cookie.Name() << " = '" << cookie.Value() << "' (HttpOnly)";
      } else {
        LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Skipping non-auth HttpOnly cookie: " << cookie.Name();
      }
    }
  }
  
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Filtering complete: Found " << httponly_tokens.size() << " authentication cookies from " << cookies.size() << " total cookies";
  
  if (httponly_tokens.empty()) {
    LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> No HttpOnly authentication tokens found - sending empty result";
    std::move(callback).Run(true, std::nullopt, "No HttpOnly authentication tokens found");
    return;
  }
  
  // FIXED: Join HttpOnly tokens with semicolon separator for proper HTTP cookie format
  // This ensures compatibility with renderer-side CombineCookieStrings() method
  std::string tokens_string = base::JoinString(httponly_tokens, "; ");
  
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Successfully scraped " 
            << httponly_tokens.size() << " HttpOnly authentication tokens";
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> Final HttpOnly string: " << tokens_string;
  LOG(INFO) << "Aadi_Kadu -> Browser HttpCookies -> ===== FLOW END: Sending results back to renderer =====";
  
  // Note: API calling functionality disabled - HttpOnly cookies only used for token scraping
  
  std::move(callback).Run(true, tokens_string, std::nullopt);
}

content::WebContents* WootzScrapingService::GetActiveWebContents() {
#if BUILDFLAG(IS_ANDROID)
  // Android implementation using TabModelList and TabModel APIs
  TabModel* current_tab_model = TabModelList::GetCurrentTabModel();
  if (!current_tab_model) {
    LOG(ERROR) << "WootzScrapingService::GetActiveWebContents: No current tab model found";
    return nullptr;
  }

  content::WebContents* active_web_contents = current_tab_model->GetActiveWebContents();
  if (!active_web_contents) {
    LOG(ERROR) << "WootzScrapingService::GetActiveWebContents: No active web contents found";
    return nullptr;
  }

  LOG(INFO) << "WootzScrapingService::GetActiveWebContents: Successfully retrieved active WebContents";
  return active_web_contents;
#else
  // Desktop implementation using Chrome's BrowserList and TabStripModel APIs
  Browser* last_active_browser = BrowserList::GetInstance()->GetLastActive();
  if (!last_active_browser) {
    LOG(ERROR) << "WootzScrapingService::GetActiveWebContents: No active browser found";
    return nullptr;
  }

  TabStripModel* tab_strip_model = last_active_browser->tab_strip_model();
  if (!tab_strip_model) {
    LOG(ERROR) << "WootzScrapingService::GetActiveWebContents: No tab strip model found";
    return nullptr;
  }

  content::WebContents* active_web_contents = tab_strip_model->GetActiveWebContents();
  if (!active_web_contents) {
    LOG(ERROR) << "WootzScrapingService::GetActiveWebContents: No active web contents found";
    return nullptr;
  }

  LOG(INFO) << "WootzScrapingService::GetActiveWebContents: Successfully retrieved active WebContents";
  return active_web_contents;
#endif
}

// Browser-side Twitter GraphQL API call using dynamic data from renderer
void WootzScrapingService::MakeTwitterGraphQLCall(const std::string& url,
                                                const std::string& method,
                                                const std::string& headers,
                                                const std::string& body,
                                                mojom::WootzScrapingService::MakeTwitterGraphQLCallCallback callback) {
  LOG(INFO) << "Aaditesh -> ===== WootzScrapingService::MakeTwitterGraphQLCall() START =====";
  LOG(INFO) << "Aaditesh Scraping Service CALL INITIATED";
  LOG(INFO) << "Aaditesh -> Browser-side Twitter GraphQL API call initiated";
  LOG(INFO) << "Aaditesh -> URL: " << url.substr(0, 100) << "...";
  LOG(INFO) << "Aaditesh -> Method: " << method;
  LOG(INFO) << "Aaditesh -> Headers length: " << headers.length() << " chars";
  LOG(INFO) << "Aaditesh -> Body length: " << body.length() << " chars";
  
  // Store the API call parameters for potential storage on success
  std::string temp_url = url;
  std::string temp_method = method;
  std::string temp_headers = headers;
  std::string temp_body = body;
  
  // Validate URL
  GURL api_url(url);
  if (!api_url.is_valid()) {
    LOG(ERROR) << "Aaditesh -> Invalid URL provided: " << url;
    std::move(callback).Run(400, "", "Invalid URL provided");
    return;
  }
  
  // Check that it's a Twitter/X domain
  if (!api_url.DomainIs("x.com") && !api_url.DomainIs("twitter.com")) {
    LOG(ERROR) << "Aaditesh -> URL is not from allowed domain: " << api_url.host();
    std::move(callback).Run(403, "", "URL not from allowed domain");
    return;
  }
  
  // Create resource request
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = api_url;
  resource_request->method = method;
  resource_request->credentials_mode = network::mojom::CredentialsMode::kInclude;
  
  // Set additional request properties to match browser context
  resource_request->site_for_cookies = net::SiteForCookies::FromUrl(api_url);
  resource_request->request_initiator = url::Origin::Create(GURL("https://x.com"));
  
  LOG(INFO) << "Aaditesh -> Request context - Site for cookies: " << resource_request->site_for_cookies.ToDebugString();
  LOG(INFO) << "Aaditesh -> Request context - Initiator: " << resource_request->request_initiator->Serialize();
  
  // Parse and set headers from the header string
  LOG(INFO) << "Aaditesh -> Parsing headers from renderer...";
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: Browser parsing headers in CURL format:";
  
  std::vector<std::string> header_lines = base::SplitString(
      headers, "\n", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  
  int header_count = 0;
  for (const auto& header_line : header_lines) {
    size_t colon_pos = header_line.find(':');
    if (colon_pos != std::string::npos) {
      std::string header_name = std::string(base::TrimWhitespaceASCII(
          header_line.substr(0, colon_pos), base::TRIM_ALL));
      std::string header_value = std::string(base::TrimWhitespaceASCII(
          header_line.substr(colon_pos + 1), base::TRIM_ALL));
      
      LOG(INFO) << "Aaditesh -> 🔍 Setting header: " << header_name << " = " << header_value.substr(0, 50) << "...";
      resource_request->headers.SetHeader(header_name, header_value);
      header_count++;
    }
  }
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: End of browser header parsing";
  
  LOG(INFO) << "Aaditesh -> Successfully parsed " << header_count << " headers";
  
  // Set body if provided
  if (!body.empty()) {
    LOG(INFO) << "Aaditesh -> Setting request body (" << body.length() << " chars)";
    resource_request->request_body = network::ResourceRequestBody::CreateFromBytes(
        body.data(), body.size());
  }
  
  // Create traffic annotation
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("twitter_graphql_api", R"(
        semantics {
          sender: "Twitter GraphQL API Bridge"
          description: "Makes Twitter GraphQL API requests from browser process using dynamic data"
          trigger: "User navigating Twitter pages with API interception enabled"
          data: "Twitter GraphQL API requests with dynamic headers and authentication"
          destination: WEBSITE
        }
        policy {
          cookies_allowed: YES
          cookies_store: "user"
          setting: "This feature can be disabled in browser settings"
        })");

  // Create URL loader
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request), traffic_annotation);
  
  // Set timeout (10 seconds)
  url_loader->SetTimeoutDuration(base::Seconds(10));
  
  LOG(INFO) << "Aaditesh -> Sending request to Twitter GraphQL API...";
  
  // Make the request
  const size_t kMaxResponseSize = 5 * 1024 * 1024; // 5MB max
  
  // Store the URL loader in a raw pointer before moving it
  auto* url_loader_ptr = url_loader.get();
  
  // Create a callback wrapper that captures the additional parameters
  auto callback_wrapper = base::BindOnce(
      [](base::WeakPtr<WootzScrapingService> service,
         mojom::WootzScrapingService::MakeTwitterGraphQLCallCallback callback,
         std::unique_ptr<network::SimpleURLLoader> url_loader,
         const std::string& url, const std::string& method,
         const std::string& headers, const std::string& body,
         std::optional<std::string> response_body) {
        if (service) {
          service->OnTwitterGraphQLAPIResponse(
              std::move(callback), std::move(url_loader), response_body,
              url, method, headers, body);
        }
      },
      weak_ptr_factory_.GetWeakPtr(),
      std::move(callback),
      std::move(url_loader),
      temp_url, temp_method, temp_headers, temp_body);
  
  url_loader_ptr->DownloadToString(
      url_loader_factory_.get(),
      std::move(callback_wrapper),
      kMaxResponseSize);
  
  LOG(INFO) << "Aaditesh -> Request sent, waiting for response...";
}

void WootzScrapingService::OnTwitterGraphQLAPIResponse(
    mojom::WootzScrapingService::MakeTwitterGraphQLCallCallback callback,
    std::unique_ptr<network::SimpleURLLoader> url_loader,
    std::optional<std::string> response_body,
    const std::string& url, const std::string& method,
    const std::string& headers, const std::string& body) {
  
  LOG(INFO) << "Aaditesh -> ===== OnTwitterGraphQLAPIResponse() START =====";
  
  int status_code = 0;
  std::string error_message;
  std::string response_data;
  
  // Get detailed response information
  if (url_loader->ResponseInfo() && url_loader->ResponseInfo()->headers) {
    status_code = url_loader->ResponseInfo()->headers->response_code();
    LOG(INFO) << "Aaditesh -> Response status code: " << status_code;
    
    // Log response headers for debugging
    if (status_code == 403) {
      LOG(ERROR) << "Aaditesh -> 403 FORBIDDEN - Authentication or authorization issue";
      LOG(ERROR) << "Aaditesh -> Check CSRF tokens, authorization headers, and cookies";
    }
  }
  
  // Store credentials if API call was successful (status 200-299)
  bool api_success = false;
  if (status_code >= 200 && status_code < 300) {
    api_success = true;
    LOG(INFO) << "Aaditesh -> ✅ API call successful - storing credentials for future use";
    
    // CRITICAL SECTION: Store credentials with safety checks (ASYNC)
    LOG(INFO) << "Aaditesh -> 🔄 Attempting to store Twitter API credentials asynchronously...";
    
    // Ensure service is properly initialized before storing
    if (!profile_prefs_) {
      LOG(WARNING) << "Aaditesh -> 🔄 Service not fully initialized, calling PostConstructionInitialize...";
      PostConstructionInitialize();
    }
    
    // Use safer error handling instead of try-catch (exceptions disabled in Chromium)
    if (!url.empty() && !method.empty() && !headers.empty()) {
      // Use async storage to prevent main thread blocking and crashes
      StoreCredentialsAsync(url, method, headers, body);
      LOG(INFO) << "Aaditesh -> ✅ Twitter API credentials storage initiated asynchronously";
    } else {
      LOG(ERROR) << "Aaditesh -> ❌ CRITICAL ERROR: Cannot store credentials - missing required data";
      #if BUILDFLAG(IS_ANDROID)
      __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", "❌ Missing data for credential storage");
      #endif
    }
    
    // Don't immediately load - credentials are already in memory
  }
  
  if (url_loader->NetError() != net::OK) {
    error_message = "Network error: " + net::ErrorToString(url_loader->NetError());
    LOG(ERROR) << "Aaditesh -> " << error_message;
    
    // Additional debugging for 403 errors
    if (status_code == 403) {
      LOG(ERROR) << "Aaditesh -> 403 Error Debug Info:";
      LOG(ERROR) << "  - URL: " << url;
      LOG(ERROR) << "  - Method: " << method;
      LOG(ERROR) << "  - Headers preview: " << headers.substr(0, 200) << "...";
    }
  } else if (response_body) {
    response_data = *response_body;
    LOG(INFO) << "Aaditesh -> Response received (" << response_data.length() << " characters)";
    
    // Log a preview of the response (first 1000 characters)
    std::string preview = response_data.substr(0, 1000);
    if (response_data.length() > 1000) {
      preview += "... [truncated]";
    }
    LOG(INFO) << "Aaditesh -> Response preview (first 1000 chars): " << preview;
    
    // Log to Android console if available
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                       "Twitter API Response (%zu chars): %s", 
                       response_data.length(), preview.c_str());
    #endif
  } else {
    error_message = "Empty response received";
    LOG(WARNING) << "Aaditesh -> " << error_message;
  }
  
  if (api_success) {
    LOG(INFO) << "Aaditesh -> 💾 Credentials stored in Chrome Storage for browser-side reuse";
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                       "✅ Twitter API credentials stored for reuse");
    #endif
  }
  
  LOG(INFO) << "Aaditesh -> Calling renderer callback with status: " << status_code;
  std::move(callback).Run(status_code, response_data, error_message);
  
  LOG(INFO) << "Aaditesh -> ===== OnTwitterGraphQLAPIResponse() END =====";
}

WootzScrapingServiceDelegate* WootzScrapingService::GetDelegate() {
  return delegate_.get();
}

// ============================================================================
// TWITTER API CREDENTIAL STORAGE AND BROWSER-SIDE API CALL IMPLEMENTATION
// ============================================================================

void WootzScrapingService::StoreTwitterAPICredentials(const std::string& url,
                                                   const std::string& method,
                                                   const std::string& headers,
                                                   const std::string& body) {
  LOG(INFO) << "Aaditesh -> ===== StoreTwitterAPICredentials() START =====";
  LOG(INFO) << "Aaditesh -> 💾 Storing successful Twitter API credentials in Chrome Storage";
  
  // CRITICAL SAFETY CHECKS to prevent crashes
  if (!profile_prefs_) {
    LOG(ERROR) << "Aaditesh -> ❌ CRITICAL: PrefService is null, cannot store credentials";
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", "❌ PrefService null in StoreTwitterAPICredentials");
    #endif
    return;
  }
  
  if (url.empty() || method.empty() || headers.empty()) {
    LOG(ERROR) << "Aaditesh -> ❌ CRITICAL: Missing required credential data";
    LOG(ERROR) << "  - URL empty: " << (url.empty() ? "YES" : "NO");
    LOG(ERROR) << "  - Method empty: " << (method.empty() ? "YES" : "NO");
    LOG(ERROR) << "  - Headers empty: " << (headers.empty() ? "YES" : "NO");
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", "❌ Missing credential data in StoreTwitterAPICredentials");
    #endif
    return;
  }
  
  // Validate data sizes to prevent memory issues
  const size_t MAX_URL_SIZE = 8192;  // 8KB
  const size_t MAX_HEADERS_SIZE = 32768;  // 32KB
  const size_t MAX_BODY_SIZE = 65536;  // 64KB
  
  if (url.length() > MAX_URL_SIZE || headers.length() > MAX_HEADERS_SIZE || body.length() > MAX_BODY_SIZE) {
    LOG(ERROR) << "Aaditesh -> ❌ CRITICAL: Credential data too large";
    LOG(ERROR) << "  - URL size: " << url.length() << " (max: " << MAX_URL_SIZE << ")";
    LOG(ERROR) << "  - Headers size: " << headers.length() << " (max: " << MAX_HEADERS_SIZE << ")";
    LOG(ERROR) << "  - Body size: " << body.length() << " (max: " << MAX_BODY_SIZE << ")";
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", "❌ Credential data too large");
    #endif
    return;
  }
  
  // Clear any existing credentials safely
  stored_twitter_credentials_.Clear();
  
  // Store the new credentials with bounds checking
  stored_twitter_credentials_.url = url.substr(0, MAX_URL_SIZE);
  stored_twitter_credentials_.method = method;
  stored_twitter_credentials_.headers = headers.substr(0, MAX_HEADERS_SIZE);
  stored_twitter_credentials_.body = body.substr(0, MAX_BODY_SIZE);
  stored_twitter_credentials_.stored_time = base::Time::Now();
  stored_twitter_credentials_.is_valid = true;
  
  // DETAILED MEMORY STORAGE LOGGING
  LOG(INFO) << "Aaditesh -> ✅ CREDENTIALS STORED IN MEMORY:";
  LOG(INFO) << "  🔗 URL: " << url.substr(0, 100) << "..." << " (Total: " << url.length() << " chars)";
  LOG(INFO) << "  🔧 Method: " << method;
  LOG(INFO) << "  📋 Headers Length: " << headers.length() << " characters";
  LOG(INFO) << "  📋 Headers (first 200 chars): " << headers.substr(0, 200) << "...";
  LOG(INFO) << "  📦 Body Length: " << body.length() << " characters";
  LOG(INFO) << "  ⏰ Stored at: " << stored_twitter_credentials_.stored_time;
  LOG(INFO) << "  ✅ Valid flag: " << (stored_twitter_credentials_.is_valid ? "YES" : "NO");
  
  // Check for key authentication tokens in headers
  if (headers.find("authorization:") != std::string::npos || headers.find("Authorization:") != std::string::npos) {
    LOG(INFO) << "  🔑 Authorization header detected in stored credentials";
  }
  if (headers.find("x-csrf-token:") != std::string::npos || headers.find("X-Csrf-Token:") != std::string::npos) {
    LOG(INFO) << "  🛡️ CSRF token detected in stored credentials";
  }
  if (headers.find("cookie:") != std::string::npos || headers.find("Cookie:") != std::string::npos) {
    LOG(INFO) << "  🍪 Cookie header detected in stored credentials";
  }
  
  // Save to persistent storage using Chrome's PrefService
  SaveCredentialsToStorage();
  
  LOG(INFO) << "Aaditesh -> 📱 Android Console: Twitter API credentials stored successfully";
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "💾 Twitter API credentials stored - URL: %zu chars, Headers: %zu chars",
                     url.length(), headers.length());
  #endif
  
  LOG(INFO) << "Aaditesh -> ===== StoreTwitterAPICredentials() END =====";
}

// Asynchronous credential storage to prevent main thread blocking
void WootzScrapingService::StoreCredentialsAsync(const std::string& url,
                                                const std::string& method,
                                                const std::string& headers,
                                                const std::string& body) {
  LOG(INFO) << "Aaditesh -> 🔄 Scheduling asynchronous credential storage...";
  
  // Post storage task to sequenced thread to prevent blocking
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(&WootzScrapingService::StoreTwitterAPICredentials,
                     weak_ptr_factory_.GetWeakPtr(),
                     url, method, headers, body));
  
  LOG(INFO) << "Aaditesh -> ✅ Credential storage task scheduled";
}

bool WootzScrapingService::HasStoredTwitterAPICredentials() const {
  // Early safety check - if service isn't properly initialized, return false
  if (!delegate_ || !profile_prefs_) {
    LOG(INFO) << "Aaditesh -> 🔍 Service not properly initialized, no credentials available";
    return false;
  }

  bool has_creds = stored_twitter_credentials_.is_valid && 
                   !stored_twitter_credentials_.url.empty() &&
                   !stored_twitter_credentials_.headers.empty();
  
  // DETAILED CREDENTIAL STATUS LOGGING
  LOG(INFO) << "Aaditesh -> 🔍 CREDENTIAL STATUS CHECK:";
  LOG(INFO) << "  ✅ is_valid: " << (stored_twitter_credentials_.is_valid ? "YES" : "NO");
  LOG(INFO) << "  🔗 URL empty: " << (stored_twitter_credentials_.url.empty() ? "YES" : "NO");
  LOG(INFO) << "  📋 Headers empty: " << (stored_twitter_credentials_.headers.empty() ? "YES" : "NO");
  LOG(INFO) << "  🎯 Overall has_creds: " << (has_creds ? "YES" : "NO");
  
  if (has_creds) {
    auto time_diff = base::Time::Now() - stored_twitter_credentials_.stored_time;
    LOG(INFO) << "  ⏰ Credentials age: " << time_diff.InMinutes() << " minutes";
    LOG(INFO) << "  📊 URL length: " << stored_twitter_credentials_.url.length() << " chars";
    LOG(INFO) << "  📊 Headers length: " << stored_twitter_credentials_.headers.length() << " chars";
    LOG(INFO) << "  📊 Body length: " << stored_twitter_credentials_.body.length() << " chars";
  } else {
    LOG(INFO) << "  ❌ No valid credentials in memory";
  }
  
  return has_creds;
}

void WootzScrapingService::MakeBrowserSideTwitterAPICall(const std::string& callback_identifier) {
  LOG(INFO) << "Aaditesh -> ===== MakeBrowserSideTwitterAPICall() START =====";
  LOG(INFO) << "Aaditesh -> 🚀 Making browser-side Twitter API call using stored credentials";
  LOG(INFO) << "Aaditesh -> Callback identifier: " << (callback_identifier.empty() ? "none" : callback_identifier);
  
  // Early safety check
  if (!delegate_ || !profile_prefs_) {
    LOG(ERROR) << "Aaditesh -> ❌ Service not properly initialized for API calls";
    return;
  }
  
  // First try to load credentials from storage if not in memory (lazy loading)
  if (!HasStoredTwitterAPICredentials()) {
    LOG(INFO) << "Aaditesh -> No credentials in memory, trying to load from storage...";
    LoadCredentialsFromStorage();
  }
  
  // Check if we have stored credentials after loading attempt
  if (!HasStoredTwitterAPICredentials()) {
    LOG(ERROR) << "Aaditesh -> ❌ No stored Twitter API credentials available";
    LOG(ERROR) << "Aaditesh -> Cannot make browser-side API call without stored credentials";
    
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", 
                       "❌ Cannot make browser-side API call: No stored credentials");
    #endif
    return;
  }
  
  LOG(INFO) << "Aaditesh -> ✅ Using stored credentials from " << stored_twitter_credentials_.stored_time;
  LOG(INFO) << "Aaditesh -> 📊 USING STORED CREDENTIALS FOR API CALL:";
  LOG(INFO) << "  🔗 URL: " << stored_twitter_credentials_.url.substr(0, 150) << "...";
  LOG(INFO) << "  🔧 Method: " << stored_twitter_credentials_.method;
  LOG(INFO) << "  📋 Headers Length: " << stored_twitter_credentials_.headers.length() << " chars";
  LOG(INFO) << "  📋 Headers (preview): " << stored_twitter_credentials_.headers.substr(0, 300) << "...";
  LOG(INFO) << "  📦 Body Length: " << stored_twitter_credentials_.body.length() << " chars";
  
  // Show key authentication components
  if (stored_twitter_credentials_.headers.find("authorization:") != std::string::npos || 
      stored_twitter_credentials_.headers.find("Authorization:") != std::string::npos) {
    LOG(INFO) << "  🔑 ✅ Authorization header present";
  } else {
    LOG(WARNING) << "  🔑 ❌ No Authorization header found!";
  }
  
  if (stored_twitter_credentials_.headers.find("x-csrf-token:") != std::string::npos || 
      stored_twitter_credentials_.headers.find("X-Csrf-Token:") != std::string::npos) {
    LOG(INFO) << "  🛡️ ✅ CSRF token present";
  } else {
    LOG(WARNING) << "  🛡️ ❌ No CSRF token found!";
  }
  
  if (stored_twitter_credentials_.headers.find("cookie:") != std::string::npos || 
      stored_twitter_credentials_.headers.find("Cookie:") != std::string::npos) {
    LOG(INFO) << "  🍪 ✅ Cookie header present";
  } else {
    LOG(WARNING) << "  🍪 ❌ No Cookie header found!";
  }
  
  // Validate stored URL
  GURL api_url(stored_twitter_credentials_.url);
  if (!api_url.is_valid()) {
    LOG(ERROR) << "Aaditesh -> ❌ Stored URL is invalid: " << stored_twitter_credentials_.url;
    return;
  }
  
  // Check that it's still a Twitter/X domain
  if (!api_url.DomainIs("x.com") && !api_url.DomainIs("twitter.com")) {
    LOG(ERROR) << "Aaditesh -> ❌ Stored URL is not from allowed domain: " << api_url.host();
    return;
  }
  
  LOG(INFO) << "Aaditesh -> 🌐 Making HTTP request with stored credentials...";
  
  // Create resource request using stored credentials
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = api_url;
  resource_request->method = stored_twitter_credentials_.method;
  resource_request->credentials_mode = network::mojom::CredentialsMode::kInclude;
  
  // Set additional request properties to match browser context
  resource_request->site_for_cookies = net::SiteForCookies::FromUrl(api_url);
  resource_request->request_initiator = url::Origin::Create(GURL("https://x.com"));
  
  // Parse and set headers from stored header string
  LOG(INFO) << "Aaditesh -> 📋 Setting headers from stored credentials...";
  std::vector<std::string> header_lines = base::SplitString(
      stored_twitter_credentials_.headers, "\n", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  
  int header_count = 0;
  for (const auto& header_line : header_lines) {
    size_t colon_pos = header_line.find(':');
    if (colon_pos != std::string::npos) {
      std::string name = header_line.substr(0, colon_pos);
      std::string value = header_line.substr(colon_pos + 1);
      
      // Trim whitespace
      base::TrimWhitespaceASCII(name, base::TRIM_ALL, &name);
      base::TrimWhitespaceASCII(value, base::TRIM_ALL, &value);
      
      if (!name.empty() && !value.empty()) {
        resource_request->headers.SetHeader(name, value);
        header_count++;
      }
    }
  }
  
  LOG(INFO) << "Aaditesh -> ✅ Set " << header_count << " headers from stored credentials";
  
  // Set body if provided
  if (!stored_twitter_credentials_.body.empty()) {
    LOG(INFO) << "Aaditesh -> 📄 Setting request body (" << stored_twitter_credentials_.body.length() << " chars)";
    resource_request->request_body = network::ResourceRequestBody::CreateFromBytes(
        stored_twitter_credentials_.body.data(), stored_twitter_credentials_.body.size());
  }
  
  // Create traffic annotation
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("browser_side_twitter_api", R"(
        semantics {
          sender: "Browser-side Twitter API Client"
          description: "Makes Twitter API requests directly from browser process using stored credentials"
          trigger: "External call to MakeBrowserSideTwitterAPICall method"
          data: "Twitter API requests with stored authentication credentials"
          destination: WEBSITE
        }
        policy {
          cookies_allowed: YES
          cookies_store: "user"
          setting: "This feature uses stored credentials from previous successful API calls"
        })");

  // Create URL loader
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request), traffic_annotation);
  
  // Set timeout (15 seconds for browser-side calls)
  url_loader->SetTimeoutDuration(base::Seconds(15));
  
  LOG(INFO) << "Aaditesh -> 🚀 Sending browser-side Twitter API request...";
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "🚀 Browser-side Twitter API call initiated with stored credentials");
  #endif
  
  // Make the request with a dedicated callback for browser-side calls
  const size_t kMaxResponseSize = 5 * 1024 * 1024; // 5MB max
  
  // Store the URL loader in a raw pointer before moving it
  auto* url_loader_ptr = url_loader.get();
  
  // Create a callback wrapper for browser-side calls
  auto callback_wrapper = base::BindOnce(
      [](base::WeakPtr<WootzScrapingService> service,
         std::unique_ptr<network::SimpleURLLoader> url_loader,
         const std::string& callback_identifier,
         std::optional<std::string> response_body) {
        if (service) {
          service->OnBrowserSideTwitterAPIResponse(
              std::move(url_loader), callback_identifier, response_body);
        }
      },
      weak_ptr_factory_.GetWeakPtr(),
      std::move(url_loader),
      callback_identifier);
  
  url_loader_ptr->DownloadToString(
      url_loader_factory_.get(),
      std::move(callback_wrapper),
      kMaxResponseSize);
  
  LOG(INFO) << "Aaditesh -> 📤 Browser-side request sent, waiting for response...";
  LOG(INFO) << "Aaditesh -> ===== MakeBrowserSideTwitterAPICall() END =====";
}

void WootzScrapingService::OnBrowserSideTwitterAPIResponse(
    std::unique_ptr<network::SimpleURLLoader> url_loader,
    const std::string& callback_identifier,
    std::optional<std::string> response_body) {
  
  LOG(INFO) << "Aaditesh -> ===== OnBrowserSideTwitterAPIResponse() START =====";
  LOG(INFO) << "Aaditesh -> 📥 Browser-side Twitter API response received";
  LOG(INFO) << "Aaditesh -> Callback identifier: " << (callback_identifier.empty() ? "none" : callback_identifier);
  
  int status_code = 0;
  std::string error_message;
  std::string response_data;
  
  // Get response information
  if (url_loader->ResponseInfo() && url_loader->ResponseInfo()->headers) {
    status_code = url_loader->ResponseInfo()->headers->response_code();
    LOG(INFO) << "Aaditesh -> Response status code: " << status_code;
  }
  
  if (url_loader->NetError() != net::OK) {
    error_message = "Network error: " + net::ErrorToString(url_loader->NetError());
    LOG(ERROR) << "Aaditesh -> " << error_message;
  } else if (response_body) {
    response_data = *response_body;
    LOG(INFO) << "Aaditesh -> Response received (" << response_data.length() << " chars)";
    
    // Log preview of response
    std::string preview = response_data.substr(0, 500);
    if (response_data.length() > 500) {
      preview += "... [truncated]";
    }
    LOG(INFO) << "Aaditesh -> Response preview: " << preview;
  } else {
    error_message = "Empty response received";
    LOG(WARNING) << "Aaditesh -> " << error_message;
  }
  
  // Log results
  if (status_code >= 200 && status_code < 300) {
    LOG(INFO) << "Aaditesh -> ✅ Browser-side Twitter API call successful!";
    
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                       "✅ Browser-side Twitter API success (status: %d, response: %zu chars)",
                       status_code, response_data.length());
    #endif
  } else {
    LOG(ERROR) << "Aaditesh -> ❌ Browser-side Twitter API call failed";
    
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", 
                       "❌ Browser-side Twitter API failed (status: %d, error: %s)",
                       status_code, error_message.c_str());
    #endif
  }
  
  LOG(INFO) << "Aaditesh -> 📊 BROWSER-SIDE API CALL SUMMARY:";
  LOG(INFO) << "  - Status Code: " << status_code;
  LOG(INFO) << "  - Response Size: " << response_data.length() << " chars";
  LOG(INFO) << "  - Error: " << (error_message.empty() ? "none" : error_message);
  LOG(INFO) << "  - Success: " << (status_code >= 200 && status_code < 300 ? "YES" : "NO");
  
  // Note: This method doesn't call any renderer callback since it's a browser-side only operation
  // Results are logged and can be accessed through other mechanisms if needed
  
  LOG(INFO) << "Aaditesh -> ===== OnBrowserSideTwitterAPIResponse() END =====";
}

void WootzScrapingService::SaveCredentialsToStorage() {
  LOG(INFO) << "Aaditesh -> 💾 Saving Twitter API credentials to Chrome Storage (PrefService)";
  
  // CRITICAL SAFETY CHECKS
  if (!profile_prefs_) {
    LOG(ERROR) << "Aaditesh -> ❌ PrefService not available for credential storage";
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", "❌ PrefService null in SaveCredentialsToStorage");
    #endif
    return;
  }
  
  // Validate stored credentials before saving
  if (!stored_twitter_credentials_.is_valid) {
    LOG(ERROR) << "Aaditesh -> ❌ Stored credentials are not valid, skipping save";
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", "❌ Invalid credentials, skipping save");
    #endif
    return;
  }
  
  if (stored_twitter_credentials_.url.empty() || 
      stored_twitter_credentials_.method.empty() || 
      stored_twitter_credentials_.headers.empty()) {
    LOG(ERROR) << "Aaditesh -> ❌ Stored credentials have empty required fields";
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", "❌ Incomplete credential data");
    #endif
    return;
  }
  
  // Use safer error handling instead of try-catch (exceptions disabled in Chromium)
  // Create JSON object with credentials - with size limits
  base::Value::Dict credentials_dict;
  
  // Safely truncate large data to prevent storage issues
  const size_t MAX_STORAGE_SIZE = 32768;  // 32KB per field max
  std::string safe_url = stored_twitter_credentials_.url.substr(0, MAX_STORAGE_SIZE);
  std::string safe_headers = stored_twitter_credentials_.headers.substr(0, MAX_STORAGE_SIZE);
  std::string safe_body = stored_twitter_credentials_.body.substr(0, MAX_STORAGE_SIZE);
  
  credentials_dict.Set("url", safe_url);
  credentials_dict.Set("method", stored_twitter_credentials_.method);
  credentials_dict.Set("headers", safe_headers);
  credentials_dict.Set("body", safe_body);
  credentials_dict.Set("stored_time", stored_twitter_credentials_.stored_time.InSecondsFSinceUnixEpoch());
  credentials_dict.Set("is_valid", stored_twitter_credentials_.is_valid);
  
  // LOG THE ACTUAL DATA BEING STORED
  LOG(INFO) << "Aaditesh -> 📊 STORING CREDENTIALS TO CHROME STORAGE:";
  LOG(INFO) << "  🔗 URL: " << stored_twitter_credentials_.url.substr(0, 150) << "..." << " (Total: " << stored_twitter_credentials_.url.length() << " chars)";
  LOG(INFO) << "  🔧 Method: " << stored_twitter_credentials_.method;
  LOG(INFO) << "  📋 Headers (first 200 chars): " << stored_twitter_credentials_.headers.substr(0, 200) << "...";
  LOG(INFO) << "  📦 Body Length: " << stored_twitter_credentials_.body.length() << " characters";
  LOG(INFO) << "  ⏰ Timestamp: " << stored_twitter_credentials_.stored_time.InSecondsFSinceUnixEpoch();
  LOG(INFO) << "  ✅ Valid: " << (stored_twitter_credentials_.is_valid ? "YES" : "NO");
  
  // Store in preferences - CRITICAL OPERATION
  LOG(INFO) << "Aaditesh -> 🔄 Writing to PrefService...";
  if (profile_prefs_) {
    profile_prefs_->SetDict(kWootzScrapingTwitterAPICredentials, std::move(credentials_dict));
    LOG(INFO) << "Aaditesh -> ✅ PrefService write completed successfully";
  } else {
    LOG(ERROR) << "Aaditesh -> ❌ PrefService is null, cannot save credentials";
    return;
  }
  
  // NOW LOG WHAT'S ACTUALLY STORED IN CHROME STORAGE
  LOG(INFO) << "Aaditesh -> ✅ Twitter API credentials saved to Chrome Storage successfully";
  LOG(INFO) << "Aaditesh -> 🔍 VERIFYING STORED DATA IN CHROME STORAGE:";
  
  // Read back what was actually stored
  const base::Value::Dict& stored_dict = profile_prefs_->GetDict(kWootzScrapingTwitterAPICredentials);
  
  if (!stored_dict.empty()) {
    const std::string* stored_url = stored_dict.FindString("url");
    const std::string* stored_method = stored_dict.FindString("method");
    const std::string* stored_headers = stored_dict.FindString("headers");
    const std::string* stored_body = stored_dict.FindString("body");
    std::optional<double> stored_time = stored_dict.FindDouble("stored_time");
    std::optional<bool> stored_valid = stored_dict.FindBool("is_valid");
    
    LOG(INFO) << "Aaditesh -> � ACTUAL STORED CREDENTIALS IN CHROME STORAGE:";
    LOG(INFO) << "  🔗 Stored URL: " << (stored_url ? stored_url->substr(0, 150) + "..." : "NULL") << " (Length: " << (stored_url ? stored_url->length() : 0) << ")";
    LOG(INFO) << "  🔧 Stored Method: " << (stored_method ? *stored_method : "NULL");
    LOG(INFO) << "  📋 Stored Headers: " << (stored_headers ? stored_headers->substr(0, 200) + "..." : "NULL") << " (Length: " << (stored_headers ? stored_headers->length() : 0) << ")";
    LOG(INFO) << "  � Stored Body Length: " << (stored_body ? stored_body->length() : 0) << " characters";
    LOG(INFO) << "  ⏰ Stored Timestamp: " << (stored_time ? *stored_time : 0.0);
    LOG(INFO) << "  ✅ Stored Valid: " << (stored_valid ? (*stored_valid ? "YES" : "NO") : "NULL");

  
    
    // Log key parts of headers and URL for debugging
    if (stored_headers && stored_headers->length() > 0) {
      LOG(INFO) << "Aaditesh -> 🔑 Header keywords found:";
      if (stored_headers->find("authorization") != std::string::npos) LOG(INFO) << "  - Authorization header: PRESENT";
      if (stored_headers->find("Authorization") != std::string::npos) LOG(INFO) << "  - Authorization header: PRESENT";
      if (stored_headers->find("x-csrf-token") != std::string::npos) LOG(INFO) << "  - CSRF token: PRESENT";
      if (stored_headers->find("cookie") != std::string::npos) LOG(INFO) << "  - Cookie header: PRESENT";
      if (stored_headers->find("Cookie") != std::string::npos) LOG(INFO) << "  - Cookie header: PRESENT";
    }
    
    if (stored_url && stored_url->length() > 0) {
      LOG(INFO) << "Aaditesh -> 🔗 URL Analysis:";
      if (stored_url->find("HomeTimeline") != std::string::npos) LOG(INFO) << "  - Endpoint: HomeTimeline";
      if (stored_url->find("variables=") != std::string::npos) LOG(INFO) << "  - Has variables parameter";
      if (stored_url->find("features=") != std::string::npos) LOG(INFO) << "  - Has features parameter";
    }
  } else {
    LOG(ERROR) << "Aaditesh -> ❌ STORED CREDENTIALS DICT IS EMPTY!";
  }
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "💾 Twitter API credentials persisted to Chrome Storage successfully");
  #endif
}

void WootzScrapingService::LoadCredentialsFromStorage() {
  LOG(INFO) << "Aaditesh -> 📂 Loading Twitter API credentials from Chrome Storage";
  
  if (!profile_prefs_) {
    LOG(WARNING) << "Aaditesh -> PrefService not available for credential loading";
    return;
  }

  // Additional safety check - ensure the service is properly initialized
  if (!delegate_) {
    LOG(WARNING) << "Aaditesh -> Service not properly initialized, skipping credential loading";
    return;
  }
  
  // Clear current credentials
  stored_twitter_credentials_.Clear();
  
  // Load from preferences - use safer method with exception handling for potential crashes
  const base::Value::Dict* credentials_dict = nullptr;
  
  // Safely attempt to access the preferences
  credentials_dict = &profile_prefs_->GetDict(kWootzScrapingTwitterAPICredentials);
  if (!credentials_dict) {
    LOG(WARNING) << "Aaditesh -> Could not access Twitter API credentials in Chrome Storage";
    return;
  }
  
  if (credentials_dict->empty()) {
    LOG(INFO) << "Aaditesh -> No stored Twitter API credentials found in Chrome Storage";
    return;
  }
  
  // LOG WHAT'S ACTUALLY IN CHROME STORAGE
  LOG(INFO) << "Aaditesh -> 🔍 FOUND STORED CREDENTIALS IN CHROME STORAGE:";
  LOG(INFO) << "Aaditesh -> 📊 RAW STORAGE CONTENTS:";
  
  // Extract credentials with safer null checks
  const std::string* url = credentials_dict->FindString("url");
  const std::string* method = credentials_dict->FindString("method");
  const std::string* headers = credentials_dict->FindString("headers");
  const std::string* body = credentials_dict->FindString("body");
  std::optional<double> stored_time_double = credentials_dict->FindDouble("stored_time");
  std::optional<bool> is_valid = credentials_dict->FindBool("is_valid");
  
  // LOG THE ACTUAL STORED VALUES FROM CHROME STORAGE
  LOG(INFO) << "Aaditesh -> � LOADED FROM CHROME STORAGE:";
  LOG(INFO) << "  🔗 URL from Storage: " << (url ? url->substr(0, 150) + "..." : "NULL") << " (Length: " << (url ? url->length() : 0) << ")";
  LOG(INFO) << "  🔧 Method from Storage: " << (method ? *method : "NULL");
  LOG(INFO) << "  📋 Headers from Storage: " << (headers ? headers->substr(0, 200) + "..." : "NULL") << " (Length: " << (headers ? headers->length() : 0) << ")";
  LOG(INFO) << "  📦 Body from Storage Length: " << (body ? body->length() : 0) << " characters";
  LOG(INFO) << "  ⏰ Timestamp from Storage: " << (stored_time_double ? *stored_time_double : 0.0);
  LOG(INFO) << "  ✅ Valid from Storage: " << (is_valid ? (*is_valid ? "YES" : "NO") : "NULL");
  
  // Log authentication details found in stored headers
  if (headers && headers->length() > 0) {
    LOG(INFO) << "Aaditesh -> 🔑 Authentication Data Found in Stored Headers:";
    if (headers->find("authorization") != std::string::npos || headers->find("Authorization") != std::string::npos) {
      LOG(INFO) << "  ✅ Authorization header FOUND in stored data";
    }
    if (headers->find("x-csrf-token") != std::string::npos) {
      LOG(INFO) << "  ✅ CSRF token FOUND in stored data";
    }
    if (headers->find("cookie") != std::string::npos || headers->find("Cookie") != std::string::npos) {
      LOG(INFO) << "  ✅ Cookie header FOUND in stored data";
    }
  }
  
  // Validate all required fields are present
  if (!url || !method || !headers || !body || !stored_time_double || !is_valid) {
    LOG(WARNING) << "Aaditesh -> Missing required credential fields in storage";
    LOG(WARNING) << "  - url: " << (url ? "present" : "missing");
    LOG(WARNING) << "  - method: " << (method ? "present" : "missing");
    LOG(WARNING) << "  - headers: " << (headers ? "present" : "missing");
    LOG(WARNING) << "  - body: " << (body ? "present" : "missing");
    LOG(WARNING) << "  - stored_time: " << (stored_time_double ? "present" : "missing");
    LOG(WARNING) << "  - is_valid: " << (is_valid ? "present" : "missing");
    return;
  }
  
  if (!*is_valid) {
    LOG(WARNING) << "Aaditesh -> Stored credentials marked as invalid";
    return;
  }
  
  // Safely copy the values with additional length checks
  if (url->empty() || method->empty()) {
    LOG(WARNING) << "Aaditesh -> Essential credential fields are empty";
    return;
  }
  
  stored_twitter_credentials_.url = *url;
  stored_twitter_credentials_.method = *method;
  stored_twitter_credentials_.headers = *headers;
  stored_twitter_credentials_.body = *body;
  stored_twitter_credentials_.stored_time = base::Time::FromSecondsSinceUnixEpoch(*stored_time_double);
  stored_twitter_credentials_.is_valid = *is_valid;
  
  // LOG THE FINAL LOADED STATE
  LOG(INFO) << "Aaditesh -> ✅ SUCCESSFULLY LOADED CREDENTIALS INTO MEMORY FROM CHROME STORAGE:";
  LOG(INFO) << "  🔗 Final URL in Memory: " << stored_twitter_credentials_.url.substr(0, 150) << "..." << " (Total: " << stored_twitter_credentials_.url.length() << " chars)";
  LOG(INFO) << "  🔧 Final Method in Memory: " << stored_twitter_credentials_.method;
  LOG(INFO) << "  � Final Headers in Memory: " << stored_twitter_credentials_.headers.substr(0, 200) << "...";
  LOG(INFO) << "  📋 Final Headers Length in Memory: " << stored_twitter_credentials_.headers.length() << " characters";
  LOG(INFO) << "  📦 Final Body Length in Memory: " << stored_twitter_credentials_.body.length() << " characters";
  LOG(INFO) << "  ⏰ Final Stored Time in Memory: " << stored_twitter_credentials_.stored_time;
  LOG(INFO) << "  ✅ Final Valid Status in Memory: " << (stored_twitter_credentials_.is_valid ? "YES" : "NO");
  
  // Confirm specific authentication tokens are preserved
  if (stored_twitter_credentials_.headers.length() > 0) {
    LOG(INFO) << "Aaditesh -> 🔑 Authentication Verification in Final Memory State:";
    if (stored_twitter_credentials_.headers.find("authorization") != std::string::npos || 
        stored_twitter_credentials_.headers.find("Authorization") != std::string::npos) {
      LOG(INFO) << "  ✅ Authorization header CONFIRMED in memory";
    }
    if (stored_twitter_credentials_.headers.find("x-csrf-token") != std::string::npos) {
      LOG(INFO) << "  ✅ CSRF token CONFIRMED in memory";
    }
    if (stored_twitter_credentials_.headers.find("cookie") != std::string::npos || 
        stored_twitter_credentials_.headers.find("Cookie") != std::string::npos) {
      LOG(INFO) << "  ✅ Cookie header CONFIRMED in memory";
    }
  }
  
  LOG(INFO) << "Aaditesh -> 🎉 CREDENTIAL LOADING FROM CHROME STORAGE COMPLETE!";
  
  // Additional Android logging
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "📂 Loaded: URL(%zu chars), Headers(%zu chars), Method(%s)", 
                     stored_twitter_credentials_.url.length(), 
                     stored_twitter_credentials_.headers.length(), 
                     stored_twitter_credentials_.method.c_str());
  #endif
}

void WootzScrapingService::MakeAPICallWithStoredCredentials() {
  LOG(INFO) << "Aaditesh -> 🚀 MakeAPICallWithStoredCredentials() - Starting standalone API call";
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "🚀 Starting standalone API call with stored credentials");
  #endif
  
  if (!profile_prefs_) {
    LOG(ERROR) << "Aaditesh -> ❌ PrefService not available for credential retrieval";
    return;
  }
  
  // Ensure post-construction initialization has been performed
  if (!delegate_) {
    LOG(WARNING) << "Aaditesh -> Service not fully initialized, attempting post-construction setup";
    PostConstructionInitialize();
    if (!delegate_) {
      LOG(ERROR) << "Aaditesh -> ❌ Service initialization failed";
      return;
    }
  }
  
  // Step 1: Retrieve credentials directly from Chrome Storage
  LOG(INFO) << "Aaditesh -> 📂 Retrieving stored credentials from Chrome Storage...";
  const base::Value::Dict& credentials_dict = 
      profile_prefs_->GetDict(kWootzScrapingTwitterAPICredentials);
  
  if (credentials_dict.empty()) {
    LOG(ERROR) << "Aaditesh -> ❌ No stored Twitter API credentials found in Chrome Storage";
    LOG(ERROR) << "Aaditesh -> Cannot make API call without stored credentials";
    
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", 
                       "❌ No stored credentials found for standalone API call");
    #endif
    return;
  }
  
  // Step 2: Extract credentials from storage
  const std::string* url = credentials_dict.FindString("url");
  const std::string* method = credentials_dict.FindString("method");
  const std::string* headers = credentials_dict.FindString("headers");
  const std::string* body = credentials_dict.FindString("body");
  std::optional<double> stored_time_double = credentials_dict.FindDouble("stored_time");
  std::optional<bool> is_valid = credentials_dict.FindBool("is_valid");
  
  // Step 3: Validate all required fields
  if (!url || !method || !headers || !body || !stored_time_double || !is_valid) {
    LOG(ERROR) << "Aaditesh -> ❌ Missing required credential fields in storage";
    LOG(ERROR) << "  - url: " << (url ? "present" : "missing");
    LOG(ERROR) << "  - method: " << (method ? "present" : "missing");
    LOG(ERROR) << "  - headers: " << (headers ? "present" : "missing");
    LOG(ERROR) << "  - body: " << (body ? "present" : "missing");
    LOG(ERROR) << "  - stored_time: " << (stored_time_double ? "present" : "missing");
    LOG(ERROR) << "  - is_valid: " << (is_valid ? "present" : "missing");
    return;
  }
  
  if (!*is_valid) {
    LOG(ERROR) << "Aaditesh -> ❌ Stored credentials marked as invalid";
    return;
  }
  
  // Step 4: Log the retrieved credentials
  LOG(INFO) << "Aaditesh -> ✅ SUCCESSFULLY RETRIEVED CREDENTIALS FROM CHROME STORAGE:";
  LOG(INFO) << "  🔗 URL: " << url->substr(0, 150) << "..." << " (Total: " << url->length() << " chars)";
  LOG(INFO) << "  🔧 Method: " << *method;
  LOG(INFO) << "  📋 Headers: " << headers->substr(0, 200) << "..." << " (Total: " << headers->length() << " chars)";
  LOG(INFO) << "  📦 Body Length: " << body->length() << " characters";
  LOG(INFO) << "  ⏰ Stored Time: " << *stored_time_double;
  LOG(INFO) << "  ✅ Valid: " << (*is_valid ? "YES" : "NO");
  
  // Step 5: Verify authentication tokens are present
  if (headers->length() > 0) {
    LOG(INFO) << "Aaditesh -> 🔑 Authentication Token Verification:";
    if (headers->find("authorization") != std::string::npos || 
        headers->find("Authorization") != std::string::npos) {
      LOG(INFO) << "  ✅ Authorization header FOUND in stored credentials";
    }
    if (headers->find("x-csrf-token") != std::string::npos) {
      LOG(INFO) << "  ✅ CSRF token FOUND in stored credentials";
    }
    if (headers->find("cookie") != std::string::npos || 
        headers->find("Cookie") != std::string::npos) {
      LOG(INFO) << "  ✅ Cookie header FOUND in stored credentials";
    }
  }
  
  // Step 6: Check credential age
  base::Time stored_time = base::Time::FromSecondsSinceUnixEpoch(*stored_time_double);
  auto time_diff = base::Time::Now() - stored_time;
  LOG(INFO) << "Aaditesh -> ⏰ Credential Age: " << time_diff.InMinutes() << " minutes";
  
  if (time_diff.InHours() > 24) {
    LOG(WARNING) << "Aaditesh -> ⚠️ Credentials are over 24 hours old, may be expired";
  }
  
  // Step 7: Make the API call using the existing MakeTwitterGraphQLCall function
  LOG(INFO) << "Aaditesh -> 🎯 Making Twitter API call using stored credentials via MakeTwitterGraphQLCall...";
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "🎯 Making API call: URL(%zu chars), Headers(%zu chars), Method(%s)",
                     url->length(), headers->length(), method->c_str());
  #endif
  
  // Create a callback to handle the response
  auto api_callback = base::BindOnce(
      [](int32_t status_code, const std::string& response_body, const std::string& error_message) {
        LOG(INFO) << "Aaditesh -> 📥 STANDALONE API CALL RESPONSE:";
        LOG(INFO) << "  📊 Status Code: " << status_code;
        LOG(INFO) << "  📄 Response Length: " << response_body.length() << " characters";
        LOG(INFO) << "  ❌ Error Message: " << (error_message.empty() ? "None" : error_message);
        
        if (status_code >= 200 && status_code < 300) {
          LOG(INFO) << "Aaditesh -> 🎉 STANDALONE API CALL SUCCESSFUL!";
          
          // Log a preview of the response
          std::string preview = response_body.substr(0, 300);
          if (response_body.length() > 300) {
            preview += "... [truncated]";
          }
          LOG(INFO) << "Aaditesh -> Response Preview: " << preview;
          
          #if BUILDFLAG(IS_ANDROID)
          __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                             "🎉 Standalone API call successful! Status: %d, Response: %zu chars",
                             status_code, response_body.length());
          #endif
        } else {
          LOG(ERROR) << "Aaditesh -> ❌ STANDALONE API CALL FAILED";
          LOG(ERROR) << "  📊 Status Code: " << status_code;
          LOG(ERROR) << "  📄 Response Body: " << response_body.substr(0, 500);
          LOG(ERROR) << "  ❌ Error Message: " << error_message;
          
          #if BUILDFLAG(IS_ANDROID)
          __android_log_print(ANDROID_LOG_ERROR, "WootzScraping", 
                             "❌ Standalone API call failed! Status: %d, Error: %s", 
                             status_code, error_message.c_str());
          #endif
        }
      });
  
  // Call the existing MakeTwitterGraphQLCall function with stored credentials
  MakeTwitterGraphQLCall(*url, *method, *headers, *body, std::move(api_callback));
  
  LOG(INFO) << "Aaditesh -> 🚀 MakeAPICallWithStoredCredentials() - API call initiated successfully";
}

// ============================================================================
// END TWITTER API CREDENTIAL STORAGE AND BROWSER-SIDE API CALL IMPLEMENTATION
// ============================================================================

// StoredTwitterAPICredentials implementation
WootzScrapingService::StoredTwitterAPICredentials::StoredTwitterAPICredentials() = default;
WootzScrapingService::StoredTwitterAPICredentials::~StoredTwitterAPICredentials() = default;

void WootzScrapingService::StoredTwitterAPICredentials::Clear() {
  url.clear();
  method.clear();
  headers.clear();
  body.clear();
  stored_time = base::Time();
  is_valid = false;
}

void WootzScrapingService::ClearExpiredCredentials() {
  LOG(INFO) << "Aaditesh -> 🧹 Checking for expired Twitter API credentials";
  
  if (!profile_prefs_) {
    LOG(WARNING) << "Aaditesh -> PrefService not available for credential cleanup";
    return;
  }
  
  // Check if we have credentials in memory or storage
  if (!HasStoredTwitterAPICredentials()) {
    // Try loading from storage if not in memory (but don't crash if it fails)
    LoadCredentialsFromStorage();
  }
  
  if (!stored_twitter_credentials_.is_valid) {
    LOG(INFO) << "Aaditesh -> No valid credentials to check for expiration";
    return;
  }
    
  // Check credential age (credentials older than 24 hours are considered expired)
  base::TimeDelta age = base::Time::Now() - stored_twitter_credentials_.stored_time;
  if (age > base::Hours(24)) {
    LOG(INFO) << "Aaditesh -> ⚠️ Credentials are " << age.InHours() << " hours old - clearing expired credentials";
    
    // Clear from memory
    stored_twitter_credentials_.Clear();
    
    // Clear from Chrome Storage
    profile_prefs_->ClearPref(kWootzScrapingTwitterAPICredentials);
    
    LOG(INFO) << "Aaditesh -> ✅ Expired Twitter API credentials cleared from Chrome Storage";
    
    #if BUILDFLAG(IS_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                       "🧹 Expired Twitter API credentials cleared (age: %d hours)",
                       static_cast<int>(age.InHours()));
    #endif
  } else {
    LOG(INFO) << "Aaditesh -> ✅ Credentials are still valid (age: " << age.InMinutes() << " minutes)";
  }
}

void WootzScrapingService::DebugTestCredentialStorage() {
  LOG(INFO) << "Aaditesh -> 🔧 DEBUG: Testing credential storage functionality";
  
  if (!profile_prefs_) {
    LOG(ERROR) << "Aaditesh -> ❌ DEBUG: PrefService not available";
    return;
  }
  
  // Test storing some dummy credentials
  LOG(INFO) << "Aaditesh -> 📝 DEBUG: Storing test credentials...";
  
  stored_twitter_credentials_.Clear();
  stored_twitter_credentials_.url = "https://x.com/i/api/graphql/test";
  stored_twitter_credentials_.method = "POST";
  stored_twitter_credentials_.headers = "authorization: Bearer test_token\ncookie: test_cookie=value";
  stored_twitter_credentials_.body = "{\"test\": \"data\"}";
  stored_twitter_credentials_.stored_time = base::Time::Now();
  stored_twitter_credentials_.is_valid = true;
  
  LOG(INFO) << "Aaditesh -> 💾 DEBUG: Saving test credentials to Chrome Storage...";
  SaveCredentialsToStorage();
  
  // Clear memory and test loading
  LOG(INFO) << "Aaditesh -> 🧹 DEBUG: Clearing memory and testing load from storage...";
  stored_twitter_credentials_.Clear();
  
  LoadCredentialsFromStorage();
  
  if (HasStoredTwitterAPICredentials()) {
    LOG(INFO) << "Aaditesh -> ✅ DEBUG: Successfully loaded credentials from Chrome Storage!";
    LOG(INFO) << "Aaditesh -> 📊 DEBUG: Loaded URL: " << stored_twitter_credentials_.url;
    LOG(INFO) << "Aaditesh -> 📊 DEBUG: Loaded Method: " << stored_twitter_credentials_.method;
    LOG(INFO) << "Aaditesh -> 📊 DEBUG: Loaded Headers length: " << stored_twitter_credentials_.headers.length();
    LOG(INFO) << "Aaditesh -> 📊 DEBUG: Loaded Body length: " << stored_twitter_credentials_.body.length();
    
    base::TimeDelta age = base::Time::Now() - stored_twitter_credentials_.stored_time;
    LOG(INFO) << "Aaditesh -> 📊 DEBUG: Credential age: " << age.InSeconds() << " seconds";
  } else {
    LOG(ERROR) << "Aaditesh -> ❌ DEBUG: Failed to load credentials from Chrome Storage!";
  }
  
  LOG(INFO) << "Aaditesh -> 🔧 DEBUG: Credential storage test completed";
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "🔧 DEBUG: Credential storage test completed - Success: %s",
                     HasStoredTwitterAPICredentials() ? "YES" : "NO");
  #endif
}

void WootzScrapingService::PostConstructionInitialize() {
  LOG(INFO) << "WootzScrapingService: PostConstructionInitialize called";
  
  // Only perform initialization if service is properly constructed
  if (!delegate_ || !profile_prefs_) {
    LOG(WARNING) << "WootzScrapingService: Service not properly initialized, skipping post-construction setup";
    return;
  }
  
  LOG(INFO) << "WootzScrapingService: Performing post-construction credential operations";
  
  // Now safely load credentials and perform cleanup
  LoadCredentialsFromStorage();
  ClearExpiredCredentials();
  
  LOG(INFO) << "WootzScrapingService: Post-construction initialization complete";
}

}  // namespace wootz_scraping
