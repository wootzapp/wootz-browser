#include "components/automation_agent/content/renderer/automation_agent.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"


#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_view.h"
#include "url/gurl.h"
#include "ui/gfx/geometry/rect.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/frame/web_local_frame_impl.h"
#include "third_party/blink/renderer/core/dom/element_traversal.h"
#include "third_party/blink/renderer/platform/wtf/text/atomic_string.h"
#include "third_party/blink/public/web/web_element_collection.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/html/html_element.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "components/automation_agent/content/common/mojom/automation_agent.mojom.h"
#include "third_party/blink/public/platform/web_string.h"

namespace automation {

namespace {
constexpr char kHighlightContainerId[] = "automation-highlight-container";
}  // namespace

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

void AutomationAgent::GetPageState(bool debug_mode,
                                 bool include_hidden,
                                 GetPageStateCallback callback) {
  LOG(INFO) << "Kartik: Starting GetPageState with debug_mode=" << debug_mode 
            << ", include_hidden=" << include_hidden;

  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame) {
    LOG(ERROR) << "Kartik: No frame available in GetPageState";
    auto result = mojom::PageStateResult::New();
    result->success = false;
    result->error_message = "No frame available";
    std::move(callback).Run(std::move(result));
    return;
  }

  LOG(INFO) << "Kartik: Frame obtained successfully, getting document";
  blink::WebDocument document = frame->GetDocument();
  LOG(INFO) << "Kartik: Document URL: " << document.Url().GetString().Utf8();

  auto page_state = std::make_unique<base::Value::Dict>();
  base::Value::List elements_list;

  LOG(INFO) << "Kartik: Starting element collection";
  blink::WebElementCollection elements = 
      document.GetElementsByHTMLTagName(blink::WebString::FromUTF8("*"));

  int element_count = 0;
  int visible_elements = 0;
  int interactive_elements = 0;

  for (auto element = elements.FirstItem(); 
       !element.IsNull(); 
       element = elements.NextItem()) {
    element_count++;
    
    if (!include_hidden && !IsElementVisible(element)) {
      LOG(INFO) << "Kartik: Skipping invisible element: " << element.TagName().Utf8();
      continue;
    }
    visible_elements++;

    if (IsElementInteractive(element)) {
      interactive_elements++;
      LOG(INFO) << "Kartik: Found interactive element: " << element.TagName().Utf8() 
                << " with attributes: id=" << element.GetAttribute("id").Utf8();
    }

    base::Value::Dict element_info;
    element_info.Set("tagName", element.TagName().Utf8());
    element_info.Set("isVisible", IsElementVisible(element));
    element_info.Set("isInteractive", IsElementInteractive(element));
    
    // Get attributes - fixed approach without GetAttributeNames
    base::Value::Dict attributes;
    if (element.HasAttribute(blink::WebString())) {
      // Get common attributes we care about
      const char* common_attrs[] = {"id", "class", "name", "type", "value", "href", "src"};
      for (const char* attr : common_attrs) {
        blink::WebString attr_name = blink::WebString::FromUTF8(attr);
        if (element.HasAttribute(attr_name)) {
          attributes.Set(attr, element.GetAttribute(attr_name).Utf8());
        }
      }
    }
    element_info.Set("attributes", std::move(attributes));

    element_info.Set("textContent", element.TextContent().Utf8());

    if (IsElementVisible(element)) {
      auto bounds = element.BoundsInWidget();
      base::Value::Dict bounds_dict;
      bounds_dict.Set("x", bounds.x());
      bounds_dict.Set("y", bounds.y());
      bounds_dict.Set("width", bounds.width());
      bounds_dict.Set("height", bounds.height());
      element_info.Set("bounds", std::move(bounds_dict));
    }

    elements_list.Append(std::move(element_info));
  }

  LOG(INFO) << "Kartik: Element collection complete. Stats:"
            << " Total=" << element_count
            << " Visible=" << visible_elements
            << " Interactive=" << interactive_elements;

  page_state->Set("elements", std::move(elements_list));
  page_state->Set("url", document.Url().GetString().Utf8());
  page_state->Set("title", document.Title().Utf8());

  // Convert to string representation for mojom
  std::string json_string;
  base::JSONWriter::Write(*page_state, &json_string);
  LOG(INFO) << "Kartik: JSON conversion complete, size=" << json_string.length();

  // Create mojom result
  auto result = mojom::PageStateResult::New();
  result->success = true;
  result->error_message = "";
  result->page_state.insert({"page_data", json_string});

