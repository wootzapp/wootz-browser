#ifndef COMPONENTS_KEYBOARD_GARBAGING_KEYBOARD_GARBAGING_PREFS_H_
#define COMPONENTS_KEYBOARD_GARBAGING_KEYBOARD_GARBAGING_PREFS_H_

class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace keyboard_garbaging_prefs {

// The pref key for enabling/disabling input obfuscation system.
inline constexpr char kObfuscationEnabled[] = "input.obfuscation_enabled";

// Registers obfuscation prefs.
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

// Helper functions to get/set obfuscation preference
bool IsObfuscationEnabled(PrefService* prefs);
void SetObfuscationEnabled(PrefService* prefs, bool enabled);

}  // namespace keyboard_garbaging_prefs

#endif  // CONTENT_PUBLIC_ANDROID_JAVA_SRC_ORG_CHROMIUM_CONTENT_BROWSER_INPUT_OBFUSCATION_PREFS_H_
