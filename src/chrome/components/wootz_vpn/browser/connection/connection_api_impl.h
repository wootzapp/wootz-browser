#include <memory>
#include <string>

#include "base/memory/raw_ref.h"
#include "base/memory/scoped_refptr.h"
#include "base/values.h"
#include "chrome/components/wootz_vpn/common/mojom/wootz_vpn.mojom.h"
#include "net/base/network_change_notifier.h"

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

namespace wootz_vpn {

class WootzVPNConnectionManager;
class WootzVpnAPIRequest;
struct Hostname;

class ConnectionAPIImpl
    : public net::NetworkChangeNotifier::NetworkChangeObserver {
 public:
  enum class Type { IKEV2 };

  ConnectionAPIImpl(
      WootzVPNConnectionManager* manager,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory);
  ~ConnectionAPIImpl() override;

  ConnectionAPIImpl(const ConnectionAPIImpl&) = delete;
  ConnectionAPIImpl& operator=(const ConnectionAPIImpl&) = delete;

  void ToggleConnection();
  mojom::ConnectionState GetConnectionState() const;
  void ResetConnectionState();
  std::string GetLastConnectionError() const;
  std::string GetHostname() const;

  virtual void Connect() = 0;
  virtual void Disconnect() = 0;
  virtual void CheckConnection() = 0;
  virtual void SetSelectedRegion(const std::string& name) = 0;
  virtual void FetchProfileCredentials() = 0;
  virtual Type type() const = 0;
  virtual void UpdateAndNotifyConnectionStateChange(
      mojom::ConnectionState state);

  // net::NetworkChangeNotifier::NetworkChangeObserver
  void OnNetworkChanged(
      net::NetworkChangeNotifier::ConnectionType type) override;

 protected:
  // True when do quick cancel.
  bool QuickCancelIfPossible();
  void ResetAPIRequestInstance();
  WootzVpnAPIRequest* GetAPIRequest();
  void SetLastConnectionError(const std::string& error);
  void FetchHostnamesForRegion(const std::string& name);
  void OnFetchHostnames(const std::string& region,
                        const std::string& hostnames,
                        bool success);
  void ParseAndCacheHostnames(const std::string& region,
                              const base::Value::List& hostnames_value);
  void ResetHostname();

  const raw_ref<WootzVPNConnectionManager> manager_;  // owner

 private:
  friend class WootzVpnButtonUnitTest;
  friend class WootzVPNServiceTest;
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest, NeedsConnectTest);
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest, HostnamesTest);
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest, ConnectionInfoTest);
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest,
                           CancelConnectingTest);
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest,
                           IgnoreDisconnectedStateWhileConnecting);
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest,
                           ClearLastConnectionErrorWhenNewConnectionStart);
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest,
                           CreateOSVPNEntryWithValidInfoWhenConnectTest);
  FRIEND_TEST_ALL_PREFIXES(SystemVPNConnectionAPIUnitTest,
                           CreateOSVPNEntryWithInvalidInfoTest);

  void SetConnectionStateForTesting(mojom::ConnectionState state);

  std::unique_ptr<Hostname> hostname_;
  std::string last_connection_error_;

  // Only not null when there is active network request.
  // When network request is done, we reset this so we can know
  // whether we're waiting the response or not.
  // We can cancel connecting request quickly when fetching hostnames or
  // profile credentials is not yet finished by reset this.
  std::unique_ptr<WootzVpnAPIRequest> api_request_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  mojom::ConnectionState connection_state_ =
      mojom::ConnectionState::DISCONNECTED;
};

}  // namespace wootz_vpn