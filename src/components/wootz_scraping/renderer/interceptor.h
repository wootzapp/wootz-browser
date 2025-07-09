/* Copyright (c) 2024 All rights reserved. */

#ifndef COMPONENTS_wootz_scraping_RENDERER_INTERCEPTOR_H_
#define COMPONENTS_wootz_scraping_RENDERER_INTERCEPTOR_H_

#include <string>
#include <memory>

#include "base/memory/weak_ptr.h"
#include "components/wootz_scraping/common/wootz_scraping.mojom.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "gin/handle.h"
#include "gin/wrappable.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "net/cookies/cookie_access_result.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/cookie_manager.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "v8/include/v8-forward.h"

namespace wootz_scraping {

// Forward declaration
class TwitterInterceptor;

// Bridge class that exposes methods to JavaScript
class TwitterScrapingBridge : public gin::Wrappable<TwitterScrapingBridge> {
 public:
  static gin::WrapperInfo kWrapperInfo;

  explicit TwitterScrapingBridge(TwitterInterceptor* interceptor);
  ~TwitterScrapingBridge() override;

  // Disallow copy and assign
  TwitterScrapingBridge(const TwitterScrapingBridge&) = delete;
  TwitterScrapingBridge& operator=(const TwitterScrapingBridge&) = delete;

  // Gin wrapper
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;
  const char* GetTypeName() override;

  // Methods exposed to JavaScript
  void OnDataCaptured(const std::string& data);
  // REMOVED: void OnFeatureDataUpdate(const std::string& encoded_features_data);
  void ExecuteAuthenticatedCurl(const std::string& curl_command);
  void OnHomeTimelineUrlCaptured(const std::string& url, const std::string& tokens);  // New method for HomeTimeline URL logging
  void OnHomeTimelineParametersIntercepted(const std::string& all_parameters);  // NEW: Comprehensive parameter logging
  bool IsReady();  // Check if bridge is working
  
  // NEW: Make API call using stored credentials
  void CallAPIWithStoredCredentials();
  
  // Global credential storage methods (exposed to JavaScript)
  // Note: Method declarations moved to private section below
  
  // Public cleanup method
  void Cleanup();

 private:
  // Log to Android console
  void LogToAndroidConsole(const std::string& message);
  
  // Log HomeTimeline URL to Android console
  void LogHomeTimelineUrl(const std::string& url, const std::string& tokens);
  
  // Process Twitter data (JavaScript accessible only)
  void ProcessTwitterData(const std::string& js_data);
  void ProcessTwitterData(base::Value::Dict data);
  void ProcessFeatureData(base::Value::Dict data);
  
  // Notify scraping system that data is ready
  void NotifyScrapingDataReady(const std::string& twitter_data);
  
  // Call browser-side token scraping via Mojom
  void CallBrowserSideTokenScraping();
  
  // Setup Mojom connection to browser process
  void EnsureMojomConnection();
  
  // Callback for browser-side token scraping result
  void OnBrowserTokenScrapingComplete(bool success, const std::optional<std::string>& tokens, const std::optional<std::string>& error_message);
  
  // Cookie combination method
  void CombineCookieStrings();
  
  // Clear stored cookie data
  void ClearCookieData();
  
  // Callback for authenticated CURL response
  void OnAuthenticatedCurlResponse(std::unique_ptr<std::string> response_body);
  
  // Batch processing methods for API call limiting
  bool CanMakeAPICall() const;
  void IncrementAPICallCount();
  void ResetAPICallCounter();
  bool HasSuccessfulResponse() const;
  std::string GetCachedResponse() const;
  bool AreAPICallsExhausted() const;
  
  // Twitter API methods
  void StoreAuthenticationTokens(const std::string& tokens_json);
  bool ValidateStoredTokens();
  void MakeTwitterAPICall(const std::string& endpoint, const std::string& method, const std::string& body);
  void MakeTwitterGraphQLCall();
  
  // Callback for browser-side Twitter GraphQL API response
  void OnBrowserSideTwitterGraphQLResponse(int32_t status_code, const std::string& response_body, const std::string& error_message);
  
  std::string GenerateTransactionId();  // Generate x-client-transaction-id for API calls
  std::string CleanCookieFormat(const std::string& cookies);  // Clean cookie formatting for API calls
  std::string ExtractCSRFFromCookies();  // Extract CSRF token from combined cookie string
  std::string BuildTwitterAPIHeaders();
  std::string BuildTwitterAPIUrl(const std::string& endpoint);
  
