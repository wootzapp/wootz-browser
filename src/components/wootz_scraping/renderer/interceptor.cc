#include "components/wootz_scraping/renderer/interceptor.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/values.h"
#include "build/build_config.h"
#include "components/wootz_scraping/renderer/resource_helper.h"
#include "components/wootz_scraping/renderer/v8_helper.h"
#include "components/wootz_scraping/common/wootz_scraping.mojom.h"
#include "content/public/common/isolated_world_ids.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/user_agent.h"
#include "base/time/time.h"
#include "base/rand_util.h"
#include "base/base64.h"
#include "base/strings/stringprintf.h"
#include "gin/arguments.h"
#include "gin/converter.h"
#include "gin/object_template_builder.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_access_result.h"
#include "net/cookies/cookie_options.h"
#include "net/base/net_errors.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "third_party/blink/public/common/browser_interface_broker_proxy.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_local_frame_client.h"
#include "third_party/blink/public/web/web_console_message.h"
#include "third_party/blink/public/web/web_script_source.h"
#include "url/gurl.h"

// Network request headers for actual API calls
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/system/simple_watcher.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "net/http/http_response_headers.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"

// Additional headers for curl functionality
#include <iomanip>
#include <sstream>
#include <ctime>
#include <map>

#if BUILDFLAG(IS_ANDROID)
#include <android/log.h>
#endif

