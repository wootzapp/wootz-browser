#ifndef CHROME_BROWSER_UI_WEBUI_DONUTS_DONUTS_H_
#define CHROME_BROWSER_UI_WEBUI_DONUTS_DONUTS_H_

#include "chrome/browser/ui/webui/donuts/donuts.mojom.h"
#include "chrome/browser/ui/webui/donuts/donuts_page_handler.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/webui_config.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"

#include "ui/webui/mojo_web_ui_controller.h"

class DonutsUI : public ui::MojoWebUIController,
                 public donuts::mojom::PageHandlerFactory {
 public:
  explicit DonutsUI(content::WebUI* web_ui);

  DonutsUI(const DonutsUI&) = delete;
  DonutsUI& operator=(const DonutsUI&) = delete;

  ~DonutsUI() override;

  // Instantiates the implementor of the mojom::PageHandlerFactory mojo
  // interface passing the pending receiver that will be internally bound.
  void BindInterface(
      mojo::PendingReceiver<donuts::mojom::PageHandlerFactory> receiver);

 private:
  // donuts::mojom::PageHandlerFactory:
  void CreatePageHandler(
      mojo::PendingRemote<donuts::mojom::Page> page,
      mojo::PendingReceiver<donuts::mojom::PageHandler> receiver) override;

  std::unique_ptr<DonutsPageHandler> page_handler_;

  mojo::Receiver<donuts::mojom::PageHandlerFactory> page_factory_receiver_{
      this};

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // CHROME_BROWSER_UI_WEBUI_DONUTS_DONUTS_H_
