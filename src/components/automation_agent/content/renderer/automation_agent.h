#ifndef COMPONENTS_AUTOMATION_AGENT_CONTENT_RENDERER_AUTOMATION_AGENT_H_
#define COMPONENTS_AUTOMATION_AGENT_CONTENT_RENDERER_AUTOMATION_AGENT_H_

#include <string>
#include <vector>
#include <set>

#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "base/json/json_writer.h"
#include "base/json/json_reader.h"
#include "components/automation_agent/content/common/mojom/automation_agent.mojom.h"
#include "components/automation_agent/content/common/mojom/automation_driver.mojom.h"
#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_node.h"
#include "third_party/blink/public/web/web_view.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/platform/web_vector.h"
#include "third_party/blink/public/web/web_element_collection.h"
#include "third_party/blink/public/web/web_frame_widget.h"
#include "third_party/blink/public/web/web_range.h"
#include "third_party/blink/renderer/core/style/computed_style.h"
#include "ui/gfx/geometry/rect.h"
#include "v8/include/v8.h"
#include "third_party/blink/renderer/core/dom/element_traversal.h"
#include "third_party/blink/renderer/platform/wtf/text/atomic_string.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/element.h"

namespace blink {
class WebLocalFrame;
class WebElement;
}  // namespace blink

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
                    const base::flat_map<std::string, std::string>& params,
                    PerformActionCallback callback) override;

 protected:
  // content::RenderFrameObserver:
  void OnDestruct() override;

 private:
  // Core element analysis methods
  bool IsElementVisible(const blink::WebElement& element);
  bool IsElementInteractive(const blink::WebElement& element);
  bool IsElementInViewport(const blink::WebElement& element);
  
  bool IsElementDistinctInteraction(const blink::WebElement& element);
  bool IsAncestorHighlighted(const blink::WebElement& element, 
                           const std::set<blink::WebElement>& highlighted_elements);
  void CleanupPreviousHighlights(blink::WebDocument& document);
  void InjectIndexedHighlightCSS(blink::WebDocument& document, 
    std::vector<std::pair<blink::WebElement, int>>& indexed_elements);
  
  std::string GetElementXPath(const blink::WebElement& element);
  std::string GenerateElementSelector(const blink::WebElement& element);
  base::Value::Dict AnalyzePageContext(const blink::WebDocument& document, blink::WebLocalFrame* frame);
  base::Value::Dict AnalyzeViewport(blink::WebLocalFrame* frame);
  std::string CategorizeElementForAI(const blink::WebElement& element);
  std::string GetElementPurpose(const blink::WebElement& element);
  base::Value::Dict AnalyzePageCapabilities(const blink::WebDocument& document);
  void InjectElementIndexes(blink::WebDocument& document, 
    std::vector<std::pair<blink::WebElement, int>>& indexed_elements);
  void InjectVisualHighlightCSS(blink::WebDocument& document, 
    std::vector<std::pair<blink::WebElement, int>>& indexed_elements);
  
  // Helper to get automation driver interface
  mojom::AutomationDriver& GetAutomationDriver();

  mojo::AssociatedReceiver<mojom::AutomationAgent> receiver_{this};
  mojo::AssociatedRemote<mojom::AutomationDriver> automation_driver_;

  base::WeakPtrFactory<AutomationAgent> weak_factory_{this};
};

}  // namespace automation

#endif  // COMPONENTS_AUTOMATION_AGENT_CONTENT_RENDERER_AUTOMATION_AGENT_H_