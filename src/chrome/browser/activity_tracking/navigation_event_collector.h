// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_NAVIGATION_EVENT_COLLECTOR_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_NAVIGATION_EVENT_COLLECTOR_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/scoped_observation.h"
#include "base/time/time.h"
#include "base/values.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
class NavigationHandle;
class WebContents;
class Page;
}  // namespace content

namespace activity_tracking {

class PrivacyFilter;
class OTelExporter;
class SecurityPolicyChecker;

// NavigationEventCollector observes navigation events across all WebContents
// and logs detailed information about each navigation.
//
// This class creates per-WebContents observers that track:
// - Navigation start (URL, referrer, initiator)
// - Navigation redirects
// - Navigation completion (success/failure, HTTP status, errors)
// - Page load completion
// - Primary page changes
class NavigationEventCollector {
 public:
  // Structure to store navigation event data
  struct NavigationEventData {
    NavigationEventData();
    ~NavigationEventData();
    NavigationEventData(const NavigationEventData&);
    NavigationEventData& operator=(const NavigationEventData&);
    
    std::string url;
    std::string title;
    std::string timestamp;  // ISO 8601 format
    std::string state;      // "started", "completed", "failed"
    std::string interaction_type;  // "clicked", "typed", "redirect"
    int64_t duration_ms = 0;
  };

  NavigationEventCollector(PrivacyFilter* privacy_filter,
                           SecurityPolicyChecker* policy_checker);
  ~NavigationEventCollector();

  NavigationEventCollector(const NavigationEventCollector&) = delete;
  NavigationEventCollector& operator=(const NavigationEventCollector&) = delete;

  // Start observing navigation events for a WebContents
  void ObserveWebContents(content::WebContents* web_contents);

  // Stop observing and clean up
  void Shutdown();

  // Get collected events and clear the buffer
  std::vector<NavigationEventData> GetAndClearEvents();
  
  // Convert events to OpenTelemetry JSON format and log
  void LogEventsAsOpenTelemetryJSON();
  
  // Set OpenTelemetry exporter (vendor-neutral)
  void SetExporter(std::unique_ptr<OTelExporter> exporter);
  
  // Export collected events to OTLP endpoint
  void ExportToOTLP();
  
  // Track security policy violations (copy/paste, download, upload blocked)
  void TrackSecurityViolation(content::WebContents* web_contents,
                              const std::string& violation_type,
                              const std::string& url);

 private:
  // Per-WebContents observer that tracks navigation events
  class NavigationObserver : public content::WebContentsObserver {
   public:
    NavigationObserver(content::WebContents* web_contents,
                       PrivacyFilter* privacy_filter,
                       SecurityPolicyChecker* policy_checker);
    ~NavigationObserver() override;

    // WebContentsObserver implementation:
    void DidStartNavigation(
        content::NavigationHandle* navigation_handle) override;
    void DidRedirectNavigation(
        content::NavigationHandle* navigation_handle) override;
    void DidFinishNavigation(
        content::NavigationHandle* navigation_handle) override;
    void PrimaryPageChanged(content::Page& page) override;
    void DocumentOnLoadCompletedInPrimaryMainFrame() override;
    void DidStartLoading() override;
    void DidStopLoading() override;
    void LoadProgressChanged(double progress) override;

   private:
    void LogNavigationStart(content::NavigationHandle* navigation_handle);
    void LogNavigationRedirect(content::NavigationHandle* navigation_handle);
    void LogNavigationComplete(content::NavigationHandle* navigation_handle);
    void LogPrimaryPageChange(content::Page& page);
    void LogPageLoadComplete();
    
    void RecordNavigationEvent(const NavigationEventData& event);

    raw_ptr<PrivacyFilter> privacy_filter_;
    raw_ptr<SecurityPolicyChecker> policy_checker_;
    base::TimeTicks navigation_start_time_;
    base::TimeTicks page_load_start_time_;
    std::string current_url_;
    std::string current_title_;
    
   public:
    raw_ptr<NavigationEventCollector> parent_collector_;
  };

  // Convert events to OpenTelemetry format
  base::Value::Dict ConvertToOpenTelemetry() const;

  raw_ptr<PrivacyFilter> privacy_filter_;
  raw_ptr<SecurityPolicyChecker> policy_checker_;

  // Map of WebContents to their observers
  std::map<raw_ptr<content::WebContents>, std::unique_ptr<NavigationObserver>>
      observers_;
  
  // Buffer for collected navigation events
  std::vector<NavigationEventData> navigation_events_;
  
  // OpenTelemetry exporter (vendor-neutral)
  std::unique_ptr<OTelExporter> otel_exporter_;
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_NAVIGATION_EVENT_COLLECTOR_H_

