#include "chrome/components/wootz_vpn/common/buildflags/buildflags.h"
#include "build/build_config.h"

namespace wootz_vpn {
namespace prefs {
inline constexpr char kManagedWootzVPNDisabled[] =
    "wootz.wootz_vpn.disabled_by_policy";
inline constexpr char kWootzVPNLocalStateMigrated[] =
    "wootz.wootz_vpn.migrated";
inline constexpr char kWootzVPNRootPref[] = "wootz.wootz_vpn";
inline constexpr char kWootzVPNShowButton[] = "wootz.wootz_vpn.show_button";
inline constexpr char kWootzVPNRegionList[] = "wootz.wootz_vpn.region_list";
// Cached fetched date for trying to refresh region_list once per day
inline constexpr char kWootzVPNRegionListFetchedDate[] =
    "wootz.wootz_vpn.region_list_fetched_date";
inline constexpr char kWootzVPNDeviceRegion[] =
    "wootz.wootz_vpn.device_region_name";
inline constexpr char kWootzVPNSelectedRegion[] =
    "wootz.wootz_vpn.selected_region_name";

// inline constexpr char kWootzVPNWireguardProfileCredentials[] =
//     "wootz.wootz_vpn.wireguard.profile_credentials";
inline constexpr char kWootzVPNEnvironment[] = "wootz.wootz_vpn.env";

// Time that session expired occurs.
inline constexpr char kWootzVPNSessionExpiredDate[] =
    "wootz.wootz_vpn.session_expired_date";

#if BUILDFLAG(IS_ANDROID)
inline constexpr char kWootzVPNPackageAndroid[] =
    "wootz.wootz_vpn.package_android";
inline constexpr char kWootzVPNProductIdAndroid[] =
    "wootz.wootz_vpn.product_id_android";
#endif

inline constexpr char kWootzVPNFirstUseTime[] =
    "wootz.wootz_vpn.first_use_time";
inline constexpr char kWootzVPNLastUseTime[] = "wootz.wootz_vpn.last_use_time";
inline constexpr char kWootzVPNUsedSecondDay[] =
    "wootz.wootz_vpn.used_second_day";
inline constexpr char kWootzVPNDaysInMonthUsed[] =
    "wootz.wootz_vpn.days_in_month_used";
}  // namespace prefs

}  // namespace wootz_vpn
