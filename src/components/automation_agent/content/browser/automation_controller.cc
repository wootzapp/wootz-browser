#include "components/automation_agent/content/browser/automation_controller.h"
#include "content/public/browser/render_frame_host.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace automation {

AutomationController::AutomationController(
    content::RenderFrameHost* render_frame_host)
    : render_frame_host_(render_frame_host) {
  render_frame_host_->GetRemoteAssociatedInterfaces()->GetInterface(
      &automation_agent_);
}

AutomationController::~AutomationController() = default;

void AutomationController::GetPageState(bool debug_mode, 
                                      bool include_hidden,
                                      GetPageStateCallback callback) {
  LOG(INFO) << "AutomationController:: browser side; Calling GetPageState";
  LOG(INFO) << "AutomationController:: debug_mode: " << debug_mode;
  LOG(INFO) << "AutomationController:: include_hidden: " << include_hidden;
  
  get_page_state_callback_ = std::move(callback);
  if (automation_agent_) {
    automation_agent_->GetPageState(
        debug_mode, include_hidden,
        base::BindOnce(&AutomationController::OnGetPageStateComplete,
                      base::Unretained(this)));
  }
}

void AutomationController::PerformAction(const std::string& action,
                                       const base::Value::Dict& action_params,
                                       PerformActionCallback callback) {
  LOG(INFO) << "AutomationController:: browser side; Calling PerformAction";
  LOG(INFO) << "AutomationController:: action: " << action;
  
  perform_action_callback_ = std::move(callback);
  
  base::flat_map<std::string, std::string> mojo_params;
  if (const std::string* selector = action_params.FindString("selector"))
    mojo_params.insert({std::string("selector"), *selector});
  if (const std::string* text = action_params.FindString("text"))
    mojo_params.insert({std::string("text"), *text});
  if (const std::string* direction = action_params.FindString("direction"))
    mojo_params.insert({std::string("direction"), *direction});
  
  if (automation_agent_) {
    automation_agent_->PerformAction(
        action, std::move(mojo_params),
        base::BindOnce(&AutomationController::OnPerformActionComplete,
                      base::Unretained(this)));
  }
}

void AutomationController::OnGetPageStateComplete(bool success,
                                                const std::string& state) {
  LOG(INFO) << "AutomationController:: browser side; Got GetPageState response";
  LOG(INFO) << "AutomationController:: success: " << success;
  LOG(INFO) << "AutomationController:: state: " << state;

  if (get_page_state_callback_) {
    std::move(get_page_state_callback_).Run(success, state);
  }
}

void AutomationController::OnPerformActionComplete(bool success) {
  LOG(INFO) << "AutomationController:: browser side; Got PerformAction response";
  LOG(INFO) << "AutomationController:: success: " << success;

  if (perform_action_callback_) {
    std::move(perform_action_callback_).Run(success);
  }
}

}  // namespace automation