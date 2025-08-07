#ifndef COMPONENTS_AUTOMATION_AGENT_CONTENT_BROWSER_AUTOMATION_CONTROLLER_FACTORY_H_
#define COMPONENTS_AUTOMATION_AGENT_CONTENT_BROWSER_AUTOMATION_CONTROLLER_FACTORY_H_

#include <map>

#include "base/memory/raw_ptr.h"
#include "components/automation_agent/content/common/mojom/automation_driver.mojom.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace content {
class WebContents;
}

namespace automation {

class AutomationController;

// Creates and owns AutomationController. There is one factory per WebContents,
// and one controller per RenderFrameHost.
class AutomationControllerFactory
    : public content::WebContentsObserver,
      public content::WebContentsUserData<AutomationControllerFactory> {
 public:
  AutomationControllerFactory(const AutomationControllerFactory&) = delete;
  AutomationControllerFactory& operator=(const AutomationControllerFactory&) = delete;

  ~AutomationControllerFactory() override;

  static void BindAutomationDriver(
      mojo::PendingAssociatedReceiver<mojom::AutomationDriver> pending_receiver,
      content::RenderFrameHost* render_frame_host);

  // Note that this may return null if the RenderFrameHost does not have a
  // live RenderFrame (e.g. it represents a crashed RenderFrameHost).
  AutomationController* GetDriverForFrame(
      content::RenderFrameHost* render_frame_host);

  static void CreateForWebContents(content::WebContents* web_contents) {
    DCHECK(web_contents);
    if (!FromWebContents(web_contents)) {
      web_contents->SetUserData(
          UserDataKey(),
          base::WrapUnique(new AutomationControllerFactory(web_contents)));
    }
  }

 private:
  friend class content::WebContentsUserData<AutomationControllerFactory>;

  explicit AutomationControllerFactory(content::WebContents* web_contents);

  // content::WebContentsObserver:
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void WebContentsDestroyed() override;

  std::map<content::RenderFrameHost*, std::unique_ptr<AutomationController>> frame_driver_map_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace automation

#endif  // COMPONENTS_AUTOMATION_AGENT_CONTENT_BROWSER_AUTOMATION_CONTROLLER_FACTORY_H_