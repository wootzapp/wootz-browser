#ifndef CHROME_BROWSER_UI_WEBUI_THROTTLE_UI_H_
#define CHROME_BROWSER_UI_WEBUI_THROTTLE_UI_H_

#include "content/public/browser/web_ui_controller.h"

namespace network::mojom {
class NetworkContext;
}

// The  back-end for the chrome://throttle webui page.
class ThrottleUI : public content::WebUIController {
 public:
  explicit ThrottleUI(content::WebUI* web_ui);

  ThrottleUI(const ThrottleUI&) = delete;
  ThrottleUI& operator=(const ThrottleUI&) = delete;
};

#endif  // CHROME_BROWSER_UI_WEBUI_THROTTLE_UI_H_