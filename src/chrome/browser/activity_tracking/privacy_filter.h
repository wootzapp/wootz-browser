// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_PRIVACY_FILTER_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_PRIVACY_FILTER_H_

#include <string>

#include "base/memory/raw_ptr.h"

class Profile;

namespace activity_tracking {

// PrivacyFilter handles filtering and sanitization of sensitive data
// before it is logged or exported.
//
// This class provides methods to:
// - Filter URLs (respect incognito mode, remove sensitive params)
// - Filter page titles (sanitize PII)
// - Check if logging is allowed for the current profile
// - Apply enterprise policies for data collection
//
// In incognito mode, all URLs and titles are redacted to protect privacy.
class PrivacyFilter {
 public:
  explicit PrivacyFilter(Profile* profile);
  ~PrivacyFilter();

  PrivacyFilter(const PrivacyFilter&) = delete;
  PrivacyFilter& operator=(const PrivacyFilter&) = delete;

  // Filter a URL - returns filtered/redacted URL based on privacy settings
  std::string FilterUrl(const std::string& url) const;

  // Filter a page title - returns filtered/redacted title
  std::string FilterTitle(const std::string& title) const;

  // Check if activity tracking is allowed for this profile
  bool IsTrackingAllowed() const;

  // Check if the current profile is in incognito/private mode
  bool IsIncognitoMode() const;

 private:
  // Remove sensitive query parameters from URL
  std::string RemoveSensitiveParams(const std::string& url) const;

  // Redact the URL for privacy (used in incognito)
  std::string RedactUrl(const std::string& url) const;

  raw_ptr<Profile> profile_;
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_PRIVACY_FILTER_H_

