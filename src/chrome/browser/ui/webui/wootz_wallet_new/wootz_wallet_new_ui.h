#ifndef CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_NEW_WOOTZ_WALLET_NEW_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_NEW_WOOTZ_WALLET_NEW_UI_H_

#include "content/public/browser/web_ui_controller.h"

// The WebUI for chrome://hello-world
class WootzWalletNewUI : public content::WebUIController {
 public:
  explicit WootzWalletNewUI(content::WebUI* web_ui);
  ~WootzWalletNewUI() override;
};

#endif // CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_NEW_WOOTZ_WALLET_NEW_UI_H_