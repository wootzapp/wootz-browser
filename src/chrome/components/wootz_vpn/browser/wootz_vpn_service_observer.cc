#include "wootz/components/wootz_vpn/browser/wootz_vpn_service_observer.h"

#include <utility>

#include "wootz/components/wootz_vpn/browser/wootz_vpn_service.h"
#include "wootz/components/wootz_vpn/common/wootz_vpn_utils.h"

namespace wootz_vpn {

WootzVPNServiceObserver::WootzVPNServiceObserver() = default;

WootzVPNServiceObserver::~WootzVPNServiceObserver() = default;

void WootzVPNServiceObserver::Observe(WootzVpnService* service) {
  if (!service) {
    return;
  }

  if (service->IsWootzVPNEnabled()) {
    mojo::PendingRemote<mojom::ServiceObserver> listener;
    receiver_.Bind(listener.InitWithNewPipeAndPassReceiver());
    service->AddObserver(std::move(listener));
  }
}

}  // namespace wootz_vpn
