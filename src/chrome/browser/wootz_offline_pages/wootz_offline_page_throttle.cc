// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_throttle.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service_factory.h"
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

  Profile* profile = Profile::FromBrowserContext(
      navigation_handle()->GetWebContents()->GetBrowserContext());
  WootzOfflinePageService* service =
      WootzOfflinePageServiceFactory::GetForProfile(profile);

  base::FilePath mhtml_path;
  if (service->GetOfflinePagePath(navigation_handle()->GetURL(), &mhtml_path)) {
    LOG(INFO) << "Kartik: Found offline page at " << mhtml_path.value();
    
    // Build content:// URI for the MHTML file
    // Format: content://org.chromium.chrome.wootz_offline_pages/<absolute_file_path>
    std::string content_uri = "content://org.chromium.chrome.wootz_offline_pages" + 
                              mhtml_path.value();
    
    GURL content_url(content_uri);
    
    LOG(INFO) << "Kartik: Redirecting to ContentProvider URI: " << content_url.spec();
    
    // Load the content:// URI
    // The ContentProvider will serve the MHTML file with MIME type "multipart/related"
    content::NavigationController::LoadURLParams params(content_url);
    params.transition_type = ui::PAGE_TRANSITION_FORWARD_BACK;
    navigation_handle()->GetWebContents()->GetController().LoadURLWithParams(params);
    
    // Cancel the original navigation
    return content::NavigationThrottle::ThrottleCheckResult(
        content::NavigationThrottle::CANCEL);
  }

  LOG(INFO) << "Kartik: No offline page for " << navigation_handle()->GetURL().spec();
  return content::NavigationThrottle::PROCEED;
}

const char* WootzOfflinePageThrottle::GetNameForLogging() {
  return "WootzOfflinePageThrottle";
}
