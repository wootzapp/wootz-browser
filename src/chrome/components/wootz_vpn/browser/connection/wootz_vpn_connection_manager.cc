#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_connection_manager.h"

#include <optional>
#include <utility>
#include <vector>

#include "base/check.h"
#include "base/check_is_test.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/task/thread_pool.h"
#include "chrome/components/wootz_vpn/browser/connection/connection_api_impl.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_utils.h"
#include "chrome/components/wootz_vpn/common/pref_names.h"
#include "build/build_config.h"
#include "components/prefs/pref_service.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace wootz_vpn {

WootzVPNConnectionManager::WootzVPNConnectionManager(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    PrefService* local_prefs,
    base::RepeatingCallback<bool()> service_installer)
    : local_prefs_(local_prefs),
      url_loader_factory_(url_loader_factory),
      region_data_manager_(url_loader_factory, local_prefs),
      weak_factory_(this) {
  DCHECK(url_loader_factory_);
  install_system_service_callback_ = std::move(service_installer);

  // Safe to use Unretained here because |region_data_manager_| is owned
  // instance.
  region_data_manager_.set_selected_region_changed_callback(base::BindRepeating(
      &WootzVPNConnectionManager::NotifySelectedRegionChanged,
      base::Unretained(this)));
  region_data_manager_.set_region_data_ready_callback(
      base::BindRepeating(&WootzVPNConnectionManager::NotifyRegionDataReady,
                          base::Unretained(this)));
}

WootzVPNConnectionManager::~WootzVPNConnectionManager() = default;

WootzVPNRegionDataManager& WootzVPNConnectionManager::GetRegionDataManager() {
  return region_data_manager_;
}

void WootzVPNConnectionManager::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void WootzVPNConnectionManager::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void WootzVPNConnectionManager::NotifyRegionDataReady(bool ready) const {
  for (auto& obs : observers_) {
    obs.OnRegionDataReady(ready);
  }
}

void WootzVPNConnectionManager::NotifySelectedRegionChanged(
    const std::string& name) const {
  for (auto& obs : observers_) {
    obs.OnSelectedRegionChanged(name);
  }
}

void WootzVPNConnectionManager::UpdateConnectionAPIImpl() {
  if (!connection_api_impl_getter_) {
    CHECK_IS_TEST();
    return;
  }

  // This could be called multiple times, so don't reset current connection
  // if prefs is matched with current |connection_api_impl_|.

  if (!connection_api_impl_) {
    // Create new connection api impl.
    return;
  }

}

mojom::ConnectionState WootzVPNConnectionManager::GetConnectionState() const {
  if (connection_api_impl_) {
    return connection_api_impl_->GetConnectionState();
  }

  return mojom::ConnectionState::DISCONNECTED;
}

void WootzVPNConnectionManager::ResetConnectionState() {
  if (connection_api_impl_) {
    connection_api_impl_->ResetConnectionState();
  }
}

void WootzVPNConnectionManager::Connect() {
  if (ScheduleConnectRequestIfNeeded()) {
    return;
  }

  if (connection_api_impl_) {
    connection_api_impl_->Connect();
  }
}

void WootzVPNConnectionManager::Disconnect() {
  if (connection_api_impl_) {
    connection_api_impl_->Disconnect();
  }
}

void WootzVPNConnectionManager::CheckConnection() {
  if (connection_api_impl_) {
    connection_api_impl_->CheckConnection();
  }
}

void WootzVPNConnectionManager::SetSelectedRegion(const std::string& name) {
  // TODO(simonhong): This method could be implemented here instead of impl
  // class.
  if (connection_api_impl_) {
    connection_api_impl_->SetSelectedRegion(name);
  }
}

std::string WootzVPNConnectionManager::GetHostname() const {
  if (connection_api_impl_) {
    return connection_api_impl_->GetHostname();
  }

  return {};
}

std::string WootzVPNConnectionManager::GetLastConnectionError() const {
  if (connection_api_impl_) {
    return connection_api_impl_->GetLastConnectionError();
  }

  return {};
}

void WootzVPNConnectionManager::ToggleConnection() {
  if (connection_api_impl_) {
    connection_api_impl_->ToggleConnection();
  }
}

std::string WootzVPNConnectionManager::GetCurrentEnvironment() const {
  return local_prefs_->GetString(prefs::kWootzVPNEnvironment);
}

void WootzVPNConnectionManager::MaybeInstallSystemServices() {
  if (!install_system_service_callback_) {
    VLOG(2) << __func__ << " : no install system service callback set";

    UpdateConnectionAPIImpl();
    return;
  }

  // Installation should only be called once per session.
  // It's safe to call more than once because the install itself checks if
  // the services are already registered before doing anything.
  if (system_service_installed_event_.is_signaled()) {
    VLOG(2)
        << __func__
        << " : installation has already been performed this session; exiting";
    return;
  }

  // This API could be called more than once because WootzVpnService is a
  // per-profile service. If service install is in-progress now, just return.
  if (install_in_progress_) {
    VLOG(2) << __func__ << " : install already in progress; exiting";
    return;
  }

#if BUILDFLAG(IS_WIN)
  install_in_progress_ = true;
  base::ThreadPool::CreateCOMSTATaskRunner({base::MayBlock()})
      ->PostTaskAndReplyWithResult(
          FROM_HERE, install_system_service_callback_,
          base::BindOnce(
              &WootzVPNConnectionManager::OnInstallSystemServicesCompleted,
              weak_factory_.GetWeakPtr()));
#endif
}

void WootzVPNConnectionManager::OnInstallSystemServicesCompleted(bool success) {
  VLOG(1) << "OnInstallSystemServicesCompleted: success=" << success;
  if (success) {
    system_service_installed_event_.Signal();
  } else {
    // On success, UpdateConnectionAPIImpl() will be called by prefs changing
    // notification.
    UpdateConnectionAPIImpl();
  }
  install_in_progress_ = false;
}

bool WootzVPNConnectionManager::ScheduleConnectRequestIfNeeded() {
  if (!install_in_progress_) {
    return false;
  }

  system_service_installed_event_.Post(
      FROM_HERE, base::BindOnce(&WootzVPNConnectionManager::Connect,
                                weak_factory_.GetWeakPtr()));
  return true;
}

void WootzVPNConnectionManager::NotifyConnectionStateChanged(
    mojom::ConnectionState state) const {
  for (auto& obs : observers_) {
    obs.OnConnectionStateChanged(state);
  }
}

}  // namespace wootz_vpn