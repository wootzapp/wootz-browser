// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/domain_block_checker.h"

#include "base/logging.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/blocked_domains_prefs.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

namespace content {

// static
DomainBlockChecker& DomainBlockChecker::GetInstance() {
  static base::NoDestructor<DomainBlockChecker> instance;
  return *instance;
}

DomainBlockChecker::DomainBlockChecker() = default;
DomainBlockChecker::~DomainBlockChecker() = default;

bool DomainBlockChecker::IsDomainBlocked(const std::string& host,
                                         PrefService* prefs) {
  if (host.empty() || !prefs) {
    return false;
  }

  const base::Value::List& blocked_list =
      prefs->GetList(blocked_domains::prefs::kBlockedDomains);

  size_t current_hash = CalculateListHash(blocked_list);

  // Refresh cache if size or content has changed
  if (blocked_list.size() != cached_size_ || current_hash != cached_hash_ ||
      blocked_domains_.empty()) {
    RefreshCache(blocked_list);
    cached_hash_ = current_hash;
  }

  // Fast hash lookup - O(1)
  if (blocked_domains_.count(host)) {
    return true;
  }

  // Check subdomains efficiently
  return IsSubdomainBlocked(host);
}

bool DomainBlockChecker::IsUrlBlocked(const GURL& url,
                                      NavigationHandle* handle) {
  if (!url.is_valid() || url.host().empty()) {
    return false;
  }

  // Streamlined null checks
  WebContents* web_contents = handle->GetWebContents();
  if (!web_contents) {
    return false;
  }

  auto* browser_context = web_contents->GetBrowserContext();
  if (!browser_context) {
    return false;
  }

  Profile* profile = Profile::FromBrowserContext(browser_context);
  if (!profile) {
    return false;
  }

  auto* prefs = profile->GetPrefs();
  if (!prefs) {
    return false;
  }

  return IsDomainBlocked(url.host(), prefs);
}

// static
bool DomainBlockChecker::IsValidDomain(const std::string& domain) {
  if (domain.empty() || domain.length() > 253) {  // RFC max domain length
    return false;
  }

  // Check for invalid start/end characters
  if (domain.front() == '.' || domain.back() == '.' || domain.front() == '-' ||
      domain.back() == '-') {
    return false;
  }

  // Enhanced security validations
  if (!IsValidDomainStructure(domain)) {
    return false;
  }
  if (IsReservedDomain(domain)) {
    return false;
  }
  if (HasSuspiciousCharacters(domain)) {
    return false;
  }

  // Extract and validate TLD
  size_t last_dot = domain.rfind('.');
  if (last_dot != std::string::npos && last_dot < domain.length() - 1) {
    std::string tld = domain.substr(last_dot + 1);
    if (!IsValidTLD(tld)) {
      return false;
    }
  }

  // Check for consecutive dots or invalid characters (original validation)
  bool prev_was_dot = false;
  for (char c : domain) {
    if (c == '.') {
      if (prev_was_dot) {
        return false;  // consecutive dots
      }
      prev_was_dot = true;
    } else if (std::isalnum(c) || c == '-') {
      prev_was_dot = false;
    } else {
      return false;  // invalid character
    }
  }

  return true;
}

// static
bool DomainBlockChecker::IsValidDomainStructure(const std::string& domain) {
  size_t dot_count = 0;
  size_t current_label_length = 0;

  for (char c : domain) {
    if (c == '.') {
      // Check label length (RFC 1035: max 63 characters per label)
      if (current_label_length == 0 || current_label_length > 63) {
        return false;
      }
      current_label_length = 0;
      dot_count++;

      // Prevent excessive subdomain depth (security limit)
      if (dot_count > 10) {
        return false;
      }
    } else {
      current_label_length++;
    }
  }

  // Check final label length
  if (current_label_length == 0 || current_label_length > 63) {
    return false;
  }

  return true;
}

// static
bool DomainBlockChecker::IsReservedDomain(const std::string& domain) {
  // Use base::NoDestructor to avoid exit-time destructor warning
  static const base::NoDestructor<std::unordered_set<std::string>>
      reserved_domains({"localhost", "local", "test", "invalid", "example",
                        "onion", ".localhost", ".local", ".test", ".invalid",
                        ".example", ".onion"});

  // Check exact match
  if (reserved_domains->count(domain)) {
    return true;
  }

  // Check if ends with reserved TLD
  for (const auto& reserved :
       {".localhost", ".local", ".test", ".invalid", ".example", ".onion"}) {
    if (domain.length() >= strlen(reserved) &&
        domain.substr(domain.length() - strlen(reserved)) == reserved) {
      return true;
    }
  }

  return false;
}

// static
bool DomainBlockChecker::HasSuspiciousCharacters(const std::string& domain) {
  bool all_numeric = true;
  bool has_letter = false;
  size_t consecutive_numbers = 0;
  size_t max_consecutive_numbers = 0;

  for (char c : domain) {
    if (c == '.') {
      consecutive_numbers = 0;
      continue;
    }

    if (std::isdigit(c)) {
      consecutive_numbers++;
      max_consecutive_numbers =
          std::max(max_consecutive_numbers, consecutive_numbers);
    } else {
      all_numeric = false;
      consecutive_numbers = 0;
      if (std::isalpha(c)) {
        has_letter = true;
      }
    }

    // Check for potentially suspicious Unicode or non-ASCII characters
    if (static_cast<unsigned char>(c) > 127) {
      return true;
    }
  }

  // Flag all-numeric domains (except IP addresses, but those should be handled
  // elsewhere)
  if (all_numeric && has_letter == false) {
    return true;
  }

  // Flag domains with excessive consecutive numbers (possible IP disguising)
  if (max_consecutive_numbers > 8) {
    return true;
  }

  return false;
}

// static
bool DomainBlockChecker::IsValidTLD(const std::string& tld) {
  // Fast validation: TLD must be 2+ characters, all letters, no numbers
  if (tld.length() < 2 || tld.length() > 63) {
    return false;
  }

  for (char c : tld) {
    if (!std::isalpha(c)) {
      return false;
    }
  }

  // Basic length-based validation (most legitimate TLDs are 2-4 chars)
  // Allow longer TLDs but flag extremely long ones as suspicious
  if (tld.length() > 20) {
    return false;
  }

  return true;
}

void DomainBlockChecker::RefreshCache(const base::Value::List& blocked_list) {
  blocked_domains_.clear();
  blocked_domains_.reserve(blocked_list.size());

  for (const auto& domain_value : blocked_list) {
    if (domain_value.is_string()) {
      const std::string& domain = domain_value.GetString();
      if (!domain.empty()) {
        blocked_domains_.insert(domain);
      }
    }
  }

  cached_size_ = blocked_list.size();
  VLOG(2) << "[DomainBlocker] Cache updated: " << blocked_domains_.size()
          << " domains";
}

bool DomainBlockChecker::IsSubdomainBlocked(const std::string& host) {
  size_t dot_pos = host.find('.');
  while (dot_pos != std::string::npos) {
    std::string parent_domain = host.substr(dot_pos + 1);
    if (blocked_domains_.count(parent_domain)) {
      return true;
    }
    dot_pos = host.find('.', dot_pos + 1);
  }
  return false;
}

size_t DomainBlockChecker::CalculateListHash(
    const base::Value::List& blocked_list) {
  size_t hash = 0;
  for (const auto& domain_value : blocked_list) {
    if (domain_value.is_string()) {
      const std::string& domain = domain_value.GetString();
      if (!domain.empty()) {
        // Simple hash combination using std::hash
        hash ^= std::hash<std::string>{}(domain) + 0x9e3779b9 + (hash << 6) +
                (hash >> 2);
      }
    }
  }
  return hash;
}

}  // namespace content
