#include <memory>

#include "base/memory/scoped_refptr.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

class PrefService;

namespace wootz_vpn {

class WootzVPNConnectionManager;

bool IsWootzVPNEnabled(content::BrowserContext* context);
bool IsAllowedForContext(content::BrowserContext* context);
std::unique_ptr<WootzVPNConnectionManager> CreateWootzVPNConnectionManager(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    PrefService* local_prefs);

}  // namespace wootz_vpn
