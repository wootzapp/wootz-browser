// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/activity_tracking_service.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "chrome/browser/activity_tracking/activity_data_store.h"
#include "chrome/browser/activity_tracking/navigation_event_collector.h"
#include "chrome/browser/activity_tracking/privacy_filter.h"
#include "chrome/browser/activity_tracking/splunk_observability_exporter.h"
#include "chrome/browser/activity_tracking/tab_event_collector.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_contents.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#else
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list_observer.h"
#endif

namespace activity_tracking {

ActivityTrackingService::ActivityTrackingService(Profile* profile)
    : profile_(profile) {
  // Initialize core components
  privacy_filter_ = std::make_unique<PrivacyFilter>(profile_);
  data_store_ = std::make_unique<ActivityDataStore>();
  navigation_collector_ =
      std::make_unique<NavigationEventCollector>(privacy_filter_.get());
  tab_collector_ = std::make_unique<TabEventCollector>(
      privacy_filter_.get(), navigation_collector_.get());
  
  // Initialize Splunk Observability Cloud exporter
  InitializeSplunkExporter();

#if !BUILDFLAG(IS_ANDROID)
  // Desktop: Start observing browsers
  BrowserList::AddObserver(this);

  // Attach to existing browsers
  for (Browser* browser : *BrowserList::GetInstance()) {
    if (browser->profile() == profile_) {
      OnBrowserAdded(browser);
    }
  }
#else
  // Android: Start observing TabModelList
  TabModelList::AddObserver(this);
  
  // Initialize with existing TabModels
  InitializeForAndroid();
#endif
}

ActivityTrackingService::~ActivityTrackingService() {
#if !BUILDFLAG(IS_ANDROID)
  BrowserList::RemoveObserver(this);
#else
  TabModelList::RemoveObserver(this);
#endif
}

void ActivityTrackingService::Shutdown() {
  // Export remaining events before shutdown
  if (navigation_collector_) {
    navigation_collector_->ExportToSplunk();
  }

  // Clean up observers
  tab_collector_->Shutdown();
  navigation_collector_->Shutdown();

#if !BUILDFLAG(IS_ANDROID)
  tracked_browsers_.clear();
#endif
}

void ActivityTrackingService::InitializeSplunkExporter() {
  // Configure Splunk Observability Cloud (SignalFx)
  SplunkObservabilityExporter::Config config;
  
  // Splunk Observability Cloud Singapore (sg0) ingest endpoint
  config.realm = "sg0";
  config.ingest_endpoint = "https://ingest.sg0.signalfx.com";
  config.access_token = "4aQ3BWjlQTrbdAF3Vn3aYA";
  config.timeout_ms = 30000;  // 30 seconds
  config.enable_metrics = true;
  config.enable_events = true;
  
  // Create exporter with profile's URL loader factory
  auto exporter = std::make_unique<SplunkObservabilityExporter>(
      profile_->GetURLLoaderFactory(),
      config);
  
  // Set exporter in navigation collector
  navigation_collector_->SetExporter(std::move(exporter));
  
  // Start periodic export timer (export every 30 seconds)
  StartPeriodicExport();
}

void ActivityTrackingService::StartPeriodicExport() {
  export_timer_.Start(
      FROM_HERE,
      base::Seconds(30),
      base::BindRepeating(&ActivityTrackingService::OnExportTimerFired,
                          base::Unretained(this)));
}

void ActivityTrackingService::OnExportTimerFired() {
  if (navigation_collector_) {
    navigation_collector_->ExportToSplunk();
  }
}

#if !BUILDFLAG(IS_ANDROID)
// Desktop-only methods

void ActivityTrackingService::OnBrowserAdded(Browser* browser) {
  if (browser->profile() != profile_) {
    return;
  }

  tracked_browsers_.push_back(browser);

  // Start observing the tab strip model
  tab_collector_->ObserveTabStripModel(browser->tab_strip_model());

  // Observe existing tabs
  TabStripModel* tab_strip = browser->tab_strip_model();
  for (int i = 0; i < tab_strip->count(); ++i) {
    content::WebContents* web_contents = tab_strip->GetWebContentsAt(i);
    OnWebContentsCreated(web_contents);
  }
}

void ActivityTrackingService::OnBrowserRemoved(Browser* browser) {
  auto it = std::find(tracked_browsers_.begin(), tracked_browsers_.end(),
                      browser);
  if (it != tracked_browsers_.end()) {
    tab_collector_->StopObservingTabStripModel(browser->tab_strip_model());
    tracked_browsers_.erase(it);
  }
}

void ActivityTrackingService::OnWebContentsCreated(
    content::WebContents* web_contents) {
  // Attach navigation collector to this WebContents
  navigation_collector_->ObserveWebContents(web_contents);
}
#endif  // !BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(IS_ANDROID)
void ActivityTrackingService::InitializeForAndroid() {
  // Observe all TabModels
  const TabModelList::TabModelVector& models = TabModelList::models();
  
  for (TabModel* tab_model : models) {
    if (tab_model->GetProfile() == profile_) {
      tab_collector_->ObserveTabModel(tab_model);

      // Attach navigation collectors to existing tabs
      for (int i = 0; i < tab_model->GetTabCount(); ++i) {
        content::WebContents* web_contents = tab_model->GetWebContentsAt(i);
        if (web_contents) {
          navigation_collector_->ObserveWebContents(web_contents);
        }
      }
    }
  }
}

void ActivityTrackingService::OnTabModelAdded() {
  // Re-scan all TabModels and observe the new one
  const TabModelList::TabModelVector& models = TabModelList::models();
  for (TabModel* tab_model : models) {
    if (tab_model->GetProfile() == profile_) {
      // ObserveTabModel checks if already observing, so safe to call
      tab_collector_->ObserveTabModel(tab_model);
      
      // Attach navigation observers to tabs in this model
      for (int i = 0; i < tab_model->GetTabCount(); ++i) {
        content::WebContents* web_contents = tab_model->GetWebContentsAt(i);
        if (web_contents) {
          navigation_collector_->ObserveWebContents(web_contents);
        }
      }
    }
  }
}

void ActivityTrackingService::OnTabModelRemoved() {
  // TabModel cleanup is handled automatically when TabModel is destroyed
}
#endif  // BUILDFLAG(IS_ANDROID)

}  // namespace activity_tracking

