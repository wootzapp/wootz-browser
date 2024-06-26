#include "chrome/components/wootz_vpn/common/features.h"

#include "base/feature_list.h"
#include "build/build_config.h"

namespace wootz_vpn {

namespace features {

BASE_FEATURE(kWootzVPN,
             "WootzVPN",
#if !BUILDFLAG(IS_LINUX)
             base::FEATURE_ENABLED_BY_DEFAULT
#else
             base::FEATURE_DISABLED_BY_DEFAULT
#endif
);


}  // namespace wootz_vpn
