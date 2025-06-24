#ifndef COMPONENTS_SUBRESOURCE_FILTER_CORE_BROWSER_SUBRESOURCE_FILTER_PREFS_H_
#define COMPONENTS_SUBRESOURCE_FILTER_CORE_BROWSER_SUBRESOURCE_FILTER_PREFS_H_

#include <string>
#include <vector>

class PrefService;

namespace user_prefs { 
    class PrefRegistrySyncable; 
}

namespace subresource_filter::prefs {

inline constexpr char kAdBlockGlobalEnabled[] = "adblock.global_enabled";
inline constexpr char kAdReplacementSelectors[] = "adblock.replacement_selectors";

// New preference keys for ad configuration
inline constexpr char kAdReplacementAdUnitPath[] = "adblock.replacement.ad_unit_path";
inline constexpr char kAdReplacementIdPrefix[] = "adblock.replacement.id_prefix";
inline constexpr char kAdReplacementScriptUrl[] = "adblock.replacement.script_url";
inline constexpr char kAdReplacementSizes[] = "adblock.replacement.sizes";

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

// Store ad replacement URL and selectors in prefs
void SetAdReplacementSelectors(PrefService* prefs, const std::vector<std::string>& selectors);
std::vector<std::string> GetAdReplacementSelectors(PrefService* prefs);

// New functions for storing and retrieving ad configuration
void SetAdReplacementConfig(PrefService* prefs, const std::string& ad_unit_path, 
                          const std::string& id_prefix, const std::string& script_url,
                          const std::string& sizes_json);
                          
std::string GetAdReplacementAdUnitPath(PrefService* prefs);
std::string GetAdReplacementIdPrefix(PrefService* prefs);
std::string GetAdReplacementScriptUrl(PrefService* prefs);
std::string GetAdReplacementSizes(PrefService* prefs);

}  // namespace subresource_filter::prefs

#endif  // COMPONENTS_SUBRESOURCE_FILTER_CORE_BROWSER_SUBRESOURCE_FILTER_PREFS_H_