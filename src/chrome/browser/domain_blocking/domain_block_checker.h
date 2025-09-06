// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DOMAIN_BLOCKING_DOMAIN_BLOCK_CHECKER_H_
#define CHROME_BROWSER_DOMAIN_BLOCKING_DOMAIN_BLOCK_CHECKER_H_

#include <string>
#include <unordered_set>

#include "base/no_destructor.h"
#include "base/values.h"

class PrefService;
class GURL;

namespace chrome {

// High-performance domain blocker with caching and hash-based lookups
class DomainBlockChecker {
 public:
  static DomainBlockChecker& GetInstance();

  // Check if a domain is blocked based on preferences
  bool IsDomainBlocked(const std::string& host, PrefService* prefs);


  // Static utility function for domain validation with enhanced security
  static bool IsValidDomain(const std::string& domain);

 private:
  friend class base::NoDestructor<DomainBlockChecker>;
  DomainBlockChecker();
  ~DomainBlockChecker();

  // Refresh the internal cache from preferences
  void RefreshCache(const base::Value::List& blocked_list);

  // Check if any parent domain is blocked (subdomain matching)
  bool IsSubdomainBlocked(const std::string& host);

  // Calculate hash of domain list content
  size_t CalculateListHash(const base::Value::List& blocked_list);

  // Enhanced security validation helpers
  static bool IsValidTLD(const std::string& tld);
  static bool IsReservedDomain(const std::string& domain);
  static bool HasSuspiciousCharacters(const std::string& domain);
  static bool IsValidDomainStructure(const std::string& domain);

  std::unordered_set<std::string> blocked_domains_;
  size_t cached_size_ = 0;
  size_t cached_hash_ = 0;
};

}  // namespace chrome

#endif  // CHROME_BROWSER_DOMAIN_BLOCKING_DOMAIN_BLOCK_CHECKER_H_