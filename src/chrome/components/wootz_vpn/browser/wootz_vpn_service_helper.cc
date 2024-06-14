#include "wootz/components/wootz_vpn/browser/wootz_vpn_service_helper.h"

#include <algorithm>
#include <optional>
#include <utility>

#include "base/base64.h"
#include "base/json/values_util.h"
#include "base/notreached.h"
#include "base/ranges/algorithm.h"
#include "base/time/time.h"
#include "base/values.h"
#include "wootz/components/wootz_vpn/common/wootz_vpn_constants.h"
#include "wootz/components/wootz_vpn/common/wootz_vpn_data_types.h"
#include "wootz/components/wootz_vpn/common/wootz_vpn_utils.h"
#include "wootz/components/wootz_vpn/common/pref_names.h"
#include "wootz/components/skus/browser/skus_utils.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"

namespace wootz_vpn {

mojom::RegionPtr GetRegionPtrWithNameFromRegionList(
    const std::string& name,
    const std::vector<mojom::Region> region_list) {
  auto it = base::ranges::find(region_list, name, &mojom::Region::name);
  if (it != region_list.end())
    return it->Clone();
  return mojom::RegionPtr();
}

}  // namespace wootz_vpn
