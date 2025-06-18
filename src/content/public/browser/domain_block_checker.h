// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_DOMAIN_BLOCK_CHECKER_H_
#define CONTENT_PUBLIC_BROWSER_DOMAIN_BLOCK_CHECKER_H_

#include <string>
#include <unordered_set>

#include "base/no_destructor.h"
#include "base/values.h"
#include "content/common/content_export.h"

class PrefService;
class GURL;

namespace content {

class NavigationHandle;

// High-performance domain blocker with caching and hash-based lookups
class CONTENT_EXPORT DomainBlockChecker {
 public:
  static DomainBlockChecker& GetInstance();

  // Check if a domain is blocked based on preferences
  bool IsDomainBlocked(const std::string& host, PrefService* prefs);

  // Convenience method for checking URLs through navigation handles
  bool IsUrlBlocked(const GURL& url, NavigationHandle* handle);

 private:
  friend class base::NoDestructor<DomainBlockChecker>;
  DomainBlockChecker();
  ~DomainBlockChecker();

  // Refresh the internal cache from preferences
  void RefreshCache(const base::Value::List& blocked_list);

  // Check if any parent domain is blocked (subdomain matching)
  bool IsSubdomainBlocked(const std::string& host);

  std::unordered_set<std::string> blocked_domains_;
  size_t cached_size_ = 0;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_DOMAIN_BLOCK_CHECKER_H_