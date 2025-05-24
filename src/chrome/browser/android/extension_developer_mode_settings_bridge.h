

#ifndef CHROME_BROWSER_ANDROID_EXTENSION_DEVELOPER_MODE_SETTINGS_BRIDGE_H_
#define CHROME_BROWSER_ANDROID_EXTENSION_DEVELOPER_MODE_SETTINGS_BRIDGE_H_

namespace user_prefs {
class PrefRegistrySyncable;
}  // namespace user_prefs

namespace chrome {
namespace android {

// Returns whether Extension Developer Mode is currently enabled.
bool IsExtensionDeveloperModeEnabled();

// Registers the preference for Extension Developer Mode.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace android
}  // namespace chrome

#endif  // CHROME_BROWSER_ANDROID_EXTENSION_DEVELOPER_MODE_SETTINGS_BRIDGE_H_