  // Reference to the interceptor
  TwitterInterceptor* interceptor_;  // Raw pointer, owned by interceptor
  
  // Mojom interface to communicate with browser process
  mojo::Remote<mojom::WootzScrapingService> wootz_scraping_service_;
  
  // URL loader factory for network requests
  mojo::Remote<network::mojom::URLLoaderFactory> url_loader_factory_;
  
    // HomeTimeline URL storage (complete URL with features and variables)
  std::string hometimeline_complete_url_;  // Complete HomeTimeline URL for API usage
  
  // Authentication tokens and session data
  std::string csrf_token_;
  std::string guest_id_;
  std::string guest_id_marketing_;  // CRITICAL FIX: Add missing guest tokens
  std::string guest_id_ads_;        // CRITICAL FIX: Add missing guest tokens
  std::string lang_;                // CRITICAL FIX: Add missing lang token
  std::string twid_;
  std::string personalization_id_;
  std::string auth_token_;
  std::string ct0_token_;
  std::string session_cookies_;
  
  // MISSING CRITICAL COOKIES from working CURL
  std::string gt_token_;            // gt=1930273986874605881
  std::string kdt_token_;           // kdt=qxxJnXDB0cNkqpNkWmOjsSZjK2JKw9PvECOkFSRV
  std::string att_token_;           // att=1-cXhNWtHv7nD1BDF8yn8CZaFdFoJn2vBNx8bZD6ET
  std::string cf_bm_token_;         // __cf_bm=iGUhu9IuZj9Qr6drJ_6mMQ2ds_3WdIPGxzQGPCDeTe4-1749049055-1.0.1.1-uoi1C1w8__0dcIfd1J2vIFN5TC4IlynF0A8OhMmwWmYihwN80SDqN6.z9OI91JH9Z1DaF7qylaEnn2eq6sinJgS8NLSrU6lUp3vzpbzZRMs
  
  // CRITICAL FIX: Store JavaScript-generated transaction ID to match working CURL exactly
  std::string js_transaction_id_;   // 88-character transaction ID from JavaScript generateTransactionId()
  
  // COMPREHENSIVE COOKIE SCRAPING - Complete cookie string for CURL headers
  std::string complete_cookie_string_;  // Complete cookie string ready for CURL (all accessible cookies)
  std::string js_accessible_cookies_;   // JavaScript-accessible cookies from renderer process
  std::string httponly_cookies_;        // HttpOnly cookies from browser process
  int total_cookie_count_ = 0;          // Total number of cookies captured
  std::string cookie_parsing_timestamp_; // When cookies were last captured
  
  // Additional tracking and analytics cookies
  std::string ga_token_;                // Google Analytics _ga cookie
  std::string gid_token_;               // Google Analytics _gid cookie
  std::string lang_setting_;            // Language setting cookie
  std::string dnt_setting_;             // Do Not Track setting
  std::string eu_cn_setting_;           // EU consent setting
  std::string night_mode_setting_;      // Night mode preference
  std::string rweb_optin_setting_;      // Responsive web opt-in setting
  
  // COMPREHENSIVE HEADER STORAGE - All headers for API calls
  // Authentication headers
  std::string authorization_header_;           // Authorization: Bearer token
  std::string x_csrf_token_header_;           // X-CSRF-Token header
  std::string x_client_transaction_id_header_; // X-Client-Transaction-ID header
  
  // Twitter-specific headers
  std::string x_twitter_active_user_;         // X-Twitter-Active-User header
  std::string x_twitter_auth_type_;           // X-Twitter-Auth-Type header
  std::string x_twitter_client_language_;     // X-Twitter-Client-Language header
  
  // Standard HTTP headers
  std::string content_type_header_;           // Content-Type header
  std::string accept_header_;                 // Accept header
  std::string accept_language_header_;        // Accept-Language header
  std::string user_agent_header_;             // User-Agent header
  std::string origin_header_;                 // Origin header
  std::string referer_header_;                // Referer header
  
  // Security headers
  std::string sec_fetch_dest_;                // Sec-Fetch-Dest header
  std::string sec_fetch_mode_;                // Sec-Fetch-Mode header
  std::string sec_fetch_site_;                // Sec-Fetch-Site header
  
