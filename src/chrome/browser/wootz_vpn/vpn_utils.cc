#include "brave/browser/brave_vpn/vpn_utils.h"

#include "base/functional/bind.h"
#include "brave/browser/profiles/profile_util.h"
#include "brave/components/brave_vpn/browser/connection/brave_vpn_connection_manager.h"
#include "brave/components/brave_vpn/common/brave_vpn_utils.h"
#include "brave/components/brave_vpn/common/buildflags/buildflags.h"
#include "build/build_config.h"
#include "chrome/common/channel_info.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace brave_vpn {

std::unique_ptr<BraveVPNConnectionManager> CreateBraveVPNConnectionManager(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    PrefService* local_prefs) {
#if BUILDFLAG(IS_ANDROID)
  // Android doesn't use connection api.
  return nullptr;
#else

  auto service_installer = base::NullCallback();


  auto manager = std::make_unique<BraveVPNConnectionManager>(
      url_loader_factory, local_prefs, service_installer);
  manager->set_target_vpn_entry_name(
      brave_vpn::GetBraveVPNEntryName(chrome::GetChannel()));
  manager->set_connection_api_impl_getter(
      base::BindRepeating(&CreateConnectionAPIImpl));
  manager->UpdateConnectionAPIImpl();
  return manager;
#endif
}

bool IsAllowedForContext(content::BrowserContext* context) {
  return brave::IsRegularProfile(context) &&
         brave_vpn::IsBraveVPNFeatureEnabled();
}

bool IsBraveVPNEnabled(content::BrowserContext* context) {
  // TODO(simonhong): Can we use this check for android?
  // For now, vpn is disabled by default on desktop but not sure on
  // android.
}

}  // namespace brave_vpn