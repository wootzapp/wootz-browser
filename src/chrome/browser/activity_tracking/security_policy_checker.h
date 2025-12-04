// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_SECURITY_POLICY_CHECKER_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_SECURITY_POLICY_CHECKER_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "base/values.h"

class PrefService;

namespace activity_tracking {

// SecurityPolicyChecker provides utility functions to check if various
// security policies (from SAML attributes) apply to a given URL.
//
// This class checks:
// - Content privacy enabled
// - Copy/paste blocking
// - Upload blocking  
// - Download blocking
// - Domain blocking
class SecurityPolicyChecker {
 public:
  // Security policy status for a URL
  struct PolicyStatus {
    bool content_privacy_enabled = false;
    bool copy_paste_blocked = false;
    bool upload_blocked = false;
    bool download_blocked = false;
    bool domain_blocked = false;
  };

  explicit SecurityPolicyChecker(PrefService* prefs);
  ~SecurityPolicyChecker();

  SecurityPolicyChecker(const SecurityPolicyChecker&) = delete;
  SecurityPolicyChecker& operator=(const SecurityPolicyChecker&) = delete;

  // Check all security policies for a given URL
  PolicyStatus CheckPoliciesForUrl(const std::string& url) const;

  // Add policy status to a base::Value::Dict (for telemetry)
  void AddPolicyStatusToDict(const std::string& url,
                             base::Value::Dict& dict) const;

 private:
  // Check if content privacy is globally enabled
  bool IsContentPrivacyEnabled() const;

  // Check if copy/paste is blocked for this URL
  bool IsCopyPasteBlocked(const std::string& url) const;

  // Check if upload is blocked for this URL
  bool IsUploadBlocked(const std::string& url) const;

  // Check if download is blocked for this URL
  bool IsDownloadBlocked(const std::string& url) const;

  // Check if domain is completely blocked
  bool IsDomainBlocked(const std::string& url) const;

  // Check if a URL matches any domain in a list
  bool UrlMatchesDomainList(const std::string& url,
                           const base::Value::List* domain_list) const;

  // Extract domain from URL (e.g., "https://example.com/path" -> "example.com")
  std::string ExtractDomain(const std::string& url) const;

  raw_ptr<PrefService> prefs_;
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_SECURITY_POLICY_CHECKER_H_