  // Dynamic header fields captured from JavaScript browser APIs
  std::string js_bearer_token_;         // Bearer token from JavaScript
  std::string js_xp_forwarded_for_;     // X-Forwarded-For from JavaScript
  std::string js_user_agent_;           // User-Agent from navigator.userAgent
  std::string js_accept_language_;      // Accept-Language from navigator.language
  std::string js_sec_ch_ua_;            // Sec-CH-UA from navigator.userAgentData
  std::string js_sec_ch_ua_mobile_;     // Sec-CH-UA-Mobile from navigator.userAgentData
  std::string js_sec_ch_ua_platform_;   // Sec-CH-UA-Platform from navigator.userAgentData
  std::string js_referer_;              // Referer header from JavaScript
  std::string js_origin_;               // Origin header from JavaScript
  
  bool tokens_ready_ = false;
  
  // SAFETY: Prevent recursive processing and memory issues
  bool is_processing_data_ = false;
  bool is_making_api_call_ = false;
  
  // Batch processing controls for API call limiting
  static constexpr int MAX_API_CALLS = 3;
  int api_call_count_ = 0;
  bool api_calls_exhausted_ = false;
  std::string last_successful_response_;
  bool has_successful_response_ = false;
  
  // ============================================================================
  // GLOBAL CREDENTIAL STORAGE - Store successful API parameters for reuse
  // ============================================================================
  
  // Global storage for successful API call parameters
  struct StoredAPICredentials {
    StoredAPICredentials();
    ~StoredAPICredentials();
    
    std::string complete_url;
    std::string headers;
    std::string cookies;
    std::string bearer_token;
    std::string csrf_token;
    std::string transaction_id;
    bool is_valid = false;
    
    void Clear();
  };
  
  // Global credentials storage
  StoredAPICredentials global_api_credentials_;
  
  // Methods for credential management
  void StoreSuccessfulAPICredentials(const std::string& url, const std::string& headers, 
                                   const std::string& cookies, const std::string& bearer_token,
                                   const std::string& csrf_token, const std::string& transaction_id);
  bool HasStoredCredentials() const;
  void ClearStoredCredentials();
  
  // Reusable API call method using stored credentials
  void MakeAPICallWithStoredCredentials();
  
  // ============================================================================
  // END GLOBAL CREDENTIAL STORAGE
  // ============================================================================
  
  // URL validation method - checks for enhanced validation criteria
  bool ValidateCapturedUrl() const;
  
  // NEW: URL construction method to create complete URL with variables and features
  std::string ConstructCompleteURL(const std::string& base_url, const std::string& variables_json, const std::string& features_json);
  
  // Weak pointer factory to safely handle callbacks
  base::WeakPtrFactory<TwitterScrapingBridge> weak_factory_{this};
};

// Main interceptor class that observes render frames
class TwitterInterceptor : public content::RenderFrameObserver {
 public:
  explicit TwitterInterceptor(content::RenderFrame* render_frame);
  ~TwitterInterceptor() override;

  // Disallow copy and assign
  TwitterInterceptor(const TwitterInterceptor&) = delete;
  TwitterInterceptor& operator=(const TwitterInterceptor&) = delete;

  // RenderFrameObserver implementation
  void DidCreateDocumentElement() override;
  void DidFinishLoad() override;
  void WillReleaseScriptContext(v8::Local<v8::Context> context, 
                                int32_t world_id) override;
  void OnDestruct() override;

  // Static method to install the interceptor on a render frame
  static void CreateForFrame(content::RenderFrame* render_frame);
  
  // Static method to check if interceptor already exists for frame
  static bool ExistsForFrame(content::RenderFrame* render_frame);
  
  // Public accessor for render frame (needed by TwitterScrapingBridge)
  content::RenderFrame* render_frame() const { return content::RenderFrameObserver::render_frame(); }

 private:
  // Install the interceptor script
  void InstallInterceptor();

  // Check if the URL is Twitter/X.com
  bool IsTwitterUrl(const GURL& url);
  
  // Load and modify the interceptor script
  std::string LoadInterceptorScript();
  std::string ModifyInterceptorScript(const std::string& script);

  // Bridge to JavaScript
  std::unique_ptr<TwitterScrapingBridge> bridge_;
};

}  // namespace wootz_scraping

#endif  // COMPONENTS_wootz_scraping_RENDERER_INTERCEPTOR_H_