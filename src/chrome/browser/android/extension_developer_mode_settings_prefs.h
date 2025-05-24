#ifndef CHROME_BROWSER_ANDROID_EXTENSION_DEVELOPER_MODE_SETTINGS_PREFS_H_
#define CHROME_BROWSER_ANDROID_EXTENSION_DEVELOPER_MODE_SETTINGS_PREFS_H_

#include "components/prefs/pref_registry_simple.h"

namespace user_prefs {
class PrefRegistrySyncable;
} 

namespace extension_developer_mode_settings {

// Registers the preferences used by the extension developer mode component.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

// Preference name constants
extern const char kExtensionDeveloperModeEnabledPref[];
extern const char kExtensionFileName[];

}  // namespace extension_developer_mode_settings

#endif  // CHROME_BROWSER_ANDROID_EXTENSION_DEVELOPER_MODE_SETTINGS_PREFS_H_
