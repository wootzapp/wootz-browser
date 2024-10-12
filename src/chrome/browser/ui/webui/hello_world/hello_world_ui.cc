#include "chrome/browser/ui/webui/hello_world/hello_world_ui.h"

#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "chrome/grit/hello_world_resources.h"
#include "chrome/grit/hello_world_resources_map.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"



HelloWorldUI::HelloWorldUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  // Set up the chrome://hello-world source.
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIHelloWorldHost);

  // Add required resources.
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kHelloWorldResources, kHelloWorldResourcesSize),
      IDR_HELLO_WORLD_HELLO_WORLD_HTML);

  // As a demonstration of passing a variable for JS to use we pass in some
  // a simple message.
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc,
      std::string("connect-src https://api-staging-0.gotartifact.com/v2/users/authentication/signin 'self';"));
  source->OverrideCrossOriginResourcePolicy("cross-origin");

  source->AddString("message", "Hello World!");
}

HelloWorldUI::~HelloWorldUI() = default;