  LOG(INFO) << "Kartik: GetPageState completed successfully";
  std::move(callback).Run(std::move(result));
}

void AutomationAgent::PerformAction(
    const std::string& action,
    const base::flat_map<std::string, std::string>& params,
    PerformActionCallback callback) {
  LOG(INFO) << "Kartik: Starting PerformAction with action=" << action 
            << ", params_count=" << params.size();

  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame) {
    LOG(ERROR) << "Kartik: No frame available in PerformAction";
    std::move(callback).Run(false);
    return;
  }

  LOG(INFO) << "Kartik: Frame obtained successfully for action";
  blink::WebDocument document = frame->GetDocument();
  bool success = false;

  if (action == "click") {
    LOG(INFO) << "Kartik: Processing click action";
    auto it = params.find("selector");
    if (it != params.end()) {
      LOG(INFO) << "Kartik: Click selector=" << it->second;
      blink::WebElement element = document.QuerySelector(
          blink::WebString::FromUTF8(it->second));
      if (!element.IsNull()) {
        LOG(INFO) << "Kartik: Found element to click: " << element.TagName().Utf8();
        element.SimulateClick();
        success = true;
        LOG(INFO) << "Kartik: Click simulation completed";
      } else {
        LOG(ERROR) << "Kartik: Element not found for click action";
      }
    }
  } 
  else if (action == "fill") {
    LOG(INFO) << "Kartik: Processing fill action";
    auto selector_it = params.find("selector");
    auto text_it = params.find("text");
    if (selector_it != params.end() && text_it != params.end()) {
      LOG(INFO) << "Kartik: Fill selector=" << selector_it->second 
                << ", text length=" << text_it->second.length();
      blink::WebElement element = document.QuerySelector(
          blink::WebString::FromUTF8(selector_it->second));
      if (!element.IsNull() && element.IsEditable()) {
        LOG(INFO) << "Kartik: Found editable element for fill: " << element.TagName().Utf8();
        element.PasteText(blink::WebString::FromUTF8(text_it->second), true);
        success = true;
        LOG(INFO) << "Kartik: Text fill completed";
      } else {
        LOG(ERROR) << "Kartik: Element not found or not editable for fill action";
      }
    }
  }
  else if (action == "scroll") {
    LOG(INFO) << "Kartik: Processing scroll action";
    auto x_it = params.find("x");
    auto y_it = params.find("y");
    if (x_it != params.end() && y_it != params.end()) {
      LOG(INFO) << "Kartik: Scroll coordinates x=" << x_it->second 
                << ", y=" << y_it->second;
      char* x_end, *y_end;
      float x = std::strtof(x_it->second.c_str(), &x_end);
      float y = std::strtof(y_it->second.c_str(), &y_end);
      if (x_end != x_it->second.c_str() && y_end != y_it->second.c_str()) {
        LOG(INFO) << "Kartik: Setting scroll offset to (" << x << ", " << y << ")";
        frame->SetScrollOffset(gfx::PointF(x, y));
        success = true;
        LOG(INFO) << "Kartik: Scroll completed";
      } else {
        LOG(ERROR) << "Kartik: Invalid scroll coordinates format";
      }
    }
  }

  LOG(INFO) << "Kartik: PerformAction completed with success=" << success;
  std::move(callback).Run(success);
}

bool AutomationAgent::IsElementVisible(const blink::WebElement& element) {
  if (element.IsNull())
    return false;

  // Create non-const copy for GetComputedValue
  blink::WebElement mutable_element = element;
  auto display = mutable_element.GetComputedValue(blink::WebString::FromUTF8("display"));
  auto visibility = mutable_element.GetComputedValue(blink::WebString::FromUTF8("visibility"));
  
  if (display == "none" || visibility == "hidden")
    return false;

  auto bounds = element.BoundsInWidget();
  return bounds.width() > 0 && bounds.height() > 0;
}

bool AutomationAgent::IsElementInteractive(const blink::WebElement& element) {
  if (element.IsNull())
    return false;

  // Check for interactive elements
  if (element.IsFormControlElement() || element.IsEditable())
    return true;

  // Check for common interactive tags
  std::string tag = element.TagName().Utf8();
  std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
  
  return tag == "a" || tag == "button" || 
         element.HasAttribute(blink::WebString::FromUTF8("onclick"));
}

gfx::Rect AutomationAgent::GetElementBounds(const blink::WebElement& element) {
  if (element.IsNull())
    return gfx::Rect();

  return element.BoundsInWidget();
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