namespace wootz_scraping {

// Static map to track existing interceptors per frame to prevent duplicates
static std::map<content::RenderFrame*, TwitterInterceptor*> g_frame_interceptors;

  
// TwitterScrapingBridge implementation
gin::WrapperInfo TwitterScrapingBridge::kWrapperInfo = {gin::kEmbedderNativeGin};

// StoredAPICredentials implementation
TwitterScrapingBridge::StoredAPICredentials::StoredAPICredentials() = default;
TwitterScrapingBridge::StoredAPICredentials::~StoredAPICredentials() = default;

void TwitterScrapingBridge::StoredAPICredentials::Clear() {
  complete_url.clear();
  headers.clear();
  cookies.clear();
  bearer_token.clear();
  csrf_token.clear();
  transaction_id.clear();
  is_valid = false;
}

TwitterScrapingBridge::TwitterScrapingBridge(TwitterInterceptor* interceptor) 
    : interceptor_(interceptor) {
  LOG(INFO) << "Aaditesh -> TwitterScrapingBridge created";
  LOG(INFO) << "Aaditesh -> Bridge methods being exposed: onDataCaptured, onHomeTimelineUrlCaptured, isReady";
}

TwitterScrapingBridge::~TwitterScrapingBridge() {
  LOG(INFO) << "Aaditesh -> TwitterScrapingBridge destructor called";
  
  // Ensure Mojom connection is properly disconnected
  if (wootz_scraping_service_.is_bound()) {
    wootz_scraping_service_.reset();
  }
  
  // Invalidate weak pointers to prevent dangling callbacks
  weak_factory_.InvalidateWeakPtrs();
}

gin::ObjectTemplateBuilder TwitterScrapingBridge::GetObjectTemplateBuilder(
    v8::Isolate* isolate) {
  return gin::Wrappable<TwitterScrapingBridge>::GetObjectTemplateBuilder(isolate)
      .SetMethod("onDataCaptured", &TwitterScrapingBridge::OnDataCaptured)
      .SetMethod("onHomeTimelineUrlCaptured", &TwitterScrapingBridge::OnHomeTimelineUrlCaptured)
      .SetMethod("isReady", &TwitterScrapingBridge::IsReady)
      .SetMethod("callAPIWithStoredCredentials", &TwitterScrapingBridge::CallAPIWithStoredCredentials);
}

const char* TwitterScrapingBridge::GetTypeName() {
  return "TwitterScrapingBridge";
}

void TwitterScrapingBridge::OnDataCaptured(const std::string& data) {
  // SAFETY: Prevent recursive calls
  if (is_processing_data_) {
    LOG(WARNING) << "Aaditesh -> OnDataCaptured already processing, skipping to prevent recursion";
    return;
  }
  
  is_processing_data_ = true;
  
  // SAFETY: Limit input data size to prevent memory issues
  if (data.length() > 1000000) {  // 1MB limit
    LOG(ERROR) << "Aaditesh -> Input data too large (" << data.length() << " bytes), truncating for safety";
    is_processing_data_ = false;
    return;
  }
  
  LOG(INFO) << "Aaditesh -> Raw data received from JavaScript: " << data.substr(0, 200) << "... (truncated for safety)";
  
  // Try to parse the JSON data for structured logging
  auto parsed_json = base::JSONReader::Read(data);
  if (parsed_json && parsed_json->is_dict()) {
    const base::Value::Dict& dict = parsed_json->GetDict();
    
    // Extract key information for structured logging
    const std::string* type = dict.FindString("type");
    const std::string* timestamp = dict.FindString("timestamp");
    const std::string* url = dict.FindString("url");
    
    LOG(INFO) << "Aaditesh -> Parsed Twitter Data - Type: " 
              << (type ? *type : "unknown")
              << ", URL: " << (url ? *url : "unknown")
              << ", Timestamp: " << (timestamp ? *timestamp : "unknown");
    
    // ENHANCED: Handle HomeTimeline Complete URL capture
    if (type && *type == "HomeTimeline_Complete_URL") {
      LOG(INFO) << "Aaditesh -> ===== HOMETIMELINE COMPLETE URL CAPTURED =====";
      
      if (url) {
        const std::string complete_url = *url;
        
        // Log the complete URL details
        LOG(INFO) << "Aaditesh -> 🎯 COMPLETE HOMETIMELINE URL: " << complete_url;
        LOG(INFO) << "Aaditesh -> 📏 URL LENGTH: " << complete_url.length() << " characters";
        
        // Check if URL contains features and variables
        std::optional<bool> has_features = dict.FindBool("hasFeatures");
        std::optional<bool> has_variables = dict.FindBool("hasVariables");
        std::optional<int> url_length = dict.FindInt("urlLength");
        
        LOG(INFO) << "Aaditesh -> 🔍 URL ANALYSIS:";
        LOG(INFO) << "  - Contains features: " << (has_features.value_or(false) ? "✅ YES" : "❌ NO");
        LOG(INFO) << "  - Contains variables: " << (has_variables.value_or(false) ? "✅ YES" : "❌ NO");
        LOG(INFO) << "  - Reported length: " << (url_length ? std::to_string(url_length.value()) : "unknown");
        
        // Extract domain and endpoint from URL for validation
        if (complete_url.find("x.com") != std::string::npos || complete_url.find("twitter.com") != std::string::npos) {
          LOG(INFO) << "Aaditesh -> ✅ VALID TWITTER DOMAIN detected in URL";
        } else {
          LOG(WARNING) << "Aaditesh -> ⚠️ UNEXPECTED DOMAIN in URL";
        }
        
        if (complete_url.find("HomeTimeline") != std::string::npos) {
          LOG(INFO) << "Aaditesh -> ✅ HOMETIMELINE ENDPOINT confirmed in URL";
        } else {
          LOG(WARNING) << "Aaditesh -> ⚠️ HomeTimeline endpoint not found in URL";
        }
        
        // Store the URL for potential API usage (don't parse, just store as-is)
        hometimeline_complete_url_ = complete_url;
        
        // Log to Android console for real-time monitoring
        LogToAndroidConsole("🎯 HOMETIMELINE URL: " + complete_url.substr(0, 100) + "...");
        LogToAndroidConsole("📊 URL STATS: " + std::to_string(complete_url.length()) + " chars, " +
                           "Features: " + (has_features.value_or(false) ? "YES" : "NO") + ", " +
                           "Variables: " + (has_variables.value_or(false) ? "YES" : "NO"));
        
        LOG(INFO) << "Aaditesh -> ✅ COMPLETE URL STORED FOR API USAGE (NO PARSING)";
        LOG(INFO) << "Aaditesh -> ===== HOMETIMELINE URL CAPTURE COMPLETE =====";
      } else {
        LOG(ERROR) << "Aaditesh -> ❌ HomeTimeline URL type received but no URL found in data";
      }
    }
    
    // Log token status and validate
    const base::Value::Dict* tokens = dict.FindDict("tokens");
    if (tokens) {
      const std::string* csrf_token = tokens->FindString("csrf_token");
      
      // CRITICAL FIX: Store the CSRF token in the member variable
      if (csrf_token && !csrf_token->empty()) {
        csrf_token_ = *csrf_token;
        LOG(INFO) << "Aaditesh -> ✅ STORED CSRF token from tokens: " << csrf_token_.substr(0, 20) << "...";
      }
      
      // Also store other available tokens from the tokens object
      const std::string* guest_id = tokens->FindString("guest_id");
      if (guest_id && !guest_id->empty() && guest_id_.empty()) {
        guest_id_ = *guest_id;
        LOG(INFO) << "Aaditesh -> ✅ STORED guest_id from tokens: " << guest_id_.substr(0, 20) << "...";
      }
      
      const std::string* bearer_token = tokens->FindString("bearer_token");
      if (bearer_token && !bearer_token->empty() && js_bearer_token_.empty()) {
        js_bearer_token_ = *bearer_token;
        LOG(INFO) << "Aaditesh -> ✅ STORED bearer_token from tokens";
      }
      
      LOG(INFO) << "Aaditesh -> Token Status - CSRF: " 
                << (csrf_token && !csrf_token->empty() ? "✓" : "✗");
      
      // Check if we have minimum required tokens for cookie retrieval
      bool has_csrf = csrf_token && !csrf_token->empty();
      
      
      if (has_csrf) {
        LOG(INFO) << "Aaditesh -> Minimum tokens present, HttpOnly cookie retrieval will be triggered";
        
        // Trigger browser-side HttpOnly cookie scraping when we have the required tokens
        LOG(INFO) << "Aaditesh -> ===== TRIGGERING HTTPONLY COOKIE SCRAPING =====";
        LOG(INFO) << "Aaditesh -> CSRF Token Present: YES";
        LOG(INFO) << "Aaditesh -> About to call CallBrowserSideTokenScraping()...";
        
        // Call browser-side token scraping for HttpOnly cookies
        CallBrowserSideTokenScraping();
        
        LOG(INFO) << "Aaditesh -> CallBrowserSideTokenScraping() method called successfully";
      } else {
        LOG(INFO) << "Aaditesh -> Insufficient tokens for HttpOnly cookie retrieval, skipping";
      }
    }

    const base::Value::Dict* cookies = dict.FindDict("cookies");
    if (cookies) {
      LOG(INFO) << "Aaditesh -> ===== COMPREHENSIVE COOKIE PROCESSING START =====";
      
      // Extract complete cookie string
      const std::string* complete_cookie_string = cookies->FindString("complete_cookie_string");
      if (complete_cookie_string) {
        js_accessible_cookies_ = *complete_cookie_string;
        LOG(INFO) << "Aaditesh -> Complete cookie string stored: " << complete_cookie_string->length() << " characters";
        LOG(INFO) << "Aaditesh -> Complete cookie preview: " << complete_cookie_string->substr(0, 100) << "...";
      }
      
      // Extract total cookie count
      std::optional<int> total_count = cookies->FindInt("total_cookie_count");
      if (total_count) {
        total_cookie_count_ = total_count.value();
        LOG(INFO) << "Aaditesh -> Total cookie count: " << total_cookie_count_;
      }
      
      // Extract all individual cookies from all_cookies dict
      const base::Value::Dict* all_cookies = cookies->FindDict("all_cookies");
      if (all_cookies) {
        LOG(INFO) << "Aaditesh -> Processing all_cookies dictionary:";
        
        // Extract each cookie individually
        const std::string* guest_id_marketing = all_cookies->FindString("guest_id_marketing");
        if (guest_id_marketing) {
          guest_id_marketing_ = *guest_id_marketing;
          LOG(INFO) << "Aaditesh -> guest_id_marketing: " << guest_id_marketing_.substr(0, 20) << "...";
        }
        
        const std::string* guest_id_ads = all_cookies->FindString("guest_id_ads");
        if (guest_id_ads) {
          guest_id_ads_ = *guest_id_ads;
          LOG(INFO) << "Aaditesh -> guest_id_ads: " << guest_id_ads_.substr(0, 20) << "...";
        }
        
        const std::string* guest_id = all_cookies->FindString("guest_id");
        if (guest_id) {
          guest_id_ = *guest_id;
          LOG(INFO) << "Aaditesh -> guest_id: " << guest_id_.substr(0, 20) << "...";
        }
        
        const std::string* gt = all_cookies->FindString("gt");
        if (gt) {
          gt_token_ = *gt;
          LOG(INFO) << "Aaditesh -> gt token: " << gt_token_.substr(0, 20) << "...";
        }
        
        const std::string* personalization_id = all_cookies->FindString("personalization_id");
        if (personalization_id) {
          personalization_id_ = *personalization_id;
          LOG(INFO) << "Aaditesh -> personalization_id: " << personalization_id_.substr(0, 20) << "...";
        }
        
        const std::string* ct0 = all_cookies->FindString("ct0");
        if (ct0) {
          ct0_token_ = *ct0;
          csrf_token_ = *ct0; // ct0 is the CSRF token
          LOG(INFO) << "Aaditesh -> ct0/CSRF token: " << ct0_token_.substr(0, 20) << "...";
        }
        
        const std::string* lang = all_cookies->FindString("lang");
        if (lang) {
          lang_setting_ = *lang;
          LOG(INFO) << "Aaditesh -> lang: " << lang_setting_;
        }
        
        const std::string* twid = all_cookies->FindString("twid");
        if (twid) {
          twid_ = *twid;
          LOG(INFO) << "Aaditesh -> twid: " << twid_.substr(0, 20) << "...";
        }
      }
      
      // // Extract Twitter authentication cookies for detailed analysis
      // const base::Value::Dict* twitter_auth_cookies = cookies->FindDict("twitter_auth_cookies");
      // if (twitter_auth_cookies) {
      //   LOG(INFO) << "Aaditesh -> Processing twitter_auth_cookies dictionary:";
        
      //   // Check for auth_token (this might be null for non-authenticated users)
      //   const std::string* auth_token = twitter_auth_cookies->FindString("auth_token");
      //   if (auth_token) {
      //     auth_token_ = *auth_token;
      //     LOG(INFO) << "Aaditesh -> ✅ AUTH TOKEN FOUND: " << auth_token_.substr(0, 20) << "...";
      //   } else {
      //     LOG(INFO) << "Aaditesh -> ⚠️ auth_token is null (guest user or not fully authenticated)";
      //   }
        
      //   // Check for additional authentication cookies
      //   const std::string* kdt = twitter_auth_cookies->FindString("kdt");
      //   if (kdt) {
      //     kdt_token_ = *kdt;
      //     LOG(INFO) << "Aaditesh -> kdt token: " << kdt_token_.substr(0, 20) << "...";
      //   }
        
      //   const std::string* att = twitter_auth_cookies->FindString("att");
      //   if (att) {
      //     att_token_ = *att;
      //     LOG(INFO) << "Aaditesh -> att token: " << att_token_.substr(0, 20) << "...";
      //   }
        
      //   // Check for additional tracking cookies
      //   const std::string* cf_bm = twitter_auth_cookies->FindString("__cf_bm");
      //   if (cf_bm) {
      //     cf_bm_token_ = *cf_bm;
      //     LOG(INFO) << "Aaditesh -> __cf_bm token: " << cf_bm_token_.substr(0, 20) << "...";
      //   }
        
      //   const std::string* ga = twitter_auth_cookies->FindString("_ga");
      //   if (ga) {
      //     ga_token_ = *ga;
      //     LOG(INFO) << "Aaditesh -> _ga token: " << ga_token_.substr(0, 20) << "...";
      //   }
        
      //   const std::string* gid = twitter_auth_cookies->FindString("_gid");
      //   if (gid) {
      //     gid_token_ = *gid;
      //     LOG(INFO) << "Aaditesh -> _gid token: " << gid_token_.substr(0, 20) << "...";
      //   }
        
      //   const std::string* dnt = twitter_auth_cookies->FindString("dnt");
      //   if (dnt) {
      //     dnt_setting_ = *dnt;
      //     LOG(INFO) << "Aaditesh -> dnt setting: " << dnt_setting_;
      //   }
        
      //   const std::string* eu_cn = twitter_auth_cookies->FindString("eu_cn");
      //   if (eu_cn) {
      //     eu_cn_setting_ = *eu_cn;
      //     LOG(INFO) << "Aaditesh -> eu_cn setting: " << eu_cn_setting_;
      //   }
        
      //   const std::string* night_mode = twitter_auth_cookies->FindString("night_mode");
      //   if (night_mode) {
      //     night_mode_setting_ = *night_mode;
      //     LOG(INFO) << "Aaditesh -> night_mode: " << night_mode_setting_;
      //   }
        
      //   const std::string* rweb_optin = twitter_auth_cookies->FindString("rweb_optin");
      //   if (rweb_optin) {
      //     rweb_optin_setting_ = *rweb_optin;
      //     LOG(INFO) << "Aaditesh -> rweb_optin: " << rweb_optin_setting_;
      //   }
      // }
      
      // Extract parsing information for validation
      // const base::Value::Dict* parsing_info = cookies->FindDict("parsing_info");
      // if (parsing_info) {
      //   LOG(INFO) << "Aaditesh -> Processing parsing_info:";
        
      //   std::optional<int> raw_cookie_length = parsing_info->FindInt("raw_cookie_length");
      //   std::optional<int> parsed_cookies_count = parsing_info->FindInt("parsed_cookies_count");
      //   std::optional<int> complete_string_length = parsing_info->FindInt("complete_string_length");
      //   const std::string* parsing_timestamp = parsing_info->FindString("timestamp");
        
      //   if (raw_cookie_length) {
      //     LOG(INFO) << "Aaditesh -> Raw cookie length: " << raw_cookie_length.value();
      //   }
      //   if (parsed_cookies_count) {
      //     LOG(INFO) << "Aaditesh -> Parsed cookies count: " << parsed_cookies_count.value();
      //   }
      //   if (complete_string_length) {
      //     LOG(INFO) << "Aaditesh -> Complete string length: " << complete_string_length.value();
      //   }
      //   if (parsing_timestamp) {
      //     cookie_parsing_timestamp_ = *parsing_timestamp;
      //     LOG(INFO) << "Aaditesh -> Cookie parsing timestamp: " << cookie_parsing_timestamp_;
      //   }
      // }
      
      
      

      
      // Update tokens_ready status based on critical authentication tokens
      tokens_ready_ = !ct0_token_.empty() && !guest_id_.empty();
      LOG(INFO) << "Aaditesh -> 🎯 TOKENS READY FOR API CALLS: " << (tokens_ready_ ? "✅ YES" : "❌ NO");
      
      // Log to Android console for monitoring
      LogToAndroidConsole("🍪 COMPREHENSIVE COOKIES PROCESSED:");
      LogToAndroidConsole("✅ Total cookies: " + std::to_string(total_cookie_count_));
      LogToAndroidConsole("✅ Complete string: " + std::to_string(js_accessible_cookies_.length()) + " chars");
      LogToAndroidConsole("🔑 Auth ready: " + std::string(tokens_ready_ ? "YES" : "NO"));
      LogToAndroidConsole("🔑 Key tokens - CSRF: " + std::string(!ct0_token_.empty() ? "✓" : "✗") + 
                         ", Guest: " + std::string(!guest_id_.empty() ? "✓" : "✗") + 
                         ", Auth: " + std::string(!auth_token_.empty() ? "✓" : "✗"));
      
      LOG(INFO) << "Aaditesh -> ===== COMPREHENSIVE COOKIE PROCESSING COMPLETE =====";
    }

    // Extract and process all headers for API calls
    const base::Value::Dict* headers = dict.FindDict("headers");
    if (headers) {
      LOG(INFO) << "Aaditesh -> ===== COMPREHENSIVE HEADER PROCESSING START =====";
      
      // Extract critical authentication headers
      const std::string* authorization = headers->FindString("authorization");
      if (authorization) {
        authorization_header_ = *authorization;
        LOG(INFO) << "Aaditesh -> Authorization header: " << authorization_header_.substr(0, 20) << "...";
      }
      
      const std::string* x_csrf_token = headers->FindString("x-csrf-token");
      if (x_csrf_token) {
        x_csrf_token_header_ = *x_csrf_token;
        LOG(INFO) << "Aaditesh -> X-CSRF-Token header: " << x_csrf_token_header_.substr(0, 20) << "...";
      }
      
      const std::string* x_client_transaction_id = headers->FindString("x-client-transaction-id");
      if (x_client_transaction_id) {
        x_client_transaction_id_header_ = *x_client_transaction_id;
        LOG(INFO) << "Aaditesh -> X-Client-Transaction-ID: " << x_client_transaction_id_header_.substr(0, 20) << "...";
      }
      
      // Extract Twitter-specific headers
      const std::string* x_twitter_active_user = headers->FindString("x-twitter-active-user");
      if (x_twitter_active_user) {
        x_twitter_active_user_ = *x_twitter_active_user;
        LOG(INFO) << "Aaditesh -> X-Twitter-Active-User: " << x_twitter_active_user_;
      }
      
      const std::string* x_twitter_auth_type = headers->FindString("x-twitter-auth-type");
      if (x_twitter_auth_type) {
        x_twitter_auth_type_ = *x_twitter_auth_type;
        LOG(INFO) << "Aaditesh -> X-Twitter-Auth-Type: " << x_twitter_auth_type_;
      }
      
      const std::string* x_twitter_client_language = headers->FindString("x-twitter-client-language");
      if (x_twitter_client_language) {
        x_twitter_client_language_ = *x_twitter_client_language;
        LOG(INFO) << "Aaditesh -> X-Twitter-Client-Language: " << x_twitter_client_language_;
      }
      
      // Extract standard HTTP headers
      const std::string* content_type = headers->FindString("content-type");
      if (content_type) {
        content_type_header_ = *content_type;
        LOG(INFO) << "Aaditesh -> Content-Type: " << content_type_header_;
      }
      
      const std::string* accept = headers->FindString("accept");
      if (accept) {
        accept_header_ = *accept;
        LOG(INFO) << "Aaditesh -> Accept: " << accept_header_;
      }
      
      const std::string* accept_language = headers->FindString("accept-language");
      if (accept_language) {
        accept_language_header_ = *accept_language;
        LOG(INFO) << "Aaditesh -> Accept-Language: " << accept_language_header_;
      }
      
      const std::string* user_agent = headers->FindString("user-agent");
      if (user_agent) {
        user_agent_header_ = *user_agent;
        LOG(INFO) << "Aaditesh -> User-Agent: " << user_agent_header_.substr(0, 50) << "...";
      }
      
      const std::string* origin = headers->FindString("origin");
      if (origin) {
        origin_header_ = *origin;
        LOG(INFO) << "Aaditesh -> Origin: " << origin_header_;
      }
      
      const std::string* referer = headers->FindString("referer");
      if (referer) {
        referer_header_ = *referer;
        LOG(INFO) << "Aaditesh -> Referer: " << referer_header_;
      }
      
      // Extract security headers
      const std::string* sec_fetch_dest = headers->FindString("sec-fetch-dest");
      if (sec_fetch_dest) {
        sec_fetch_dest_ = *sec_fetch_dest;
        LOG(INFO) << "Aaditesh -> Sec-Fetch-Dest: " << sec_fetch_dest_;
      }
      
      const std::string* sec_fetch_mode = headers->FindString("sec-fetch-mode");
      if (sec_fetch_mode) {
        sec_fetch_mode_ = *sec_fetch_mode;
        LOG(INFO) << "Aaditesh -> Sec-Fetch-Mode: " << sec_fetch_mode_;
      }
      
      const std::string* sec_fetch_site = headers->FindString("sec-fetch-site");
      if (sec_fetch_site) {
        sec_fetch_site_ = *sec_fetch_site;
        LOG(INFO) << "Aaditesh -> Sec-Fetch-Site: " << sec_fetch_site_;
      }
      
      // Log comprehensive header summary
      LOG(INFO) << "Aaditesh -> ===== COMPREHENSIVE HEADER SUMMARY =====";
      LOG(INFO) << "Aaditesh -> 🔐 AUTHENTICATION HEADERS:";
      LOG(INFO) << "  - Authorization: " << (!authorization_header_.empty() ? "✅ PRESENT" : "❌ MISSING");
      LOG(INFO) << "  - X-CSRF-Token: " << (!x_csrf_token_header_.empty() ? "✅ PRESENT" : "❌ MISSING");
      LOG(INFO) << "  - X-Client-Transaction-ID: " << (!x_client_transaction_id_header_.empty() ? "✅ PRESENT" : "❌ MISSING");
      
      LOG(INFO) << "Aaditesh -> 🐦 TWITTER-SPECIFIC HEADERS:";
      LOG(INFO) << "  - X-Twitter-Active-User: " << x_twitter_active_user_;
      LOG(INFO) << "  - X-Twitter-Auth-Type: " << x_twitter_auth_type_;
      LOG(INFO) << "  - X-Twitter-Client-Language: " << x_twitter_client_language_;
      
      LOG(INFO) << "Aaditesh -> 🌐 STANDARD HTTP HEADERS:";
      LOG(INFO) << "  - Content-Type: " << content_type_header_;
      LOG(INFO) << "  - Accept: " << accept_header_;
      LOG(INFO) << "  - Accept-Language: " << accept_language_header_;
      LOG(INFO) << "  - Origin: " << origin_header_;
      LOG(INFO) << "  - Referer: " << referer_header_;
      
      LOG(INFO) << "Aaditesh -> 🔒 SECURITY HEADERS:";
      LOG(INFO) << "  - Sec-Fetch-Dest: " << sec_fetch_dest_;
      LOG(INFO) << "  - Sec-Fetch-Mode: " << sec_fetch_mode_;
      LOG(INFO) << "  - Sec-Fetch-Site: " << sec_fetch_site_;
      
      // Validate header completeness for API calls
      bool headers_ready = !authorization_header_.empty() && !x_csrf_token_header_.empty() && 
                          !x_client_transaction_id_header_.empty() && !origin_header_.empty();
      LOG(INFO) << "Aaditesh -> 🎯 HEADERS READY FOR API CALLS: " << (headers_ready ? "✅ YES" : "❌ NO");
      
      // Log to Android console for monitoring
      LogToAndroidConsole("📋 COMPREHENSIVE HEADERS PROCESSED:");
      LogToAndroidConsole("🔐 Auth headers - Bearer: " + std::string(!authorization_header_.empty() ? "✓" : "✗") + 
                         ", CSRF: " + std::string(!x_csrf_token_header_.empty() ? "✓" : "✗") + 
                         ", Transaction: " + std::string(!x_client_transaction_id_header_.empty() ? "✓" : "✗"));
      LogToAndroidConsole("🐦 Twitter headers - Active: " + x_twitter_active_user_ + 
                         ", Auth-Type: " + x_twitter_auth_type_ + 
                         ", Language: " + x_twitter_client_language_);
      LogToAndroidConsole("🎯 Headers ready: " + std::string(headers_ready ? "YES" : "NO"));
      
      LOG(INFO) << "Aaditesh -> ===== COMPREHENSIVE HEADER PROCESSING COMPLETE =====";
    }

    // Extract and process captured browser headers from JavaScript payload
    const base::Value::Dict* captured_browser_headers = dict.FindDict("capturedBrowserHeaders");
    if (captured_browser_headers) {
      LOG(INFO) << "Aaditesh -> ===== CAPTURED BROWSER HEADERS PROCESSING START =====";
      
      // Extract x-xp-forwarded-for header captured by JavaScript
      const std::string* xp_forwarded_for = captured_browser_headers->FindString("x-xp-forwarded-for");
      if (xp_forwarded_for) {
        js_xp_forwarded_for_ = *xp_forwarded_for;
        LOG(INFO) << "Aaditesh -> ✅ X-XP-Forwarded-For header captured from browser: " << js_xp_forwarded_for_.substr(0, 30) << "...";
      } else {
        // Try alternative key format (camelCase)
        const std::string* xp_forwarded_for_alt = captured_browser_headers->FindString("xpForwardedFor");
        if (xp_forwarded_for_alt) {
          js_xp_forwarded_for_ = *xp_forwarded_for_alt;
          LOG(INFO) << "Aaditesh -> ✅ X-XP-Forwarded-For header captured from browser (alt format): " << js_xp_forwarded_for_.substr(0, 30) << "...";
        }
      }
      
      // Extract capture metadata for validation
      std::optional<int64_t> last_capture_time = captured_browser_headers->FindInt("lastCaptureTime");
      const std::string* capture_source = captured_browser_headers->FindString("captureSource");
      
      if (last_capture_time) {
        LOG(INFO) << "Aaditesh -> Header capture timestamp: " << last_capture_time.value();
      }
      if (capture_source) {
        LOG(INFO) << "Aaditesh -> Header capture source: " << *capture_source;
      }
      
      // Log captured browser headers summary
      LOG(INFO) << "Aaditesh -> 📡 CAPTURED BROWSER HEADERS SUMMARY:";
      LOG(INFO) << "  - X-XP-Forwarded-For: " << (!js_xp_forwarded_for_.empty() ? "✅ CAPTURED" : "❌ MISSING");
      
      // Log to Android console for monitoring
      LogToAndroidConsole("📡 CAPTURED BROWSER HEADERS:");
      LogToAndroidConsole("✅ X-XP-Forwarded-For: " + std::string(!js_xp_forwarded_for_.empty() ? "CAPTURED" : "MISSING"));
      
      LOG(INFO) << "Aaditesh -> ===== CAPTURED BROWSER HEADERS PROCESSING COMPLETE =====";
    } else {
      LOG(INFO) << "Aaditesh -> ⚠️ No capturedBrowserHeaders found in payload";
    }

    
    // // Log validation status
    // const base::Value::Dict* validation = dict.FindDict("validation");
    // if (validation) {
    //   bool required_tokens_available = validation->FindBool("required_tokens_available").value_or(false);
    //   bool all_tokens_for_logged_user = validation->FindBool("all_tokens_for_logged_user").value_or(false);
    //   bool data_sent_to_bridge = validation->FindBool("data_sent_to_bridge").value_or(false);
      
    //   LOG(INFO) << "Aaditesh -> Validation Status - Required Tokens: " 
    //             << (required_tokens_available ? "YES" : "NO")
    //             << ", Complete User Data: " << (all_tokens_for_logged_user ? "YES" : "NO")
    //             << ", Data Sent: " << (data_sent_to_bridge ? "YES" : "NO");
      
    //   if (!required_tokens_available) {
    //     LOG(WARNING) << "Aaditesh -> WARNING: Data received but required tokens were missing!";
    //     should_retrieve_cookies = false;  // Don't retrieve cookies if validation fails
    //   }
      
    //   // Additional check: only retrieve cookies if tokens are validated as available
    //   if (required_tokens_available && should_retrieve_cookies) {
    //     LOG(INFO) << "Aaditesh -> Validation passed, confirming HttpOnly cookie retrieval";
    //   }
    // }
    
    // // Log ready status and check login
    // const base::Value::Dict* status = dict.FindDict("status");
    // if (status) {
    //   bool logged_in = status->FindBool("logged_in").value_or(false);
    //   bool ready_for_api = status->FindBool("ready_for_api").value_or(false);
      
    //   LOG(INFO) << "Aaditesh -> User Status - Logged In: " 
    //             << (logged_in ? "YES" : "NO")
    //             << ", API Ready: " << (ready_for_api ? "YES" : "NO");
      
    //   // Final check: only retrieve cookies if user is logged in
    //   if (!logged_in) {
    //     should_retrieve_cookies = false;
    //     LOG(INFO) << "Aadi_Kadu -> User not logged in, skipping HttpOnly cookie retrieval";
    //   } else if (should_retrieve_cookies) {
    //     // User is logged in and we have required tokens - trigger browser-side HttpOnly cookie scraping
    //     LOG(INFO) << "Aadi_Kadu -> ===== FLOW START: User authenticated with required tokens =====";
    //     LOG(INFO) << "Aadi_Kadu -> About to call browser-side HttpOnly cookie scraping via Mojom";
    //     LOG(INFO) << "Aadi_Kadu -> CSRF Token Present: YES";
    //     LOG(INFO) << "Aadi_Kadu -> Guest ID Present: YES"; 
    //     LOG(INFO) << "Aadi_Kadu -> User Logged In: YES";
    //     LOG(INFO) << "Aadi_Kadu -> Triggering CallBrowserSideTokenScraping() now...";
        
    //     // RE-ENABLED: Call browser-side token scraping
    //     CallBrowserSideTokenScraping();
        
    //     LOG(INFO) << "Aadi_Kadu -> CallBrowserSideTokenScraping() method called successfully";
    //   }
    // }

    // Process the data without HttpOnly cookies (not accessible from renderer)
    LOG(INFO) << "Aaditesh -> Processing Twitter data (HttpOnly cookies not accessible from renderer)";
    ProcessTwitterData(data);
  } else {
    LOG(WARNING) << "Aaditesh -> Failed to parse JSON data, logging as raw text (no cookie retrieval)";
    LogToAndroidConsole("Twitter Data Captured (unparsed): " + data);
  }
  
  // SAFETY: Reset processing flag
  is_processing_data_ = false;
}

bool TwitterScrapingBridge::IsReady() {
  LOG(INFO) << "Aaditesh -> Bridge readiness check called from JavaScript";
  return true; // Bridge is always ready once created
}

void TwitterScrapingBridge::Cleanup() {
  LOG(INFO) << "Aaditesh -> TwitterScrapingBridge cleanup called";
  
  // Invalidate weak pointers to prevent dangling callbacks
  weak_factory_.InvalidateWeakPtrs();
  
  // Disconnect Mojom connection
  if (wootz_scraping_service_.is_bound()) {
    wootz_scraping_service_.reset();
  }
}

void TwitterScrapingBridge::LogToAndroidConsole(const std::string& message) {
#if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", "%s", message.c_str());
#endif
}

void TwitterScrapingBridge::LogHomeTimelineUrl(const std::string& url, const std::string& tokens) {
#if BUILDFLAG(IS_ANDROID)
  // Enhanced logging for complete URL capture
  __android_log_print(ANDROID_LOG_INFO, "cr_WootzTwitter", "🎯 COMPLETE HomeTimeline URL Captured: %s", url.c_str());
  __android_log_print(ANDROID_LOG_INFO, "WootzHomeTimeline", "📏 URL Length: %zu characters", url.length());
  __android_log_print(ANDROID_LOG_INFO, "WootzHomeTimeline", "🔍 Contains Features: %s", 
                     (url.find("features=") != std::string::npos) ? "YES" : "NO");
  __android_log_print(ANDROID_LOG_INFO, "WootzHomeTimeline", "🔍 Contains Variables: %s", 
                     (url.find("variables=") != std::string::npos) ? "YES" : "NO");
  __android_log_print(ANDROID_LOG_INFO, "WootzHomeTimeline", "📊 Tokens: %s", tokens.c_str());
  
  // Generate timestamp for logging
  std::time_t now = std::time(nullptr);
  char timestamp[100];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
  __android_log_print(ANDROID_LOG_INFO, "WootzHomeTimeline", "📅 Captured at: %s", timestamp);
  
  // Log URL preview (first 200 characters)
  std::string url_preview = url.length() > 200 ? url.substr(0, 200) + "..." : url;
  __android_log_print(ANDROID_LOG_INFO, "WootzHomeTimeline", "📋 URL Preview: %s", url_preview.c_str());
#endif
}

void TwitterScrapingBridge::OnHomeTimelineUrlCaptured(const std::string& url, const std::string& tokens) {
  LOG(INFO) << "Aaditesh -> ===== COMPLETE HOMETIMELINE URL CAPTURED VIA OnHomeTimelineUrlCaptured =====";
  LOG(INFO) << "Aaditesh -> 🎯 COMPLETE URL: " << url;
  LOG(INFO) << "Aaditesh -> 📏 URL LENGTH: " << url.length() << " characters";
  
  // Enhanced validation: Check for both 'variables=' and 'features=' parameters
  bool has_features = url.find("features=") != std::string::npos;
  bool has_variables = url.find("variables=") != std::string::npos;
  
  LOG(INFO) << "Aaditesh -> 🔍 ENHANCED URL VALIDATION:";
  LOG(INFO) << "Aaditesh -> 🔍 Contains features: " << (has_features ? "YES" : "NO");
  LOG(INFO) << "Aaditesh -> 🔍 Contains variables: " << (has_variables ? "YES" : "NO");
  
  // Additional content validation: Ensure parameters have meaningful content
  bool valid_features = false;
  bool valid_variables = false;
  
  if (has_features) {
    size_t features_pos = url.find("features=");
    size_t start = features_pos + 9; // "features=" length
    size_t end = url.find("&", start);
    if (end == std::string::npos) end = url.length();
    
    std::string features_content = url.substr(start, end - start);
    valid_features = !features_content.empty() && features_content.length() > 10;
    LOG(INFO) << "Aaditesh -> 🔍 Features content length: " << features_content.length() << " chars";
  }
  
  if (has_variables) {
    size_t variables_pos = url.find("variables=");
    size_t start = variables_pos + 10; // "variables=" length
    size_t end = url.find("&", start);
    if (end == std::string::npos) end = url.length();
    
    std::string variables_content = url.substr(start, end - start);
    valid_variables = !variables_content.empty() && variables_content.length() > 5;
    LOG(INFO) << "Aaditesh -> 🔍 Variables content length: " << variables_content.length() << " chars";
  }
  
  bool has_valid_required_params = has_features && has_variables && valid_features && valid_variables;
  
  LOG(INFO) << "Aaditesh -> 🎯 VALIDATION RESULT:";
  LOG(INFO) << "  - Features content valid: " << (valid_features ? "✅ YES" : "❌ NO");
  LOG(INFO) << "  - Variables content valid: " << (valid_variables ? "✅ YES" : "❌ NO"); 
  LOG(INFO) << "  - Overall validation: " << (has_valid_required_params ? "✅ PASS" : "❌ FAIL");
  
  if (!has_valid_required_params) {
    LOG(WARNING) << "Aaditesh -> ❌ URL VALIDATION FAILED - URL will NOT be stored";
    LogToAndroidConsole("❌ HomeTimeline URL validation failed - missing required parameters");
    
    if (!has_features) {
      LOG(WARNING) << "  - Missing 'features=' parameter in URL";
      LogToAndroidConsole("  ❌ Missing 'features=' parameter");
    } else if (!valid_features) {
      LOG(WARNING) << "  - 'features=' parameter present but content invalid/empty";
      LogToAndroidConsole("  ❌ 'features=' parameter has insufficient content");
    }
    
    if (!has_variables) {
      LOG(WARNING) << "  - Missing 'variables=' parameter in URL";
      LogToAndroidConsole("  ❌ Missing 'variables=' parameter");
    } else if (!valid_variables) {
      LOG(WARNING) << "  - 'variables=' parameter present but content invalid/empty";
      LogToAndroidConsole("  ❌ 'variables=' parameter has insufficient content");
    }
    
    LogToAndroidConsole("🔄 URL will only be accepted when BOTH parameters are present with valid content");
    LOG(INFO) << "Aaditesh -> ===== URL VALIDATION FAILED - NOT STORED =====";
    return; // Early return - do not store invalid URL
  }
  
  // URL passed validation - proceed with storage
  LOG(INFO) << "Aaditesh -> ✅ URL VALIDATION PASSED - Storing URL for API usage";
  
  LogHomeTimelineUrl(url, tokens);
  LogToAndroidConsole("🎯 HomeTimeline COMPLETE URL captured and validated: " + url.substr(0, 100) + "...");
  LogToAndroidConsole("📊 URL Stats: " + std::to_string(url.length()) + " chars, " +
                     "Features: " + (has_features ? "YES" : "NO") + ", " +
                     "Variables: " + (has_variables ? "YES" : "NO"));
  
  // Store the complete URL for potential API usage (NO PARSING - just store as-is)
  hometimeline_complete_url_ = url;
  
  // Store JavaScript-accessible cookies for later combination
  LOG(INFO) << "Aaditesh -> ===== STORING JAVASCRIPT COOKIES =====";
  LOG(INFO) << "Aaditesh -> JavaScript cookie string length: " << tokens.length();
  LOG(INFO) << "Aaditesh -> JavaScript cookies preview: " << tokens.substr(0, 100) << "...";
  
  // Store the JavaScript cookies in a dedicated member variable
  js_accessible_cookies_ = tokens;
  
  LogToAndroidConsole("✅ JavaScript cookies stored for combination: " + 
                     std::to_string(tokens.length()) + " characters");
  LOG(INFO) << "Aaditesh -> ✅ VALIDATED URL STORED FOR API USAGE (NO PARSING)";
  LOG(INFO) << "Aaditesh -> ===== JAVASCRIPT COOKIES STORED =====";
}

void TwitterScrapingBridge::ProcessTwitterData(const std::string& js_data) {
  LOG(INFO) << "Aaditesh -> Processing Twitter data from JavaScript string";
  
  // Parse the JSON string
  auto parsed_json = base::JSONReader::Read(js_data);
  if (!parsed_json || !parsed_json->is_dict()) {
    LOG(ERROR) << "Aaditesh -> Failed to parse Twitter data JSON";
    return;
  }
  
  // Call the Dict version with parsed data
  ProcessTwitterData(parsed_json->GetDict().Clone());
}

void TwitterScrapingBridge::ProcessTwitterData(base::Value::Dict data) {
  if (!interceptor_) {
    LOG(ERROR) << "Twitter API: Cannot process data - interceptor not available";
    return;
  }

  const auto* tokens = data.FindDict("tokens");
  const auto* headers = data.FindDict("headers");
  const auto* cookies = data.FindDict("cookies");

  if (!tokens || !headers || !cookies) {
    LOG(ERROR) << "Twitter API: Missing required data sections";
    return;
  }

  // Check for API readiness
  bool is_authenticated = false;
  const auto* status = data.FindDict("status");
  if (status) {
    is_authenticated = status->FindBool("ready_for_api").value_or(false);
  }

  if (is_authenticated) {
    LOG(INFO) << "Twitter API: Authentication ready for API calls";
  } else {
    LOG(ERROR) << "Twitter API: Not fully authenticated for API calls";
  }
}

void TwitterScrapingBridge::ProcessFeatureData(base::Value::Dict data) {
  if (!interceptor_) {
    LOG(ERROR) << "Twitter API: Cannot process feature data - interceptor not available";
    return;
  }

  const auto* metadata = data.FindDict("metadata");
  if (!metadata) {
    LOG(ERROR) << "Twitter API: Missing metadata in feature data";
    return;
  }

  const std::string* endpoint = metadata->FindString("endpoint");
  if (endpoint && endpoint->find("/2/") != std::string::npos) {
    LOG(INFO) << "Twitter API v2 feature data for endpoint: " << *endpoint;
  }
}

void TwitterScrapingBridge::CallBrowserSideTokenScraping() {
  if (!interceptor_) {
    LOG(ERROR) << "Twitter API: Cannot call browser-side token scraping - interceptor not available";
    return;
  }

  content::RenderFrame* render_frame = interceptor_->render_frame();
  if (!render_frame) {
    LOG(ERROR) << "Twitter API: Cannot call browser-side token scraping - render frame not available";
    return;
  }

  EnsureMojomConnection();
  
  // Call the browser service to scrape HttpOnly cookies
  LOG(INFO) << "Aaditesh -> 🍪 Calling ScrapeHttpOnlyTokens() through Mojom interface";
  wootz_scraping_service_->ScrapeHttpOnlyTokens(
      base::BindOnce(&TwitterScrapingBridge::OnBrowserTokenScrapingComplete,
                     weak_factory_.GetWeakPtr()));
}

void TwitterScrapingBridge::CallAPIWithStoredCredentials() {
  LOG(INFO) << "Aaditesh -> 🚀 CallAPIWithStoredCredentials() - Renderer calling browser-side stored credentials API";
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "🚀 Renderer calling MakeAPICallWithStoredCredentials via Mojom");
  #endif
  
