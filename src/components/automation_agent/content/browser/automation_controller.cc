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
  LOG(INFO) << "Kartik: Starting GetPageState in browser controller";
  LOG(INFO) << "Kartik: Parameters - debug_mode=" << debug_mode 
            << ", include_hidden=" << include_hidden;
  
  get_page_state_callback_ = std::move(callback);
  if (automation_agent_) {
    LOG(INFO) << "Kartik: Automation agent interface available, forwarding request";
    automation_agent_->GetPageState(
        debug_mode, include_hidden,
        base::BindOnce([](GetPageStateCallback callback,
                         mojom::PageStateResultPtr result) {
          LOG(INFO) << "Kartik: Processing PageState result in callback";
          LOG(INFO) << "Kartik: Result success=" << result->success;
          
          if (!result->success) {
            LOG(ERROR) << "Kartik: GetPageState failed";
            std::move(callback).Run(false, "{}");
            return;
          }

          base::Value::Dict dict;
          dict.Set("success", result->success);
          dict.Set("error_message", result->error_message);
          
          LOG(INFO) << "Kartik: Processing page state map with " 
                    << result->page_state.size() << " entries";
          
          base::Value::Dict state_dict;
          for (const auto& pair : result->page_state) {
            LOG(INFO) << "Kartik: Adding state key=" << pair.first;
            state_dict.Set(pair.first, pair.second);
          }
          dict.Set("state", std::move(state_dict));

          std::string json;
          bool write_success = base::JSONWriter::Write(base::Value(std::move(dict)), &json);
          
          if (!write_success) {
            LOG(ERROR) << "Kartik: Failed to serialize JSON response";
            std::move(callback).Run(false, "{}");
            return;
          }
          
          LOG(INFO) << "Kartik: JSON conversion complete, size=" << json.length();
          LOG(INFO) << "Kartik: Running callback with result";
          std::move(callback).Run(result->success, json);
        },
        std::move(get_page_state_callback_)));
  } else {
    LOG(ERROR) << "Kartik: Automation agent interface not available";
    std::move(get_page_state_callback_).Run(false, "{}");
  }
}

void AutomationController::PerformAction(const std::string& action,
                                       const base::Value::Dict& action_params,
                                       PerformActionCallback callback) {
  LOG(INFO) << "Kartik: Starting PerformAction in browser controller";
  LOG(INFO) << "Kartik: Action type=" << action;
  LOG(INFO) << "Kartik: Action params size=" << action_params.size();
  
  perform_action_callback_ = std::move(callback);
  
  base::flat_map<std::string, std::string> mojo_params;
  if (const std::string* selector = action_params.FindString("selector")) {
    LOG(INFO) << "Kartik: Found selector param=" << *selector;
    mojo_params.insert({std::string("selector"), *selector});
  }
  if (const std::string* text = action_params.FindString("text")) {
    LOG(INFO) << "Kartik: Found text param, length=" << text->length();
    mojo_params.insert({std::string("text"), *text});
  }
  if (const std::string* direction = action_params.FindString("direction")) {
    LOG(INFO) << "Kartik: Found direction param=" << *direction;
    mojo_params.insert({std::string("direction"), *direction});
  }
  
  LOG(INFO) << "Kartik: Converted " << mojo_params.size() << " parameters for mojo";
  
  if (automation_agent_) {
    LOG(INFO) << "Kartik: Automation agent interface available, forwarding action";
    automation_agent_->PerformAction(
        action, std::move(mojo_params),
        base::BindOnce(&AutomationController::OnPerformActionComplete,
                      base::Unretained(this)));
  } else {
    LOG(ERROR) << "Kartik: Automation agent interface not available for action";
  }
}

void AutomationController::OnGetPageStateComplete(bool success,
                                                const std::string& state) {
  LOG(INFO) << "Kartik: GetPageState completion callback received";
  LOG(INFO) << "Kartik: Operation success=" << success;
  LOG(INFO) << "Kartik: State response size=" << state.length();

  if (get_page_state_callback_) {
    LOG(INFO) << "Kartik: Running GetPageState callback";
    std::move(get_page_state_callback_).Run(success, state);
  } else {
    LOG(ERROR) << "Kartik: No callback available for GetPageState completion";
  }
}

void AutomationController::OnPerformActionComplete(bool success) {
  LOG(INFO) << "Kartik: PerformAction completion callback received";
  LOG(INFO) << "Kartik: Action success=" << success;

  if (perform_action_callback_) {
    LOG(INFO) << "Kartik: Running PerformAction callback";
    std::move(perform_action_callback_).Run(success);
  } else {
    LOG(ERROR) << "Kartik: No callback available for PerformAction completion";
  }
}

}  // namespace automation