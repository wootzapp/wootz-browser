// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/network/cookie_settings.h"

#include <functional>
#include <iterator>
#include <memory>
#include <optional>

#include "base/containers/contains.h"
#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/metrics/histogram_macros.h"
#include "base/ranges/algorithm.h"
#include "base/strings/to_string.h"
#include "base/types/optional_util.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "components/content_settings/core/common/content_settings_rules.h"
#include "components/content_settings/core/common/content_settings_types.h"
#include "components/content_settings/core/common/content_settings_utils.h"
#include "components/content_settings/core/common/cookie_settings_base.h"
#include "components/content_settings/core/common/features.h"
#include "components/content_settings/core/common/host_indexed_content_settings.h"
#include "net/base/network_delegate.h"
#include "net/base/schemeful_site.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_constants.h"
#include "net/cookies/cookie_inclusion_status.h"
#include "net/cookies/cookie_setting_override.h"
#include "net/cookies/cookie_util.h"
#include "net/cookies/static_cookie_policy.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace network {
namespace {

bool AffectedByThirdPartyCookiePhaseout(
    const net::CookieSameSite cookie_same_site,
    const bool is_third_party_request,
    const bool is_cookie_partitioned) {
  return cookie_same_site == net::CookieSameSite::NO_RESTRICTION &&
         is_third_party_request && !is_cookie_partitioned;
}

bool IsValidType(ContentSettingsType type) {
  // Metadata exceptions are updated separately by
  // tpcd::metadata::UpdaterService.
  if (type == ContentSettingsType::TPCD_METADATA_GRANTS) {
    return true;
  }
  return CookieSettings::GetContentSettingsTypes().contains(type);
}

net::CookieInclusionStatus::ExemptionReason GetExemptionReason(
    CookieSettings::ThirdPartyCookieAllowMechanism allow_mechanism) {
  switch (allow_mechanism) {
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowByExplicitSetting:
      return net::CookieInclusionStatus::ExemptionReason::kUserSetting;
    case CookieSettings::ThirdPartyCookieAllowMechanism::kAllowBy3PCDHeuristics:
      return net::CookieInclusionStatus::ExemptionReason::k3PCDHeuristics;
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSourceUnspecified:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSourceTest:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSource1pDt:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSource3pDt:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSourceDogFood:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSourceCriticalSector:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSourceCuj:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowBy3PCDMetadataSourceGovEduTld:
      return net::CookieInclusionStatus::ExemptionReason::k3PCDMetadata;
    case CookieSettings::ThirdPartyCookieAllowMechanism::kAllowBy3PCD:
    case CookieSettings::ThirdPartyCookieAllowMechanism::kAllowByTopLevel3PCD:
      return net::CookieInclusionStatus::ExemptionReason::k3PCDDeprecationTrial;
    case CookieSettings::ThirdPartyCookieAllowMechanism::kAllowByGlobalSetting:
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowByEnterprisePolicyCookieAllowedForUrls:
      return net::CookieInclusionStatus::ExemptionReason::kEnterprisePolicy;
    case CookieSettings::ThirdPartyCookieAllowMechanism::kAllowByStorageAccess:
      return net::CookieInclusionStatus::ExemptionReason::kStorageAccess;
    case CookieSettings::ThirdPartyCookieAllowMechanism::
        kAllowByTopLevelStorageAccess:
      return net::CookieInclusionStatus::ExemptionReason::
          kTopLevelStorageAccess;
    case CookieSettings::ThirdPartyCookieAllowMechanism::kAllowByCORSException:
      return net::CookieInclusionStatus::ExemptionReason::kCorsOptIn;
    case CookieSettings::ThirdPartyCookieAllowMechanism::kNone:
      return net::CookieInclusionStatus::ExemptionReason::kNone;
  }
}

bool IsOriginOpaqueHttpOrHttps(const url::Origin* top_frame_origin) {
  if (!top_frame_origin) {
    return false;
  }
  if (!top_frame_origin->opaque()) {
    return false;
  }
  const GURL url =
      top_frame_origin->GetTupleOrPrecursorTupleIfOpaque().GetURL();
  return url.SchemeIsHTTPOrHTTPS();
}

}  // namespace

// static
bool CookieSettings::IsCookieAllowed(const net::CanonicalCookie& cookie,
                                     const CookieSettingWithMetadata& setting) {
  return IsAllowed(setting.cookie_setting()) ||
         (cookie.IsPartitioned() && setting.allow_partitioned_cookies());
}