  if (!interceptor_) {
    LOG(ERROR) << "Aaditesh -> ❌ Interceptor not available for stored credentials API call";
    return;
  }

  content::RenderFrame* render_frame = interceptor_->render_frame();
  if (!render_frame) {
    LOG(ERROR) << "Aaditesh -> ❌ Render frame not available for stored credentials API call";
    return;
  }

  EnsureMojomConnection();
  
  // Call the browser service to make API call with stored credentials
  LOG(INFO) << "Aaditesh -> 🎯 Calling MakeAPICallWithStoredCredentials() through Mojom interface";
  wootz_scraping_service_->MakeAPICallWithStoredCredentials();
  
  LOG(INFO) << "Aaditesh -> ✅ MakeAPICallWithStoredCredentials Mojom call initiated successfully";
  
  #if BUILDFLAG(IS_ANDROID)
  __android_log_print(ANDROID_LOG_INFO, "WootzScraping", 
                     "✅ MakeAPICallWithStoredCredentials Mojom call initiated");
  #endif
}

void TwitterScrapingBridge::EnsureMojomConnection() {
  if (!wootz_scraping_service_.is_bound()) {
    content::RenderFrame* render_frame = interceptor_->render_frame();
    if (!render_frame) {
      LOG(ERROR) << "Twitter API: Cannot establish Mojom connection - render frame not available";
      return;
    }
    render_frame->GetBrowserInterfaceBroker()->GetInterface(
        wootz_scraping_service_.BindNewPipeAndPassReceiver());
  }
}

