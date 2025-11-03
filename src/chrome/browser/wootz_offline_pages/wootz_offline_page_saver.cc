// Copyright 2024 Wootzapp Authors
// Use of a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_saver.h"

#include "base/logging.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service_factory.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents.h"

WootzOfflinePageSaver::WootzOfflinePageSaver(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<WootzOfflinePageSaver>(*web_contents) {}

WootzOfflinePageSaver::~WootzOfflinePageSaver() = default;

void WootzOfflinePageSaver::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  if (!navigation_handle->IsInPrimaryMainFrame() ||
      !navigation_handle->HasCommitted() ||
      navigation_handle->IsErrorPage() ||
      !navigation_handle->GetURL().SchemeIsHTTPOrHTTPS()) {
    return;
  }

  // Store redirect chain for later use when page is fully loaded
  last_redirect_chain_ = navigation_handle->GetRedirectChain();
  
  LOG(INFO) << "Kartik: WootzOfflinePageSaver::DidFinishNavigation for "
            << navigation_handle->GetURL().spec() 
            << " - waiting for page to fully load";
}

void WootzOfflinePageSaver::DidStopLoading() {
  // Only save if we have a valid redirect chain from a previous navigation
  if (last_redirect_chain_.empty())
    return;

  GURL url = web_contents()->GetLastCommittedURL();
  if (!url.SchemeIsHTTPOrHTTPS())
    return;

  LOG(INFO) << "Kartik: WootzOfflinePageSaver::DidStopLoading for " << url.spec()
            << " - page fully loaded, saving now";
  
  GetService()->SavePage(web_contents(), last_redirect_chain_);
  
  // Clear redirect chain after saving
  last_redirect_chain_.clear();
}

WootzOfflinePageService* WootzOfflinePageSaver::GetService() {
  Profile* profile =
      Profile::FromBrowserContext(web_contents()->GetBrowserContext());
  return WootzOfflinePageServiceFactory::GetForProfile(profile);
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(WootzOfflinePageSaver);
