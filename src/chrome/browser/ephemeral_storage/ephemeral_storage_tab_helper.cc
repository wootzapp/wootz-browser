/* Copyright (c) 2020 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/ephemeral_storage/ephemeral_storage_tab_helper.h"

#include "base/feature_list.h"
#include "chrome/browser/ephemeral_storage/ephemeral_storage_service_factory.h"
#include "components/ephemeral_storage/ephemeral_storage_service.h"
#include "chrome/browser/content_settings/cookie_settings_factory.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "net/base/features.h"
#include "net/base/url_util.h"

using content::BrowserContext;
using content::NavigationHandle;
using content::WebContents;

namespace ephemeral_storage {

// EphemeralStorageTabHelper helps to manage the lifetime of ephemeral storage.
// For more information about the design of ephemeral storage please see the
// design document at:
// https://github.com/brave/brave-browser/wiki/Ephemeral-Storage-Design
EphemeralStorageTabHelper::EphemeralStorageTabHelper(WebContents* web_contents)
    : WebContentsObserver(web_contents),
      content::WebContentsUserData<EphemeralStorageTabHelper>(*web_contents),
      host_content_settings_map_(HostContentSettingsMapFactory::GetForProfile(
          web_contents->GetBrowserContext())),
      cookie_settings_(CookieSettingsFactory::GetForProfile(
          Profile::FromBrowserContext(web_contents->GetBrowserContext()))) {
  DCHECK(base::FeatureList::IsEnabled(net::features::kWootzEphemeralStorage));

  // The URL might not be empty if this is a restored WebContents, for instance.
  // In that case we want to make sure it has valid ephemeral storage.
  const GURL& url = web_contents->GetLastCommittedURL();
  const std::string ephemeral_storage_domain =
      net::URLToEphemeralStorageDomain(url);
  CreateEphemeralStorageAreasForDomainAndURL(ephemeral_storage_domain, url);
}

EphemeralStorageTabHelper::~EphemeralStorageTabHelper() = default;

std::optional<base::UnguessableToken>
EphemeralStorageTabHelper::GetEphemeralStorageToken(const url::Origin& origin) {
  if (auto* ephemeral_storage_service =
          EphemeralStorageServiceFactory::GetForContext(
              web_contents()->GetBrowserContext())) {
    return ephemeral_storage_service->Get1PESToken(origin);
  }
  return std::nullopt;
}

void EphemeralStorageTabHelper::WebContentsDestroyed() {}

void EphemeralStorageTabHelper::ReadyToCommitNavigation(
    NavigationHandle* navigation_handle) {
  if (!navigation_handle->IsInMainFrame()) {
    return;
  }
  if (navigation_handle->IsSameDocument()) {
    return;
  }

  const GURL& new_url = navigation_handle->GetURL();
  const GURL& last_committed_url = web_contents()->GetLastCommittedURL();

  std::string new_domain = net::URLToEphemeralStorageDomain(new_url);
  std::string previous_domain =
      net::URLToEphemeralStorageDomain(last_committed_url);
  if (new_domain != previous_domain) {
    // Create new storage areas for new ephemeral storage domain.
    CreateEphemeralStorageAreasForDomainAndURL(new_domain, new_url);
  }
}

void EphemeralStorageTabHelper::CreateEphemeralStorageAreasForDomainAndURL(
    const std::string& new_domain,
    const GURL& new_url) {
  if (new_url.is_empty()) {
    return;
  }

  auto* browser_context = web_contents()->GetBrowserContext();
  auto* site_instance = web_contents()->GetSiteInstance();

  tld_ephemeral_lifetime_ = TLDEphemeralLifetime::GetOrCreate(
      browser_context, new_domain, site_instance->GetStoragePartitionConfig());
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(EphemeralStorageTabHelper);

}  // namespace ephemeral_storage
