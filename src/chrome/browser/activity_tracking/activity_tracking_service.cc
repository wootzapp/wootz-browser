// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/activity_tracking_service.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "chrome/browser/activity_tracking/activity_data_store.h"
#include "chrome/browser/activity_tracking/navigation_event_collector.h"
#include "chrome/browser/activity_tracking/otel_config_presets.h"
#include "chrome/browser/activity_tracking/otel_exporter.h"
#include "chrome/browser/activity_tracking/privacy_filter.h"
#include "chrome/browser/activity_tracking/security_policy_checker.h"
#include "chrome/browser/activity_tracking/tab_event_collector.h"
#include "chrome/browser/prefs/activity_tracking_prefs.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
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
  policy_checker_ = std::make_unique<SecurityPolicyChecker>(profile_->GetPrefs());
  navigation_collector_ = std::make_unique<NavigationEventCollector>(
      privacy_filter_.get(), policy_checker_.get());
  tab_collector_ = std::make_unique<TabEventCollector>(
      privacy_filter_.get(), navigation_collector_.get());
  
  // Register pref change observer
  pref_change_registrar_.Init(profile_->GetPrefs());
  pref_change_registrar_.Add(
      activity_tracking::prefs::kActivityTrackingEndpoint,
      base::BindRepeating(&ActivityTrackingService::OnActivityTrackingPrefsChanged,
                          base::Unretained(this)));
  pref_change_registrar_.Add(
      activity_tracking::prefs::kActivityTrackingToken,
      base::BindRepeating(&ActivityTrackingService::OnActivityTrackingPrefsChanged,
                          base::Unretained(this)));
  pref_change_registrar_.Add(
      activity_tracking::prefs::kActivityTrackingUserEmail,
      base::BindRepeating(&ActivityTrackingService::OnActivityTrackingPrefsChanged,
                          base::Unretained(this)));
  
  // Initialize OpenTelemetry exporter (vendor-neutral)
  InitializeOTelExporter();

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
    navigation_collector_->ExportToOTLP();
  }

  // Clean up observers
  tab_collector_->Shutdown();
  navigation_collector_->Shutdown();

#if !BUILDFLAG(IS_ANDROID)
  tracked_browsers_.clear();
#endif
}

void ActivityTrackingService::InitializeOTelExporter() {
  // Read configuration from prefs (set by extension API or SAML)
  PrefService* prefs = profile_->GetPrefs();
  std::string endpoint = prefs->GetString(
      activity_tracking::prefs::kActivityTrackingEndpoint);
  std::string token = prefs->GetString(
      activity_tracking::prefs::kActivityTrackingToken);
  std::string user_email = prefs->GetString(
      activity_tracking::prefs::kActivityTrackingUserEmail);
  
  OTelExporter::Config config;
  
  // TEMPORARY HARDCODED VALUES (until extension API is ready)
  // TODO: Remove hardcoded values once extension API is implemented
  const std::string kHardcodedRealm = "sg0";  // Singapore realm
  const std::string kHardcodedToken = "PSBc1JGvSsGHoZLLc5dAqw";
  
  // If prefs are set (configured via extension), use them
  // Otherwise, use hardcoded values for SignalFx
  if (!endpoint.empty()) {
    LOG(INFO) << "[ActivityTracking] Using endpoint from prefs: " << endpoint;
    
    // Create generic OTLP config with endpoint from prefs
    config = otel_presets::CreateGenericOTLPConfig(endpoint);
    
    // If token is provided, add it to headers
    if (!token.empty()) {
      // Detect endpoint type and add appropriate auth header
      if (endpoint.find("signalfx.com") != std::string::npos ||
          endpoint.find("splunk.com") != std::string::npos) {
        // Splunk/SignalFx uses X-SF-Token (mixed case)
        config.headers["X-SF-Token"] = token;
        LOG(INFO) << "[ActivityTracking] Added Splunk/SignalFx authentication";
      } else if (endpoint.find("nr-data.net") != std::string::npos) {
        // New Relic uses Api-Key
        config.headers["Api-Key"] = token;
        LOG(INFO) << "[ActivityTracking] Added New Relic authentication";
      } else if (endpoint.find("datadoghq.com") != std::string::npos) {
        // Datadog uses DD-API-KEY
        config.headers["DD-API-KEY"] = token;
        LOG(INFO) << "[ActivityTracking] Added Datadog authentication";
      } else {
        // Generic: use Authorization Bearer token
        config.headers["Authorization"] = "Bearer " + token;
        LOG(INFO) << "[ActivityTracking] Added generic Bearer token authentication";
      }
    }
  } else {
    // Use hardcoded SignalFx endpoint (temporary until extension API is ready)
    LOG(INFO) << "[ActivityTracking] No endpoint in prefs, using hardcoded SignalFx configuration";
    LOG(INFO) << "[ActivityTracking] Realm: " << kHardcodedRealm << " (Singapore)";
    
    // Create SignalFx config using the preset helper
    config = otel_presets::CreateSplunkConfig(kHardcodedRealm, kHardcodedToken);
    
    LOG(INFO) << "[ActivityTracking] SignalFx endpoint: " << config.otlp_endpoint;
    LOG(INFO) << "[ActivityTracking] Using hardcoded SignalFx authentication";
  }
  
  // Set user email in config (will be included in all events)
  // IMPORTANT: Do this AFTER creating the config to avoid overwriting
  config.user_email = user_email;
  if (!user_email.empty()) {
    LOG(INFO) << "[ActivityTracking] User email set: " << user_email;
  } else {
    LOG(WARNING) << "[ActivityTracking] No user email found in prefs";
  }
  
  // Create exporter with profile's URL loader factory
  auto exporter = std::make_unique<OTelExporter>(
      profile_->GetURLLoaderFactory(),
      config);
  
  // Set exporter in navigation collector
  navigation_collector_->SetExporter(std::move(exporter));
  
  LOG(INFO) << "[ActivityTracking] OpenTelemetry exporter initialized";
  
  // Start periodic export timer (export every 30 seconds)
  StartPeriodicExport();
}

void ActivityTrackingService::OnActivityTrackingPrefsChanged() {
  LOG(INFO) << "[ActivityTracking] Configuration changed, reinitializing exporter";
  
  // Re-initialize the exporter with new prefs
  // Note: This will replace the existing exporter
  InitializeOTelExporter();
}

void ActivityTrackingService::StartPeriodicExport() {
  export_timer_.Start(
      FROM_HERE,
      base::Seconds(30),
      base::BindRepeating(&ActivityTrackingService::OnExportTimerFired,
                          base::Unretained(this)));
}

void ActivityTrackingService::OnExportTimerFired() {
  // Only export if activity tracking is enabled
  PrefService* prefs = profile_->GetPrefs();
  bool tracking_enabled = prefs->GetBoolean(
      activity_tracking::prefs::kActivityTrackingEnabled);
  
  if (!tracking_enabled) {
    // Silently skip export when tracking is disabled
    return;
  }
  
  if (navigation_collector_) {
    navigation_collector_->ExportToOTLP();
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

