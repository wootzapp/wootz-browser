#include <memory>
#include <string>
#include <vector>

#include "base/values.h"

namespace base {
class Value;
}  // namespace base

namespace wootz_vpn {

struct Hostname;

std::unique_ptr<Hostname> PickBestHostname(
    const std::vector<Hostname>& hostnames);
std::vector<Hostname> ParseHostnames(const base::Value::List& hostnames);
std::string GetTimeZoneName();
base::Value::Dict GetValueWithTicketInfos(
    const std::string& email,
    const std::string& subject,
    const std::string& body,
    const std::string& timezone);

}  // namespace wootz_vpn