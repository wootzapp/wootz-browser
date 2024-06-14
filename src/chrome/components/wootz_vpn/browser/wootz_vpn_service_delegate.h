#include "wootz/components/wootz_vpn/common/mojom/wootz_vpn.mojom.h"

namespace wootz_vpn {

class WootzVPNServiceDelegate {
 public:
  WootzVPNServiceDelegate() = default;
  virtual ~WootzVPNServiceDelegate() = default;

  WootzVPNServiceDelegate(const WootzVPNServiceDelegate&) = delete;
  WootzVPNServiceDelegate& operator=(const WootzVPNServiceDelegate&) = delete;

  virtual void WriteConnectionState(mojom::ConnectionState state) = 0;
  virtual void ShowWootzVpnStatusTrayIcon() = 0;
};

}  // namespace wootz_vpn

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_BRAVE_VPN_SERVICE_DELEGATE_H_
