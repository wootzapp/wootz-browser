#ifndef CONTENT_PUBLIC_BROWSER_CONTENT_PRIVACY_PREFS_H_
#define CONTENT_PUBLIC_BROWSER_CONTENT_PRIVACY_PREFS_H_

#include <string>

class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace content_privacy::prefs {

// The pref key for content privacy enabled state from SAML.
inline constexpr char kContentPrivacyEnabled[] = "wootz.content_privacy_enabled";

// The pref key for content privacy last updated timestamp.
inline constexpr char kContentPrivacyLastUpdated[] = "wootz.content_privacy_last_updated";

// Registers content privacy prefs.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace content_privacy::prefs

#endif  // CONTENT_PUBLIC_BROWSER_CONTENT_PRIVACY_PREFS_H_ 