#include "content/public/browser/web_ui_controller.h"

namespace content {
class BrowserContext;
}  // namespace content

// The  back-end for the wootzapp://reward webui page.
class HelloWorldUI : public content::WebUIController {
 public:
  explicit HelloWorldUI(content::WebUI* web_ui);

  HelloWorldUI(const HelloWorldUI&) = delete;
  HelloWorldUI& operator=(const HelloWorldUI&) = delete;
};