// static
net::NetworkDelegate::PrivacySetting CookieSettings::PrivacySetting(
    const CookieSettingWithMetadata& setting) {
  if (IsAllowed(setting.cookie_setting())) {
    return net::NetworkDelegate::PrivacySetting::kStateAllowed;
  }

  if (setting.allow_partitioned_cookies()) {
    return net::NetworkDelegate::PrivacySetting::kPartitionedStateAllowedOnly;
  }

  return net::NetworkDelegate::PrivacySetting::kStateDisallowed;
}

CookieSettings::CookieSettings() {
  if (base::FeatureList::IsEnabled(
          content_settings::features::kHostIndexedMetadataGrants)) {
    content_settings_ = EntryIndex();
  } else {
    content_settings_ = EntryMap();
  }
  // Initialize content_settings_ until we receive data.
  for (auto type : GetContentSettingsTypes()) {
    set_content_settings(type, {});
  }
  // Metadata grants are relevant for CookieSettings but not synced
  // automatically. Initialize them as well.
  set_content_settings(ContentSettingsType::TPCD_METADATA_GRANTS, {});
}

CookieSettings::~CookieSettings() = default;

void CookieSettings::set_content_settings(
    ContentSettingsType type,
    const ContentSettingsForOneType& settings) {
  CHECK(IsValidType(type)) << static_cast<int>(type);
  // EntryIndex is only used if kHostIndexedMetadataGrants is enabled. Check
  // holds_alternative<>, not the flag, because b/328475709 is changing the flag
  // value during execution and leading to "bad variant access".
  if (absl::holds_alternative<EntryIndex>(content_settings_)) {
    absl::get<EntryIndex>(content_settings_)[type] =
        content_settings::HostIndexedContentSettings::Create(settings);
  } else {
    absl::get<EntryMap>(content_settings_)[type] = settings;
  }

  if (type == ContentSettingsType::COOKIES) {
    // Ensure that a default cookie setting is specified.
    if (settings.empty() ||
        settings.back().primary_pattern != ContentSettingsPattern::Wildcard() ||
        settings.back().secondary_pattern !=
            ContentSettingsPattern::Wildcard()) {
      // EntryIndex is only used if kHostIndexedMetadataGrants is enabled. Check
      // holds_alternative<>, not the flag, because b/328475709 is changing the
      // flag value during execution and leading to "bad variant access".
      if (absl::holds_alternative<EntryIndex>(content_settings_)) {
        auto& index =
            absl::get<EntryIndex>(content_settings_)[type].emplace_back(
                "default", false);
        index.SetValue(ContentSettingsPattern::Wildcard(),
                       ContentSettingsPattern::Wildcard(),
                       base::Value(CONTENT_SETTING_ALLOW), /*metadata=*/{});
      } else {
        absl::get<EntryMap>(content_settings_)[type].emplace_back(
            ContentSettingsPattern::Wildcard(),
            ContentSettingsPattern::Wildcard(),
            base::Value(CONTENT_SETTING_ALLOW),
            /*source=*/std::string(),
            /*incognito=*/false);
      }
    }
  }
}

DeleteCookiePredicate CookieSettings::CreateDeleteCookieOnExitPredicate()
    const {
  if (!HasSessionOnlyOrigins()) {
    return DeleteCookiePredicate();
  }
  ContentSettingsForOneType settings;
  // EntryIndex is only used if kHostIndexedMetadataGrants is enabled. Check
  // holds_alternative<>, not the flag, because b/328475709 is changing the flag
  // value during execution and leading to "bad variant access".
  if (absl::holds_alternative<EntryIndex>(content_settings_)) {
    // TODO(b/316530672): Ideally, clear on exit would work with the index
    // directly to benefit from faster lookup times instead of iterating over
    // a vector of content settings.
    for (const auto& index :
         GetHostIndexedContentSettings(ContentSettingsType::COOKIES)) {
      for (const auto& entry : index) {
        settings.emplace_back(entry.first.primary_pattern,
                              entry.first.secondary_pattern,
                              entry.second.value.Clone(), *index.source(),
                              *index.off_the_record(), entry.second.metadata);
      }
    }
  } else {
    settings = GetContentSettings(ContentSettingsType::COOKIES);
  }
  return base::BindRepeating(&CookieSettings::ShouldDeleteCookieOnExit,
                             base::Unretained(this), std::move(settings));
}

bool CookieSettings::ShouldIgnoreSameSiteRestrictions(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies) const {
  return base::Contains(secure_origin_cookies_allowed_schemes_,
                        site_for_cookies.scheme()) &&
         url.SchemeIsCryptographic();
}

