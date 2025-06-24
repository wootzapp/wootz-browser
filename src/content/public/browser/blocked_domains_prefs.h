#ifndef CONTENT_PUBLIC_BROWSER_BLOCKED_DOMAINS_PREFS_H_
#define CONTENT_PUBLIC_BROWSER_BLOCKED_DOMAINS_PREFS_H_

#include <string>

class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace blocked_domains::prefs {

// The pref key for blocked domains.
inline constexpr char kBlockedDomains[] = "blocked_domains.blocked_domains";

// The pref key for blocked domains in JSON format (used by Okta integration).
inline constexpr char kBlockedDomainsJson[] = "blocked_domains.blocked_domains_json";

// Registers blocked domains prefs.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

// Utility function to generate blocked domain error page
std::string GetBlockedDomainErrorPage();

}  // namespace blocked_domains::prefs

#endif  // CONTENT_PUBLIC_BROWSER_BLOCKED_DOMAINS_PREFS_H_