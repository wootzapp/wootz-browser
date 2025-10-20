// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/navigation_event_collector.h"

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/browser/activity_tracking/privacy_filter.h"
#include "chrome/browser/activity_tracking/splunk_observability_exporter.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/page.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_errors.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_util.h"
#include "ui/base/page_transition_types.h"

namespace activity_tracking {

// NavigationEventData implementation
NavigationEventCollector::NavigationEventData::NavigationEventData() = default;
NavigationEventCollector::NavigationEventData::~NavigationEventData() = default;
NavigationEventCollector::NavigationEventData::NavigationEventData(
    const NavigationEventData&) = default;
NavigationEventCollector::NavigationEventData& 
NavigationEventCollector::NavigationEventData::operator=(
    const NavigationEventData&) = default;

NavigationEventCollector::NavigationEventCollector(
    PrivacyFilter* privacy_filter)
    : privacy_filter_(privacy_filter) {
}

NavigationEventCollector::~NavigationEventCollector() {
  Shutdown();
}

void NavigationEventCollector::ObserveWebContents(
    content::WebContents* web_contents) {
  if (!web_contents) {
    return;
  }

  // Check if already observing
  if (observers_.find(web_contents) != observers_.end()) {
    return;
  }

  auto observer =
      std::make_unique<NavigationObserver>(web_contents, privacy_filter_);
  observer->parent_collector_ = this;  // Set parent for event recording
  observers_[web_contents] = std::move(observer);
}

void NavigationEventCollector::Shutdown() {
  observers_.clear();
}

// NavigationObserver implementation

NavigationEventCollector::NavigationObserver::NavigationObserver(
    content::WebContents* web_contents,
    PrivacyFilter* privacy_filter)
    : content::WebContentsObserver(web_contents),
      privacy_filter_(privacy_filter),
      parent_collector_(nullptr) {}

NavigationEventCollector::NavigationObserver::~NavigationObserver() = default;

void NavigationEventCollector::NavigationObserver::DidStartNavigation(
    content::NavigationHandle* navigation_handle) {
  navigation_start_time_ = base::TimeTicks::Now();
  LogNavigationStart(navigation_handle);
}

void NavigationEventCollector::NavigationObserver::DidRedirectNavigation(
    content::NavigationHandle* navigation_handle) {
  LogNavigationRedirect(navigation_handle);
}

void NavigationEventCollector::NavigationObserver::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  LogNavigationComplete(navigation_handle);
}

void NavigationEventCollector::NavigationObserver::PrimaryPageChanged(
    content::Page& page) {
  LogPrimaryPageChange(page);
}

void NavigationEventCollector::NavigationObserver::
    DocumentOnLoadCompletedInPrimaryMainFrame() {
  LogPageLoadComplete();
}

void NavigationEventCollector::NavigationObserver::DidStartLoading() {
  page_load_start_time_ = base::TimeTicks::Now();
}

void NavigationEventCollector::NavigationObserver::DidStopLoading() {
}

void NavigationEventCollector::NavigationObserver::LoadProgressChanged(
    double progress) {
}

void NavigationEventCollector::NavigationObserver::LogNavigationStart(
    content::NavigationHandle* navigation_handle) {
  std::string url =
      privacy_filter_->FilterUrl(navigation_handle->GetURL().spec());
  std::string referrer =
      privacy_filter_->FilterUrl(navigation_handle->GetReferrer().url.spec());
  
  current_url_ = url;  // Store for later use

  // Determine interaction type
  std::string interaction_type;
  ui::PageTransition transition = navigation_handle->GetPageTransition();
  if (ui::PageTransitionCoreTypeIs(transition, ui::PAGE_TRANSITION_LINK)) {
    interaction_type = "clicked";
  } else if (ui::PageTransitionCoreTypeIs(transition, ui::PAGE_TRANSITION_TYPED)) {
    interaction_type = "typed";
  } else if (ui::PageTransitionCoreTypeIs(transition, ui::PAGE_TRANSITION_AUTO_BOOKMARK)) {
    interaction_type = "bookmark";
  } else if (ui::PageTransitionCoreTypeIs(transition, ui::PAGE_TRANSITION_RELOAD)) {
    interaction_type = "reload";
  } else {
    interaction_type = "other";
  }

  // Record navigation start event
  if (parent_collector_ && navigation_handle->IsInPrimaryMainFrame()) {
    NavigationEventData event;
    event.url = url;
    event.title = "";  // Title not available yet
    event.timestamp = net::HttpUtil::TimeFormatHTTP(base::Time::Now());
    event.state = "started";
    event.interaction_type = interaction_type;
    event.duration_ms = 0;
    parent_collector_->navigation_events_.push_back(event);
  }
}

void NavigationEventCollector::NavigationObserver::LogNavigationRedirect(
    content::NavigationHandle* navigation_handle) {
}

void NavigationEventCollector::NavigationObserver::LogNavigationComplete(
    content::NavigationHandle* navigation_handle) {
  std::string url =
      privacy_filter_->FilterUrl(navigation_handle->GetURL().spec());

  auto navigation_duration = base::TimeTicks::Now() - navigation_start_time_;

  if (!navigation_handle->HasCommitted()) {
    // Record failed navigation
    if (parent_collector_ && navigation_handle->IsInPrimaryMainFrame()) {
      NavigationEventData event;
      event.url = url;
      event.title = "";
      event.timestamp = net::HttpUtil::TimeFormatHTTP(base::Time::Now());
      event.state = "failed";
      event.interaction_type = "navigation";
      event.duration_ms = navigation_duration.InMilliseconds();
      parent_collector_->navigation_events_.push_back(event);
    }
    return;
  }

  // Record completed navigation
  if (parent_collector_ && navigation_handle->IsInPrimaryMainFrame()) {
    NavigationEventData event;
    event.url = url;
    event.title = "";  // Title will be updated in page load complete
    event.timestamp = net::HttpUtil::TimeFormatHTTP(base::Time::Now());
    event.state = "completed";
    event.interaction_type = "navigation";
    event.duration_ms = navigation_duration.InMilliseconds();
    parent_collector_->navigation_events_.push_back(event);
  }
}

