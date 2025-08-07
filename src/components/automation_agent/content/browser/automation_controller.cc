#include "components/automation_agent/content/browser/automation_controller.h"
#include "content/public/browser/render_frame_host.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace automation {

using GetPageStateCallback = base::OnceCallback<void(bool, const std::string&)>;

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
  
  get_page_state_callback_ = std::move(callback);
  if (automation_agent_) {
    automation_agent_->GetPageState(
        debug_mode, include_hidden,
        base::BindOnce([](GetPageStateCallback callback,
                         mojom::PageStateResultPtr result) {
          
          if (!result->success) {
            LOG(ERROR) << "GetPageState failed";
            std::move(callback).Run(false, "{}");
            return;
          }

          base::Value::Dict dict;
          dict.Set("success", result->success);
          dict.Set("error_message", result->error_message);
          
          base::Value::Dict state_dict;
          for (const auto& pair : result->page_state) {
            state_dict.Set(pair.first, pair.second);
          }
          dict.Set("state", std::move(state_dict));

          std::string json;
          bool write_success = base::JSONWriter::Write(base::Value(std::move(dict)), &json);
          
          if (!write_success) {
            std::move(callback).Run(false, "{}");
            return;
          }
          std::move(callback).Run(result->success, json);
        },
        std::move(get_page_state_callback_)));
  } else {
    LOG(ERROR) << "Automation agent interface not available";
    std::move(get_page_state_callback_).Run(false, "{}");
  }
}

void AutomationController::PerformAction(const std::string& action,
                                       const base::Value::Dict& action_params,
                                       PerformActionCallback callback) {
  
  perform_action_callback_ = std::move(callback);
  
  base::flat_map<std::string, std::string> mojo_params;
  if (const std::string* selector = action_params.FindString("selector")) {
    mojo_params.insert({std::string("selector"), *selector});
  }
  if (auto index = action_params.FindInt("index")) {  // Handle integer
    mojo_params.insert({std::string("index"), std::to_string(index.value())});  // Convert to string
  }
  if (const std::string* text = action_params.FindString("text")) {
    mojo_params.insert({std::string("text"), *text});
  }
  if (const std::string* direction = action_params.FindString("direction")) {
    mojo_params.insert({std::string("direction"), *direction});
  }
  if (const std::string* amount = action_params.FindString("amount")) {
    mojo_params.insert({std::string("amount"), *amount});
  }
  
  if (automation_agent_) {
    automation_agent_->PerformAction(
        action, std::move(mojo_params),
        base::BindOnce(&AutomationController::OnPerformActionComplete,
                      base::Unretained(this)));
  } else {
    std::move(perform_action_callback_).Run(false);
  }
}

void AutomationController::OnGetPageStateComplete(bool success,
                                                const std::string& state) {

  if (get_page_state_callback_) {
    std::move(get_page_state_callback_).Run(success, state);
  } else {
    LOG(ERROR) << "No callback available for GetPageState completion";
  }
}

void AutomationController::OnPerformActionComplete(bool success) {

  if (perform_action_callback_) {
    std::move(perform_action_callback_).Run(success);
  } else {
    LOG(ERROR) << "No callback available for PerformAction completion";
  }
}

}  // namespace automation