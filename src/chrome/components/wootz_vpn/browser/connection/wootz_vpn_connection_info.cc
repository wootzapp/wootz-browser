#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_connection_info.h"

namespace wootz_vpn {

WootzVPNConnectionInfo::WootzVPNConnectionInfo() = default;
WootzVPNConnectionInfo::~WootzVPNConnectionInfo() = default;

WootzVPNConnectionInfo::WootzVPNConnectionInfo(
    const WootzVPNConnectionInfo& info) = default;
WootzVPNConnectionInfo& WootzVPNConnectionInfo::operator=(
    const WootzVPNConnectionInfo& info) = default;

void WootzVPNConnectionInfo::Reset() {
  connection_name_.clear();
  hostname_.clear();
  username_.clear();
  password_.clear();
}

bool WootzVPNConnectionInfo::IsValid() const {
  return !hostname_.empty() && !username_.empty() && !password_.empty();
}

void WootzVPNConnectionInfo::SetConnectionInfo(
    const std::string& connection_name,
    const std::string& hostname,
    const std::string& username,
    const std::string& password) {
  connection_name_ = connection_name;
  hostname_ = hostname;
  username_ = username;
  password_ = password;
}

}  // namespace wootz_vpn