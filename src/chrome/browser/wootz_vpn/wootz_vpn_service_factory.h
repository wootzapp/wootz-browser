#include <memory>

#include "chrome/components/wootz_vpn/common/mojom/wootz_vpn.mojom.h"
#include "build/build_config.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

namespace wootz_vpn {

class WootzVpnService;

class WootzVpnServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static WootzVpnService* GetForProfile(Profile* profile);
  static WootzVpnServiceFactory* GetInstance();

  WootzVpnServiceFactory(const WootzVpnServiceFactory&) = delete;
  WootzVpnServiceFactory& operator=(const WootzVpnServiceFactory&) = delete;

  static void BindForContext(
      content::BrowserContext* context,
      mojo::PendingReceiver<wootz_vpn::mojom::ServiceHandler> receiver);

  // Returns the default factory, useful in tests.
  static TestingFactory GetDefaultFactory();

 private:
  friend base::NoDestructor<WootzVpnServiceFactory>;

  WootzVpnServiceFactory();
  ~WootzVpnServiceFactory() override;

  // BrowserContextKeyedServiceFactory overrides:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  void RegisterProfilePrefs(
      user_prefs::PrefRegistrySyncable* registry) override;
};

}  // namespace wootz_vpn

#endif  // WOOTZ_BROWSER_WOOTZ_VPN_WOOTZ_VPN_SERVICE_FACTORY_H_