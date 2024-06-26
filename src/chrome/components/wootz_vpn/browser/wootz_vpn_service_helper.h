#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/values.h"
#include "wootz/components/wootz_vpn/common/mojom/wootz_vpn.mojom.h"

class PrefService;

namespace base {
class Time;
class Value;
}  // namespace base

namespace wootz_vpn {
mojom::RegionPtr GetRegionPtrWithNameFromRegionList(
    const std::string& name,
    const std::vector<mojom::Region> region_list);
}  // namespace wootz_vpn
