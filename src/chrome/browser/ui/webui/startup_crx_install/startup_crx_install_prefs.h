#ifndef CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_PREFS_H_
#define CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_PREFS_H_

class PrefRegistrySimple;

namespace startup_crx_install {

// Registers the preferences used by the startup CRX install component.
void RegisterProfilePrefs(PrefRegistrySimple* registry);

// Preference name constants
extern const char kUtmSourcePref[];

}  // namespace startup_crx_install

#endif  // CHROME_BROWSER_UI_WEBUI_STARTUP_CRX_INSTALL_STARTUP_CRX_INSTALL_PREFS_H_
