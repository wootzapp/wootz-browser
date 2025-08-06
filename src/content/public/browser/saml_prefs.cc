#include "content/public/browser/saml_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "base/logging.h"
#include "base/time/time.h"
#include <string>

namespace saml {
namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterStringPref(kSamlResponse, std::string());
  registry->RegisterStringPref(kSamlIssuer, std::string());
  registry->RegisterTimePref(kSamlLastProcessed, base::Time());
}

}  // namespace prefs
}  // namespace saml 