#ifndef COMPONENTS_AUTOMATION_AGENT_CONTENT_BROWSER_AUTOMATION_CONTROLLER_H_
#define COMPONENTS_AUTOMATION_AGENT_CONTENT_BROWSER_AUTOMATION_CONTROLLER_H_

#include "base/memory/raw_ptr.h"
#include "components/automation_agent/content/common/mojom/automation_agent.mojom.h"
#include "components/automation_agent/content/common/mojom/automation_driver.mojom.h"
#include "content/public/browser/render_frame_host.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "base/json/json_writer.h"

namespace automation {

class AutomationController : public mojom::AutomationDriver {
 public:
  using GetPageStateCallback = base::OnceCallback<void(bool, const std::string&)>;
  using PerformActionCallback = base::OnceCallback<void(bool)>;

  AutomationController(content::RenderFrameHost* render_frame_host);
  ~AutomationController() override;

  // mojom::AutomationDriver:
  void OnGetPageStateComplete(bool success, const std::string& state) override;
  void OnPerformActionComplete(bool success) override;

  void GetPageState(bool debug_mode, bool include_hidden, GetPageStateCallback callback);
  void PerformAction(const std::string& action,
                    const base::Value::Dict& action_params,
                    PerformActionCallback callback);

  void BindPendingReceiver(
      mojo::PendingAssociatedReceiver<mojom::AutomationDriver> pending_receiver) {
    receiver_.Bind(std::move(pending_receiver));
  }

 private:
  const raw_ptr<content::RenderFrameHost> render_frame_host_;
  mojo::AssociatedRemote<mojom::AutomationAgent> automation_agent_;
  mojo::AssociatedReceiver<mojom::AutomationDriver> receiver_{this};

  GetPageStateCallback get_page_state_callback_;
  PerformActionCallback perform_action_callback_;
};

}  // namespace automation

#endif  // COMPONENTS_AUTOMATION_AGENT_CONTENT_BROWSER_AUTOMATION_CONTROLLER_H_