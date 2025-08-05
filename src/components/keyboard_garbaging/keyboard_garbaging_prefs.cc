#include "components/keyboard_garbaging/keyboard_garbaging_prefs.h"

#include "base/logging.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"

namespace keyboard_garbaging_prefs {

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterBooleanPref(keyboard_garbaging_prefs::kObfuscationEnabled,
                                false);
}

bool IsObfuscationEnabled(PrefService* prefs) {
  if (!prefs) {
    return false;
  }
  return prefs->GetBoolean(kObfuscationEnabled);
}

void SetObfuscationEnabled(PrefService* prefs, bool enabled) {
  if (!prefs) {
    return;
  }
  prefs->SetBoolean(kObfuscationEnabled, enabled);
}

}  // namespace keyboard_garbaging_prefs
