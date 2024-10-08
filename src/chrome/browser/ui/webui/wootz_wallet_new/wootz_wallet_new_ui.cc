#include "chrome/browser/ui/webui/wootz_wallet_new/wootz_wallet_new_ui.h"

#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "chrome/grit/wootz_wallet_new_page_resources.h"
#include "chrome/grit/wootz_wallet_new_page_resources_map.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"


WootzWalletNewUI::WootzWalletNewUI(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  // Set up the chrome://hello-world source.
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIWootzWalletNewHost);

  // Add required resources.
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kWootzWalletNewPageResources, kWootzWalletNewPageResourcesSize),
      IDR_WOOTZ_WALLET_NEW_PAGE);

  // As a demonstration of passing a variable for JS to use we pass in some
  // a simple message.
  source->AddString("message", "Hello World!");
}

WootzWalletNewUI::~WootzWalletNewUI() = default;