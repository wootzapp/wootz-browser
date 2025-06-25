#ifndef COMPONENTS_AUTOMATION_AGENT_BROWSER_AUTOMATION_CONTROLLER_H_
#define COMPONENTS_AUTOMATION_AGENT_BROWSER_AUTOMATION_CONTROLLER_H_

#include <string>
#include "base/component_export.h"
#include "base/functional/callback.h"
#include "base/values.h"

namespace automation_agent {

class COMPONENT_EXPORT(AUTOMATION_AGENT) AutomationController {
 public:
  AutomationController();
  ~AutomationController();

  // Prevent copying
  AutomationController(const AutomationController&) = delete;
  AutomationController& operator=(const AutomationController&) = delete;

  // Gets the current page state based on provided options
  void GetPageState(bool debug_mode, 
                   bool include_hidden,
                   base::OnceCallback<void(base::Value::Dict)> callback);

  // Performs an automation action with the given parameters
  void PerformAction(const std::string& action,
                    const std::string& selector,
                    const std::string& text,
                    const std::string& direction,
                    base::OnceCallback<void(bool)> callback);

 private:
  // Add any private members if needed
};

}  // namespace automation_agent

#endif  // COMPONENTS_AUTOMATION_AGENT_BROWSER_AUTOMATION_CONTROLLER_H_