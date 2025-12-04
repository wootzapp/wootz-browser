// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_DATA_STORE_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_DATA_STORE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/values.h"

namespace activity_tracking {

// ActivityDataStore manages storage and export of activity tracking data.
//
// This class provides:
// - In-memory buffering of activity events
// - Export to OpenTelemetry Protocol (OTLP) format
// - Support for any OTLP-compatible backend (Splunk, Datadog, etc.)
class ActivityDataStore {
 public:
  struct ActivityEvent {
    std::string event_type;     // "navigation", "tab_action", etc.
    std::string timestamp;      // ISO 8601 format
    base::Value::Dict data;     // Event-specific data
  };

  ActivityDataStore();
  ~ActivityDataStore();

  ActivityDataStore(const ActivityDataStore&) = delete;
  ActivityDataStore& operator=(const ActivityDataStore&) = delete;

  // Store an activity event (takes ownership via move)
  void StoreEvent(ActivityEvent event);

  // Export events as a list for OTLP exporter
  base::Value::List ExportAsOTelEvents() const;

  // Export events to a file (for debugging/backup)
  bool ExportToFile(const std::string& file_path) const;

  // Clear all stored events
  void Clear();

  // Get number of stored events
  size_t GetEventCount() const { return events_.size(); }

 private:
  // In-memory event buffer
  std::vector<ActivityEvent> events_;

  base::WeakPtrFactory<ActivityDataStore> weak_factory_{this};
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_DATA_STORE_H_