std::string BuildTwitterAPIUrl(const std::string& endpoint) {
  std::string base_url;
  if (endpoint.find("/2/") == 0) {
    base_url = "https://api.twitter.com";
    LOG(INFO) << "Twitter API v2 request to: " << endpoint;
  } else if (endpoint.find("/graphql/") == 0) {
    base_url = "https://twitter.com";
    LOG(INFO) << "Twitter GraphQL request to: " << endpoint;
  } else {
    base_url = "https://api.twitter.com";
  }
  
  return base_url + endpoint;
}

void TwitterScrapingBridge::NotifyScrapingDataReady(const std::string& twitter_data) {
  LOG(INFO) << "Aaditesh -> Notifying scraping system of ready Twitter authentication data";
  
  // Here we would typically send the data to the scraping system
  // For now, we'll just log that the data is ready
  LOG(INFO) << "Aaditesh -> Twitter authentication data processed and available for scraping integration";
}

void TwitterScrapingBridge::OnBrowserTokenScrapingComplete(
    bool success, 
    const std::optional<std::string>& tokens, 
    const std::optional<std::string>& error_message) {
  
  LOG(INFO) << "Aadi_Kadu -> ===== OnBrowserTokenScrapingComplete() CALLBACK START =====";
  LOG(INFO) << "Aadi_Kadu -> Renderer -> Browser-side token scraping completed";
  LOG(INFO) << "Aadi_Kadu -> Renderer -> Success: " << (success ? "YES" : "NO");
  
  if (success && tokens.has_value()) {
    LOG(INFO) << "Aadi_Kadu -> Renderer -> ✅ SUCCESS: HttpOnly tokens successfully retrieved from browser";
    LOG(INFO) << "Aadi_Kadu -> Renderer -> Token data length: " << tokens->length() << " characters";
    
    // Log first few characters for verification (avoid logging full tokens for security)
    if (!tokens->empty()) {
      std::string preview = tokens->length() > 50 ? 
          tokens->substr(0, 50) + "..." : *tokens;
      LOG(INFO) << "Aadi_Kadu -> Renderer -> Token preview: " << preview;
    }
    
    // Store HttpOnly cookies for combination
    httponly_cookies_ = *tokens;
    LOG(INFO) << "Aadi_Kadu -> Renderer -> HttpOnly cookies stored: " << httponly_cookies_.length() << " characters";
    
    // Store authentication tokens for API calls
    StoreAuthenticationTokens(*tokens);
    
    // COMBINE JAVASCRIPT AND HTTPONLY COOKIES
    CombineCookieStrings();
    
    // Process the received HttpOnly tokens
    LOG(INFO) << "Aadi_Kadu -> Renderer -> Processing received HttpOnly cookies data";
    LogToAndroidConsole(std::string("HttpOnly tokens retrieved from browser: ") + 
                       (tokens->empty() ? "empty" : "data received"));
    
    // If tokens are valid, make test Twitter API calls
    bool token_validation_result = ValidateStoredTokens();
    LOG(INFO) << "Aadi_Kadu -> 🔍 DEBUG: ValidateStoredTokens() result: " << (token_validation_result ? "PASS" : "FAIL");
    
    if (token_validation_result) {
      LOG(INFO) << "Aadi_Kadu -> Renderer -> ✅ Stored tokens validated, checking for dynamic HomeTimeline data";
      LogToAndroidConsole("Tokens validated, checking for captured HomeTimeline features and variables");
      
      // DEBUG: Log the current state of captured data
      LOG(INFO) << "Aadi_Kadu -> 🔍 DEBUG: Checking captured data state:";
      LOG(INFO) << "Aadi_Kadu -> 🔍 - hometimeline_complete_url_ length: " << hometimeline_complete_url_.length();
      LOG(INFO) << "Aadi_Kadu -> 🔍 - hometimeline_complete_url_ preview: " << hometimeline_complete_url_.substr(0, 100) << "...";
      
      LogToAndroidConsole("🔍 DEBUG: Complete URL length=" + std::to_string(hometimeline_complete_url_.length()));
      
      // Check if we have a valid captured HomeTimeline URL with enhanced validation
      if (!hometimeline_complete_url_.empty() && ValidateCapturedUrl()) {
        LOG(INFO) << "Aadi_Kadu -> Renderer -> ✅ Valid HomeTimeline URL available - making GraphQL API call";
        LogToAndroidConsole("Using validated captured HomeTimeline URL for API call");
        
        MakeTwitterGraphQLCall();
      } else {
        LOG(WARNING) << "Aadi_Kadu -> Renderer -> ❌ No valid HomeTimeline URL available for API call";
        if (hometimeline_complete_url_.empty()) {
          LogToAndroidConsole("Missing complete HomeTimeline URL - cannot make API call");
          LogToAndroidConsole("🔍 ISSUE: OnHomeTimelineUrlCaptured may not have been called yet or URL was not stored");
        } else {
          LogToAndroidConsole("HomeTimeline URL failed validation - missing required parameters");
          LogToAndroidConsole("🔍 URL must contain both 'variables=' and 'features=' parameters with valid content");
        }
      }
    } else {
      LOG(WARNING) << "Aadi_Kadu -> Renderer -> ❌ Token validation failed, skipping API calls";
      LogToAndroidConsole("Token validation failed, cannot make API calls");
      LogToAndroidConsole("❌ No API calls will be made due to invalid authentication tokens");
    }
    
    LOG(INFO) << "Aadi_Kadu -> Renderer -> ✅ HttpOnly token processing completed successfully";
    
  } else {
    LOG(WARNING) << "Aadi_Kadu -> Renderer -> ❌ FAILURE: Browser-side token scraping failed";
    if (error_message.has_value()) {
      LOG(WARNING) << "Aadi_Kadu -> Renderer -> Error details: " << *error_message;
      LogToAndroidConsole("Token scraping error: " + *error_message);
    } else {
      LOG(WARNING) << "Aadi_Kadu -> Renderer -> No error message provided by browser";
      LogToAndroidConsole("Token scraping failed with no error message");
    }
  }
  
  LOG(INFO) << "Aadi_Kadu -> ===== OnBrowserTokenScrapingComplete() CALLBACK END =====";
}

// ============================================================================
// CURL FUNCTIONALITY IMPLEMENTATION - Core Network Request Methods
// ============================================================================

void TwitterScrapingBridge::StoreAuthenticationTokens(const std::string& tokens_json) {
  LOG(INFO) << "Aaditesh -> ===== StoreAuthenticationTokens() START =====";
  
  // SAFETY: Limit token data size to prevent memory issues
  if (tokens_json.length() > 100000) {  // 100KB limit
    LOG(ERROR) << "Aaditesh -> Token data too large (" << tokens_json.length() << " bytes), skipping for safety";
    return;
  }
  
  // Clear existing tokens and cookie data
  csrf_token_.clear();
  guest_id_.clear();
  twid_.clear();
  personalization_id_.clear();
  auth_token_.clear();
  ct0_token_.clear();
  session_cookies_.clear();
  tokens_ready_ = false;
  
  // Clear cookie combination data for fresh start
  // Note: Don't clear js_accessible_cookies_ here as they come from a different source
  // Only clear the HttpOnly cookies as they're being replaced
  httponly_cookies_.clear();
  complete_cookie_string_.clear();
  
  // Parse JSON token data from browser
  auto parsed_json = base::JSONReader::Read(tokens_json);
  if (!parsed_json || !parsed_json->is_dict()) {
    LOG(WARNING) << "Aaditesh -> Failed to parse token JSON, treating as raw cookie string";
    // Fallback: treat as raw cookie string
    session_cookies_ = tokens_json;
    
    // CRITICAL FIX: Store the HttpOnly cookies in the correct field for combination
    httponly_cookies_ = tokens_json;
    LOG(INFO) << "Aaditesh -> ✅ STORED HttpOnly cookies for combination (length: " << httponly_cookies_.length() << ")";
    
    // Try to extract specific tokens from cookie string
    if (tokens_json.find("auth_token=") != std::string::npos) {
      size_t start = tokens_json.find("auth_token=") + 11;
      size_t end = tokens_json.find(";", start);
      if (end == std::string::npos) end = tokens_json.length();
      auth_token_ = tokens_json.substr(start, end - start);
      LOG(INFO) << "Aaditesh -> Extracted auth_token from cookies";
    }
    
    if (tokens_json.find("ct0=") != std::string::npos) {
      size_t start = tokens_json.find("ct0=") + 4;
      size_t end = tokens_json.find(";", start);
      if (end == std::string::npos) end = tokens_json.length();
      ct0_token_ = tokens_json.substr(start, end - start);
      csrf_token_ = ct0_token_; // ct0 is the CSRF token
      LOG(INFO) << "Aaditesh -> Extracted ct0/csrf_token from cookies";
    }
    
    tokens_ready_ = !auth_token_.empty() || !ct0_token_.empty();
    LOG(INFO) << "Aaditesh -> Token parsing complete (fallback mode), ready: " << (tokens_ready_ ? "YES" : "NO");
    return;
  }

  const base::Value::Dict& dict = parsed_json->GetDict();
  
  // Extract individual tokens (preserve existing csrf_token if already set)
  const std::string* csrf = dict.FindString("csrf_token");
  if (csrf && !csrf->empty()) {
    csrf_token_ = *csrf;
    LOG(INFO) << "Aaditesh -> Stored CSRF token (length: " << csrf_token_.length() << ")";
  } else if (!csrf_token_.empty()) {
    LOG(INFO) << "Aaditesh -> Preserving existing CSRF token (length: " << csrf_token_.length() << ")";
  }
  
  const std::string* guest = dict.FindString("guest_id");
  if (guest && !guest->empty()) {
    guest_id_ = *guest;
    LOG(INFO) << "Aaditesh -> Stored guest_id (length: " << guest_id_.length() << ")";
  }
  
  const std::string* twid = dict.FindString("twid");
  if (twid && !twid->empty()) {
    twid_ = *twid;
    LOG(INFO) << "Aaditesh -> Stored twid (length: " << twid_.length() << ")";
  }
  
  const std::string* person = dict.FindString("personalization_id");
  if (person && !person->empty()) {
    personalization_id_ = *person;
    LOG(INFO) << "Aaditesh -> Stored personalization_id (length: " << personalization_id_.length() << ")";
  }
  
  const std::string* auth = dict.FindString("auth_token");
  if (auth && !auth->empty()) {
    auth_token_ = *auth;
    LOG(INFO) << "Aaditesh -> Stored auth_token (length: " << auth_token_.length() << ")";
  }
  
  const std::string* ct0 = dict.FindString("ct0");
  if (ct0 && !ct0->empty()) {
    ct0_token_ = *ct0;
    // Only update csrf_token if it's currently empty
    if (csrf_token_.empty()) {
      csrf_token_ = *ct0;
    }
    LOG(INFO) << "Aaditesh -> Stored ct0 token (length: " << ct0_token_.length() << ")";
  }
  
  const std::string* cookies = dict.FindString("session_cookies");
  if (cookies) {
    session_cookies_ = *cookies;
    LOG(INFO) << "Aaditesh -> Stored session cookies (length: " << session_cookies_.length() << ")";
  }
  
  // CRITICAL FIX: Reconstruct HttpOnly cookie string from parsed tokens
  // This ensures that HttpOnly cookies are available for combination
  std::vector<std::string> httponly_parts;
  
  if (!auth_token_.empty()) {
    httponly_parts.push_back("auth_token=" + auth_token_);
  }
  
  if (!ct0_token_.empty() && ct0_token_ != csrf_token_) {
    httponly_parts.push_back("ct0=" + ct0_token_);
  } else if (!csrf_token_.empty()) {
    httponly_parts.push_back("ct0=" + csrf_token_);
  }
  
  // Add any other HttpOnly cookies from session_cookies if available
  if (!session_cookies_.empty()) {
    httponly_parts.push_back(session_cookies_);
  }
  
  // Join all HttpOnly cookie parts
  if (!httponly_parts.empty()) {
    httponly_cookies_ = "";
    for (size_t i = 0; i < httponly_parts.size(); ++i) {
      if (i > 0) {
        httponly_cookies_ += "; ";
      }
      httponly_cookies_ += httponly_parts[i];
    }
    LOG(INFO) << "Aaditesh -> ✅ RECONSTRUCTED HttpOnly cookies for combination (length: " << httponly_cookies_.length() << ")";
  }
  
  // Set ready flag if we have minimum required tokens
  tokens_ready_ = !csrf_token_.empty() || !auth_token_.empty() || !ct0_token_.empty();
  
  LOG(INFO) << "Aaditesh -> Token storage complete. Ready: " << (tokens_ready_ ? "YES" : "NO");
  LOG(INFO) << "Aaditesh -> Token summary - CSRF: " << (!csrf_token_.empty() ? "✓" : "✗")
            << ", Auth: " << (!auth_token_.empty() ? "✓" : "✗")
            << ", CT0: " << (!ct0_token_.empty() ? "✓" : "✗")
            << ", Session: " << (!session_cookies_.empty() ? "✓" : "✗");
  LOG(INFO) << "Aaditesh -> ===== StoreAuthenticationTokens() END =====";
}

