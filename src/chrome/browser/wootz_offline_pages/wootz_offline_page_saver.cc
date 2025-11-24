// Copyright 2024 Wootzapp Authors
// Use of a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_saver.h"

#include "base/logging.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_prefs.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service_factory.h"
#include "components/prefs/pref_service.h"
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

  // Cancel any pending saves/updates from previous navigation
  delayed_save_timer_.Stop();
  periodic_update_timer_.Stop();
  periodic_update_count_ = 0;
  
  // Store redirect chain for later use when page is fully loaded
  last_redirect_chain_ = navigation_handle->GetRedirectChain();
  
  // Log redirect chain for debugging
  if (last_redirect_chain_.size() > 1) {
    LOG(INFO) << "Kartik: Redirect chain detected with " << last_redirect_chain_.size() << " URLs:";
    for (size_t i = 0; i < last_redirect_chain_.size(); i++) {
      LOG(INFO) << "  [" << i << "] " << last_redirect_chain_[i].spec();
    }
  }
  
  // Reset save state for new navigation
  save_in_progress_ = false;
  
  LOG(INFO) << "Kartik: WootzOfflinePageSaver::DidFinishNavigation for "
            << navigation_handle->GetURL().spec() 
            << " - waiting for page to fully load";
}

void WootzOfflinePageSaver::DidStopLoading() {
  // Cancel any pending delayed saves
  delayed_save_timer_.Stop();
  
  // Schedule a delayed save to allow dynamic content to load
  ScheduleDelayedSave();
}

void WootzOfflinePageSaver::DocumentOnLoadCompletedInPrimaryMainFrame() {
  LOG(INFO) << "Kartik: DocumentOnLoadCompletedInPrimaryMainFrame - onload event fired";
  
  // Also schedule a delayed save here as an additional trigger point
  // This fires when the page's onload event completes
  ScheduleDelayedSave();
}

void WootzOfflinePageSaver::ScheduleDelayedSave() {
  // Only schedule if we have a valid redirect chain from a previous navigation
  if (last_redirect_chain_.empty())
    return;

  GURL url = web_contents()->GetLastCommittedURL();
  if (!url.SchemeIsHTTPOrHTTPS())
    return;

  // Check if offline browsing is enabled
  Profile* profile = Profile::FromBrowserContext(web_contents()->GetBrowserContext());
  if (!profile->GetPrefs()->GetBoolean(
          wootz_offline_pages::prefs::kOfflineBrowsingEnabled)) {
    LOG(INFO) << "Kartik: Offline browsing disabled, skipping save for " << url.spec();
    last_redirect_chain_.clear();
    return;
  }

  LOG(INFO) << "Kartik: Scheduling delayed save for " << url.spec()
            << " in 2 seconds to allow dynamic content to load";
  
  // Schedule save after 2 seconds to allow JavaScript/dynamic content to load
  delayed_save_timer_.Start(
      FROM_HERE,
      base::Seconds(2),
      base::BindOnce(&WootzOfflinePageSaver::PerformDelayedSave,
                     weak_factory_.GetWeakPtr()));
}

void WootzOfflinePageSaver::PerformDelayedSave() {
  // Only save if we have a valid redirect chain
  if (last_redirect_chain_.empty())
    return;

  GURL url = web_contents()->GetLastCommittedURL();
  if (!url.SchemeIsHTTPOrHTTPS())
    return;

  // Prevent concurrent save operations
  if (save_in_progress_) {
    LOG(WARNING) << "Kartik: Save already in progress for this WebContents, skipping";
    return;
  }
  
  // Prevent saving the same URL multiple times (unless it's a periodic update)
  if (url == last_saved_url_ && periodic_update_count_ == 0) {
    LOG(INFO) << "Kartik: URL already saved: " << url.spec() << ", skipping duplicate save";
    last_redirect_chain_.clear();
    return;
  }

  LOG(INFO) << "Kartik: Performing delayed save for " << url.spec();
  
  // Mark save as in progress
  save_in_progress_ = false;  // Reset for the actual save
  last_saved_url_ = url;
  
  GetService()->SavePage(web_contents(), last_redirect_chain_);
  
  // Schedule periodic updates to capture dynamic content changes
  SchedulePeriodicUpdate();
  
  // Clear redirect chain after initiating save
  last_redirect_chain_.clear();
}

void WootzOfflinePageSaver::SchedulePeriodicUpdate() {
  // Stop any existing periodic timer
  periodic_update_timer_.Stop();
  periodic_update_count_ = 0;
  
  GURL url = web_contents()->GetLastCommittedURL();
  LOG(INFO) << "Kartik: Scheduling periodic updates for " << url.spec()
            << " every 5 seconds (max 3 updates)";
  
  // Schedule periodic updates every 5 seconds
  periodic_update_timer_.Start(
      FROM_HERE,
      base::Seconds(5),
      base::BindRepeating(&WootzOfflinePageSaver::PerformPeriodicUpdate,
                          weak_factory_.GetWeakPtr()));
}

void WootzOfflinePageSaver::PerformPeriodicUpdate() {
  periodic_update_count_++;
  
  // Stop after 3 periodic updates (15 seconds total)
  if (periodic_update_count_ > 3) {
    LOG(INFO) << "Kartik: Maximum periodic updates reached, stopping";
    periodic_update_timer_.Stop();
    return;
  }
  
  GURL url = web_contents()->GetLastCommittedURL();
  if (!url.SchemeIsHTTPOrHTTPS()) {
    periodic_update_timer_.Stop();
    return;
  }
  
  // Check if offline browsing is still enabled
  Profile* profile = Profile::FromBrowserContext(web_contents()->GetBrowserContext());
  if (!profile->GetPrefs()->GetBoolean(
          wootz_offline_pages::prefs::kOfflineBrowsingEnabled)) {
    LOG(INFO) << "Kartik: Offline browsing disabled, stopping periodic updates";
    periodic_update_timer_.Stop();
    return;
  }
  
  LOG(INFO) << "Kartik: Performing periodic update #" << periodic_update_count_
            << " for " << url.spec();
  
  // Create a single-element redirect chain for the update
  std::vector<GURL> update_chain = {url};
  GetService()->SavePage(web_contents(), update_chain);
}

WootzOfflinePageService* WootzOfflinePageSaver::GetService() {
  Profile* profile =
      Profile::FromBrowserContext(web_contents()->GetBrowserContext());
  return WootzOfflinePageServiceFactory::GetForProfile(profile);
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(WootzOfflinePageSaver);
