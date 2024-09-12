#ifndef CHROME_BROWSER_UI_WEBUI_HELLO_WORLD_HELLO_WORLD_H_
#define CHROME_BROWSER_UI_WEBUI_HELLO_WORLD_HELLO_WORLD_H_

#include "content/public/browser/web_ui_controller.h"

// The WebUI for chrome://hello-world
class HelloWorldUI : public content::WebUIController {
 public:
  explicit HelloWorldUI(content::WebUI* web_ui);
  ~HelloWorldUI() override;
};

#endif // CHROME_BROWSER_UI_WEBUI_HELLO_WORLD_HELLO_WORLD_H_