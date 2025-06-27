#include "components/automation_agent/content/renderer/automation_agent.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace automation {

AutomationAgent::AutomationAgent(
    content::RenderFrame* render_frame,
    blink::AssociatedInterfaceRegistry* registry)
    : content::RenderFrameObserver(render_frame) {
  registry->AddInterface<mojom::AutomationAgent>(
      base::BindRepeating(&AutomationAgent::BindPendingReceiver,
                         base::Unretained(this)));
}

AutomationAgent::~AutomationAgent() = default;

void AutomationAgent::BindPendingReceiver(
    mojo::PendingAssociatedReceiver<mojom::AutomationAgent> pending_receiver) {
  receiver_.Bind(std::move(pending_receiver));
}

void AutomationAgent::GetPageState(bool debug_mode, bool include_hidden,
                                 GetPageStateCallback callback) {
  LOG(INFO) << "AutomationAgent:: renderer side; GetPageState called";
  LOG(INFO) << "AutomationAgent:: debug_mode: " << debug_mode;
  LOG(INFO) << "AutomationAgent:: include_hidden: " << include_hidden;

  // For now just send a dummy response
  std::move(callback).Run(true, "{ \"state\": \"dummy\" }");
}

void AutomationAgent::PerformAction(
    const std::string& action,
    const base::flat_map<std::string, std::string>& action_params,
    PerformActionCallback callback) {
  LOG(INFO) << "AutomationAgent:: renderer side; PerformAction called";
  LOG(INFO) << "AutomationAgent:: action: " << action;

  auto selector_it = action_params.find("selector");
  auto text_it = action_params.find("text");
  auto direction_it = action_params.find("direction");

  LOG(INFO) << "AutomationAgent:: selector: " 
            << (selector_it != action_params.end() ? selector_it->second : "null");
  LOG(INFO) << "AutomationAgent:: text: " 
            << (text_it != action_params.end() ? text_it->second : "null");
  LOG(INFO) << "AutomationAgent:: direction: " 
            << (direction_it != action_params.end() ? direction_it->second : "null");

  // For now just send success response
  std::move(callback).Run(true);
}

mojom::AutomationDriver& AutomationAgent::GetAutomationDriver() {
  if (!automation_driver_) {
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(
        &automation_driver_);
  }
  return *automation_driver_;
}

void AutomationAgent::OnDestruct() {
  delete this;
}

}  // namespace automation