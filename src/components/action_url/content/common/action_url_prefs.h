#ifndef COMPONENTS_ACTION_URL_CONTENT_COMMON_ACTION_URL_PREFS_H_
#define COMPONENTS_ACTION_URL_CONTENT_COMMON_ACTION_URL_PREFS_H_

class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace action_url::prefs {

// The pref key for enabling/disabling Action URL (blinks) system.
inline constexpr char kBlinksEnabled[] = "action_url.blinks_enabled";

// Registers Action URL prefs.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace action_url::prefs

#endif  // COMPONENTS_ACTION_URL_CONTENT_COMMON_ACTION_URL_PREFS_H_