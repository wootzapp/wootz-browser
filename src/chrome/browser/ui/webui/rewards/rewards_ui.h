#include "content/public/browser/web_ui_controller.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace network::mojom {
class NetworkContext;
}

class RewardsUI : public content::WebUIController {
 public:
  explicit RewardsUI(content::WebUI* web_ui);

  RewardsUI(const RewardsUI&) = delete;
  RewardsUI& operator=(const RewardsUI&) = delete;
};
