#include "components/subresource_filter/core/browser/subresource_filter_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"

namespace subresource_filter {
namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(kAdBlockGlobalEnabled, false);
  registry->RegisterListPref(kAdReplacementSelectors);
  
  // Register new preference keys
  registry->RegisterStringPref(kAdReplacementAdUnitPath, std::string());
  registry->RegisterStringPref(kAdReplacementIdPrefix, std::string());
  registry->RegisterStringPref(kAdReplacementScriptUrl, 
                             "https://securepubads.g.doubleclick.net/tag/js/gpt.js");
  registry->RegisterStringPref(kAdReplacementSizes, "[[728, 90], [468, 60], [300, 250]]");
}

void SetAdReplacementSelectors(PrefService* prefs, const std::vector<std::string>& selectors) {
  base::Value::List selector_list;
  for (const auto& sel : selectors) {
    selector_list.Append(sel);
  }
  prefs->SetList(kAdReplacementSelectors, std::move(selector_list));
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

// New accessor implementations
void SetAdReplacementConfig(PrefService* prefs, const std::string& ad_unit_path, 
                          const std::string& id_prefix, const std::string& script_url,
                          const std::string& sizes_json) {
  prefs->SetString(kAdReplacementAdUnitPath, ad_unit_path);
  prefs->SetString(kAdReplacementIdPrefix, id_prefix);
  prefs->SetString(kAdReplacementScriptUrl, script_url);
  prefs->SetString(kAdReplacementSizes, sizes_json);
}

std::string GetAdReplacementAdUnitPath(PrefService* prefs) {
  return prefs->GetString(kAdReplacementAdUnitPath);
}

std::string GetAdReplacementIdPrefix(PrefService* prefs) {
  return prefs->GetString(kAdReplacementIdPrefix);
}

std::string GetAdReplacementScriptUrl(PrefService* prefs) {
  return prefs->GetString(kAdReplacementScriptUrl);
}

std::string GetAdReplacementSizes(PrefService* prefs) {
  return prefs->GetString(kAdReplacementSizes);
}

}  // namespace prefs
}  // namespace subresource_filter