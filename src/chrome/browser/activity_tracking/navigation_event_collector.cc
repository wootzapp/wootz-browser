// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/navigation_event_collector.h"

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/browser/activity_tracking/otel_exporter.h"
#include "chrome/browser/activity_tracking/privacy_filter.h"
#include "chrome/browser/activity_tracking/security_policy_checker.h"
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
    PrivacyFilter* privacy_filter,
    SecurityPolicyChecker* policy_checker)
    : privacy_filter_(privacy_filter), policy_checker_(policy_checker) {
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

  auto observer = std::make_unique<NavigationObserver>(
      web_contents, privacy_filter_, policy_checker_);
  observer->parent_collector_ = this;  // Set parent for event recording
  observers_[web_contents] = std::move(observer);
}

void NavigationEventCollector::Shutdown() {
  observers_.clear();
}

// NavigationObserver implementation

NavigationEventCollector::NavigationObserver::NavigationObserver(
    content::WebContents* web_contents,
    PrivacyFilter* privacy_filter,
    SecurityPolicyChecker* policy_checker)
    : content::WebContentsObserver(web_contents),
      privacy_filter_(privacy_filter),
      policy_checker_(policy_checker),
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
  resource_attrs.Set("service.name", "wootzapp-enterprise");
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
    std::unique_ptr<OTelExporter> exporter) {
  otel_exporter_ = std::move(exporter);
}

void NavigationEventCollector::ExportToOTLP() {
  if (!otel_exporter_ || navigation_events_.empty()) {
    return;
  }
  
  // Convert navigation events to base::Value::List for OTLP export
  base::Value::List events_list;
  for (const auto& event : navigation_events_) {
    base::Value::Dict event_dict;
    event_dict.Set("url", event.url);
    event_dict.Set("title", event.title);
    event_dict.Set("timestamp", event.timestamp);
    event_dict.Set("state", event.state);
    event_dict.Set("interaction_type", event.interaction_type);
    event_dict.Set("duration_ms", static_cast<double>(event.duration_ms));
    
    // Add security policy metadata for this URL
    if (policy_checker_) {
      policy_checker_->AddPolicyStatusToDict(event.url, event_dict);
    }
    
    events_list.Append(std::move(event_dict));
  }
  
  // Export as Splunk HEC events (flattened, simpler format)
  otel_exporter_->ExportAsSplunkEvents(
      events_list,
      base::BindOnce([](OTelExporter::ExportResult result) {
        if (result != OTelExporter::ExportResult::SUCCESS) {
          LOG(ERROR) << "[NavigationCollector] Splunk export failed";
        } else {
          LOG(INFO) << "[NavigationCollector] Splunk export successful";
        }
      }));
  
  // Clear events after export
  navigation_events_.clear();
}

void NavigationEventCollector::TrackSecurityViolation(
    content::WebContents* web_contents,
    const std::string& violation_type,
    const std::string& url) {
  if (!otel_exporter_) {
    LOG(WARNING) << "[NavigationCollector] No exporter configured for security violation tracking";
    return;
  }

  LOG(INFO) << "[NavigationCollector] Security violation: " << violation_type 
            << " on " << url;

  // Create immediate event for security violation
  base::Value::List violation_event_list;
  base::Value::Dict violation_event;
  
  violation_event.Set("url", url);
  violation_event.Set("title", "Security Policy Violation");
  violation_event.Set("timestamp", net::HttpUtil::TimeFormatHTTP(base::Time::Now()));
  violation_event.Set("state", "blocked");
  violation_event.Set("interaction_type", violation_type);  // "copy_paste_blocked", "download_blocked", "upload_blocked"
  violation_event.Set("duration_ms", 0.0);
  violation_event.Set("warning", "security_policy_violation");  // Warning tag
  
  // Add current security policies
  if (policy_checker_) {
    policy_checker_->AddPolicyStatusToDict(url, violation_event);
  }
  
  violation_event_list.Append(std::move(violation_event));
  
  LOG(INFO) << "[NavigationCollector] Exporting security violation event immediately";
  
  // Export immediately (don't wait for periodic export)
  otel_exporter_->ExportAsSplunkEvents(
      violation_event_list,
      base::BindOnce([](OTelExporter::ExportResult result) {
        if (result == OTelExporter::ExportResult::SUCCESS) {
          LOG(INFO) << "[NavigationCollector] Security violation event exported successfully";
        } else {
          LOG(ERROR) << "[NavigationCollector] Failed to export security violation event";
        }
      }));
}

}  // namespace activity_tracking

