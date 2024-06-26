#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/functional/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/scoped_observation.h"
#include "base/sequence_checker.h"
#include "base/timer/timer.h"
#include "base/values.h"
#include "chrome/components/wootz_vpn/browser/api/wootz_vpn_api_request.h"
#include "chrome/components/wootz_vpn/browser/wootz_vpn_service_delegate.h"
#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_connection_manager.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_data_types.h"
#include "chrome/components/wootz_vpn/common/mojom/wootz_vpn.mojom.h"
#include "chrome/components/skus/browser/skus_utils.h"
#include "chrome/components/skus/common/skus_sdk.mojom.h"
#include "build/build_config.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_change_registrar.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "url/gurl.h"

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

class PrefService;

#if !BUILDFLAG(IS_ANDROID)
class WootzAppMenuBrowserTest;
class WootzAppMenuModelBrowserTest;
class WootzBrowserCommandControllerTest;
#endif  // !BUILDFLAG(IS_ANDROID)

namespace wootz_vpn {

class WootzVPNServiceDelegate;

inline constexpr char kNewUserReturningHistogramName[] =
    "Wootz.VPN.NewUserReturning";
inline constexpr char kDaysInMonthUsedHistogramName[] =
    "Wootz.VPN.DaysInMonthUsed";
inline constexpr char kLastUsageTimeHistogramName[] = "Wootz.VPN.LastUsageTime";

// This class is used by desktop and android.
// However, it includes desktop specific impls and it's hidden
// by IS_ANDROID ifdef.
class WootzVpnService :
#if !BUILDFLAG(IS_ANDROID)
    public WootzVPNConnectionManager::Observer,
#endif
    public mojom::ServiceHandler,
    public KeyedService {
 public:
  WootzVpnService(
      WootzVPNConnectionManager* connection_manager,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
      PrefService* local_prefs,
      PrefService* profile_prefs,
      base::RepeatingCallback<mojo::PendingRemote<skus::mojom::SkusService>()>
          skus_service_getter);
  ~WootzVpnService() override;

  WootzVpnService(const WootzVpnService&) = delete;
  WootzVpnService& operator=(const WootzVpnService&) = delete;

  std::string GetCurrentEnvironment() const;
  
  void BindInterface(mojo::PendingReceiver<mojom::ServiceHandler> receiver);
  bool IsWootzVPNEnabled() const;
#if !BUILDFLAG(IS_ANDROID)
  void ToggleConnection();
  mojom::ConnectionState GetConnectionState() const;
  bool IsConnected() const;

  // mojom::vpn::ServiceHandler
  void GetConnectionState(GetConnectionStateCallback callback) override;
  void Connect() override;
  void Disconnect() override;
  void GetAllRegions(GetAllRegionsCallback callback) override;
  void GetSelectedRegion(GetSelectedRegionCallback callback) override;
  void SetSelectedRegion(mojom::RegionPtr region) override;
  void GetProductUrls(GetProductUrlsCallback callback) override;
  void CreateSupportTicket(const std::string& email,
                           const std::string& subject,
                           const std::string& body,
                           CreateSupportTicketCallback callback) override;
  void GetSupportData(GetSupportDataCallback callback) override;
  void ResetConnectionState() override;
  void EnableOnDemand(bool enable) override;
  void GetOnDemandState(GetOnDemandStateCallback callback) override;
#else
  // mojom::vpn::ServiceHandler
#endif  // !BUILDFLAG(IS_ANDROID)

  using ResponseCallback =
      base::OnceCallback<void(const std::string&, bool success)>;

  // mojom::vpn::ServiceHandler
  void AddObserver(
      mojo::PendingRemote<mojom::ServiceObserver> observer) override;

  void GetAllServerRegions(ResponseCallback callback);
  void GetTimezonesForRegions(ResponseCallback callback);
  void GetHostnamesForRegion(ResponseCallback callback,
                             const std::string& region);
  void GetProfileCredentials(ResponseCallback callback,
                             const std::string& hostname);
  void VerifyCredentials(ResponseCallback callback,
                         const std::string& hostname,
                         const std::string& client_id,
                         const std::string& api_auth_token);
  void InvalidateCredentials(ResponseCallback callback,
                             const std::string& hostname,
                             const std::string& client_id,
                             const std::string& api_auth_token);

  void set_delegate(std::unique_ptr<WootzVPNServiceDelegate> delegate) {
    delegate_ = std::move(delegate);
  }

  // new_usage should be set to true if a new VPN connection was just
  // established.
  void RecordP3A(bool new_usage);
#if BUILDFLAG(IS_ANDROID)
  void RecordAndroidBackgroundP3A(int64_t session_start_time_ms,
                                  int64_t session_end_time_ms);
#endif

 private:
  friend class WootzVPNServiceTest;
  friend class WootzVpnButtonUnitTest;

#if !BUILDFLAG(IS_ANDROID)
  friend class ::WootzAppMenuBrowserTest;
  friend class ::WootzAppMenuModelBrowserTest;
  friend class ::WootzBrowserCommandControllerTest;

  // WootzVPNConnectionManager::Observer overrides:
  void OnConnectionStateChanged(mojom::ConnectionState state) override;
  void OnRegionDataReady(bool success) override;
  void OnSelectedRegionChanged(const std::string& region_name) override;

  void OnCreateSupportTicket(CreateSupportTicketCallback callback,
                             const std::string& ticket,
                             bool success);

  void OnPreferenceChanged(const std::string& pref_name);

#endif  // !BUILDFLAG(IS_ANDROID)

  // KeyedService overrides:
  void Shutdown() override;

  void InitP3A();
  void OnP3AInterval();

  void SetCurrentEnvironment(const std::string& env);
  void EnsureMojoConnected();
  void OnMojoConnectionError();
  void RequestCredentialSummary(const std::string& domain);
  void OnCredentialSummary(const std::string& domain,
                           const std::string& summary_string);
  void OnPrepareCredentialsPresentation(
      const std::string& domain,
      const std::string& credential_as_cookie);
  
#if !BUILDFLAG(IS_ANDROID)
  base::ScopedObservation<WootzVPNConnectionManager,
                          WootzVPNConnectionManager::Observer>
      observed_{this};
  bool wait_region_data_ready_ = false;
  raw_ptr<WootzVPNConnectionManager> connection_manager_ = nullptr;

  PrefChangeRegistrar policy_pref_change_registrar_;
#endif  // !BUILDFLAG(IS_ANDROID)

  SEQUENCE_CHECKER(sequence_checker_);

  raw_ptr<PrefService> local_prefs_ = nullptr;
  raw_ptr<PrefService> profile_prefs_ = nullptr;
  mojo::ReceiverSet<mojom::ServiceHandler> receivers_;
  base::RepeatingCallback<mojo::PendingRemote<skus::mojom::SkusService>()>
      skus_service_getter_;
  mojo::Remote<skus::mojom::SkusService> skus_service_;
  mojo::RemoteSet<mojom::ServiceObserver> observers_;
  std::unique_ptr<WootzVpnAPIRequest> api_request_;
  std::unique_ptr<WootzVPNServiceDelegate> delegate_;
  base::RepeatingTimer p3a_timer_;
  base::WeakPtrFactory<WootzVpnService> weak_ptr_factory_{this};
};

}  // namespace wootz_vpn