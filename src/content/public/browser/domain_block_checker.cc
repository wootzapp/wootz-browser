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

bool DomainBlockChecker::IsDomainBlocked(const std::string& host, PrefService* prefs) {
  if (host.empty() || !prefs) return false;

  const base::Value::List& blocked_list = prefs->GetList(blocked_domains::prefs::kBlockedDomains);
  if (blocked_list.size() != cached_size_ || blocked_domains_.empty()) {
    RefreshCache(blocked_list);
  }

  // Fast hash lookup - O(1)
  if (blocked_domains_.count(host)) {
    return true;
  }
  
  // Check subdomains efficiently
  return IsSubdomainBlocked(host);
}

bool DomainBlockChecker::IsUrlBlocked(const GURL& url, NavigationHandle* handle) {
  if (!url.is_valid() || url.host().empty()) {
    return false;
  }

  // Streamlined null checks
  WebContents* web_contents = handle->GetWebContents();
  if (!web_contents) return false;
  
  auto* browser_context = web_contents->GetBrowserContext();
  if (!browser_context) return false;
  
  Profile* profile = Profile::FromBrowserContext(browser_context);
  if (!profile) return false;
  
  auto* prefs = profile->GetPrefs();
  if (!prefs) return false;

  return IsDomainBlocked(url.host(), prefs);
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
  VLOG(2) << "[DomainBlocker] Cache updated: " << blocked_domains_.size() << " domains";
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

}  // namespace content