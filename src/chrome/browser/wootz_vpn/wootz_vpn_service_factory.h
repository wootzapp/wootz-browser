#include <memory>

#include "chrome/components/wootzapp_vpn/common/mojom/wootzapp_vpn.mojom.h"
#include "build/build_config.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

namespace wootzapp_vpn {

class WootzAppVpnService;

class WootzAppVpnServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static WootzAppVpnService* GetForProfile(Profile* profile);
  static WootzAppVpnServiceFactory* GetInstance();

  WootzAppVpnServiceFactory(const WootzAppVpnServiceFactory&) = delete;
  WootzAppVpnServiceFactory& operator=(const WootzAppVpnServiceFactory&) = delete;

  static void BindForContext(
      content::BrowserContext* context,
      mojo::PendingReceiver<wootzapp_vpn::mojom::ServiceHandler> receiver);

  // Returns the default factory, useful in tests.
  static TestingFactory GetDefaultFactory();

 private:
  friend base::NoDestructor<WootzAppVpnServiceFactory>;

  WootzAppVpnServiceFactory();
  ~WootzAppVpnServiceFactory() override;

  // BrowserContextKeyedServiceFactory overrides:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  void RegisterProfilePrefs(
      user_prefs::PrefRegistrySyncable* registry) override;
};

}  // namespace wootzapp_vpn

#endif  // WOOTZ_BROWSER_WOOTZ_VPN_WOOTZ_VPN_SERVICE_FACTORY_H_