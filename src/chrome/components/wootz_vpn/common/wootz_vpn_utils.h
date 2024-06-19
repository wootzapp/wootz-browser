#include <string>

#include "build/build_config.h"

class PrefRegistrySimple;
class PrefService;
namespace user_prefs {
class PrefRegistrySyncable;
}  // namespace user_prefs

namespace version_info {
enum class Channel;
}  // namespace version_info

namespace wootz_vpn {

std::string GetWootzVPNEntryName(version_info::Channel channel);
bool IsWootzVPNEnabled(PrefService* prefs);
bool IsWootzVPNFeatureEnabled();
bool IsWootzVPNDisabledByPolicy(PrefService* prefs);
std::string GetWootzVPNPaymentsEnv(const std::string& env);
std::string GetManageUrl(const std::string& env);
void MigrateVPNSettings(PrefService* profile_prefs, PrefService* local_prefs);
void RegisterLocalStatePrefs(PrefRegistrySimple* registry);
void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);
void RegisterAndroidProfilePrefs(PrefRegistrySimple* registry);
bool HasValidSkusCredential(PrefService* local_prefs);
std::string GetSkusCredential(PrefService* local_prefs);
}  // namespace wootz_vpn