bool TwitterScrapingBridge::ValidateStoredTokens() {
  // Check if we have minimum required tokens for API calls
  bool has_auth = !auth_token_.empty();
  bool has_csrf = !csrf_token_.empty() || !ct0_token_.empty();
  bool has_session = !session_cookies_.empty();
  
  // For Twitter API calls, we need either auth token or both CSRF and session
  return has_auth || (has_csrf && has_session);
}

void TwitterScrapingBridge::CombineCookieStrings() {
  LOG(INFO) << "Aaditesh -> ===== CombineCookieStrings() START =====";
  
  // Clear the previous combined cookie string
  complete_cookie_string_.clear();
  
  LOG(INFO) << "Aaditesh -> 🍪 COOKIE COMBINATION PROCESS:";
  LOG(INFO) << "Aaditesh -> - JavaScript cookies length: " << js_accessible_cookies_.length();
  LOG(INFO) << "Aaditesh -> - HttpOnly cookies length: " << httponly_cookies_.length();
  
  // Start with JavaScript-accessible cookies
  if (!js_accessible_cookies_.empty()) {
    complete_cookie_string_ = js_accessible_cookies_;
    LOG(INFO) << "Aaditesh -> ✅ Added JavaScript cookies as base";
  }
  
  // Add HttpOnly cookies if available
  if (!httponly_cookies_.empty()) {
    // If we already have JavaScript cookies, add a separator
    if (!complete_cookie_string_.empty()) {
      // Check if the JavaScript cookies end with a semicolon
      if (complete_cookie_string_.back() != ';') {
        complete_cookie_string_ += "; ";
      } else {
        complete_cookie_string_ += " ";
      }
    }
    
    complete_cookie_string_ += httponly_cookies_;
    LOG(INFO) << "Aaditesh -> ✅ Added HttpOnly cookies";
  }
  
  // Log the complete combined cookie string (LIMITED FOR SAFETY)
  LOG(INFO) << "Aaditesh -> 🎯 COMPLETE COMBINED COOKIE STRING:";
  LOG(INFO) << "Aaditesh -> Total length: " << complete_cookie_string_.length() << " characters";
  
  // SAFETY: Only log first 200 characters to prevent memory issues
  if (complete_cookie_string_.length() > 200) {
    LOG(INFO) << "Aaditesh -> Complete cookie string preview: " << complete_cookie_string_.substr(0, 200) << "... (truncated for safety)";
  } else {
    LOG(INFO) << "Aaditesh -> Complete cookie string: " << complete_cookie_string_;
  }
  
  // Also log to Android console for easy monitoring (LIMITED)
  LogToAndroidConsole("🍪 COMBINED COOKIES COMPLETE:");
  LogToAndroidConsole("📊 JavaScript cookies: " + std::to_string(js_accessible_cookies_.length()) + " chars");
  LogToAndroidConsole("📊 HttpOnly cookies: " + std::to_string(httponly_cookies_.length()) + " chars");
  LogToAndroidConsole("📊 Combined total: " + std::to_string(complete_cookie_string_.length()) + " chars");
  
  // SAFETY: Truncate Android console output to prevent memory issues
  std::string truncated_cookies = complete_cookie_string_.length() > 150 ? 
      complete_cookie_string_.substr(0, 150) + "... (truncated)" : complete_cookie_string_;
  LogToAndroidConsole("🎯 COMBINED COOKIES: " + truncated_cookies);
  
  // Update timestamp
  std::time_t now = std::time(nullptr);
  char timestamp[100];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
  cookie_parsing_timestamp_ = std::string(timestamp);
  
  LOG(INFO) << "Aaditesh -> Cookie combination completed at: " << cookie_parsing_timestamp_;
  LOG(INFO) << "Aaditesh -> ===== CombineCookieStrings() END =====";
}

void TwitterScrapingBridge::ClearCookieData() {
  LOG(INFO) << "Aaditesh -> ===== ClearCookieData() START =====";
  
  // Clear all cookie-related data
  js_accessible_cookies_.clear();
  httponly_cookies_.clear();
  complete_cookie_string_.clear();
  total_cookie_count_ = 0;
  cookie_parsing_timestamp_.clear();
  
  LOG(INFO) << "Aaditesh -> ✅ All cookie data cleared";
  LogToAndroidConsole("🧹 Cookie data cleared - ready for new capture");
  
  LOG(INFO) << "Aaditesh -> ===== ClearCookieData() END =====";
}



// OnTwitterAPIResponse function removed - no longer needed

