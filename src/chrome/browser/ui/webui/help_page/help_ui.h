#ifndef CHROME_BROWSER_UI_WEBUI_HELP_PAGE_HELP_H_
#define CHROME_BROWSER_UI_WEBUI_HELP_PAGE_HELP_H_

#include "content/public/browser/web_ui_controller.h"

// The WebUI for chrome://help-page
class HelpUI : public content::WebUIController {
 public:
  explicit HelpUI(content::WebUI* web_ui);
  ~HelpUI() override;
};

#endif // CCHROME_BROWSER_UI_WEBUI_HELP_PAGE_HELP_H_
