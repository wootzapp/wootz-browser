
#include "content/public/browser/web_ui_controller.h"

class ReactUI : public content::WebUIController {
 public:
  explicit ReactUI(content::WebUI* web_ui);
  ~ReactUI() override;
};