bool CookieSettings::IsCookieAccessible(
    const net::CanonicalCookie& cookie,
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const std::optional<url::Origin>& top_frame_origin,
    const net::FirstPartySetMetadata& first_party_set_metadata,
    net::CookieSettingOverrides overrides,
    net::CookieInclusionStatus* cookie_inclusion_status) const {
  const CookieSettingWithMetadata setting_with_metadata =
      GetCookieSettingWithMetadata(url, site_for_cookies,
                                   base::OptionalToPtr(top_frame_origin),
                                   overrides);
  bool allowed = IsCookieAllowed(cookie, setting_with_metadata);
  bool is_third_party_request = IsThirdPartyRequest(url, site_for_cookies);
  if (cookie_inclusion_status) {
    if (allowed) {
      if (AffectedByThirdPartyCookiePhaseout(cookie.SameSite(),
                                             is_third_party_request,
                                             cookie.IsPartitioned())) {
        if (ShouldBlockThirdPartyCookies()) {
          cookie_inclusion_status->MaybeSetExemptionReason(GetExemptionReason(
              setting_with_metadata.third_party_cookie_allow_mechanism()));
        } else if (!setting_with_metadata.is_explicit_setting()) {
          // The cookie should be allowed by default to have this warning
          // reason.
          cookie_inclusion_status->AddWarningReason(
              net::CookieInclusionStatus::WARN_THIRD_PARTY_PHASEOUT);
        }
      }
    } else {
      if (IsThirdPartyPhaseoutEnabled() &&
          AffectedByThirdPartyCookiePhaseout(cookie.SameSite(),
                                             is_third_party_request,
                                             cookie.IsPartitioned()) &&
          !setting_with_metadata.is_explicit_setting()) {
        cookie_inclusion_status->AddExclusionReason(
            net::CookieInclusionStatus::EXCLUDE_THIRD_PARTY_PHASEOUT);

        if (first_party_set_metadata.AreSitesInSameFirstPartySet()) {
          cookie_inclusion_status->AddExclusionReason(
              net::CookieInclusionStatus::
                  EXCLUDE_THIRD_PARTY_BLOCKED_WITHIN_FIRST_PARTY_SET);
        }
      } else {
        cookie_inclusion_status->AddExclusionReason(
            net::CookieInclusionStatus::EXCLUDE_USER_PREFERENCES);
      }
    }
  }
  return allowed;
}

// Returns whether third-party cookie blocking should be bypassed (i.e. always
// allow the cookie regardless of cookie content settings and third-party
// cookie blocking settings.
// This just checks the scheme of the |url| and |site_for_cookies|:
//  - Allow cookies if the |site_for_cookies| is a chrome:// scheme URL, and
//    the |url| has a secure scheme.
//  - Allow cookies if the |site_for_cookies| and the |url| match in scheme
//    and both have the Chrome extensions scheme.
bool CookieSettings::ShouldAlwaysAllowCookies(
    const GURL& url,
    const GURL& first_party_url) const {
  return (base::Contains(secure_origin_cookies_allowed_schemes_,
                         first_party_url.scheme()) &&
          url.SchemeIsCryptographic()) ||
         (base::Contains(matching_scheme_cookies_allowed_schemes_,
                         url.scheme()) &&
          url.SchemeIs(first_party_url.scheme_piece()));
}

net::NetworkDelegate::PrivacySetting CookieSettings::IsPrivacyModeEnabled(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const std::optional<url::Origin>& top_frame_origin,
    net::CookieSettingOverrides overrides) const {
  return PrivacySetting(GetCookieSettingWithMetadata(
      url, site_for_cookies, base::OptionalToPtr(top_frame_origin), overrides));
}

CookieSettings::CookieSettingWithMetadata
CookieSettings::GetCookieSettingWithMetadata(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const url::Origin* top_frame_origin,
    net::CookieSettingOverrides overrides) const {
  return GetCookieSettingInternal(
      url, FirstPartyURLForMetadata(site_for_cookies, top_frame_origin),
      IsThirdPartyRequest(url, site_for_cookies), overrides, nullptr);
}

// static
GURL CookieSettings::FirstPartyURLForMetadata(
    const net::SiteForCookies& site_for_cookies,
    const url::Origin* top_frame_origin) {
  return IsOriginOpaqueHttpOrHttps(top_frame_origin)
             ? top_frame_origin->GetTupleOrPrecursorTupleIfOpaque().GetURL()
             : GetFirstPartyURL(site_for_cookies, top_frame_origin);
}

