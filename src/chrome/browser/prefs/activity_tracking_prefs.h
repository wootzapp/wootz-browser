// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PREFS_ACTIVITY_TRACKING_PREFS_H_
#define CHROME_BROWSER_PREFS_ACTIVITY_TRACKING_PREFS_H_

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace activity_tracking {
namespace prefs {

// Pref names for activity tracking configuration
constexpr char kActivityTrackingEndpoint[] = "activity_tracking.endpoint";
constexpr char kActivityTrackingToken[] = "activity_tracking.token";
constexpr char kActivityTrackingEnabled[] = "activity_tracking.enabled";
constexpr char kActivityTrackingUserEmail[] = "activity_tracking.user_email";

// Register activity tracking preferences
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace prefs
}  // namespace activity_tracking

#endif  // CHROME_BROWSER_PREFS_ACTIVITY_TRACKING_PREFS_H_

