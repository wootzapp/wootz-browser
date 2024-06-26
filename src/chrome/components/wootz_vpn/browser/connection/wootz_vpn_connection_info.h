#include <string>

namespace wootz_vpn {

class WootzVPNConnectionInfo {
 public:
  WootzVPNConnectionInfo();
  ~WootzVPNConnectionInfo();
  WootzVPNConnectionInfo(const WootzVPNConnectionInfo& info);
  WootzVPNConnectionInfo& operator=(const WootzVPNConnectionInfo& info);

  void Reset();
  bool IsValid() const;
  void SetConnectionInfo(const std::string& connection_name,
                         const std::string& hostname,
                         const std::string& username,
                         const std::string& password);

  std::string connection_name() const { return connection_name_; }
  std::string hostname() const { return hostname_; }
  std::string username() const { return username_; }
  std::string password() const { return password_; }

 private:
  std::string connection_name_;
  std::string hostname_;
  std::string username_;
  std::string password_;
};

}  // namespace wootz_vpn