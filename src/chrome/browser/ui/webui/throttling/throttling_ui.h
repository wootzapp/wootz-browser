#include "base/memory/raw_ptr.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "chrome/browser/ui/webui/throttling/throttling.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "ui/webui/mojo_web_ui_controller.h"
#include "chrome/browser/profiles/profile.h"

class Profile;
class ThrottlingPageHandler;

// The WebUI controller for chrome://throttling
class ThrottlingUI
    : public ui::MojoWebUIController,
      public throttling::mojom::PageHandlerFactory {
 public:
  explicit ThrottlingUI(content::WebUI* web_ui);
  ~ThrottlingUI() override;

  ThrottlingUI(const ThrottlingUI&) = delete;
  ThrottlingUI& operator=(const ThrottlingUI&) = delete;

  void BindInterface(
      mojo::PendingReceiver<throttling::mojom::PageHandlerFactory>
          receiver);

 private:
  
   void CreatePageHandler(
      mojo::PendingRemote<throttling::mojom::Page> page,
      mojo::PendingReceiver<throttling::mojom::PageHandler>
          receiver) override;

  raw_ptr<Profile> profile_;
  std::unique_ptr<ThrottlingPageHandler>
      throttling_page_handler_;
  mojo::Receiver<throttling::mojom::PageHandlerFactory>
      throttling_page_factory_receiver_{this};    

  WEB_UI_CONTROLLER_TYPE_DECL();
};

