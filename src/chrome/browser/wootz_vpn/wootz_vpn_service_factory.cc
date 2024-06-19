#include "chrome/browser/wootz_vpn/wootz_vpn_service_factory.h"

#include <utility>

#include "base/feature_list.h"
#include "base/no_destructor.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_util.h"
#include "chrome/browser/skus/skus_service_factory.h"
#include "chrome/browser/wootz_browser_process.h"
#include "chrome/browser/wootz_vpn/vpn_utils.h"
#include "chrome/components/skus/common/features.h"
#include "chrome/components/wootz_vpn/browser/wootz_vpn_service.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_utils.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"

namespace wootz_vpn {
namespace {

std::unique_ptr<KeyedService> BuildVpnService(
    content::BrowserContext* context) {
  if (!wootz_vpn::IsAllowedForContext(context)) {
    return nullptr;
  }

#if !BUILDFLAG(IS_ANDROID)
  if (!g_wootz_browser_process->wootz_vpn_connection_manager()) {
    return nullptr;
  }
#endif

  auto* default_storage_partition = context->GetDefaultStoragePartition();
  auto shared_url_loader_factory =
      default_storage_partition->GetURLLoaderFactoryForBrowserProcess();
  auto* local_state = g_browser_process->local_state();
  wootz_vpn::MigrateVPNSettings(user_prefs::UserPrefs::Get(context),
                                local_state);
  auto callback = base::BindRepeating(
      [](content::BrowserContext* context) {
        return skus::SkusServiceFactory::GetForContext(context);
      },
      context);

  std::unique_ptr<WootzVpnService> vpn_service =
      std::make_unique<WootzVpnService>(
          g_wootz_browser_process->wootz_vpn_connection_manager(),
          shared_url_loader_factory, local_state,
          user_prefs::UserPrefs::Get(context), callback);

  return vpn_service;
}

}  // namespace

// static
WootzVpnServiceFactory* WootzVpnServiceFactory::GetInstance() {
  static base::NoDestructor<WootzVpnServiceFactory> instance;
  return instance.get();
}

// static
WootzVpnService* WootzVpnServiceFactory::GetForProfile(Profile* profile) {
  return static_cast<WootzVpnService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
void WootzVpnServiceFactory::BindForContext(
    content::BrowserContext* context,
    mojo::PendingReceiver<wootz_vpn::mojom::ServiceHandler> receiver) {
  auto* service = static_cast<WootzVpnService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
  if (service) {
    service->BindInterface(std::move(receiver));
  }
}

WootzVpnServiceFactory::WootzVpnServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "WootzVpnService",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(skus::SkusServiceFactory::GetInstance());
}

WootzVpnServiceFactory::~WootzVpnServiceFactory() = default;

std::unique_ptr<KeyedService>
WootzVpnServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return BuildVpnService(context);
}

// static
BrowserContextKeyedServiceFactory::TestingFactory
WootzVpnServiceFactory::GetDefaultFactory() {
  return base::BindRepeating(&BuildVpnService);
}

void WootzVpnServiceFactory::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  wootz_vpn::RegisterProfilePrefs(registry);
}

}  // namespace wootz_vpn
