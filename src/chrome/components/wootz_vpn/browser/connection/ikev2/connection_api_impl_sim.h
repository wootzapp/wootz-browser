#include <optional>
#include <string>

#include "base/gtest_prod_util.h"
#include "base/memory/weak_ptr.h"
#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_connection_info.h"
#include "chrome/components/wootz_vpn/browser/connection/ikev2/system_vpn_connection_api_impl_base.h"

namespace wootz_vpn {

class ConnectionAPIImplSim : public SystemVPNConnectionAPIImplBase {
 public:
  ConnectionAPIImplSim(
      WootzVPNConnectionManager* manager,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory);
  ~ConnectionAPIImplSim() override;

  // RasConnectionAPIImpl overrides:
  void Connect() override;
  void Disconnect() override;
  void CheckConnection() override;
  Type type() const override;
  void CreateVPNConnectionImpl(const WootzVPNConnectionInfo& info) override;
  void ConnectImpl(const std::string& name) override;
  void DisconnectImpl(const std::string& name) override;
  void CheckConnectionImpl(const std::string& name) override;
  bool IsPlatformNetworkAvailable() override;

  bool IsConnectionCreated() const;
  bool IsConnectionChecked() const;
  void SetNetworkAvailableForTesting(bool value);

  void OnCreated(const std::string& name, bool success);
  void OnConnected(const std::string& name, bool success);
  void OnIsConnecting(const std::string& name);
  void OnDisconnected(const std::string& name, bool success);
  void OnIsDisconnecting(const std::string& name);
  void OnRemoved(const std::string& name, bool success);

  bool disconnect_requested_ = false;
  bool connection_created_ = false;
  bool check_connection_called_ = false;

  std::optional<bool> network_available_;
  base::WeakPtrFactory<ConnectionAPIImplSim> weak_factory_{this};
};

}  // namespace wootz_vpn
