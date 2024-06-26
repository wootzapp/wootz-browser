#include <string>

namespace wootz_vpn {

struct Hostname {
  std::string hostname;
  std::string display_name;
  bool is_offline;
  int capacity_score;
};

}  // namespace wootz_vpn