// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_TRACKING_SERVICE_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_TRACKING_SERVICE_H_

#include <memory>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "base/timer/timer.h"
#include "build/build_config.h"
#include "components/keyed_service/core/keyed_service.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/browser_list_observer.h"
class Browser;
#else
#include "chrome/browser/ui/android/tab_model/tab_model_list_observer.h"
#endif

class Profile;

namespace content {
class WebContents;
}

namespace activity_tracking {

class NavigationEventCollector;
class TabEventCollector;
class ActivityDataStore;
class PrivacyFilter;

// ActivityTrackingService is the main coordination service for tracking
// user navigation and tab activities in the browser. This service is
// designed for enterprise deployments and research builds where detailed
// activity tracking is required.
//
// The service manages:
// - Navigation event collection (URLs, referrers, timing)
// - Tab operation tracking (open, close, switch)
// - Data storage and export
// - Privacy filtering
//
// Data can be exported in OpenTelemetry JSON format for integration with
// systems like Splunk via HEC (HTTP Event Collector).
class ActivityTrackingService : public KeyedService
#if !BUILDFLAG(IS_ANDROID)
                                 , public BrowserListObserver
#else
                                 , public TabModelListObserver
#endif
{
 public:
  explicit ActivityTrackingService(Profile* profile);
  ~ActivityTrackingService() override;

  ActivityTrackingService(const ActivityTrackingService&) = delete;
  ActivityTrackingService& operator=(const ActivityTrackingService&) = delete;

  // KeyedService implementation:
  void Shutdown() override;

#if !BUILDFLAG(IS_ANDROID)
  // BrowserListObserver implementation (Desktop only):
  void OnBrowserAdded(Browser* browser) override;
  void OnBrowserRemoved(Browser* browser) override;
#else
  // TabModelListObserver implementation (Android only):
  void OnTabModelAdded() override;
  void OnTabModelRemoved() override;
#endif

  // Android-specific initialization
  void InitializeForAndroid();

  // Accessors for testing and internal use
  NavigationEventCollector* navigation_collector() const {
    return navigation_collector_.get();
  }
  TabEventCollector* tab_collector() const { return tab_collector_.get(); }
  ActivityDataStore* data_store() const { return data_store_.get(); }
  PrivacyFilter* privacy_filter() const { return privacy_filter_.get(); }

  Profile* profile() const { return profile_; }

 private:
  // Called when a new WebContents is created
  void OnWebContentsCreated(content::WebContents* web_contents);
  
  // Initialize Splunk Observability Cloud exporter
  void InitializeSplunkExporter();
  
  // Start periodic export timer
  void StartPeriodicExport();
  
  // Called when export timer fires
  void OnExportTimerFired();

  raw_ptr<Profile> profile_;

  // Core components
  std::unique_ptr<NavigationEventCollector> navigation_collector_;
  std::unique_ptr<TabEventCollector> tab_collector_;
  std::unique_ptr<ActivityDataStore> data_store_;
  std::unique_ptr<PrivacyFilter> privacy_filter_;
  
  // Timer for periodic export
  base::RepeatingTimer export_timer_;

#if !BUILDFLAG(IS_ANDROID)
  // Track browsers we're observing (Desktop only)
  std::vector<raw_ptr<Browser>> tracked_browsers_;
#endif
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_TRACKING_SERVICE_H_

