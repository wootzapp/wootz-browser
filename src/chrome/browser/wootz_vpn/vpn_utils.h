#include <memory>

#include "base/memory/scoped_refptr.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

class PrefService;

namespace brave_vpn {

class BraveVPNConnectionManager;

bool IsBraveVPNEnabled(content::BrowserContext* context);
bool IsAllowedForContext(content::BrowserContext* context);
std::unique_ptr<BraveVPNConnectionManager> CreateBraveVPNConnectionManager(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    PrefService* local_prefs);

}  // namespace brave_vpn