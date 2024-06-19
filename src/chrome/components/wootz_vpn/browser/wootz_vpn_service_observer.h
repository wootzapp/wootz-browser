#include <optional>
#include <string>

#include "build/build_config.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "wootz/components/wootz_vpn/common/mojom/wootz_vpn.mojom.h"

namespace wootz_vpn {

class WootzVpnService;

class WootzVPNServiceObserver : public mojom::ServiceObserver {
 public:
  WootzVPNServiceObserver();
  ~WootzVPNServiceObserver() override;
  WootzVPNServiceObserver(const WootzVPNServiceObserver&) = delete;
  WootzVPNServiceObserver& operator=(const WootzVPNServiceObserver&) = delete;

  void Observe(WootzVpnService* service);

  // mojom::ServiceObserver overrides:
  void OnPurchasedStateChanged(
      mojom::PurchasedState state,
      const std::optional<std::string>& description) override {}
#if !BUILDFLAG(IS_ANDROID)
  void OnConnectionStateChanged(mojom::ConnectionState state) override {}
  void OnSelectedRegionChanged(mojom::RegionPtr region) override {}
#endif  // !BUILDFLAG(IS_ANDROID)

 private:
  mojo::Receiver<mojom::ServiceObserver> receiver_{this};
};

}  // namespace wootz_vpn
