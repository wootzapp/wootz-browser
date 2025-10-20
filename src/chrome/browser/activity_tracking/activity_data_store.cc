// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/activity_data_store.h"

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/time/time.h"

namespace activity_tracking {

ActivityDataStore::ActivityDataStore() = default;

ActivityDataStore::~ActivityDataStore() = default;

void ActivityDataStore::StoreEvent(ActivityEvent event) {
  events_.push_back(std::move(event));
}

std::string ActivityDataStore::ExportAsOpenTelemetryJson() const {
  base::Value::Dict otel_data = ConvertToOpenTelemetry();

  std::string json_output;
  if (base::JSONWriter::WriteWithOptions(
          otel_data, base::JSONWriter::OPTIONS_PRETTY_PRINT, &json_output)) {
    return json_output;
  }

  return "{}";
}

bool ActivityDataStore::ExportToFile(const std::string& file_path) const {
  // TODO: Implement file export
  return false;
}

void ActivityDataStore::SendToHEC(const std::string& hec_endpoint,
                                  const std::string& hec_token) {
  // TODO: Implement HEC/Splunk integration
}

void ActivityDataStore::Clear() {
  events_.clear();
}

base::Value::Dict ActivityDataStore::ConvertToOpenTelemetry() const {
  // OpenTelemetry JSON format structure
  // This will be fully implemented when we add export functionality
  base::Value::Dict root;

  // Resource attributes (describes the source)
  base::Value::Dict resource;
  base::Value::Dict resource_attrs;
  resource_attrs.Set("service.name", "chromium-activity-tracker");
  resource_attrs.Set("service.version", "1.0.0");
  resource.Set("attributes", std::move(resource_attrs));
  root.Set("resource", std::move(resource));

  // Scope spans (activity events)
  base::Value::List scope_spans;
  base::Value::Dict scope;
  scope.Set("name", "activity_tracking");
  scope.Set("version", "1.0");

  base::Value::List spans;
  for (const auto& event : events_) {
    base::Value::Dict span;
    span.Set("name", event.event_type);
    span.Set("timestamp", event.timestamp);
    span.Set("attributes", event.data.Clone());
    spans.Append(std::move(span));
  }

  scope.Set("spans", std::move(spans));
  scope_spans.Append(std::move(scope));
  root.Set("scopeSpans", std::move(scope_spans));

  return root;
}

}  // namespace activity_tracking

