// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/privacy_filter.h"

#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "chrome/browser/profiles/profile.h"
#include "url/gurl.h"

namespace activity_tracking {

namespace {

// Sensitive query parameters to remove from URLs
const char* kSensitiveParams[] = {
    "token",     "access_token", "api_key",  "apikey",   "auth",
    "password",  "pwd",          "secret",   "session",  "sid",
    "sessionid", "key",          "api_token", "private_key"};

}  // namespace

PrivacyFilter::PrivacyFilter(Profile* profile) : profile_(profile) {}

PrivacyFilter::~PrivacyFilter() = default;

std::string PrivacyFilter::FilterUrl(const std::string& url) const {
  if (!IsTrackingAllowed()) {
    return "[tracking disabled]";
  }

  if (IsIncognitoMode()) {
    return RedactUrl(url);
  }

  // For regular profiles, remove sensitive query parameters
  return RemoveSensitiveParams(url);
}

std::string PrivacyFilter::FilterTitle(const std::string& title) const {
  if (!IsTrackingAllowed()) {
    return "[tracking disabled]";
  }

  if (IsIncognitoMode()) {
    return "[incognito]";
  }

  // For regular profiles, return the title as-is
  // In the future, could add PII detection and filtering here
  return title;
}

bool PrivacyFilter::IsTrackingAllowed() const {
  if (!profile_) {
    return false;
  }

  // Don't track in incognito mode by default
  // Enterprise deployments can override this policy
  if (IsIncognitoMode()) {
    // TODO: Check enterprise policy to allow incognito tracking if configured
    return false;
  }

  return true;
}

bool PrivacyFilter::IsIncognitoMode() const {
  return profile_ && profile_->IsOffTheRecord();
}

std::string PrivacyFilter::RemoveSensitiveParams(const std::string& url) const {
  GURL gurl(url);
  if (!gurl.is_valid() || !gurl.has_query()) {
    return url;
  }

  std::string query = gurl.query();
  bool has_sensitive_param = false;

  for (const char* param : kSensitiveParams) {
    if (query.find(param) != std::string::npos) {
      has_sensitive_param = true;
      break;
    }
  }

  if (!has_sensitive_param) {
    return url;
  }

  // Reconstruct URL without sensitive parameters
  GURL::Replacements replacements;
  replacements.ClearQuery();

  // Parse and filter query parameters
  std::vector<std::string> filtered_params;
  std::vector<std::string> params = base::SplitString(
      query, "&", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

  for (const auto& param : params) {
    bool is_sensitive = false;
    for (const char* sensitive_param : kSensitiveParams) {
      if (base::StartsWith(param, std::string(sensitive_param) + "=",
                           base::CompareCase::INSENSITIVE_ASCII)) {
        is_sensitive = true;
        break;
      }
    }

    if (!is_sensitive) {
      filtered_params.push_back(param);
    }
  }

  if (!filtered_params.empty()) {
    std::string filtered_query = base::JoinString(filtered_params, "&");
    replacements.SetQueryStr(filtered_query);
  }

  return gurl.ReplaceComponents(replacements).spec();
}

std::string PrivacyFilter::RedactUrl(const std::string& url) const {
  GURL gurl(url);
  if (!gurl.is_valid()) {
    return "[invalid-url]";
  }

  // For incognito, only show scheme and host
  return gurl.scheme() + "://" + gurl.host() + "/[incognito]";
}

}  // namespace activity_tracking

