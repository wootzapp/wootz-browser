#ifndef CONTENT_PUBLIC_BROWSER_SAML_PREFS_H_
#define CONTENT_PUBLIC_BROWSER_SAML_PREFS_H_

#include <string>

class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace saml::prefs {

// The pref key for SAML response.
inline constexpr char kSamlResponse[] = "wootz.saml_response";

// The pref key for SAML issuer URL.
inline constexpr char kSamlIssuer[] = "wootz.saml_issuer";

// The pref key for SAML last processed timestamp.
inline constexpr char kSamlLastProcessed[] = "wootz.saml_last_processed";

// Registers SAML prefs.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace saml::prefs

#endif  // CONTENT_PUBLIC_BROWSER_SAML_PREFS_H_ 