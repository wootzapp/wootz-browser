#include <string>

class PrefRegistrySimple;
class PrefService;

namespace skus {

inline constexpr char kEnvProduction[] = "production";
inline constexpr char kEnvStaging[] = "staging";
inline constexpr char kEnvDevelopment[] = "development";

std::string GetDefaultEnvironment();
std::string GetDomain(const std::string& prefix, const std::string& domain);
std::string GetEnvironmentForDomain(const std::string& domain);
bool DomainIsForProduct(const std::string& domain, const std::string& product);
void RegisterLocalStatePrefs(PrefRegistrySimple* registry);
void RegisterProfilePrefsForMigration(PrefRegistrySimple* registry);
void MigrateSkusSettings(PrefService* profile_prefs, PrefService* local_prefs);
}  // namespace skus