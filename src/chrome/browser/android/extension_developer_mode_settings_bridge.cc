#include "chrome/browser/android/extension_developer_mode_settings_bridge.h"

#include "base/android/jni_android.h"
#include "base/logging.h"
#include "components/prefs/pref_service.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "chromecast/common/cors_exempt_headers.h"
#include "net/http/http_request_headers.h"
#include "url/gurl.h"
#include "base/logging.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "chrome/browser/android/extension_developer_mode_settings_prefs.h"

// The JNI header will be generated during build
#include "chrome/android/chrome_jni_headers/ExtensionDeveloperModeSettings_jni.h"

namespace chrome {
namespace android {

static void JNI_ExtensionDeveloperModeSettings_IsDeveloperModeEnabled(
    JNIEnv* env, 
    jboolean isDeveloperModeEnabled) {
    bool enabled = static_cast<bool>(isDeveloperModeEnabled);
    LOG(INFO) << "Extension Developer Mode enabled state received from Java: " << enabled;
    Profile* profile = ProfileManager::GetLastUsedProfile();
    if (!profile) {
        LOG(ERROR) << "Failed to get profile for storing Extension Developer Mode enabled state";
        return;
    }
    PrefService* prefs = profile->GetPrefs();
    if (prefs) {
        // Store in preference key
        prefs->SetBoolean(extension_developer_mode_settings::kExtensionDeveloperModeEnabledPref, enabled);
        
        LOG(INFO) << "Successfully stored Extension Developer Mode enabled state in preferences";
    } else {
        LOG(ERROR) << "Failed to get preferences for storing Extension Developer Mode enabled state";
    }
}

}  // namespace android
}  // namespace chrome

