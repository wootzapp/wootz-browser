#include "chrome/browser/ui/webui/startup_crx_install/startup_crx_install_prefs.h"

#include "components/prefs/pref_registry_simple.h"
#include "base/logging.h"
#include "startup_crx_install_prefs.h"

namespace startup_crx_install {

// Define the preference name constant
const char kUtmSourcePref[] = "startup_crx_install.utm_source";

void RegisterProfilePrefs(PrefRegistrySimple* registry) {
  LOG(INFO) << "RegisterProfilePrefs called for startup_crx_install";
  // Register the utm_source preference with an empty string as default value
  registry->RegisterStringPref(kUtmSourcePref, "");
  
  // Also register the non-namespaced version that IntentHandler is using
  registry->RegisterStringPref("utm_source", "");
  
  LOG(INFO) << "Registered both utm_source preferences";
}

}  // namespace startup_crx_install
