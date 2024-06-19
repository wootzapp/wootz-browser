#include "chrome/browser/wootz_vpn/vpn_utils.h"

#include "base/functional/bind.h"
#include "build/build_config.h"
#include "chrome/browser/profiles/profile_util.h"
#include "chrome/common/channel_info.h"
#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_connection_manager.h"
#include "chrome/components/wootz_vpn/common/buildflags/buildflags.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_utils.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace wootz_vpn {

std::unique_ptr<WootzVPNConnectionManager> CreateWootzVPNConnectionManager(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    PrefService* local_prefs) {
#if BUILDFLAG(IS_ANDROID)
  // Android doesn't use connection api.
  return nullptr;
#else

  auto service_installer = base::NullCallback();

  auto manager = std::make_unique<WootzVPNConnectionManager>(
      url_loader_factory, local_prefs, service_installer);
  manager->set_target_vpn_entry_name(
      wootz_vpn::GetWootzVPNEntryName(chrome::GetChannel()));
  manager->set_connection_api_impl_getter(
      base::BindRepeating(&CreateConnectionAPIImpl));
  manager->UpdateConnectionAPIImpl();
  return manager;
#endif
}

bool IsAllowedForContext(content::BrowserContext* context) {
  return wootz::IsRegularProfile(context) &&
         wootz_vpn::IsWootzVPNFeatureEnabled();
}

bool IsWootzVPNEnabled(content::BrowserContext* context) {
  // TODO(simonhong): Can we use this check for android?
  // For now, vpn is disabled by default on desktop but not sure on
  // android.
}

}  // namespace wootz_vpn
