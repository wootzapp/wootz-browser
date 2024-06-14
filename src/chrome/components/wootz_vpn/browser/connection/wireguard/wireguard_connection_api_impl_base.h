#include <string>

#include "base/memory/weak_ptr.h"
#include "chrome/components/wootz_vpn/browser/connection/connection_api_impl.h"
#include "chrome/components/wootz_vpn/browser/connection/wireguard/credentials/wootz_vpn_wireguard_profile_credentials.h"
#include "chrome/components/wootz_vpn/common/wireguard/wireguard_utils.h"

namespace wootz_vpn {

class WireguardConnectionAPIImplBase : public ConnectionAPIImpl {
 public:
  WireguardConnectionAPIImplBase(
      BraveVPNConnectionManager* manager,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory);
  ~WireguardConnectionAPIImplBase() override;

  // ConnectionAPIImpl overrides:
  void FetchProfileCredentials() override;
  void SetSelectedRegion(const std::string& name) override;
  void Connect() override;
  void UpdateAndNotifyConnectionStateChange(
      mojom::ConnectionState state) override;
  Type type() const override;

  // Platform dependent APIs.
  virtual void PlatformConnectImpl(
      const wireguard::WireguardProfileCredentials& credentials) = 0;

 protected:
  void OnDisconnected(bool success);
  void RequestNewProfileCredentials(
      wootz_vpn::wireguard::WireguardKeyPair key_pair);
  void OnGetProfileCredentials(const std::string& client_private_key,
                               const std::string& profile_credential,
                               bool success);
  void OnVerifyCredentials(const std::string& result, bool success);

 private:
  void ResetConnectionInfo();

  base::WeakPtrFactory<WireguardConnectionAPIImplBase> weak_factory_{this};
};

}  // namespace wootz_vpn