void TwitterScrapingBridge::MakeTwitterGraphQLCall() {
  LOG(INFO) << "Aaditesh -> ===== MakeTwitterGraphQLCall() START =====";
  
  // SAFETY: Prevent multiple simultaneous API calls
  if (is_making_api_call_) {
    LOG(WARNING) << "Aaditesh -> API call already in progress, skipping to prevent overlap";
    return;
  }
  
  is_making_api_call_ = true;
  
  // CRITICAL: Check if we have a valid URL BEFORE doing any batch processing
  if (hometimeline_complete_url_.empty()) {
    LOG(ERROR) << "Aaditesh -> ❌ No complete HomeTimeline URL captured yet";
    LogToAndroidConsole("❌ Cannot make API call - no complete URL captured");
    LogToAndroidConsole("🔍 No API call will be made and batch counter will not be incremented");
    is_making_api_call_ = false;
    return;
  }
  
  // ENHANCED: Validate the URL content before proceeding
  if (!ValidateCapturedUrl()) {
    LOG(ERROR) << "Aaditesh -> ❌ HomeTimeline URL validation failed";
    LogToAndroidConsole("❌ Cannot make API call - URL validation failed");
    LogToAndroidConsole("🔍 URL must contain both 'variables=' and 'features=' parameters with valid content");
    LogToAndroidConsole("🔍 No API call will be made and batch counter will not be incremented");
    is_making_api_call_ = false;
    return;
  }
  
  LOG(INFO) << "Aaditesh -> ✅ URL validation passed - proceeding with API call";
  
  // BATCH PROCESSING: Check API call limits AFTER URL validation
  if (!CanMakeAPICall()) {
    if (HasSuccessfulResponse()) {
      LOG(INFO) << "Aaditesh -> 🔄 API call limit reached, using cached response";
      LogToAndroidConsole("🔄 Using cached response (API limit reached)");
      // Note: Response would be cached but no callback needed
      is_making_api_call_ = false;
      return;
    } else {
      LOG(WARNING) << "Aaditesh -> ⚠️ API call limit exceeded and no cached response available";
      LogToAndroidConsole("⚠️ API call limit exceeded, no cached response");
      // Note: API limit exceeded, no further action needed
      is_making_api_call_ = false;
      return;
    }
  }
  
  // Increment API call counter ONLY after all validations pass
  IncrementAPICallCount();
  LOG(INFO) << "Aaditesh -> 📊 API call count: " << api_call_count_ << "/" << MAX_API_CALLS;
  
  LOG(INFO) << "Aaditesh -> Making DYNAMIC GraphQL HomeTimeline request using VALIDATED captured URL";
  
  // SAFETY: Only log first 200 characters of URL to prevent memory issues
  std::string url_preview = hometimeline_complete_url_.length() > 200 ? 
      hometimeline_complete_url_.substr(0, 200) + "... (truncated for safety)" : hometimeline_complete_url_;
  LOG(INFO) << "Aaditesh -> 🏠 HOMETIMELINE GraphQL URL: " << url_preview;
  
  LogToAndroidConsole("🚀 Making API call using validated captured URL: " + hometimeline_complete_url_.substr(0, 100) + "...");
  
  // Create resource request (GET method like cURL) using complete URL directly
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(hometimeline_complete_url_);
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kInclude;
  
  // Build headers using CAPTURED DYNAMIC DATA from OnDataCaptured()
  LOG(INFO) << "Aaditesh -> 🔄 USING DYNAMIC CAPTURED HEADERS (NOT HARDCODED)";
  
  // Use captured headers with fallbacks to defaults
  if (!accept_header_.empty()) {
    resource_request->headers.SetHeader("accept", accept_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Accept: " << accept_header_;
  } else {
    resource_request->headers.SetHeader("accept", "*/*");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback Accept: */*";
  }
  
  if (!accept_language_header_.empty()) {
    resource_request->headers.SetHeader("accept-language", accept_language_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Accept-Language: " << accept_language_header_;
  } else {
    resource_request->headers.SetHeader("accept-language", "en-US");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback Accept-Language: en-US";
  }
  
  if (!authorization_header_.empty()) {
    resource_request->headers.SetHeader("authorization", authorization_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Authorization: " << authorization_header_.substr(0, 30) << "...";
  } else {
    resource_request->headers.SetHeader("authorization", "Bearer AAAAAAAAAAAAAAAAAAAAANRILgAAAAAAnNwIzUejRCOuH5E6I8xnZz4puTs%3D1Zv7ttfk8LF81IUq16cHjhLTvJu4FA33AGWWjCpTnA");
    LOG(WARNING) << "Aaditesh -> ⚠️ Using hardcoded Authorization bearer token";
  }
  
  if (!content_type_header_.empty()) {
    resource_request->headers.SetHeader("content-type", content_type_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Content-Type: " << content_type_header_;
  } else {
    resource_request->headers.SetHeader("content-type", "application/json");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback Content-Type: application/json";
  }
  
  // Use the complete combined cookie string (CRITICAL)
  if (!complete_cookie_string_.empty()) {
    resource_request->headers.SetHeader("cookie", complete_cookie_string_);
    LOG(INFO) << "Aaditesh -> ✅ Using COMBINED cookie string (" << complete_cookie_string_.length() << " chars)";
    LogToAndroidConsole("🍪 Using combined cookies: " + complete_cookie_string_.substr(0, 100) + "...");
  } else {
    // Fallback: build cookies manually if combined string not available
    LOG(WARNING) << "Aaditesh -> ⚠️ Combined cookie string not available, building manually";
    std::ostringstream cookie_stream;
    if (!guest_id_.empty()) {
      cookie_stream << "guest_id_marketing=v1%3A" << guest_id_ << "; ";
      cookie_stream << "guest_id_ads=v1%3A" << guest_id_ << "; ";
      cookie_stream << "guest_id=v1%3A" << guest_id_;
    }
    if (!personalization_id_.empty()) {
      if (!cookie_stream.str().empty()) cookie_stream << "; ";
      cookie_stream << "personalization_id=" << personalization_id_;
    }
    if (!auth_token_.empty()) {
      if (!cookie_stream.str().empty()) cookie_stream << "; ";
      cookie_stream << "auth_token=" << auth_token_;
    }
    if (!ct0_token_.empty()) {
      if (!cookie_stream.str().empty()) cookie_stream << "; ";
      cookie_stream << "ct0=" << ct0_token_;
    }
    if (!cookie_stream.str().empty()) cookie_stream << "; ";
    cookie_stream << "lang=en";
    if (!twid_.empty()) {
      cookie_stream << "; twid=" << twid_;
    }
    resource_request->headers.SetHeader("cookie", cookie_stream.str());
    LOG(INFO) << "Aaditesh -> ✅ Using manually built cookies (" << cookie_stream.str().length() << " chars)";
  }
  
  // Add all other headers using captured values with fallbacks
  resource_request->headers.SetHeader("priority", "u=1, i");
  
  if (!referer_header_.empty()) {
    resource_request->headers.SetHeader("referer", referer_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Referer: " << referer_header_;
  } else {
    resource_request->headers.SetHeader("referer", "https://x.com/");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback Referer: https://x.com/";
  }
  
  resource_request->headers.SetHeader("sec-ch-ua", "\"Chromium\";v=\"127\", \"Not)A;Brand\";v=\"99\"");
  resource_request->headers.SetHeader("sec-ch-ua-mobile", "?1");
  resource_request->headers.SetHeader("sec-ch-ua-platform", "\"Android\"");
  
  if (!sec_fetch_dest_.empty()) {
    resource_request->headers.SetHeader("sec-fetch-dest", sec_fetch_dest_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Sec-Fetch-Dest: " << sec_fetch_dest_;
  } else {
    resource_request->headers.SetHeader("sec-fetch-dest", "empty");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback Sec-Fetch-Dest: empty";
  }
  
  if (!sec_fetch_mode_.empty()) {
    resource_request->headers.SetHeader("sec-fetch-mode", sec_fetch_mode_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Sec-Fetch-Mode: " << sec_fetch_mode_;
  } else {
    resource_request->headers.SetHeader("sec-fetch-mode", "cors");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback Sec-Fetch-Mode: cors";
  }
  
  if (!sec_fetch_site_.empty()) {
    resource_request->headers.SetHeader("sec-fetch-site", sec_fetch_site_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured Sec-Fetch-Site: " << sec_fetch_site_;
  } else {
    resource_request->headers.SetHeader("sec-fetch-site", "same-origin");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback Sec-Fetch-Site: same-origin";
  }
  
  if (!user_agent_header_.empty()) {
    resource_request->headers.SetHeader("user-agent", user_agent_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured User-Agent: " << user_agent_header_.substr(0, 50) << "...";
  } else {
    // Generate dynamic User-Agent using Chromium's system
    std::string dynamic_user_agent = content::GetReducedUserAgent(true, "127");
    resource_request->headers.SetHeader("user-agent", dynamic_user_agent);
    LOG(INFO) << "Aaditesh -> ✅ Using DYNAMIC generated User-Agent: " << dynamic_user_agent.substr(0, 50) << "...";
  }
  
  // Add CSRF token (prioritize captured header, then use ct0/csrf fallbacks)
  if (!x_csrf_token_header_.empty()) {
    resource_request->headers.SetHeader("x-csrf-token", x_csrf_token_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured x-csrf-token header: " << x_csrf_token_header_.substr(0, 20) << "...";
  } else if (!ct0_token_.empty()) {
    resource_request->headers.SetHeader("x-csrf-token", ct0_token_);
    LOG(INFO) << "Aaditesh -> ⚠️ Using ct0_token fallback: " << ct0_token_.substr(0, 20) << "...";
  } else if (!csrf_token_.empty()) {
    resource_request->headers.SetHeader("x-csrf-token", csrf_token_);
    LOG(INFO) << "Aaditesh -> ⚠️ Using csrf_token fallback";
  } else {
    LOG(ERROR) << "Aaditesh -> ❌ NO CSRF TOKEN AVAILABLE!";
  }
  
  // Add transaction ID (prioritize captured header, then use js_transaction_id fallback)
  if (!x_client_transaction_id_header_.empty()) {
    resource_request->headers.SetHeader("x-client-transaction-id", x_client_transaction_id_header_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured x-client-transaction-id header: " << x_client_transaction_id_header_.substr(0, 20) << "...";
  } else if (!js_transaction_id_.empty()) {
    resource_request->headers.SetHeader("x-client-transaction-id", js_transaction_id_);
    LOG(INFO) << "Aaditesh -> ⚠️ Using js_transaction_id fallback";
  } else {
    // Generate dynamic transaction ID using timestamp and random components
    auto now = base::Time::Now();
    int64_t timestamp = now.ToInternalValue();
    std::string transaction_id = base::Base64Encode(
        base::StringPrintf("wootz_%lld_%u", 
                          static_cast<long long>(timestamp),
                          base::RandInt(10000, 99999)));
    resource_request->headers.SetHeader("x-client-transaction-id", transaction_id);
    LOG(INFO) << "Aaditesh -> ✅ Using DYNAMIC generated transaction ID: " << transaction_id.substr(0, 20) << "...";
  }
  
  // Add Twitter-specific headers using captured values with fallbacks
  if (!x_twitter_active_user_.empty()) {
    resource_request->headers.SetHeader("x-twitter-active-user", x_twitter_active_user_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured X-Twitter-Active-User: " << x_twitter_active_user_;
  } else {
    resource_request->headers.SetHeader("x-twitter-active-user", "yes");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback X-Twitter-Active-User: yes";
  }
  
  if (!x_twitter_auth_type_.empty()) {
    resource_request->headers.SetHeader("x-twitter-auth-type", x_twitter_auth_type_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured X-Twitter-Auth-Type: " << x_twitter_auth_type_;
  } else {
    resource_request->headers.SetHeader("x-twitter-auth-type", "OAuth2Session");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback X-Twitter-Auth-Type: OAuth2Session";
  }
  
  if (!x_twitter_client_language_.empty()) {
    resource_request->headers.SetHeader("x-twitter-client-language", x_twitter_client_language_);
    LOG(INFO) << "Aaditesh -> ✅ Using captured X-Twitter-Client-Language: " << x_twitter_client_language_;
  } else {
    resource_request->headers.SetHeader("x-twitter-client-language", "en");
    LOG(INFO) << "Aaditesh -> ⚠️ Using fallback X-Twitter-Client-Language: en";
  }
  
  // Add x-xp-forwarded-for if available
  if (!js_xp_forwarded_for_.empty()) {
    resource_request->headers.SetHeader("x-xp-forwarded-for", js_xp_forwarded_for_);
    LOG(INFO) << "Aaditesh -> ✅ Added captured x-xp-forwarded-for";
  } else {
    // Use a sample value from the cURL
    std::string xp_forwarded = "6c30eaa591628b0f7e5f0afde8fead06037b4a841cb7ce13379f34f524b97561ac4d890be51d4a0e57f21825e36bb6ac26f6c1634c12a28a23325a63db63de337451ef71d4b8859cebbb37471ab2744ff25f73c98d99194f7d49fca8bbd055598af9a0998427e263b56643072a84bfc8d9229ffc20da7a8422c092ab18275a26824aef8f3a97a3234a86fc16810d51099526fa76d83f777ad2ef7679a6c040cdbac94d27f5a78f95da732ed3a37990efad30373c0718863a6c2bbc1b031e02c609636cf445e58080bed24092445afe3906a3c4b28f6de188384c98c1c8bec9d0f017b69cf3e783d87b8a3b354bc84484c8034a4a86231cf8420148a5ad708bc1";
    resource_request->headers.SetHeader("x-xp-forwarded-for", xp_forwarded);
    LOG(INFO) << "Aaditesh -> ✅ Added sample x-xp-forwarded-for";
  }
  
  LOG(INFO) << "Aaditesh -> 📊 DYNAMIC GRAPHQL REQUEST HEADERS SUMMARY:";
  LOG(INFO) << "Aaditesh -> - Method: GET";
  LOG(INFO) << "Aaditesh -> - URL: " << hometimeline_complete_url_.substr(0, 100) << "...";
  LOG(INFO) << "Aaditesh -> 🔐 AUTHENTICATION STATUS:";
  LOG(INFO) << "  - Authorization: " << (!authorization_header_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "  - X-CSRF-Token: " << (!x_csrf_token_header_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "  - X-Client-Transaction-ID: " << (!x_client_transaction_id_header_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "Aaditesh -> 🌐 STANDARD HEADERS STATUS:";
  LOG(INFO) << "  - Accept: " << (!accept_header_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "  - User-Agent: " << (!user_agent_header_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "  - Referer: " << (!referer_header_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "Aaditesh -> 🐦 TWITTER HEADERS STATUS:";
  LOG(INFO) << "  - X-Twitter-Active-User: " << (!x_twitter_active_user_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "  - X-Twitter-Auth-Type: " << (!x_twitter_auth_type_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "  - X-Twitter-Client-Language: " << (!x_twitter_client_language_.empty() ? "✅ CAPTURED" : "⚠️ FALLBACK");
  LOG(INFO) << "Aaditesh -> 🍪 COOKIE STATUS: " << (!complete_cookie_string_.empty() ? "✅ COMBINED STRING" : "⚠️ MANUAL BUILD");
  
  // Count dynamic vs fallback headers
  int captured_headers = 0;
  int total_headers = 0;
  
  // Authentication headers
  if (!authorization_header_.empty()) captured_headers++;
  total_headers++;
  if (!x_csrf_token_header_.empty()) captured_headers++;
  total_headers++;
  if (!x_client_transaction_id_header_.empty()) captured_headers++;
  total_headers++;
  
  // Standard headers
  if (!accept_header_.empty()) captured_headers++;
  total_headers++;
  if (!user_agent_header_.empty()) captured_headers++;
  total_headers++;
  if (!referer_header_.empty()) captured_headers++;
  total_headers++;
  
  // Twitter headers
  if (!x_twitter_active_user_.empty()) captured_headers++;
  total_headers++;
  if (!x_twitter_auth_type_.empty()) captured_headers++;
  total_headers++;
  if (!x_twitter_client_language_.empty()) captured_headers++;
  total_headers++;
  
  // Security headers
  if (!sec_fetch_dest_.empty()) captured_headers++;
  total_headers++;
  if (!sec_fetch_mode_.empty()) captured_headers++;
  total_headers++;
  if (!sec_fetch_site_.empty()) captured_headers++;
  total_headers++;
  
  double percentage = total_headers > 0 ? (static_cast<double>(captured_headers) / total_headers) * 100.0 : 0.0;
  
  LOG(INFO) << "Aaditesh -> 🎯 DYNAMIC HEADER SCORE: " << captured_headers << "/" << total_headers 
            << " (" << std::fixed << std::setprecision(1) << percentage << "% dynamic)";
  
  LogToAndroidConsole("🎯 GraphQL Headers - Dynamic: " + std::to_string(captured_headers) + 
                     "/" + std::to_string(total_headers) + 
                     " (" + std::to_string(static_cast<int>(percentage)) + "% captured)");
  LOG(INFO) << "Aaditesh -> - CSRF Token: " << (!x_csrf_token_header_.empty() ? "✅ CAPTURED HEADER" : (!ct0_token_.empty() ? "ct0 value" : (!csrf_token_.empty() ? "csrf value" : "MISSING")));
  
  // Create traffic annotation
  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("twitter_api_interceptor", R"(
        semantics {
          sender: "Twitter API Interceptor"
          description: "Intercepts and makes Twitter API requests with proper authentication"
          trigger: "User navigating Twitter pages"
          data: "Twitter API GraphQL requests"
          destination: WEBSITE
        }
        policy {
          cookies_allowed: YES
          cookies_store: "user"
          setting: "This feature can be disabled in browser settings"
        })");

  // Create URL loader and start request
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request),
      traffic_annotation);
  
  // Get render frame for URLLoaderFactory access
  content::RenderFrame* render_frame = interceptor_->render_frame();
  if (!render_frame) {
    LOG(ERROR) << "Aaditesh -> ❌ CRITICAL: RenderFrame not available for GraphQL request";
    return;
  }
  
  // Get URLLoaderFactory through WebFrame Client (CORRECT approach for renderer context)
  blink::WebLocalFrame* web_frame = render_frame->GetWebFrame();
  if (!web_frame) {
    LOG(ERROR) << "Aaditesh -> ❌ CRITICAL: WebFrame not available for GraphQL request";
    return;
  }
  
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory = 
      web_frame->Client()->GetURLLoaderFactory();
  
  LogToAndroidConsole("🚀 Sending GraphQL request with combined cookies and proper headers");
  
  // ============================================================================
  // GENERATE AND LOG WORKING CURL COMMAND
  // ============================================================================
  LOG(INFO) << "Aaditesh -> 🔧 GENERATING WORKING CURL COMMAND...";
  
  // Debug the conditions first
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: resource_request pointer: " << static_cast<void*>(resource_request.get());
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: hometimeline_complete_url_.empty(): " << (hometimeline_complete_url_.empty() ? "TRUE" : "FALSE");
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: hometimeline_complete_url_ length: " << hometimeline_complete_url_.length();
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: hometimeline_complete_url_ content: " << hometimeline_complete_url_;
  
  // Always try to generate curl command regardless of conditions
  std::ostringstream curl_command;
  
  // Use the URL we know works
  if (!hometimeline_complete_url_.empty()) {
    curl_command << "curl -X GET '" << hometimeline_complete_url_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added URL to curl command";
  } else {
    curl_command << "curl -X GET 'URL_NOT_AVAILABLE'";
    LOG(ERROR) << "Aaditesh -> ❌ URL not available for curl command";
  }
  
  // Use captured member variables instead of null resource_request
  LOG(INFO) << "Aaditesh -> ✅ Using captured member variables for headers (resource_request moved)";
  
  // Add Accept header
  if (!accept_header_.empty()) {
    curl_command << " \\\n  -H 'accept: " << accept_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added accept header to curl: " << accept_header_;
  } else {
    curl_command << " \\\n  -H 'accept: */*'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback accept header to curl";
  }
  
  // Add Authorization header (FULL TOKEN)
  if (!authorization_header_.empty()) {
    curl_command << " \\\n  -H 'authorization: " << authorization_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added authorization header to curl (FULL TOKEN, " << authorization_header_.length() << " chars)";
  } else {
    curl_command << " \\\n  -H 'authorization: Bearer AAAAAAAAAAAAAAAAAAAAANRILgAAAAAAnNwIzUejRCOuH5E6I8xnZz4puTs%3D1Zv7ttfk8LF81IUq16cHjhLTvJu4FA33AGWWjCpTnA'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback authorization header to curl";
  }
  
  // Add Cookie header (COMPLETE COMBINED COOKIES)
  if (!complete_cookie_string_.empty()) {
    curl_command << " \\\n  -H 'cookie: " << complete_cookie_string_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added cookie header to curl (COMPLETE COMBINED COOKIES, " << complete_cookie_string_.length() << " chars)";
  } else {
    LOG(WARNING) << "Aaditesh -> ⚠️ No combined cookie string available for curl";
  }
  
  // Add User-Agent header
  if (!user_agent_header_.empty()) {
    curl_command << " \\\n  -H 'user-agent: " << user_agent_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added user-agent header to curl";
  } else {
    curl_command << " \\\n  -H 'user-agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback user-agent header to curl";
  }
  
  // Add Referer header
  if (!referer_header_.empty()) {
    curl_command << " \\\n  -H 'referer: " << referer_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added referer header to curl: " << referer_header_;
  } else {
    curl_command << " \\\n  -H 'referer: https://x.com/home'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback referer header to curl";
  }
  
  // Add Content-Type header
  if (!content_type_header_.empty()) {
    curl_command << " \\\n  -H 'content-type: " << content_type_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added content-type header to curl: " << content_type_header_;
  } else {
    curl_command << " \\\n  -H 'content-type: application/json'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback content-type header to curl";
  }
  
  // Add X-CSRF-Token header (FULL TOKEN)
  if (!x_csrf_token_header_.empty()) {
    curl_command << " \\\n  -H 'x-csrf-token: " << x_csrf_token_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-csrf-token header to curl (FULL TOKEN, " << x_csrf_token_header_.length() << " chars)";
  } else if (!ct0_token_.empty()) {
    curl_command << " \\\n  -H 'x-csrf-token: " << ct0_token_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-csrf-token header to curl (from ct0_token_, " << ct0_token_.length() << " chars)";
  } else if (!csrf_token_.empty()) {
    curl_command << " \\\n  -H 'x-csrf-token: " << csrf_token_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-csrf-token header to curl (from csrf_token_, " << csrf_token_.length() << " chars)";
  } else {
    LOG(WARNING) << "Aaditesh -> ⚠️ No CSRF token available for curl";
  }
  
  // Add X-Client-Transaction-ID header
  if (!x_client_transaction_id_header_.empty()) {
    curl_command << " \\\n  -H 'x-client-transaction-id: " << x_client_transaction_id_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-client-transaction-id header to curl: " << x_client_transaction_id_header_;
  } else if (!js_transaction_id_.empty()) {
    curl_command << " \\\n  -H 'x-client-transaction-id: " << js_transaction_id_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-client-transaction-id header to curl (from js_transaction_id_): " << js_transaction_id_;
  } else {
    LOG(WARNING) << "Aaditesh -> ⚠️ No transaction ID available for curl";
  }
  
  // Add Accept-Language header
  if (!accept_language_header_.empty()) {
    curl_command << " \\\n  -H 'accept-language: " << accept_language_header_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added accept-language header to curl: " << accept_language_header_;
  } else {
    curl_command << " \\\n  -H 'accept-language: en-US,en;q=0.9'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback accept-language header to curl";
  }
  
  // Add Priority header
  curl_command << " \\\n  -H 'priority: u=1, i'";
  
  // Add Twitter-specific headers
  if (!x_twitter_active_user_.empty()) {
    curl_command << " \\\n  -H 'x-twitter-active-user: " << x_twitter_active_user_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-twitter-active-user header to curl: " << x_twitter_active_user_;
  } else {
    curl_command << " \\\n  -H 'x-twitter-active-user: yes'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback x-twitter-active-user header to curl";
  }
  
  if (!x_twitter_auth_type_.empty()) {
    curl_command << " \\\n  -H 'x-twitter-auth-type: " << x_twitter_auth_type_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-twitter-auth-type header to curl: " << x_twitter_auth_type_;
  } else {
    curl_command << " \\\n  -H 'x-twitter-auth-type: OAuth2Session'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback x-twitter-auth-type header to curl";
  }
  
  if (!x_twitter_client_language_.empty()) {
    curl_command << " \\\n  -H 'x-twitter-client-language: " << x_twitter_client_language_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added x-twitter-client-language header to curl: " << x_twitter_client_language_;
  } else {
    curl_command << " \\\n  -H 'x-twitter-client-language: en'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback x-twitter-client-language header to curl";
  }
  
  // Add Security headers
  if (!sec_fetch_dest_.empty()) {
    curl_command << " \\\n  -H 'sec-fetch-dest: " << sec_fetch_dest_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added sec-fetch-dest header to curl: " << sec_fetch_dest_;
  } else {
    curl_command << " \\\n  -H 'sec-fetch-dest: empty'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback sec-fetch-dest header to curl";
  }
  
  if (!sec_fetch_mode_.empty()) {
    curl_command << " \\\n  -H 'sec-fetch-mode: " << sec_fetch_mode_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added sec-fetch-mode header to curl: " << sec_fetch_mode_;
  } else {
    curl_command << " \\\n  -H 'sec-fetch-mode: cors'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback sec-fetch-mode header to curl";
  }
  
  if (!sec_fetch_site_.empty()) {
    curl_command << " \\\n  -H 'sec-fetch-site: " << sec_fetch_site_ << "'";
    LOG(INFO) << "Aaditesh -> ✅ Added sec-fetch-site header to curl: " << sec_fetch_site_;
  } else {
    curl_command << " \\\n  -H 'sec-fetch-site: same-origin'";
    LOG(INFO) << "Aaditesh -> ✅ Added fallback sec-fetch-site header to curl";
  }
  
  LOG(INFO) << "Aaditesh -> ✅ Finished adding all available headers to curl command using member variables";
  
  curl_command << " \\\n  --compressed \\\n  --insecure";
  
  // Log the complete working curl command (NO TRUNCATION)
  std::string complete_curl = curl_command.str();
  LOG(INFO) << "Aaditesh -> 🚀 ===== COMPLETE WORKING CURL COMMAND START =====";
  LOG(INFO) << "Aaditesh -> " << complete_curl;
  LOG(INFO) << "Aaditesh -> 🚀 ===== COMPLETE WORKING CURL COMMAND END =====";
  
  // Also log to Android console (FULL COMMAND, NO TRUNCATION)
  LogToAndroidConsole("🚀 COMPLETE CURL COMMAND: " + complete_curl);
  LogToAndroidConsole("🔧 Full curl command logged above - ready to copy/paste");
  
  // ============================================================================
  
  // ============================================================================
  // ACTUAL API CALL COMMENTED OUT - ONLY LOGGING CURL COMMAND NOW
  // ============================================================================
  
  // ============================================================================
  // BROWSER-SIDE API CALL - Use browser process for network request
  // ============================================================================
  
  LOG(INFO) << "Aaditesh -> 🚀 Making browser-side Twitter GraphQL API call...";
  LogToAndroidConsole("🚀 Using browser-side API call with EXACT SAME DATA as working CURL");
  
  // Check mojo connection
  if (!wootz_scraping_service_) {
    LOG(ERROR) << "Aaditesh -> ❌ WootzScrapingService mojo interface not available";
    LogToAndroidConsole("❌ Browser API not available");
    LOG(INFO) << "Aaditesh -> ===== MakeTwitterGraphQLCall() END =====";
    is_making_api_call_ = false;
    return;
  }
  
  // CRITICAL FIX: Use exactly the same headers as the working CURL command
  std::ostringstream exact_headers_stream;
  
  // Use the exact same header generation logic as the working CURL command
  if (!accept_header_.empty()) {
    exact_headers_stream << "accept: " << accept_header_ << "\n";
  } else {
    exact_headers_stream << "accept: */*\n";
  }
  
  // Add Authorization header (EXACT SAME AS CURL)
  if (!authorization_header_.empty()) {
    exact_headers_stream << "authorization: " << authorization_header_ << "\n";
  } else {
    exact_headers_stream << "authorization: Bearer AAAAAAAAAAAAAAAAAAAAANRILgAAAAAAnNwIzUejRCOuH5E6I8xnZz4puTs%3D1Zv7ttfk8LF81IUq16cHjhLTvJu4FA33AGWWjCpTnA\n";
  }
  
  // Add Cookie header (EXACT SAME AS CURL - CRITICAL)
  if (!complete_cookie_string_.empty()) {
    exact_headers_stream << "cookie: " << complete_cookie_string_ << "\n";
    LOG(INFO) << "Aaditesh -> ✅ Using EXACT same cookie string as working CURL (" << complete_cookie_string_.length() << " chars)";
  } else {
    LOG(ERROR) << "Aaditesh -> ❌ No complete cookie string available";
  }
  
  // Add User-Agent header (EXACT SAME AS CURL)
  if (!user_agent_header_.empty()) {
    exact_headers_stream << "user-agent: " << user_agent_header_ << "\n";
  } else {
    exact_headers_stream << "user-agent: Mozilla/5.0 (Linux; Android 10; K) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Mobile Safari/537.36\n";
  }
  
  // Add Referer header (EXACT SAME AS CURL)
  if (!referer_header_.empty()) {
    exact_headers_stream << "referer: " << referer_header_ << "\n";
  } else {
    exact_headers_stream << "referer: https://x.com/home\n";
  }
  
  // Add Origin header (CRITICAL - might be missing)
  if (!origin_header_.empty()) {
    exact_headers_stream << "origin: " << origin_header_ << "\n";
  } else {
    exact_headers_stream << "origin: https://x.com\n";
  }
  
  // Add Content-Type header (EXACT SAME AS CURL)
  exact_headers_stream << "content-type: application/json\n";
  
  // Add X-CSRF-Token header (EXACT SAME AS CURL)
  if (!x_csrf_token_header_.empty()) {
    exact_headers_stream << "x-csrf-token: " << x_csrf_token_header_ << "\n";
  } else if (!ct0_token_.empty()) {
    exact_headers_stream << "x-csrf-token: " << ct0_token_ << "\n";
  } else if (!csrf_token_.empty()) {
    exact_headers_stream << "x-csrf-token: " << csrf_token_ << "\n";
  }
  
  // Add X-Client-Transaction-ID header (EXACT SAME AS CURL)
  if (!x_client_transaction_id_header_.empty()) {
    exact_headers_stream << "x-client-transaction-id: " << x_client_transaction_id_header_ << "\n";
  } else if (!js_transaction_id_.empty()) {
    exact_headers_stream << "x-client-transaction-id: " << js_transaction_id_ << "\n";
  }
  
  // Add Accept-Language header (EXACT SAME AS CURL)
  if (!accept_language_header_.empty()) {
    exact_headers_stream << "accept-language: " << accept_language_header_ << "\n";
  } else {
    exact_headers_stream << "accept-language: en-US,en;q=0.9\n";
  }
  
  // Add Priority header (EXACT SAME AS CURL)
  exact_headers_stream << "priority: u=1, i\n";
  
  // Add Twitter-specific headers (EXACT SAME AS CURL)
  if (!x_twitter_active_user_.empty()) {
    exact_headers_stream << "x-twitter-active-user: " << x_twitter_active_user_ << "\n";
  } else {
    exact_headers_stream << "x-twitter-active-user: yes\n";
  }
  
  if (!x_twitter_auth_type_.empty()) {
    exact_headers_stream << "x-twitter-auth-type: " << x_twitter_auth_type_ << "\n";
  } else {
    exact_headers_stream << "x-twitter-auth-type: OAuth2Session\n";
  }
  
  if (!x_twitter_client_language_.empty()) {
    exact_headers_stream << "x-twitter-client-language: " << x_twitter_client_language_ << "\n";
  } else {
    exact_headers_stream << "x-twitter-client-language: en\n";
  }
  
  // Add Security headers (EXACT SAME AS CURL)
  if (!sec_fetch_dest_.empty()) {
    exact_headers_stream << "sec-fetch-dest: " << sec_fetch_dest_ << "\n";
  } else {
    exact_headers_stream << "sec-fetch-dest: empty\n";
  }
  
  if (!sec_fetch_mode_.empty()) {
    exact_headers_stream << "sec-fetch-mode: " << sec_fetch_mode_ << "\n";
  } else {
    exact_headers_stream << "sec-fetch-mode: cors\n";
  }
  
  if (!sec_fetch_site_.empty()) {
    exact_headers_stream << "sec-fetch-site: " << sec_fetch_site_ << "\n";
  } else {
    exact_headers_stream << "sec-fetch-site: same-origin\n";
  }
  
  // Add additional headers that might be missing
  exact_headers_stream << "dnt: 1\n";
  exact_headers_stream << "sec-ch-ua: \"Chromium\";v=\"127\", \"Not)A;Brand\";v=\"99\"\n";
  exact_headers_stream << "sec-ch-ua-mobile: ?1\n";
  exact_headers_stream << "sec-ch-ua-platform: \"Android\"\n";
  
  std::string exact_headers_string = exact_headers_stream.str();
  
  LOG(INFO) << "Aaditesh -> 📊 Prepared " << std::count(exact_headers_string.begin(), exact_headers_string.end(), '\n') << " headers for browser API";
  LOG(INFO) << "Aaditesh -> 🌐 URL: " << hometimeline_complete_url_;
  LOG(INFO) << "Aaditesh -> 📝 Method: GET";
  LOG(INFO) << "Aaditesh -> 📋 Headers length: " << exact_headers_string.length() << " chars";
  
  // DEBUG: Log the complete headers string being sent to browser in CURL format
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: Headers being sent to browser (CURL format):";
  std::vector<std::string> header_lines = base::SplitString(
      exact_headers_string, "\n", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  
  for (const auto& header_line : header_lines) {
    size_t colon_pos = header_line.find(':');
    if (colon_pos != std::string::npos) {
      std::string name = header_line.substr(0, colon_pos);
      std::string value = header_line.substr(colon_pos + 1);
      base::TrimWhitespaceASCII(name, base::TRIM_ALL, &name);
      base::TrimWhitespaceASCII(value, base::TRIM_ALL, &value);
      
      // Mask sensitive values for logging
      std::string masked_value = value;
      if (name == "authorization" || name == "cookie" || name == "x-csrf-token" || name == "x-client-transaction-id") {
        if (value.length() > 20) {
          masked_value = value.substr(0, 20) + "...[" + std::to_string(value.length()) + " chars total]";
        }
      }
      LOG(INFO) << "Aaditesh ->   -H '" << name << ": " << masked_value << "'";
    }
  }
  LOG(INFO) << "Aaditesh -> 🔍 DEBUG: End of CURL format headers";
  
  // Make the browser-side API call
  wootz_scraping_service_->MakeTwitterGraphQLCall(
      hometimeline_complete_url_,  // URL
      "GET",                       // Method
      exact_headers_string,        // Headers (EXACT SAME AS WORKING CURL)
      "",                          // Body (empty for GET)
      base::BindOnce(&TwitterScrapingBridge::OnBrowserSideTwitterGraphQLResponse,
                     weak_factory_.GetWeakPtr()));
  
  LOG(INFO) << "Aaditesh -> ✅ Browser-side API call initiated with EXACT CURL headers - waiting for response...";
  LogToAndroidConsole("✅ Browser-side API call sent with EXACT CURL headers - waiting for response...");
  
  LOG(INFO) << "Aaditesh -> ===== MakeTwitterGraphQLCall() END =====";
  
  // Note: is_making_api_call_ flag will be reset in the callback OnBrowserSideTwitterGraphQLResponse
}

// Callback for browser-side Twitter GraphQL API response
void TwitterScrapingBridge::OnBrowserSideTwitterGraphQLResponse(
    int32_t status_code, 
    const std::string& response_body, 
    const std::string& error_message) {
  
  // SAFETY: Reset API call flag
  is_making_api_call_ = false;
  
  LOG(INFO) << "Aaditesh -> ===== OnBrowserSideTwitterGraphQLResponse() START =====";
  LOG(INFO) << "Aaditesh -> Browser-side Twitter GraphQL API response received";
  LOG(INFO) << "Aaditesh -> Status code: " << status_code;
  LOG(INFO) << "Aaditesh -> Response body length: " << response_body.length() << " chars";
  
  if (!error_message.empty()) {
    LOG(ERROR) << "Aaditesh -> Error message: " << error_message;
    LogToAndroidConsole("❌ Twitter API Error: " + error_message);
  }
  
  // Log success/failure based on status code
  if (status_code >= 200 && status_code < 300) {
    LOG(INFO) << "Aaditesh -> ✅ SUCCESS: Twitter GraphQL API call successful";
    LogToAndroidConsole("✅ Twitter GraphQL API call successful (status: " + std::to_string(status_code) + ")");
    
    // Log first 1000 characters of response as requested
    if (!response_body.empty()) {
      std::string response_preview = response_body.length() > 1000 ? 
          response_body.substr(0, 1000) + "..." : response_body;
      LOG(INFO) << "Aaditesh -> Response preview (first 1000 chars): " << response_preview;
      LogToAndroidConsole("📄 Response preview: " + response_preview);
      
      // Store successful response for batch processing
      last_successful_response_ = response_body;
      has_successful_response_ = true;
      LOG(INFO) << "Aaditesh -> Successful response cached for future use";
    }
  } else {
    LOG(ERROR) << "Aaditesh -> ❌ FAILED: Twitter GraphQL API call failed";
    LogToAndroidConsole("❌ Twitter GraphQL API call failed (status: " + std::to_string(status_code) + ")");
    
    // Log response body for debugging even on failure
    if (!response_body.empty()) {
      std::string error_preview = response_body.length() > 500 ? 
          response_body.substr(0, 500) + "..." : response_body;
      LOG(ERROR) << "Aaditesh -> Error response: " << error_preview;
      LogToAndroidConsole("📄 Error response: " + error_preview);
    }
  }
  
  // Update API call count tracking
  IncrementAPICallCount();
  
  LOG(INFO) << "Aaditesh -> Total API calls made: " << api_call_count_ << "/" << MAX_API_CALLS;
  LOG(INFO) << "Aaditesh -> ===== OnBrowserSideTwitterGraphQLResponse() END =====";
}

// OnNetworkRequestComplete function removed - no longer needed

std::string TwitterScrapingBridge::BuildTwitterAPIHeaders() {
  LOG(INFO) << "Aaditesh -> ===== BuildTwitterAPIHeaders() START =====";
  
  std::ostringstream headers;
  
  // Standard headers for Twitter API with dynamic User-Agent
  std::string dynamic_user_agent = content::GetReducedUserAgent(true, "127");
  headers << "User-Agent: " << dynamic_user_agent << "\n";
  headers << "Accept: application/json, text/plain, */*\n";
  headers << "Accept-Language: en-US,en;q=0.9\n";
  headers << "Accept-Encoding: gzip, deflate, br\n";
  headers << "DNT: 1\n";
  headers << "Connection: keep-alive\n";
  headers << "Sec-Fetch-Dest: empty\n";
  headers << "Sec-Fetch-Mode: cors\n";
  headers << "Sec-Fetch-Site: same-site\n";
  
  // Add authentication headers
  if (!auth_token_.empty()) {
    headers << "Authorization: Bearer " << auth_token_ << "\n";
    LOG(INFO) << "Aaditesh -> Added Authorization header with auth_token";
  }
  
  if (!csrf_token_.empty()) {
    headers << "x-csrf-token: " << csrf_token_ << "\n";
    LOG(INFO) << "Aaditesh -> Added x-csrf-token header";
  } else if (!ct0_token_.empty()) {
    headers << "x-csrf-token: " << ct0_token_ << "\n";
    LOG(INFO) << "Aaditesh -> Added x-csrf-token header (from ct0)";
  }
  
  if (!guest_id_.empty()) {
    headers << "x-guest-token: " << guest_id_ << "\n";
    LOG(INFO) << "Aaditesh -> Added x-guest-token header";
  }
  
  // Add Twitter-specific headers
  headers << "x-twitter-auth-type: OAuth2Session\n";
  headers << "x-twitter-client-language: en\n";
  headers << "x-twitter-active-user: yes\n";
  
  // Add cookies if available
  if (!session_cookies_.empty()) {
    headers << "Cookie: " << session_cookies_ << "\n";
    LOG(INFO) << "Aaditesh -> Added session cookies";
  }
  
  std::string result = headers.str();
  LOG(INFO) << "Aaditesh -> Built headers (length: " << result.length() << " characters)";
  LOG(INFO) << "Aaditesh -> ===== BuildTwitterAPIHeaders() END =====";
  
  return result;
}

std::string TwitterScrapingBridge::BuildTwitterAPIUrl(const std::string& endpoint) {
  LOG(INFO) << "Aaditesh -> ===== BuildTwitterAPIUrl() START =====";
  LOG(INFO) << "Aaditesh -> Building URL for endpoint: " << endpoint;
  
  std::string base_url;
  
  // Determine base URL based on endpoint
  if (endpoint.find("/1.1/") == 0) {
    base_url = "https://api.twitter.com";
    LOG(INFO) << "Aaditesh -> Using API v1.1 base URL";
  } else if (endpoint.find("/2/") == 0) {
    base_url = "https://api.twitter.com";
    LOG(INFO) << "Aaditesh -> Using API v2 base URL";
  } else if (endpoint.find("/graphql/") == 0) {
    base_url = "https://twitter.com";
    LOG(INFO) << "Aaditesh -> Using GraphQL base URL";
  } else {
    // Default to API v2
    base_url = "https://api.twitter.com";
    LOG(INFO) << "Aaditesh -> Using default API v2 base URL";
  }
  
  std::string full_url = base_url + endpoint;
  
  LOG(INFO) << "Aaditesh -> Complete URL: " << full_url;
  LOG(INFO) << "Aaditesh -> ===== BuildTwitterAPIUrl() END =====";
  
  return full_url;
}

// ============================================================================
// END OF CURL FUNCTIONALITY IMPLEMENTATION
// ============================================================================

// TwitterInterceptor implementation
wootz_scraping::TwitterInterceptor::TwitterInterceptor(content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame),
      bridge_(std::make_unique<TwitterScrapingBridge>(this)) {
  LOG(INFO) << "Aaditesh -> TwitterInterceptor created for frame: " 
            << render_frame->GetRoutingID();
  
  // Register this interceptor in the global map
  g_frame_interceptors[render_frame] = this;
}

wootz_scraping::TwitterInterceptor::~TwitterInterceptor() {
  // Remove this interceptor from the global map
  if (render_frame()) {
    g_frame_interceptors.erase(render_frame());
    LOG(INFO) << "Aaditesh -> TwitterInterceptor destroyed for frame: " 
              << render_frame()->GetRoutingID();
  }
}

void wootz_scraping::TwitterInterceptor::OnDestruct() {
  // Remove from global map before destruction
  if (render_frame()) {
    g_frame_interceptors.erase(render_frame());
  }
  // This method is called when the RenderFrame is destroyed
  delete this;
}

void wootz_scraping::TwitterInterceptor::CreateForFrame(content::RenderFrame* render_frame) {
  DCHECK(render_frame);
  
  // Check if an interceptor already exists for this frame
  if (g_frame_interceptors.find(render_frame) != g_frame_interceptors.end()) {
    LOG(INFO) << "Aaditesh -> TwitterInterceptor already exists for frame: " 
              << render_frame->GetRoutingID() << " - skipping creation";
    return;
  }
  
  LOG(INFO) << "Aaditesh -> Creating new TwitterInterceptor for frame: " 
            << render_frame->GetRoutingID();
  
  // Observer will delete itself when the frame is destroyed via OnDestruct
  new TwitterInterceptor(render_frame);
}

bool wootz_scraping::TwitterInterceptor::ExistsForFrame(content::RenderFrame* render_frame) {
  return g_frame_interceptors.find(render_frame) != g_frame_interceptors.end();
}

bool wootz_scraping::TwitterInterceptor::IsTwitterUrl(const GURL& url) {
  return url.host() == "twitter.com" || 
         url.host() == "x.com" || 
         url.host() == "www.twitter.com" || 
         url.host() == "www.x.com" ||
         url.host() == "mobile.twitter.com";
}

void wootz_scraping::TwitterInterceptor::DidCreateDocumentElement() {
  if (!render_frame() || !render_frame()->GetWebFrame()) {
    return;
  }
  
  GURL url = render_frame()->GetWebFrame()->GetDocument().Url();
  if (IsTwitterUrl(url)) {    InstallInterceptor();
  }
}

void wootz_scraping::TwitterInterceptor::DidFinishLoad() {
  if (!render_frame() || !render_frame()->GetWebFrame()) {
    return;
  }
  
  GURL url = render_frame()->GetWebFrame()->GetDocument().Url();
  if (IsTwitterUrl(url)) {
    InstallInterceptor();
  }
}

void wootz_scraping::TwitterInterceptor::WillReleaseScriptContext(v8::Local<v8::Context> context, 
                                                 int32_t world_id) {
  if (world_id != content::ISOLATED_WORLD_ID_GLOBAL) {
    return;
  }
  
  if (bridge_) {
    bridge_->Cleanup();
  }
}

void wootz_scraping::TwitterInterceptor::InstallInterceptor() {
  // Safety check: ensure we're still alive
  if (!render_frame()) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: render frame not available";
    return;
  }
  
  blink::WebLocalFrame* web_frame = render_frame()->GetWebFrame();
  if (!web_frame) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: no web frame available";
    return;
  }
  
  // Check if V8 is available and isolate is valid
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  if (!isolate) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: no V8 isolate available";
    return;
  }
  
  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = web_frame->MainWorldScriptContext();
  if (context.IsEmpty()) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: no context available";
    return;
  }
  
  // Check if context is still valid
  if (!context->GetIsolate()) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: context isolate is null";
    return;
  }
  
  v8::Context::Scope context_scope(context);
  
  if (!bridge_) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: bridge not available";
    return;
  }
  
  // Log bridge creation attempt
  LOG(INFO) << "Aaditesh -> Creating TwitterScrapingBridge for JavaScript exposure";
  
  v8::TryCatch try_catch(isolate);
  
  // Create the bridge object and expose it to JavaScript
  v8::Local<v8::Object> global = context->Global();
  if (global.IsEmpty()) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: global object is empty";
    return;
  }
  
  gin::Handle<TwitterScrapingBridge> bridge = 
      gin::CreateHandle(isolate, bridge_.get());
  if (bridge.IsEmpty()) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: could not create gin handle";
    return;
  }
  
  v8::Local<v8::Value> bridge_value = bridge.ToV8();
  if (bridge_value.IsEmpty()) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: could not convert bridge to V8 value";
    return;
  }
  
  // Add bridge to window.twitterScrapingBridge
  v8::Maybe<bool> result = global->CreateDataProperty(
      context, 
      gin::StringToV8(isolate, "twitterScrapingBridge"), 
      bridge_value);
  
  if (result.IsNothing() || !result.FromJust()) {
    LOG(ERROR) << "Aaditesh -> Failed to install interceptor: could not create bridge property";
    if (try_catch.HasCaught()) {
      v8::String::Utf8Value exception(isolate, try_catch.Exception());
      LOG(ERROR) << "Aaditesh -> V8 Exception: " << *exception;
    }
    return;
  }
  
  // Log successful bridge installation
  LOG(INFO) << "Aaditesh -> ✅ TwitterScrapingBridge successfully exposed to window.twitterScrapingBridge";
  
  // Get the interceptor script and execute it with error handling
  blink::WebScriptSource script_source = GetTwitterInterceptorScriptSource();
  web_frame->ExecuteScript(script_source);
  
  // Check for V8 exceptions after script execution
  if (try_catch.HasCaught()) {
    v8::String::Utf8Value exception(isolate, try_catch.Exception());
    LOG(ERROR) << "Aaditesh -> V8 Exception during script execution: " << *exception;
    return;
  }
  
  LOG(INFO) << "Aaditesh -> ✅ Twitter interceptor script executed successfully";
  
  web_frame->AddMessageToConsole(
      blink::WebConsoleMessage(blink::mojom::ConsoleMessageLevel::kInfo,
                             "Twitter API interceptor installed"));
  
  LOG(INFO) << "Aaditesh -> ✅ TwitterInterceptor installation completed successfully";
}