bool CookieSettings::AnnotateAndMoveUserBlockedCookies(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const url::Origin* top_frame_origin,
    const net::FirstPartySetMetadata& first_party_set_metadata,
    net::CookieSettingOverrides overrides,
    net::CookieAccessResultList& maybe_included_cookies,
    net::CookieAccessResultList& excluded_cookies) const {
  const CookieSettingWithMetadata setting_with_metadata =
      GetCookieSettingWithMetadata(url, site_for_cookies, top_frame_origin,
                                   overrides);
  bool is_any_allowed = false;
  if (IsAllowed(setting_with_metadata.cookie_setting())) {
    is_any_allowed = true;
  }

  bool is_third_party_request = IsThirdPartyRequest(url, site_for_cookies);
  // Add `WARN_THIRD_PARTY_PHASEOUT` `WarningReason` for allowed cookies
  // that meets the conditions and add the `ExclusionReason` for cookies
  // that ought to be blocked.
  for (net::CookieWithAccessResult& cookie : maybe_included_cookies) {
    if (IsCookieAllowed(cookie.cookie, setting_with_metadata)) {
      is_any_allowed = true;
      if (AffectedByThirdPartyCookiePhaseout(cookie.cookie.SameSite(),
                                             is_third_party_request,
                                             cookie.cookie.IsPartitioned())) {
        if (ShouldBlockThirdPartyCookies()) {
          cookie.access_result.status.MaybeSetExemptionReason(
              GetExemptionReason(
                  setting_with_metadata.third_party_cookie_allow_mechanism()));
        } else if (!setting_with_metadata.is_explicit_setting()) {
          // The cookie should be allowed by default to have this warning
          // reason.
          cookie.access_result.status.AddWarningReason(
              net::CookieInclusionStatus::WARN_THIRD_PARTY_PHASEOUT);
        }
      }
    } else {
      // Use a different exclusion reason when the 3pc is blocked by browser.
      if (IsThirdPartyPhaseoutEnabled() &&
          AffectedByThirdPartyCookiePhaseout(cookie.cookie.SameSite(),
                                             is_third_party_request,
                                             cookie.cookie.IsPartitioned()) &&
          !setting_with_metadata.is_explicit_setting()) {
        cookie.access_result.status.AddExclusionReason(
            net::CookieInclusionStatus::EXCLUDE_THIRD_PARTY_PHASEOUT);

        if (first_party_set_metadata.AreSitesInSameFirstPartySet()) {
          cookie.access_result.status.AddExclusionReason(
              net::CookieInclusionStatus::
                  EXCLUDE_THIRD_PARTY_BLOCKED_WITHIN_FIRST_PARTY_SET);
        }
      } else {
        // User has a explicit setting to block 3pc.
        cookie.access_result.status.AddExclusionReason(
            net::CookieInclusionStatus::EXCLUDE_USER_PREFERENCES);
      }
    }
  }
  for (net::CookieWithAccessResult& cookie : excluded_cookies) {
    if (!IsCookieAllowed(cookie.cookie, setting_with_metadata)) {
      // Use a different exclusion reason when the 3pc is blocked by browser.
      if (IsThirdPartyPhaseoutEnabled() &&
          AffectedByThirdPartyCookiePhaseout(cookie.cookie.SameSite(),
                                             is_third_party_request,
                                             cookie.cookie.IsPartitioned()) &&
          !setting_with_metadata.is_explicit_setting()) {
        cookie.access_result.status.AddExclusionReason(
            net::CookieInclusionStatus::EXCLUDE_THIRD_PARTY_PHASEOUT);

        if (first_party_set_metadata.AreSitesInSameFirstPartySet()) {
          cookie.access_result.status.AddExclusionReason(
              net::CookieInclusionStatus::
                  EXCLUDE_THIRD_PARTY_BLOCKED_WITHIN_FIRST_PARTY_SET);
        }
      } else {
        // User has a explicit setting to block 3pc.
        cookie.access_result.status.AddExclusionReason(
            net::CookieInclusionStatus::EXCLUDE_USER_PREFERENCES);
      }
    }
  }
  const auto to_be_moved = base::ranges::stable_partition(
      maybe_included_cookies, [](const net::CookieWithAccessResult& cookie) {
        return cookie.access_result.status.IsInclude();
      });
  excluded_cookies.insert(
      excluded_cookies.end(), std::make_move_iterator(to_be_moved),
      std::make_move_iterator(maybe_included_cookies.end()));
  maybe_included_cookies.erase(to_be_moved, maybe_included_cookies.end());

  net::cookie_util::DCheckIncludedAndExcludedCookieLists(maybe_included_cookies,
                                                         excluded_cookies);

  return is_any_allowed;
}

