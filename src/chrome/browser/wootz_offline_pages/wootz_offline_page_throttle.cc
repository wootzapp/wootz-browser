// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_throttle.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_prefs.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service_factory.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/page_transition_types.h"
#include "url/gurl.h"

// static
std::unique_ptr<WootzOfflinePageThrottle>
WootzOfflinePageThrottle::MaybeCreateThrottleFor(
    content::NavigationHandle* handle) {
  if (!handle->IsInMainFrame())
    return nullptr;
  return std::make_unique<WootzOfflinePageThrottle>(handle);
}

WootzOfflinePageThrottle::WootzOfflinePageThrottle(
    content::NavigationHandle* handle)
    : content::NavigationThrottle(handle) {}

WootzOfflinePageThrottle::~WootzOfflinePageThrottle() = default;

content::NavigationThrottle::ThrottleCheckResult
WootzOfflinePageThrottle::WillStartRequest() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  LOG(INFO) << "Kartik: WootzOfflinePageThrottle::WillStartRequest for "
            << navigation_handle()->GetURL().spec();
  return CheckForOfflinePage();
}

content::NavigationThrottle::ThrottleCheckResult
WootzOfflinePageThrottle::WillRedirectRequest() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  LOG(INFO) << "Kartik: WootzOfflinePageThrottle::WillRedirectRequest for "
            << navigation_handle()->GetURL().spec();
  return CheckForOfflinePage();
}

content::NavigationThrottle::ThrottleCheckResult
WootzOfflinePageThrottle::CheckForOfflinePage() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  
  GURL url = navigation_handle()->GetURL();
  
  // Only handle HTTP and HTTPS URLs - skip all special schemes
  if (!url.SchemeIsHTTPOrHTTPS()) {
    return content::NavigationThrottle::PROCEED;
  }

  Profile* profile = Profile::FromBrowserContext(
      navigation_handle()->GetWebContents()->GetBrowserContext());
  
  // if (!profile->GetPrefs()->GetBoolean(
  //         wootz_offline_pages::prefs::kAutoOpenOfflinePages)) {
  //   LOG(INFO) << "Kartik: Auto-open offline pages disabled, proceeding with online navigation";
  //   return content::NavigationThrottle::PROCEED;
  // }
  
  WootzOfflinePageService* service =
      WootzOfflinePageServiceFactory::GetForProfile(profile);

  base::FilePath mhtml_path;
  if (service->GetOfflinePagePath(url, &mhtml_path)) {
    LOG(INFO) << "Kartik: Found offline page at " << mhtml_path.value();
    
    if (!base::PathExists(mhtml_path)) {
      LOG(ERROR) << "Kartik: Offline page file does not exist";
      return content::NavigationThrottle::PROCEED;
    }
    
    std::string content_uri = "content://org.chromium.chrome.wootz_offline_pages" + 
                              mhtml_path.value();
    GURL content_url(content_uri);
    
    LOG(INFO) << "Kartik: Loading offline page via: " << content_url.spec();
    
    content::NavigationController::LoadURLParams params(content_url);
    params.transition_type = ui::PAGE_TRANSITION_CLIENT_REDIRECT;
    navigation_handle()->GetWebContents()->GetController().LoadURLWithParams(params);
    
    return content::NavigationThrottle::ThrottleCheckResult(
        content::NavigationThrottle::CANCEL);
  }

  // NOT FOUND - Check if this is a LinkedIn search page
  std::string host = url.host();
  std::string path = url.path();
  bool is_linkedin = (host.find("linkedin.com") != std::string::npos);
  bool is_search_page = (path.find("/search") != std::string::npos);
  
  base::FilePath fallback_page_path;
  std::string page_type;
  
  if (is_linkedin && is_search_page) {
    // For LinkedIn search pages, use the "no result found" page
    fallback_page_path = service->GetNoResultFoundPagePath();
    page_type = "no result found";
    LOG(INFO) << "Kartik: LinkedIn search page not found: " << url.spec() 
              << " - loading no result found page";
  } else {
    // For other pages, use the standard 404 page
    fallback_page_path = service->Get404PagePath();
    page_type = "404";
    LOG(INFO) << "Kartik: No offline page for " << url.spec() 
              << " - loading 404 page";
  }
  
  if (!fallback_page_path.empty() && base::PathExists(fallback_page_path)) {
    std::string content_uri = "content://org.chromium.chrome.wootz_offline_pages" + 
                              fallback_page_path.value();
    GURL content_url(content_uri);
    
    LOG(INFO) << "Kartik: Loading " << page_type << " page: " << content_url.spec();
    
    content::NavigationController::LoadURLParams params(content_url);
    params.transition_type = ui::PAGE_TRANSITION_CLIENT_REDIRECT;
    navigation_handle()->GetWebContents()->GetController().LoadURLWithParams(params);
    
    return content::NavigationThrottle::ThrottleCheckResult(
        content::NavigationThrottle::CANCEL);
  }
  
  // No fallback page available - proceed with online navigation
  LOG(INFO) << "Kartik: No " << page_type << " page available, proceeding with online navigation";
  return content::NavigationThrottle::PROCEED;
}

const char* WootzOfflinePageThrottle::GetNameForLogging() {
  return "WootzOfflinePageThrottle";
}
