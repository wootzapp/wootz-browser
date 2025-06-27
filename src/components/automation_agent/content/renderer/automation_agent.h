#ifndef COMPONENTS_AUTOMATION_AGENT_CONTENT_RENDERER_AUTOMATION_AGENT_H_
#define COMPONENTS_AUTOMATION_AGENT_CONTENT_RENDERER_AUTOMATION_AGENT_H_

#include "components/automation_agent/content/common/mojom/automation_agent.mojom.h"
#include "components/automation_agent/content/common/mojom/automation_driver.mojom.h"
#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace automation {

class AutomationAgent : public content::RenderFrameObserver,
                       public mojom::AutomationAgent {
 public:
  AutomationAgent(content::RenderFrame* render_frame,
                 blink::AssociatedInterfaceRegistry* registry);
  ~AutomationAgent() override;

  void BindPendingReceiver(
      mojo::PendingAssociatedReceiver<mojom::AutomationAgent> pending_receiver);

  // mojom::AutomationAgent:
  void GetPageState(bool debug_mode, 
                   bool include_hidden,
                   GetPageStateCallback callback) override;
  
  void PerformAction(const std::string& action,
                    const base::flat_map<std::string, std::string>& action_params,
                    PerformActionCallback callback) override;

 protected:
  // content::RenderFrameObserver:
  void OnDestruct() override;

 private:
  mojom::AutomationDriver& GetAutomationDriver();

  mojo::AssociatedReceiver<mojom::AutomationAgent> receiver_{this};
  mojo::AssociatedRemote<mojom::AutomationDriver> automation_driver_;
};

}  // namespace automation

#endif  // COMPONENTS_AUTOMATION_AGENT_CONTENT_RENDERER_AUTOMATION_AGENT_H_