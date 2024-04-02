// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browsing_data/content/local_shared_objects_container.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/scoped_refptr.h"
#include "base/strings/string_piece.h"
#include "components/browsing_data/content/canonical_cookie_hash.h"
#include "components/browsing_data/content/cookie_helper.h"
#include "components/browsing_data/content/local_storage_helper.h"
#include "components/browsing_data/core/browsing_data_utils.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/url_constants.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_util.h"
#include "third_party/blink/public/common/storage_key/storage_key.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace browsing_data {
namespace {

bool SameDomainOrHost(const GURL& gurl1, const GURL& gurl2) {
  return net::registry_controlled_domains::SameDomainOrHost(
      gurl1, gurl2,
      net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
}

}  // namespace

LocalSharedObjectsContainer::LocalSharedObjectsContainer(
    content::StoragePartition* storage_partition,
    bool ignore_empty_localstorage,
    browsing_data::CookieHelper::IsDeletionDisabledCallback callback)
    : cookies_(base::MakeRefCounted<CannedCookieHelper>(storage_partition,
                                                        std::move(callback))),
      local_storages_(base::MakeRefCounted<CannedLocalStorageHelper>(
          storage_partition,
          /*update_ignored_empty_keys_on_fetch=*/ignore_empty_localstorage)),
      session_storages_(base::MakeRefCounted<CannedLocalStorageHelper>(
          storage_partition,
          /*update_ignored_empty_keys_on_fetch=*/false)) {}

LocalSharedObjectsContainer::~LocalSharedObjectsContainer() = default;

size_t LocalSharedObjectsContainer::GetObjectCount() const {
  size_t count = 0;
  count += cookies()->GetCookieCount();
  count += local_storages()->GetCount();
  count += session_storages()->GetCount();
  return count;
}

size_t LocalSharedObjectsContainer::GetObjectCountForDomain(
    const GURL& origin) const {
  size_t count = 0;

  auto origins = GetObjectCountPerOriginMap();

  for (const auto& it_origin : origins) {
    if (SameDomainOrHost(origin, it_origin.first.GetURL()))
      count += it_origin.second;
  }

  return count;
}

size_t LocalSharedObjectsContainer::GetHostCountForDomain(
    const GURL& registrable_domain) const {
  auto origins = GetObjectCountPerOriginMap();
  std::set<std::string> hosts;

  for (const auto& it_origin : origins) {
    if (SameDomainOrHost(registrable_domain, it_origin.first.GetURL()))
      hosts.insert(it_origin.first.host());
  }
  return hosts.size();
}

size_t LocalSharedObjectsContainer::GetHostCount() const {
  auto origins = GetObjectCountPerOriginMap();
  std::set<std::string> hosts;

  for (const auto& it_origin : origins)
    hosts.insert(it_origin.first.host());
  return hosts.size();
}

std::set<std::string> LocalSharedObjectsContainer::GetHosts() const {
  auto origins = GetObjectCountPerOriginMap();
  std::set<std::string> hosts;
  for (auto host : origins) {
    hosts.insert(host.first.host());
  }

  return hosts;
}

std::map<url::Origin, int>
LocalSharedObjectsContainer::GetObjectCountPerOriginMap() const {
  std::map<url::Origin, int> origins;
  for (const auto& cookie : cookies()->origin_cookie_set()) {
    GURL domain_url = net::cookie_util::CookieOriginToURL(
        cookie.Domain(), IsHttpsCookieSourceScheme(cookie.SourceScheme()));
    origins[url::Origin::Create(domain_url)]++;
  }

  for (const auto& storage_key : local_storages()->GetStorageKeys()) {
    // TODO(https://crbug.com/1199077): Use the real StorageKey once migrated.
    origins[storage_key.origin()]++;
  }

  for (const auto& storage_key : session_storages()->GetStorageKeys()) {
    // TODO(https://crbug.com/1199077): Use the real StorageKey once migrated.
    origins[storage_key.origin()]++;
  }

  return origins;
}

void LocalSharedObjectsContainer::UpdateIgnoredEmptyStorageKeys(
    base::OnceClosure done) const {
  local_storages_->UpdateIgnoredEmptyKeys(std::move(done));
}

void LocalSharedObjectsContainer::Reset() {
  cookies_->Reset();
  local_storages_->Reset();
  session_storages_->Reset();
}

}  // namespace browsing_data
