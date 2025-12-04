// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/security_policy_checker.h"

#include "base/logging.h"
#include "chrome/browser/prefs/blocked_domains_prefs.h"
#include "chrome/browser/prefs/content_privacy_prefs.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/copy_paste_blocker_prefs.h"
#include "content/public/browser/upload_blocking_prefs.h"
#include "url/gurl.h"

namespace activity_tracking {

SecurityPolicyChecker::SecurityPolicyChecker(PrefService* prefs)
    : prefs_(prefs) {}

SecurityPolicyChecker::~SecurityPolicyChecker() = default;

SecurityPolicyChecker::PolicyStatus
SecurityPolicyChecker::CheckPoliciesForUrl(const std::string& url) const {
  PolicyStatus status;

  if (!prefs_) {
    return status;
  }

  status.content_privacy_enabled = IsContentPrivacyEnabled();
  status.copy_paste_blocked = IsCopyPasteBlocked(url);
  status.upload_blocked = IsUploadBlocked(url);
  status.download_blocked = IsDownloadBlocked(url);
  status.domain_blocked = IsDomainBlocked(url);

  return status;
}

void SecurityPolicyChecker::AddPolicyStatusToDict(const std::string& url,
                                                  base::Value::Dict& dict) const {
  if (!prefs_) {
    return;
  }

  PolicyStatus status = CheckPoliciesForUrl(url);

  // Add security policy metadata to the dict
  base::Value::Dict security_policies;
  security_policies.Set("content_privacy_enabled", status.content_privacy_enabled);
  security_policies.Set("copy_paste_blocked", status.copy_paste_blocked);
  security_policies.Set("upload_blocked", status.upload_blocked);
  security_policies.Set("download_blocked", status.download_blocked);
  security_policies.Set("domain_blocked", status.domain_blocked);

  dict.Set("security_policies", std::move(security_policies));
}

bool SecurityPolicyChecker::IsContentPrivacyEnabled() const {
  // Content privacy is a global setting, not URL-specific
  return prefs_->GetBoolean(content_privacy::prefs::kContentPrivacyEnabled);
}

bool SecurityPolicyChecker::IsCopyPasteBlocked(const std::string& url) const {
  // Check if copy/paste blocking is globally enabled
  bool globally_enabled = prefs_->GetBoolean(
      copy_paste_blocker::prefs::kCopyPasteBlockingEnabled);

  if (!globally_enabled) {
    return false;
  }

  // Check if this URL is in the blocked domains list
  const base::Value::List& blocked_domains = prefs_->GetList(
      copy_paste_blocker::prefs::kCopyPasteBlockingDomains);

  if (blocked_domains.empty()) {
    // If globally enabled but no domain list, block all domains
    return true;
  }

  return UrlMatchesDomainList(url, &blocked_domains);
}

bool SecurityPolicyChecker::IsUploadBlocked(const std::string& url) const {
  const base::Value::List& blocked_domains = prefs_->GetList(
      content::upload_blocking_prefs::kBlockedUploadDomains);

  return UrlMatchesDomainList(url, &blocked_domains);
}

bool SecurityPolicyChecker::IsDownloadBlocked(const std::string& url) const {
  // TODO: Add proper download blocking pref
  // For now, return false (no downloads blocked)
  return false;
}

bool SecurityPolicyChecker::IsDomainBlocked(const std::string& url) const {
  const base::Value::List& blocked_domains = prefs_->GetList(
      blocked_domains::prefs::kBlockedDomains);

  return UrlMatchesDomainList(url, &blocked_domains);
}

bool SecurityPolicyChecker::UrlMatchesDomainList(
    const std::string& url,
    const base::Value::List* domain_list) const {
  if (!domain_list || domain_list->empty()) {
    return false;
  }

  std::string domain = ExtractDomain(url);
  if (domain.empty()) {
    return false;
  }

  // Check if the domain or any parent domain is in the blocked list
  for (const auto& blocked_domain_value : *domain_list) {
    if (!blocked_domain_value.is_string()) {
      continue;
    }

    std::string blocked_domain = blocked_domain_value.GetString();
    if (blocked_domain.empty()) {
      continue;
    }

    // Exact match
    if (domain == blocked_domain) {
      return true;
    }

    // Subdomain match (e.g., "sub.example.com" matches "example.com")
    if (domain.size() > blocked_domain.size()) {
      size_t pos = domain.size() - blocked_domain.size();
      if (domain.substr(pos) == blocked_domain &&
          (pos == 0 || domain[pos - 1] == '.')) {
        return true;
      }
    }

    // Wildcard match if blocked_domain starts with "*."
    if (blocked_domain.size() > 2 && blocked_domain.substr(0, 2) == "*.") {
      std::string pattern = blocked_domain.substr(2);  // Remove "*."
      if (domain == pattern) {
        return true;
      }
      // Match any subdomain
      if (domain.size() > pattern.size()) {
        size_t pos = domain.size() - pattern.size();
        if (domain.substr(pos) == pattern && domain[pos - 1] == '.') {
          return true;
        }
      }
    }
  }

  return false;
}

std::string SecurityPolicyChecker::ExtractDomain(const std::string& url) const {
  GURL gurl(url);
  if (!gurl.is_valid()) {
    return "";
  }

  // Return the host (e.g., "example.com" from "https://example.com/path")
  return gurl.host();
}

}  // namespace activity_tracking

