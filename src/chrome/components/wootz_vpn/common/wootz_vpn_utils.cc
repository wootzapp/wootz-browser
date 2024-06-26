#include "chrome/components/wootz_vpn/common/wootz_vpn_utils.h"

#include <utility>

#include "base/feature_list.h"
#include "base/json/json_writer.h"
#include "base/json/values_util.h"
#include "base/notreached.h"
#include "base/strings/string_split.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_constants.h"
#include "chrome/components/wootz_vpn/common/features.h"
#include "chrome/components/wootz_vpn/common/pref_names.h"
#include "chrome/components/p3a_utils/feature_usage.h"
#include "chrome/components/skus/browser/skus_utils.h"
#include "chrome/components/skus/common/features.h"
#include "build/build_config.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/version_info/channel.h"

namespace wootz_vpn {

namespace {
void RegisterVPNLocalStatePrefs(PrefRegistrySimple* registry) {
#if !BUILDFLAG(IS_ANDROID)
  registry->RegisterListPref(prefs::kWootzVPNRegionList);
  registry->RegisterTimePref(prefs::kWootzVPNRegionListFetchedDate, {});
  registry->RegisterStringPref(prefs::kWootzVPNDeviceRegion, "");
  registry->RegisterStringPref(prefs::kWootzVPNSelectedRegion, "");
#endif
  registry->RegisterStringPref(prefs::kWootzVPNEnvironment,
                               skus::GetDefaultEnvironment());
  // registry->RegisterStringPref(prefs::kWootzVPNWireguardProfileCredentials, "");
  registry->RegisterDictionaryPref(prefs::kWootzVPNRootPref);
  registry->RegisterDictionaryPref(prefs::kWootzVPNSubscriberCredential);
  registry->RegisterBooleanPref(prefs::kWootzVPNLocalStateMigrated, false);
  registry->RegisterTimePref(prefs::kWootzVPNSessionExpiredDate, {});

}

}  // namespace

void MigrateVPNSettings(PrefService* profile_prefs, PrefService* local_prefs) {
  if (local_prefs->GetBoolean(prefs::kWootzVPNLocalStateMigrated)) {
    return;
  }

  if (!profile_prefs->HasPrefPath(prefs::kWootzVPNRootPref)) {
    local_prefs->SetBoolean(prefs::kWootzVPNLocalStateMigrated, true);
    return;
  }
  base::Value::Dict obsolete_pref =
      profile_prefs->GetDict(prefs::kWootzVPNRootPref).Clone();
  base::Value::Dict result;
  if (local_prefs->HasPrefPath(prefs::kWootzVPNRootPref)) {
    result = local_prefs->GetDict(prefs::kWootzVPNRootPref).Clone();
    auto& result_dict = result;
    result_dict.Merge(std::move(obsolete_pref));
  } else {
    result = std::move(obsolete_pref);
  }
  // Do not migrate wootz_vpn::prefs::kWootzVPNShowButton, we want it to be
  // inside the profile preferences.
  auto tokens =
      base::SplitString(wootz_vpn::prefs::kWootzVPNShowButton, ".",
                        base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  if (result.FindBool(tokens.back())) {
    result.Remove(tokens.back());
  }
  local_prefs->Set(prefs::kWootzVPNRootPref, base::Value(std::move(result)));
  local_prefs->SetBoolean(prefs::kWootzVPNLocalStateMigrated, true);

  bool show_button =
      profile_prefs->GetBoolean(wootz_vpn::prefs::kWootzVPNShowButton);
  profile_prefs->ClearPref(prefs::kWootzVPNRootPref);
  // Set kWootzVPNShowButton back, it is only one per profile preference for
  // now.
  profile_prefs->SetBoolean(wootz_vpn::prefs::kWootzVPNShowButton, show_button);
}

bool IsWootzVPNDisabledByPolicy(PrefService* prefs) {
  DCHECK(prefs);
  return prefs->FindPreference(prefs::kManagedWootzVPNDisabled) &&
  // Need to investigate more about this.
  // IsManagedPreference() gives false on macOS when it's configured by
  // "defaults write com.wootz.Browser.beta WootzVPNDisabled -bool true".
  // As kManagedWootzVPNDisabled is false by default and only can be set
  // by policy, I think skipping this condition checking will be fine.
#if !BUILDFLAG(IS_MAC)
         prefs->IsManagedPreference(prefs::kManagedWootzVPNDisabled) &&
#endif
         prefs->GetBoolean(prefs::kManagedWootzVPNDisabled);
}

bool IsWootzVPNFeatureEnabled() {
  return base::FeatureList::IsEnabled(wootz_vpn::features::kWootzVPN) &&
         base::FeatureList::IsEnabled(skus::features::kSkusFeature);
}

bool IsWootzVPNEnabled(PrefService* prefs) {
  return !IsWootzVPNDisabledByPolicy(prefs) && IsWootzVPNFeatureEnabled();
}

std::string GetWootzVPNEntryName(version_info::Channel channel) {
  constexpr char kWootzVPNEntryName[] = "WootzVPN";

  const std::string entry_name(kWootzVPNEntryName);
  switch (channel) {
    case version_info::Channel::UNKNOWN:
      return entry_name + "Development";
    case version_info::Channel::CANARY:
      return entry_name + "Nightly";
    case version_info::Channel::DEV:
      return entry_name + "Dev";
    case version_info::Channel::BETA:
      return entry_name + "Beta";
    case version_info::Channel::STABLE:
      return entry_name;
    default:
      return entry_name;
  }
}

std::string GetManageUrl(const std::string& env) {
  if (env == skus::kEnvProduction)
    return wootz_vpn::kManageUrlProd;
  if (env == skus::kEnvStaging)
    return wootz_vpn::kManageUrlStaging;
  if (env == skus::kEnvDevelopment)
    return wootz_vpn::kManageUrlDev;

  NOTREACHED();
  return wootz_vpn::kManageUrlProd;
}

// On desktop, the environment is tied to SKUs because you would purchase it
// from `account.wootz.com` (or similar, based on env). The credentials for VPN
// will always be in the same environment as the SKU environment.
//
// When the vendor receives a credential from us during auth, it also includes
// the environment. The vendor then can do a lookup using Payment Service.
std::string GetWootzVPNPaymentsEnv(const std::string& env) {
  // Use same string as payment env.
  return env;
}

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(prefs::kManagedWootzVPNDisabled, false);
  registry->RegisterDictionaryPref(prefs::kWootzVPNRootPref);
  registry->RegisterBooleanPref(prefs::kWootzVPNShowButton, true);

#if BUILDFLAG(IS_ANDROID)
  registry->RegisterStringPref(prefs::kWootzVPNPackageAndroid, "");
  registry->RegisterStringPref(prefs::kWootzVPNProductIdAndroid, "");
#endif
}

void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
  p3a_utils::RegisterFeatureUsagePrefs(
      registry, prefs::kWootzVPNFirstUseTime, prefs::kWootzVPNLastUseTime,
      prefs::kWootzVPNUsedSecondDay, prefs::kWootzVPNDaysInMonthUsed, nullptr);
  RegisterVPNLocalStatePrefs(registry);
}

