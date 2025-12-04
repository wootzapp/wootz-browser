// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/activity_data_store.h"

#include "base/json/json_writer.h"
#include "base/logging.h"

namespace activity_tracking {

ActivityDataStore::ActivityDataStore() = default;

ActivityDataStore::~ActivityDataStore() = default;

void ActivityDataStore::StoreEvent(ActivityEvent event) {
  events_.push_back(std::move(event));
}

base::Value::List ActivityDataStore::ExportAsOTelEvents() const {
  base::Value::List event_list;

  for (const auto& event : events_) {
    base::Value::Dict event_dict;
    event_dict.Set("event_type", event.event_type);
    event_dict.Set("timestamp", event.timestamp);
    event_dict.Set("data", event.data.Clone());
    event_list.Append(std::move(event_dict));
  }

  return event_list;
}

bool ActivityDataStore::ExportToFile(const std::string& file_path) const {
  base::Value::List events = ExportAsOTelEvents();
  
  std::string json_output;
  if (!base::JSONWriter::WriteWithOptions(
          events, base::JSONWriter::OPTIONS_PRETTY_PRINT, &json_output)) {
    LOG(ERROR) << "Failed to serialize events to JSON";
    return false;
  }

  // TODO: Implement file export if needed
  LOG(INFO) << "Export to file: " << file_path;
  return false;
}

void ActivityDataStore::Clear() {
  events_.clear();
}

}  // namespace activity_tracking