// Load the interceptor script from resources
std::string wootz_scraping::TwitterInterceptor::LoadInterceptorScript() {
  return GetTwitterInterceptorScript();
}

// Modify the interceptor script to send data to our bridge
std::string wootz_scraping::TwitterInterceptor::ModifyInterceptorScript(const std::string& script) {
  // Add code to send data to our bridge at the end of the original script
  std::string modified_script = script;
  
  // Add before the final closing parentheses
  size_t pos = modified_script.rfind("})();");
  if (pos != std::string::npos) {
    std::string bridge_code = R"(
    // Send data to C++ bridge
    function sendDataToBridge(data) {
        if (window.twitterScrapingBridge && window.twitterScrapingBridge.onDataCaptured) {
            window.twitterScrapingBridge.onDataCaptured(JSON.stringify(data));
        }
    }
    
    // Add interval to periodically send timeline data
    setInterval(function() {
        if (window.timelineRequestData && window.timelineRequestData.url) {
            sendDataToBridge(window.timelineRequestData);
        }
        if (window.likedTweetsRequestData && window.likedTweetsRequestData.url) {
            sendDataToBridge(window.likedTweetsRequestData);
        }
    }, 5000);
    )";
    
    modified_script.insert(pos, bridge_code);
  }
  
  return modified_script;
}

