#include "components/subresource_filter/core/browser/subresource_filter_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"

namespace subresource_filter {
namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(kAdBlockGlobalEnabled, false);
  registry->RegisterStringPref(kAdReplacementUrl, std::string());
  registry->RegisterListPref(kAdReplacementSelectors);
}

void SetAdReplacementUrlAndSelectors(PrefService* prefs, const std::string& url, const std::vector<std::string>& selectors) {
  prefs->SetString(kAdReplacementUrl, url);
  base::Value::List selector_list;
  for (const auto& sel : selectors) {
    selector_list.Append(sel);
  }
  prefs->SetList(kAdReplacementSelectors, std::move(selector_list));
}

std::string GetAdReplacementUrl(PrefService* prefs) {
  return prefs->GetString(kAdReplacementUrl);
}

std::vector<std::string> GetAdReplacementSelectors(PrefService* prefs) {
  std::vector<std::string> result;
  const base::Value::List& list = prefs->GetList(kAdReplacementSelectors);
  for (const auto& val : list) {
    if (val.is_string())
      result.push_back(val.GetString());
  }
  return result;
}

}  // namespace prefs
}  // namespace subresource_filter