// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/prefs/activity_tracking_prefs.h"

#include <string>

#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"

namespace activity_tracking {
namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  // Register endpoint URL (e.g., Splunk ingest or OTLP collector)
  registry->RegisterStringPref(kActivityTrackingEndpoint, std::string());
  
  // Register authentication token (e.g., Splunk HEC token)
  registry->RegisterStringPref(kActivityTrackingToken, std::string());
  
  // Register enabled flag (default: false/disabled)
  registry->RegisterBooleanPref(kActivityTrackingEnabled, false);
  
  // Register user email for tracking attribution
  registry->RegisterStringPref(kActivityTrackingUserEmail, std::string());
}

}  // namespace prefs
}  // namespace activity_tracking