// Batch processing method implementations
bool TwitterScrapingBridge::CanMakeAPICall() const {
  return api_call_count_ < MAX_API_CALLS && !api_calls_exhausted_;
}

void TwitterScrapingBridge::IncrementAPICallCount() {
  api_call_count_++;
  LOG(INFO) << "Aaditesh -> 📊 API call count incremented to: " << api_call_count_ << "/" << MAX_API_CALLS;
  
  if (api_call_count_ >= MAX_API_CALLS) {
    api_calls_exhausted_ = true;
    LOG(INFO) << "Aaditesh -> 🚫 API call limit reached - no more calls will be made";
    LogToAndroidConsole("🚫 API call limit reached - no more network requests");
  }
}

void TwitterScrapingBridge::ResetAPICallCounter() {
  api_call_count_ = 0;
  api_calls_exhausted_ = false;
  has_successful_response_ = false;
  last_successful_response_.clear();
  LOG(INFO) << "Aaditesh -> 🔄 API call counter reset - can make up to " << MAX_API_CALLS << " calls again";
}

bool TwitterScrapingBridge::HasSuccessfulResponse() const {
  return has_successful_response_ && !last_successful_response_.empty();
}

std::string TwitterScrapingBridge::GetCachedResponse() const {
  return last_successful_response_;
}

bool TwitterScrapingBridge::AreAPICallsExhausted() const {
  return api_calls_exhausted_;
}

bool TwitterScrapingBridge::ValidateCapturedUrl() const {
  LOG(INFO) << "Aaditesh -> ===== ValidateCapturedUrl() START =====";
  
  if (hometimeline_complete_url_.empty()) {
    LOG(WARNING) << "Aaditesh -> URL validation failed: URL is empty";
    return false;
  }
  
  // Enhanced validation: Check for both 'variables=' and 'features=' parameters
  bool has_features = hometimeline_complete_url_.find("features=") != std::string::npos;
  bool has_variables = hometimeline_complete_url_.find("variables=") != std::string::npos;
  
  LOG(INFO) << "Aaditesh -> URL validation check:";
  LOG(INFO) << "  - Contains features=: " << (has_features ? "YES" : "NO");
  LOG(INFO) << "  - Contains variables=: " << (has_variables ? "YES" : "NO");
  
  if (!has_features || !has_variables) {
    LOG(WARNING) << "Aaditesh -> URL validation failed: Missing required parameters";
    LOG(WARNING) << "  - Missing features parameter: " << (!has_features ? "YES" : "NO");
    LOG(WARNING) << "  - Missing variables parameter: " << (!has_variables ? "YES" : "NO");
    return false;
  }
  
  // Additional content validation: Ensure parameters have meaningful content
  bool valid_features = false;
  bool valid_variables = false;
  
  // Extract and validate features parameter content
  size_t features_pos = hometimeline_complete_url_.find("features=");
  if (features_pos != std::string::npos) {
    size_t start = features_pos + 9; // "features=" length
    size_t end = hometimeline_complete_url_.find("&", start);
    if (end == std::string::npos) end = hometimeline_complete_url_.length();
    
    std::string features_content = hometimeline_complete_url_.substr(start, end - start);
    valid_features = !features_content.empty() && features_content.length() > 10;
    LOG(INFO) << "  - Features content length: " << features_content.length() << " chars";
  }
  
  // Extract and validate variables parameter content
  size_t variables_pos = hometimeline_complete_url_.find("variables=");
  if (variables_pos != std::string::npos) {
    size_t start = variables_pos + 10; // "variables=" length
    size_t end = hometimeline_complete_url_.find("&", start);
    if (end == std::string::npos) end = hometimeline_complete_url_.length();
    
    std::string variables_content = hometimeline_complete_url_.substr(start, end - start);
    valid_variables = !variables_content.empty() && variables_content.length() > 5;
    LOG(INFO) << "  - Variables content length: " << variables_content.length() << " chars";
  }
  
  bool has_valid_content = valid_features && valid_variables;
  LOG(INFO) << "  - Features content valid: " << (valid_features ? "YES" : "NO");
  LOG(INFO) << "  - Variables content valid: " << (valid_variables ? "YES" : "NO");
  LOG(INFO) << "  - Overall validation result: " << (has_valid_content ? "PASS" : "FAIL");
  
  if (!has_valid_content) {
    LOG(WARNING) << "Aaditesh -> URL validation failed: Insufficient parameter content";
    if (!valid_features) {
      LOG(WARNING) << "  - Features parameter has insufficient content (need >10 chars)";
    }
    if (!valid_variables) {
      LOG(WARNING) << "  - Variables parameter has insufficient content (need >5 chars)";
    }
  }
  
  LOG(INFO) << "Aaditesh -> ===== ValidateCapturedUrl() END =====";
  return has_valid_content;
}

}  // namespace wootz_scraping