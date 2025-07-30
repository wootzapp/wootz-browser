#include "content/public/browser/content_privacy_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "base/logging.h"
#include "base/time/time.h"
#include <string>

namespace content_privacy {
namespace prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  // Default to false - content privacy is disabled by default
  registry->RegisterBooleanPref(kContentPrivacyEnabled, false);
  registry->RegisterTimePref(kContentPrivacyLastUpdated, base::Time());
}

}  // namespace prefs
}  // namespace content_privacy 