bool CookieSettings::HasSessionOnlyOrigins() const {
  // EntryIndex is only used if kHostIndexedMetadataGrants is enabled. Check
  // holds_alternative<>, not the flag, because b/328475709 is changing the flag
  // value during execution and leading to "bad variant access".
  if (absl::holds_alternative<EntryIndex>(content_settings_)) {
    for (const auto& index :
         GetHostIndexedContentSettings(ContentSettingsType::COOKIES)) {
      for (const auto& entry : index) {
        if (content_settings::ValueToContentSetting(entry.second.value) ==
            CONTENT_SETTING_SESSION_ONLY) {
          return true;
        }
      }
    }
    return false;
  }
  return base::ranges::any_of(
      GetContentSettings(ContentSettingsType::COOKIES), [](const auto& entry) {
        return entry.GetContentSetting() == CONTENT_SETTING_SESSION_ONLY;
      });
}

const ContentSettingsForOneType& CookieSettings::GetContentSettings(
    ContentSettingsType type) const {
  CHECK(IsValidType(type)) << "network::CookieSettings::GetContentSettings() "
                              "called with invalid type "
                           << type;
  CHECK(absl::holds_alternative<EntryMap>(content_settings_))
      << "network::CookieSettings::content_settings_ held an EntryIndex "
         "instead of an EntryMap";
  CHECK(absl::get<EntryMap>(content_settings_).contains(type))
      << "network::CookieSettings::content_settings_ did not contain type "
      << type;
  return absl::get<EntryMap>(content_settings_).at(type);
}

const std::vector<content_settings::HostIndexedContentSettings>&
CookieSettings::GetHostIndexedContentSettings(ContentSettingsType type) const {
  CHECK(IsValidType(type)) << static_cast<int>(type);
  return absl::get<EntryIndex>(content_settings_).at(type);
}

ContentSetting CookieSettings::GetContentSetting(
    const GURL& primary_url,
    const GURL& secondary_url,
    ContentSettingsType content_type,
    content_settings::SettingInfo* info) const {
  SCOPED_UMA_HISTOGRAM_TIMER_MICROS(
      "ContentSettings.GetContentSetting.Network.Duration");
  // EntryIndex is only used if kHostIndexedMetadataGrants is enabled. Check
  // holds_alternative<>, not the flag, because b/328475709 is changing the flag
  // value during execution and leading to "bad variant access".
  if (absl::holds_alternative<EntryIndex>(content_settings_)) {
    for (const auto& index : GetHostIndexedContentSettings(content_type)) {
      const content_settings::RuleEntry* result =
          index.Find(primary_url, secondary_url);
      if (result) {
        if (info) {
          info->primary_pattern = result->first.primary_pattern;
          info->secondary_pattern = result->first.secondary_pattern;
          info->metadata = result->second.metadata;
        }
        return content_settings::ValueToContentSetting(result->second.value);
      }
    }
  } else {
    const ContentSettingPatternSource* result =
        content_settings::FindContentSetting(primary_url, secondary_url,
                                             GetContentSettings(content_type));
    if (result) {
      if (info) {
        info->primary_pattern = result->primary_pattern;
        info->secondary_pattern = result->secondary_pattern;
        info->metadata = result->metadata;
      }
      return result->GetContentSetting();
    }
  }
  if (info) {
    info->primary_pattern = ContentSettingsPattern::Wildcard();
    info->secondary_pattern = ContentSettingsPattern::Wildcard();
    info->metadata = {};
  }
  return CONTENT_SETTING_BLOCK;
}

bool CookieSettings::IsThirdPartyCookiesAllowedScheme(
    const std::string& scheme) const {
  return base::Contains(third_party_cookies_allowed_schemes_, scheme);
}

bool CookieSettings::ShouldBlockThirdPartyCookies() const {
  return block_third_party_cookies_;
}

bool CookieSettings::IsThirdPartyPhaseoutEnabled() const {
  return net::cookie_util::IsForceThirdPartyCookieBlockingEnabled() ||
         tracking_protection_enabled_for_3pcd_;
}

bool CookieSettings::MitigationsEnabledFor3pcd() const {
  return net::cookie_util::IsForceThirdPartyCookieBlockingEnabled() ||
         mitigations_enabled_for_3pcd_;
}

bool CookieSettings::IsStorageAccessApiEnabled() const {
  // The network service relies on the browser process passing
  // storage_access_grants_ correctly.
  return true;
}

}  // namespace network
