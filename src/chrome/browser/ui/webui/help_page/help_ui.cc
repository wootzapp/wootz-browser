#include "chrome/browser/ui/webui/help_page/help_ui.h"

#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "chrome/grit/help_resources.h"
#include "chrome/grit/help_resources_map.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"

// #include "services/network/public/mojom/content_security_policy.mojom.h"


HelpUI::HelpUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  // Set up the chrome://help-page source.
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIHelpPageHost);

  // Add required resources.
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kHelpResources, kHelpResourcesSize),
      IDR_HELP_HELP_HTML);
  
//   source->OverrideContentSecurityPolicy(
//     network::mojom::CSPDirectiveName::ConnectSrc,
//     std::string("connect-src https://api-staging-0.gotartifact.com/v2/users/authentication/signin 'self';"));
//   source->OverrideCrossOriginResourcePolicy("cross-origin");

  // As a demonstration of passing a variable for JS to use we pass in some
  // a simple message.
  source->AddString("message", "Koushik!");
}

HelpUI::~HelpUI() = default;