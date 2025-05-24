#include "chrome/browser/android/extension_developer_mode_settings_prefs.h"

#include "components/pref_registry/pref_registry_syncable.h"
#include "base/logging.h"

namespace extension_developer_mode_settings {

const char kExtensionDeveloperModeEnabledPref[] = "extension_developer_mode_enabled";
const char kExtensionFileName[] = "extension_file_name";

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  LOG(INFO)<<"extension_developer_mode_settings pref registered";
  registry->RegisterBooleanPref(kExtensionDeveloperModeEnabledPref, false);
  registry->RegisterStringPref(kExtensionFileName, "");
}

}   // namespace extension_developer_mode_settings
