#include "chrome/browser/ui/webui/react/react_ui.h"

#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "chrome/grit/wootz_react_app_resources.h"
#include "chrome/grit/wootz_react_app_resources_map.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"


ReactUI::ReactUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIReactHost);

  // Add required resources.
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kWootzReactAppResources, kWootzReactAppResourcesSize),
      IDR_WOOTZ_REACT_APP_BUILD_INDEX_HTML);

  
  source->AddString("message", "Hello World!");
}

ReactUI::~ReactUI() = default;