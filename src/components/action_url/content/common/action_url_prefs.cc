#include "components/action_url/content/common/action_url_prefs.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "base/logging.h"

namespace action_url {
namespace prefs {


void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(kBlinksEnabled, false);
}

}  // namespace prefs
}  // namespace action_url