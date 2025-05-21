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
inline constexpr char kAdReplacementUrl[] = "adblock.replacement_url";
inline constexpr char kAdReplacementSelectors[] = "adblock.replacement_selectors";

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

// Store ad replacement URL and selectors in prefs
void SetAdReplacementUrlAndSelectors(PrefService* prefs, const std::string& url, const std::vector<std::string>& selectors);
std::string GetAdReplacementUrl(PrefService* prefs);
std::vector<std::string> GetAdReplacementSelectors(PrefService* prefs);

}  // namespace subresource_filter::prefs

#endif  // COMPONENTS_SUBRESOURCE_FILTER_CORE_BROWSER_SUBRESOURCE_FILTER_PREFS_H_