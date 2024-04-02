#include "chrome/browser/ui/webui/donuts/donuts_ui.h"

#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"

DonutsUI::DonutsUI(content::WebUI* web_ui)
    : ui::MojoWebUIController(web_ui, true) {
  // Normal constructor steps (e.g. setting up data source) go here.
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(), "donuts");
  source->AddString("mmmDonuts", "Mmm, donuts!");  // Translations.
  //  source->AddResourcePath("", IDR_DONUTS_HTML);  // Home page.
}

WEB_UI_CONTROLLER_TYPE_IMPL(DonutsUI)

DonutsUI::~DonutsUI() = default;

void DonutsUI::BindInterface(
    mojo::PendingReceiver<donuts::mojom::PageHandlerFactory> receiver) {
  page_factory_receiver_.reset();
  page_factory_receiver_.Bind(std::move(receiver));
}

void DonutsUI::CreatePageHandler(
    mojo::PendingRemote<donuts::mojom::Page> page,
    mojo::PendingReceiver<donuts::mojom::PageHandler> receiver) {
  DCHECK(page);
  page_handler_ =
      std::make_unique<DonutsPageHandler>(std::move(receiver), std::move(page));
}