void NavigationEventCollector::NavigationObserver::LogPrimaryPageChange(
    content::Page& page) {
}

void NavigationEventCollector::NavigationObserver::LogPageLoadComplete() {
  auto* web_contents = this->web_contents();
  if (!web_contents) {
    return;
  }

  std::string url =
      privacy_filter_->FilterUrl(web_contents->GetVisibleURL().spec());
  std::string title = privacy_filter_->FilterTitle(
      base::UTF16ToUTF8(web_contents->GetTitle()));

  auto load_duration = base::TimeTicks::Now() - page_load_start_time_;

  // Record final event with title
  if (parent_collector_) {
    NavigationEventData event;
    event.url = url;
    event.title = title;
    event.timestamp = net::HttpUtil::TimeFormatHTTP(base::Time::Now());
    event.state = "page_loaded";
    event.interaction_type = "navigation";
    event.duration_ms = load_duration.InMilliseconds();
    parent_collector_->navigation_events_.push_back(event);
    
    // Log collected events every 10 navigations or on significant events
    if (parent_collector_->navigation_events_.size() >= 10) {
      parent_collector_->LogEventsAsOpenTelemetryJSON();
    }
  }
}

void NavigationEventCollector::NavigationObserver::RecordNavigationEvent(
    const NavigationEventData& event) {
  if (parent_collector_) {
    parent_collector_->navigation_events_.push_back(event);
  }
}

// NavigationEventCollector methods for OpenTelemetry

std::vector<NavigationEventCollector::NavigationEventData>
NavigationEventCollector::GetAndClearEvents() {
  std::vector<NavigationEventData> events = std::move(navigation_events_);
  navigation_events_.clear();
  return events;
}

void NavigationEventCollector::LogEventsAsOpenTelemetryJSON() {
  if (navigation_events_.empty()) {
    return;
  }

  // Clear events after logging
  navigation_events_.clear();
}

base::Value::Dict NavigationEventCollector::ConvertToOpenTelemetry() const {
  base::Value::Dict root;
  
  // OpenTelemetry Log Data Model
  root.Set("resourceLogs", base::Value::List());
  base::Value::List* resource_logs = root.FindList("resourceLogs");
  
  base::Value::Dict resource_log;
  
  // Resource attributes
  base::Value::Dict resource;
  base::Value::Dict resource_attrs;
  resource_attrs.Set("service.name", "chromium-browser");
  resource_attrs.Set("service.version", "1.0.0");
  resource.Set("attributes", std::move(resource_attrs));
  resource_log.Set("resource", std::move(resource));
  
  // Scope logs
  base::Value::List scope_logs;
  base::Value::Dict scope_log;
  
  base::Value::Dict scope;
  scope.Set("name", "navigation.tracker");
  scope.Set("version", "1.0.0");
  scope_log.Set("scope", std::move(scope));
  
  // Log records
  base::Value::List log_records;
  
  for (const auto& event : navigation_events_) {
    base::Value::Dict log_record;
    
    // Timestamp in Unix nano
    log_record.Set("timeUnixNano", static_cast<double>(
        base::Time::Now().InMillisecondsSinceUnixEpoch() * 1000000));
    
    // Severity
    log_record.Set("severityNumber", 9);  // INFO
    log_record.Set("severityText", "INFO");
    
    // Body
    log_record.Set("body", base::Value::Dict()
        .Set("stringValue", event.state));
    
    // Attributes - The actual navigation data
    base::Value::Dict attributes;
    attributes.Set("url", event.url);
    attributes.Set("title", event.title);
    attributes.Set("timestamp", event.timestamp);
    attributes.Set("state", event.state);
    attributes.Set("interaction_type", event.interaction_type);
    attributes.Set("duration_ms", static_cast<double>(event.duration_ms));
    
    log_record.Set("attributes", std::move(attributes));
    
    log_records.Append(std::move(log_record));
  }
  
  scope_log.Set("logRecords", std::move(log_records));
  scope_logs.Append(std::move(scope_log));
  resource_log.Set("scopeLogs", std::move(scope_logs));
  
  resource_logs->Append(std::move(resource_log));
  
  return root;
}

void NavigationEventCollector::SetExporter(
    std::unique_ptr<SplunkObservabilityExporter> exporter) {
  splunk_exporter_ = std::move(exporter);
}

void NavigationEventCollector::ExportToSplunk() {
  if (!splunk_exporter_ || navigation_events_.empty()) {
    return;
  }
  
  // Convert to OpenTelemetry format
  base::Value::Dict otel_json = ConvertToOpenTelemetry();
  
  // Export both as events and metrics
  splunk_exporter_->ExportEvents(
      otel_json,
      base::BindOnce([](SplunkObservabilityExporter::ExportResult result) {
        if (result != SplunkObservabilityExporter::ExportResult::SUCCESS) {
          LOG(ERROR) << "[NavigationCollector] Events export failed";
        }
      }));
  
  splunk_exporter_->ExportMetrics(
      otel_json,
      base::BindOnce([](SplunkObservabilityExporter::ExportResult result) {
        if (result != SplunkObservabilityExporter::ExportResult::SUCCESS) {
          LOG(ERROR) << "[NavigationCollector] Metrics export failed";
        }
      }));
  
  // Clear events after export
  navigation_events_.clear();
}

}  // namespace activity_tracking

