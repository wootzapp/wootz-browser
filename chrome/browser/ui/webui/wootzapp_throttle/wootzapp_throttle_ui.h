#ifndef CHROME_BROWSER_UI_WEBUI_WOOTZAPP_THROTTLE_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WOOTZAPP_THROTTLE_UI_H_

#include "content/public/browser/web_ui_controller.h"

namespace network::mojom {
class NetworkContext;
}

// The  back-end for the wootzapp://throttle webui page.
class WootzappThrottleUI : public content::WebUIController {
 public:
  explicit WootzappThrottleUI(content::WebUI* web_ui);

  WootzappThrottleUI(const WootzappThrottleUI&) = delete;
  WootzappThrottleUI& operator=(const WootzappThrottleUI&) = delete;
};

#endif  // CHROME_BROWSER_UI_WEBUI_WOOTZAPP_THROTTLE_UI_H_