bool HasValidSubscriberCredential(PrefService* local_prefs) {
  const base::Value::Dict& sub_cred_dict =
      local_prefs->GetDict(prefs::kWootzVPNSubscriberCredential);
  if (sub_cred_dict.empty())
    return false;

  const std::string* cred = sub_cred_dict.FindString(kSubscriberCredentialKey);
  const base::Value* expiration_time_value =
      sub_cred_dict.Find(kSubscriberCredentialExpirationKey);

  if (!cred || !expiration_time_value)
    return false;

  if (cred->empty())
    return false;

  auto expiration_time = base::ValueToTime(expiration_time_value);
  if (!expiration_time || expiration_time < base::Time::Now())
    return false;

  return true;
}

std::string GetSubscriberCredential(PrefService* local_prefs) {
  if (!HasValidSubscriberCredential(local_prefs))
    return "";
  const base::Value::Dict& sub_cred_dict =
      local_prefs->GetDict(prefs::kWootzVPNSubscriberCredential);
  const std::string* cred = sub_cred_dict.FindString(kSubscriberCredentialKey);
  DCHECK(cred);
  return *cred;
}

bool HasValidSkusCredential(PrefService* local_prefs) {
  const base::Value::Dict& sub_cred_dict =
      local_prefs->GetDict(prefs::kWootzVPNSubscriberCredential);
  if (sub_cred_dict.empty()) {
    return false;
  }

  const std::string* skus_cred = sub_cred_dict.FindString(kSkusCredentialKey);
  const base::Value* expiration_time_value =
      sub_cred_dict.Find(kSubscriberCredentialExpirationKey);

  if (!skus_cred || !expiration_time_value) {
    return false;
  }

  if (skus_cred->empty()) {
    return false;
  }

  auto expiration_time = base::ValueToTime(expiration_time_value);
  if (!expiration_time || expiration_time < base::Time::Now()) {
    return false;
  }

  return true;
}

std::string GetSkusCredential(PrefService* local_prefs) {
  CHECK(HasValidSkusCredential(local_prefs))
      << "Don't call when there is no valid skus credential.";

  const base::Value::Dict& sub_cred_dict =
      local_prefs->GetDict(prefs::kWootzVPNSubscriberCredential);
  const std::string* skus_cred = sub_cred_dict.FindString(kSkusCredentialKey);
  DCHECK(skus_cred);
  return *skus_cred;
}

}  // namespace wootz_vpn
