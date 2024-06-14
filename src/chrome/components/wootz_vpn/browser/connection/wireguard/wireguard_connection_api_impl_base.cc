#include "chrome/components/wootz_vpn/browser/connection/wireguard/wireguard_connection_api_impl_base.h"

#include "chrome/components/wootz_vpn/browser/api/wootz_vpn_api_request.h"
#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_connection_manager.h"
#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_region_data_manager.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_utils.h"
#include "chrome/components/wootz_vpn/common/mojom/wootz_vpn.mojom.h"
#include "chrome/components/wootz_vpn/common/pref_names.h"
#include "chrome/components/wootz_vpn/common/wireguard/wireguard_utils.h"
#include "components/prefs/pref_service.h"

namespace wootz_vpn {

using ConnectionState = mojom::ConnectionState;

WireguardConnectionAPIImplBase::WireguardConnectionAPIImplBase(
    WootzVPNConnectionManager* manager,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : ConnectionAPIImpl(manager, url_loader_factory) {}

WireguardConnectionAPIImplBase::~WireguardConnectionAPIImplBase() = default;

void WireguardConnectionAPIImplBase::SetSelectedRegion(
    const std::string& name) {
  manager_->GetRegionDataManager().SetSelectedRegion(name);
  ResetConnectionInfo();
}

void WireguardConnectionAPIImplBase::RequestNewProfileCredentials(
    wootz_vpn::wireguard::WireguardKeyPair key_pair) {
  if (!key_pair.has_value()) {
    VLOG(1) << __func__ << " : failed to get keypair";
    UpdateAndNotifyConnectionStateChange(ConnectionState::CONNECT_FAILED);
    SetLastConnectionError("Failed to create keypair");
    return;
  }
  const auto [public_key, private_key] = key_pair.value();
  GetAPIRequest()->GetWireguardProfileCredentials(
      base::BindOnce(&WireguardConnectionAPIImplBase::OnGetProfileCredentials,
                     base::Unretained(this), private_key),
      GetSubscriberCredential(manager_->local_prefs()),
      public_key,
      GetHostname());
}

void WireguardConnectionAPIImplBase::Connect() {
  VLOG(2) << __func__ << " : start connecting!";
  SetLastConnectionError(std::string());
  UpdateAndNotifyConnectionStateChange(ConnectionState::CONNECTING);

  // There's some fetched
  if (!GetHostname().empty()) {
    FetchProfileCredentials();
    return;
  }
  // If user doesn't select region explicitely, use default device region.
  std::string target_region_name =
      manager_->GetRegionDataManager().GetSelectedRegion();
  if (target_region_name.empty()) {
    target_region_name = manager_->GetRegionDataManager().GetDeviceRegion();
    VLOG(2) << __func__ << " : start connecting with valid default_region: "
            << target_region_name;
  }
  DCHECK(!target_region_name.empty());
  FetchHostnamesForRegion(target_region_name);
}

void WireguardConnectionAPIImplBase::OnGetProfileCredentials(
    const std::string& client_private_key,
    const std::string& profile_credentials,
    bool success) {
  if (!success) {
    VLOG(1) << __func__ << " : failed to get profile credential";
    UpdateAndNotifyConnectionStateChange(ConnectionState::CONNECT_FAILED);
    SetLastConnectionError("Failed to get profile credential");
    return;
  }
  auto parsed_credentials =
      wireguard::WireguardProfileCredentials::FromServerResponse(
          profile_credentials, client_private_key);
  if (!parsed_credentials.has_value()) {
    VLOG(1) << __func__ << " : failed to get correct credentials";
    UpdateAndNotifyConnectionStateChange(ConnectionState::CONNECT_FAILED);
    SetLastConnectionError("Failed to get correct credentials");
    return;
  }
  auto serialized = parsed_credentials->ToString();
  if (serialized.has_value()) {
    manager_->local_prefs()->SetString(
        prefs::kWootzVPNWireguardProfileCredentials, serialized.value());
  }
  PlatformConnectImpl(parsed_credentials.value());
}

void WireguardConnectionAPIImplBase::FetchProfileCredentials() {
  if (!GetAPIRequest()) {
    return;
  }
  auto existing_credentials =
      wireguard::WireguardProfileCredentials::FromString(
          manager_->local_prefs()->GetString(
              prefs::kWootzVPNWireguardProfileCredentials));
  if (!existing_credentials.has_value()) {
    RequestNewProfileCredentials(wireguard::GenerateNewX25519Keypair());
    return;
  }
  GetAPIRequest()->VerifyCredentials(
      base::BindOnce(&WireguardConnectionAPIImplBase::OnVerifyCredentials,
                     weak_factory_.GetWeakPtr()),
      GetHostname(), existing_credentials->client_id,
      GetSubscriberCredential(manager_->local_prefs()),
      existing_credentials->api_auth_token);
}

void WireguardConnectionAPIImplBase::ResetConnectionInfo() {
  VLOG(2) << __func__;
  ResetHostname();
  manager_->local_prefs()->SetString(
      prefs::kWootzVPNWireguardProfileCredentials, std::string());
}

void WireguardConnectionAPIImplBase::OnVerifyCredentials(
    const std::string& result,
    bool success) {
  auto existing_credentials =
      wireguard::WireguardProfileCredentials::FromString(
          manager_->local_prefs()->GetString(
              prefs::kWootzVPNWireguardProfileCredentials));
  if (!success || !existing_credentials.has_value()) {
    VLOG(1) << __func__ << " : credentials verification failed ( " << result
            << " ), request new";
    RequestNewProfileCredentials(wireguard::GenerateNewX25519Keypair());
    return;
  }
  PlatformConnectImpl(existing_credentials.value());
}

void WireguardConnectionAPIImplBase::UpdateAndNotifyConnectionStateChange(
    mojom::ConnectionState state) {
  if (GetConnectionState() != state &&
      state == ConnectionState::CONNECT_FAILED) {
    ResetConnectionInfo();
  }

  ConnectionAPIImpl::UpdateAndNotifyConnectionStateChange(state);
}

ConnectionAPIImpl::Type WireguardConnectionAPIImplBase::type() const {
  return Type::WIREGUARD;
}

void WireguardConnectionAPIImplBase::OnDisconnected(bool success) {
  if (!success) {
    VLOG(1) << "Failed to stop wireguard tunnel service";
    SetLastConnectionError("Failed to stop wireguard tunnel service");
    UpdateAndNotifyConnectionStateChange(ConnectionState::CONNECTED);
    return;
  }

  UpdateAndNotifyConnectionStateChange(ConnectionState::DISCONNECTED);
}

}  // namespace wootz_vpn