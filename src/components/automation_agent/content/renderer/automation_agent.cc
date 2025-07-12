#include "components/automation_agent/content/renderer/automation_agent.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/web/web_script_source.h"


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

  blink::WebDocument document = frame->GetDocument();

  // 🔧 CRITICAL FIX: Clean up previous highlights and reset index
  CleanupPreviousHighlights(document);
  
  LOG(INFO) << "Kartik: Frame obtained successfully, getting document";
  LOG(INFO) << "Kartik: Document URL: " << document.Url().GetString().Utf8();

  auto page_state = std::make_unique<base::Value::Dict>();
  base::Value::List elements_list;

  LOG(INFO) << "Kartik: Starting element collection";
  blink::WebElementCollection elements = 
      document.GetElementsByHTMLTagName(blink::WebString::FromUTF8("*"));

  int element_count = 0;
  int visible_elements = 0;
  int interactive_elements = 0;
  int highlight_index = 0;  // 🔧 RESET: Always start from 0

  // Store elements with their indices for highlighting
  std::vector<std::pair<blink::WebElement, int>> indexed_elements;

  // Limit the number of elements to prevent memory issues
  const int kMaxElements = 2000;

  for (auto element = elements.FirstItem(); 
       !element.IsNull() && element_count < kMaxElements; 
       element = elements.NextItem()) {
    element_count++;
    
    if (!include_hidden && !IsElementVisible(element)) {
      continue;
    }
    visible_elements++;

    // NEW: Check viewport visibility - Skip elements outside viewport
    if (!IsElementInViewport(element)) {
      continue; // Skip elements not in current viewport
    }

    bool is_interactive = IsElementInteractive(element);
    
    // Only include interactive or visible elements to reduce payload size
    if (!is_interactive && !debug_mode) {
      continue;
    }

    if (is_interactive) {
      interactive_elements++;
      LOG(INFO) << "Kartik: Found interactive element: " << element.TagName().Utf8() 
                << " with attributes: id=" << element.GetAttribute("id").Utf8();
    }

    base::Value::Dict element_info;
    element_info.Set("tagName", element.TagName().Utf8());
    
    // Add explicit index for interactive elements
    if (is_interactive) {
      element_info.Set("index", highlight_index);
      indexed_elements.push_back(std::make_pair(element, highlight_index));
      LOG(INFO) << "Kartik: Assigned index " << highlight_index << " to " << element.TagName().Utf8() 
                << " (total interactive: " << (highlight_index + 1) << ")";
      highlight_index++;
    }
    
    // Only include visibility info if debug mode is on
    if (debug_mode) {
      element_info.Set("isVisible", IsElementVisible(element));
      element_info.Set("isInteractive", is_interactive);
    }
    
    // Only include essential attributes
    base::Value::Dict attributes;
    const char* essential_attrs[] = {"id", "class", "name", "type", "href", "data-testid"};
    for (const char* attr : essential_attrs) {
      blink::WebString attr_name = blink::WebString::FromUTF8(attr);
      if (element.HasAttribute(attr_name)) {
        attributes.Set(attr, element.GetAttribute(attr_name).Utf8());
      }
    }
    
    if (!attributes.empty()) {
      element_info.Set("attributes", std::move(attributes));
    }

    // Only include text content for interactive elements
    if (is_interactive) {
      element_info.Set("textContent", element.TextContent().Utf8());
    }

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
            << " CSS-Visible=" << visible_elements  
            << " In-Viewport=" << elements_list.size()  // This shows viewport-filtered count
            << " Interactive=" << interactive_elements
            << " Indexed=" << highlight_index;

  // Apply highlighting CSS when debug mode is enabled
  if (debug_mode) {
    InjectIndexedHighlightCSS(document, indexed_elements);
  }

  page_state->Set("elements", std::move(elements_list));
  page_state->Set("url", document.Url().GetString().Utf8());
  page_state->Set("title", document.Title().Utf8());

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
  blink::WebElement target_element;

  // First, try to find element by index, then fall back to selector
  auto index_it = params.find("index");
  auto selector_it = params.find("selector");
  
  if (index_it != params.end()) {
    // Index-based element selection
    int target_index = std::stoi(index_it->second);
    LOG(INFO) << "Kartik: Looking for element with index=" << target_index;
    
    std::string index_selector = "[data-automation-index='" + std::to_string(target_index) + "']";
    target_element = document.QuerySelector(blink::WebString::FromUTF8(index_selector));
    
    if (!target_element.IsNull()) {
      LOG(INFO) << "Kartik: Found element by index " << target_index << ": " << target_element.TagName().Utf8();
    } else {
      LOG(ERROR) << "Kartik: Element with index " << target_index << " not found";
    }
  } else if (selector_it != params.end()) {
    // Selector-based element selection (existing logic)
    LOG(INFO) << "Kartik: Looking for element with selector=" << selector_it->second;
    target_element = document.QuerySelector(blink::WebString::FromUTF8(selector_it->second));
    
    if (!target_element.IsNull()) {
      LOG(INFO) << "Kartik: Found element by selector: " << target_element.TagName().Utf8();
    } else {
      LOG(ERROR) << "Kartik: Element with selector not found";
    }
  }

  if (target_element.IsNull()) {
    LOG(ERROR) << "Kartik: No target element found for action";
    std::move(callback).Run(false);
    return;
  }

  if (action == "click") {
    LOG(INFO) << "Kartik: Processing click action on " << target_element.TagName().Utf8();
    target_element.SimulateClick();
    success = true;
    LOG(INFO) << "Kartik: Click simulation completed";
  } 
  else if (action == "fill") {
    LOG(INFO) << "Kartik: Processing fill action";
    auto text_it = params.find("text");
    if (text_it != params.end()) {
      LOG(INFO) << "Kartik: Fill text length=" << text_it->second.length();
      if (target_element.IsEditable()) {
        LOG(INFO) << "Kartik: Found editable element for fill: " << target_element.TagName().Utf8();
        target_element.PasteText(blink::WebString::FromUTF8(text_it->second), true);
        success = true;
        LOG(INFO) << "Kartik: Text fill completed";
      } else {
        LOG(ERROR) << "Kartik: Element is not editable for fill action";
      }
    } else {
      LOG(ERROR) << "Kartik: Missing text parameter for fill action";
    }
  }
  else if (action == "scroll") {
    LOG(INFO) << "Kartik: Processing scroll action";
    
    auto direction_it = params.find("direction");
    if (direction_it == params.end()) {
      LOG(ERROR) << "Kartik: Missing required 'direction' parameter for scroll action";
      success = false;
    } else {
      std::string direction = direction_it->second;
      int amount = 300; // default scroll amount
      
      // Get amount parameter if provided
      auto amount_it = params.find("amount");
      if (amount_it != params.end()) {
        char* end;
        int parsed_amount = std::strtol(amount_it->second.c_str(), &end, 10);
        if (end != amount_it->second.c_str() && parsed_amount > 0) {
          amount = parsed_amount;
        }
      }
      
      LOG(INFO) << "Kartik: Direction scroll - " << direction << " by " << amount << "px";
      
      // Get current scroll position
      gfx::PointF current_offset = frame->GetScrollOffset();
      LOG(INFO) << "Kartik: Current scroll offset: (" << current_offset.x() << ", " << current_offset.y() << ")";
      
      // Calculate new scroll position based on direction
      gfx::PointF new_offset = current_offset;
      
      if (direction == "down") {
        new_offset.set_y(current_offset.y() + amount);
        success = true;
        LOG(INFO) << "Kartik: Scrolling down by " << amount << "px";
      } else if (direction == "up") {
        new_offset.set_y(std::max(0.0f, current_offset.y() - amount));
        success = true;
        LOG(INFO) << "Kartik: Scrolling up by " << amount << "px";
      } else if (direction == "right") {
        new_offset.set_x(current_offset.x() + amount);
        success = true;
        LOG(INFO) << "Kartik: Scrolling right by " << amount << "px";
      } else if (direction == "left") {
        new_offset.set_x(std::max(0.0f, current_offset.x() - amount));
        success = true;
        LOG(INFO) << "Kartik: Scrolling left by " << amount << "px";
      } else {
        LOG(ERROR) << "Kartik: Invalid scroll direction: " << direction 
                   << ". Valid directions: up, down, left, right";
        success = false;
      }
      
      if (success) {
        frame->SetScrollOffset(new_offset);
        LOG(INFO) << "Kartik: New scroll offset set to: (" << new_offset.x() << ", " << new_offset.y() << ")";
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

  // Basic form controls and editable elements
  if (element.IsFormControlElement() || element.IsEditable() || element.IsContentEditable())
    return true;

  std::string tag = element.TagName().Utf8();
  std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);

  // Define interactive element sets
  static const std::set<std::string> interactive_tags = {
    "a", "button", "input", "select", "textarea", "details", "summary", 
    "label", "option", "optgroup", "fieldset", "legend"
  };

  // Check basic interactive tags
  if (interactive_tags.count(tag)) {
    return true;
  }

  // Create non-const copy for GetComputedValue
  blink::WebElement mutable_element = element;
  blink::WebString cursor_style = mutable_element.GetComputedValue(blink::WebString::FromUTF8("cursor"));
  std::string cursor = cursor_style.Utf8();

  // Check for interactive cursor styles
  static const std::set<std::string> interactive_cursors = {
    "pointer", "move", "text", "grab", "grabbing", "cell", "copy", "alias", 
    "all-scroll", "col-resize", "context-menu", "crosshair", "help", 
    "zoom-in", "zoom-out"
  };
  
  if (interactive_cursors.count(cursor)) {
    LOG(INFO) << "Kartik: Found interactive cursor '" << cursor << "' on " << tag;
    return true;
  }

  // Check ARIA roles
  blink::WebString role_attr = element.GetAttribute(blink::WebString::FromUTF8("role"));
  if (!role_attr.IsEmpty()) {
    std::string role = role_attr.Utf8();
    static const std::set<std::string> interactive_roles = {
      "button", "link", "menuitem", "menuitemradio", "menuitemcheckbox",
      "radio", "checkbox", "tab", "switch", "slider", "spinbutton",
      "combobox", "searchbox", "textbox", "listbox", "option", "scrollbar"
    };
    
    if (interactive_roles.count(role)) {
      LOG(INFO) << "Kartik: Found interactive role '" << role << "' on " << tag;
      return true;
    }
  }

  // Check for tabindex (interactive if >= 0) - No exceptions version
  blink::WebString tabindex_attr = element.GetAttribute(blink::WebString::FromUTF8("tabindex"));
  if (!tabindex_attr.IsEmpty()) {
    std::string tabindex = tabindex_attr.Utf8();
    
    bool valid_number = true;
    int tabindex_val = 0;
    
    if (!tabindex.empty()) {
      char* end_ptr;
      long parsed_val = std::strtol(tabindex.c_str(), &end_ptr, 10);
      
      if (end_ptr == tabindex.c_str() + tabindex.length() && 
          parsed_val >= INT_MIN && parsed_val <= INT_MAX) {
        tabindex_val = static_cast<int>(parsed_val);
      } else {
        valid_number = false;
      }
      
      if (valid_number && tabindex_val >= 0) {
        LOG(INFO) << "Kartik: Found interactive tabindex '" << tabindex << "' on " << tag;
        return true;
      }
    }
  }

  // Check for common interactive attributes
  static const std::vector<std::string> interactive_attrs = {
    "onclick", "onmousedown", "onmouseup", "ondblclick", "onkeydown", "onkeyup",
    "data-testid", "data-test", "data-cy", "data-action", "data-toggle", 
    "data-dismiss", "data-target", "aria-haspopup", "aria-expanded"
  };

  for (const auto& attr : interactive_attrs) {
    if (element.HasAttribute(blink::WebString::FromUTF8(attr))) {
      LOG(INFO) << "Kartik: Found interactive attribute '" << attr << "' on " << tag;
      return true;
    }
  }

  // Check for interactive class patterns
  blink::WebString class_attr = element.GetAttribute(blink::WebString::FromUTF8("class"));
  if (!class_attr.IsEmpty()) {
    std::string class_name = class_attr.Utf8();
    std::transform(class_name.begin(), class_name.end(), class_name.begin(), ::tolower);
    
    static const std::vector<std::string> interactive_class_patterns = {
      "btn", "button", "clickable", "link", "menu", "dropdown", "toggle",
      "tab", "accordion", "modal", "dialog", "popup", "tooltip", "nav",
      "search", "filter", "sort", "close", "cancel", "submit", "login",
      "signin", "signup", "register", "play", "pause", "stop", "next", "prev"
    };
    
    for (const auto& pattern : interactive_class_patterns) {
      if (class_name.find(pattern) != std::string::npos) {
        LOG(INFO) << "Kartik: Found interactive class pattern '" << pattern << "' on " << tag;
        return true;
      }
    }
  }

  // Check for elements with specific types
  if (tag == "input") {
    blink::WebString type_attr = element.GetAttribute(blink::WebString::FromUTF8("type"));
    std::string type = type_attr.Utf8();
    // All input types are potentially interactive
    LOG(INFO) << "Kartik: Found input with type '" << type << "'";
    return true;
  }

  // Check for elements that might be search boxes
  blink::WebString name_attr = element.GetAttribute(blink::WebString::FromUTF8("name"));
  blink::WebString id_attr = element.GetAttribute(blink::WebString::FromUTF8("id"));
  blink::WebString placeholder_attr = element.GetAttribute(blink::WebString::FromUTF8("placeholder"));
  
  std::string name = name_attr.Utf8();
  std::string id = id_attr.Utf8();
  std::string placeholder = placeholder_attr.Utf8();
  
  // Convert to lowercase for pattern matching
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  std::transform(id.begin(), id.end(), id.begin(), ::tolower);
  std::transform(placeholder.begin(), placeholder.end(), placeholder.begin(), ::tolower);
  
  static const std::vector<std::string> search_patterns = {
    "search", "query", "q", "find", "filter", "lookup"
  };
  
  for (const auto& pattern : search_patterns) {
    if (name.find(pattern) != std::string::npos || 
        id.find(pattern) != std::string::npos || 
        placeholder.find(pattern) != std::string::npos) {
      LOG(INFO) << "Kartik: Found search pattern '" << pattern << "' on " << tag;
      return true;
    }
  }

  // Check for div/span elements that might be custom buttons
  if (tag == "div" || tag == "span") {
    // Check if it has pointer cursor or click handlers
    if (cursor == "pointer" || 
        element.HasAttribute(blink::WebString::FromUTF8("onclick")) ||
        element.HasAttribute(blink::WebString::FromUTF8("role"))) {
      LOG(INFO) << "Kartik: Found interactive " << tag << " with pointer cursor or handlers";
      return true;
    }
  }

  return false;
}

bool AutomationAgent::IsElementInViewport(const blink::WebElement& element) {
  if (element.IsNull())
    return false;

  // Get element bounds
  gfx::Rect element_bounds = element.BoundsInWidget();
  
  // Element has no size, skip it
  if (element_bounds.width() <= 0 || element_bounds.height() <= 0) {
    return false;
  }

  // Get viewport size using the correct method
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame) {
    return false;
  }

  blink::WebView* web_view = frame->View();
  if (!web_view) {
    return false;
  }

  // FIX: Use VisualViewportSize() instead of GetLayoutSize()
  gfx::SizeF viewport_size_f = web_view->VisualViewportSize();
  gfx::Size viewport_size(static_cast<int>(viewport_size_f.width()), 
                         static_cast<int>(viewport_size_f.height()));
  
  // Get scroll position
  gfx::PointF scroll_offset = frame->GetScrollOffset();
  
  // Calculate viewport bounds (what's currently visible)
  gfx::Rect viewport_bounds(
    static_cast<int>(scroll_offset.x()), 
    static_cast<int>(scroll_offset.y()), 
    viewport_size.width(), 
    viewport_size.height()
  );

  // Check if element intersects with viewport
  bool in_viewport = viewport_bounds.Intersects(element_bounds);
  
  if (in_viewport) {
    LOG(INFO) << "Kartik: Element " << element.TagName().Utf8() 
              << " is in viewport - bounds(" << element_bounds.x() << "," << element_bounds.y() 
              << "," << element_bounds.width() << "," << element_bounds.height() << ")"
              << " viewport(" << viewport_bounds.x() << "," << viewport_bounds.y()
              << "," << viewport_bounds.width() << "," << viewport_bounds.height() << ")";
  }

  return in_viewport;
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


void AutomationAgent::InjectIndexedHighlightCSS(blink::WebDocument& document, 
                                               std::vector<std::pair<blink::WebElement, int>>& indexed_elements) {
  LOG(INFO) << "Kartik: Injecting indexed highlight CSS for " << indexed_elements.size() << " elements";
  
  // Color array matching buildDomTree.js
  static const std::vector<std::string> colors = {
    "#FF0000", "#00FF00", "#0000FF", "#FFA500", "#800080", "#008080",
    "#FF69B4", "#4B0082", "#FF4500", "#2E8B57", "#DC143C", "#4682B4"
  };
  
  // First inject base CSS
  std::string base_css = R"(
    .automation-highlight {
      outline: 2px solid var(--highlight-color) !important;
      outline-offset: 1px !important;
      background: var(--highlight-bg) !important;
      position: relative !important;
    }
    
    .automation-highlight-label {
      position: absolute !important;
      top: -2px !important;
      right: -2px !important;
      background: var(--highlight-color) !important;
      color: white !important;
      font-size: 12px !important;
      padding: 1px 4px !important;
      border-radius: 4px !important;
      z-index: 2147483647 !important;
      font-family: Arial, sans-serif !important;
      font-weight: bold !important;
      line-height: 1 !important;
      min-width: 16px !important;
      text-align: center !important;
      pointer-events: none !important;
    }
  )";

  document.InsertStyleSheet(blink::WebString::FromUTF8(base_css));

  // Now highlight each element individually with its unique color
  for (auto& pair : indexed_elements) {
    blink::WebElement element = pair.first;
    int index = pair.second;
    
    // Get color for this index (cycle through colors array)
    int color_index = index % colors.size();
    std::string base_color = colors[color_index];
    std::string bg_color = base_color + "1A"; // Add 10% opacity
    
    // Create unique CSS for this specific index
    std::string element_css = 
      "[data-automation-index='" + std::to_string(index) + "'] { "
      "--highlight-color: " + base_color + " !important; "
      "--highlight-bg: " + bg_color + " !important; "
      "}";
    
    document.InsertStyleSheet(blink::WebString::FromUTF8(element_css));
    
    // Add highlight class and index attribute
    std::string existing_class = element.GetAttribute("class").Utf8();
    std::string new_class = existing_class + " automation-highlight";
    
    element.SetAttribute("class", blink::WebString::FromUTF8(new_class));
    element.SetAttribute("data-automation-index", blink::WebString::FromUTF8(std::to_string(index)));
    
    std::string tag_name = element.TagName().Utf8();
    std::transform(tag_name.begin(), tag_name.end(), tag_name.begin(), ::tolower);
    
    LOG(INFO) << "Kartik: Highlighted " << tag_name << " with index " << index << " using color " << base_color;
  }
  
  // Inject JavaScript to add numbered labels with matching colors
  std::string js_code = R"(
    (function() {
      const colors = [
        '#FF0000', '#00FF00', '#0000FF', '#FFA500', '#800080', '#008080',
        '#FF69B4', '#4B0082', '#FF4500', '#2E8B57', '#DC143C', '#4682B4'
      ];
      
      const elements = document.querySelectorAll('[data-automation-index]');
      elements.forEach(element => {
        const index = parseInt(element.getAttribute('data-automation-index'));
        const colorIndex = index % colors.length;
        const baseColor = colors[colorIndex];
        
        const label = document.createElement('div');
        label.className = 'automation-highlight-label';
        label.textContent = index;
        label.style.backgroundColor = baseColor;
        label.style.color = 'white';
        label.style.position = 'absolute';
        label.style.top = '-2px';
        label.style.right = '-2px';
        label.style.fontSize = '12px';
        label.style.padding = '1px 4px';
        label.style.borderRadius = '4px';
        label.style.zIndex = '2147483647';
        label.style.fontFamily = 'Arial, sans-serif';
        label.style.fontWeight = 'bold';
        label.style.lineHeight = '1';
        label.style.minWidth = '16px';
        label.style.textAlign = 'center';
        label.style.pointerEvents = 'none';
        
        element.appendChild(label);
      });
    })();
  )";
  
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (frame) {
    frame->ExecuteScript(blink::WebScriptSource(blink::WebString::FromUTF8(js_code)));
  } else {
    LOG(ERROR) << "Kartik: Could not execute script for index highlighting due to no frame.";
  }
}

void AutomationAgent::CleanupPreviousHighlights(blink::WebDocument& document) {
  LOG(INFO) << "Kartik: Cleaning up previous highlights";
  
  // Remove all elements with automation highlighting
  blink::WebString cleanup_script = blink::WebString::FromUTF8(R"(
    (function() {
      // Remove all automation highlight classes and attributes
      const highlightedElements = document.querySelectorAll('[data-automation-index]');
      highlightedElements.forEach(element => {
        // Remove automation classes
        element.classList.remove('automation-highlight');
        
        // Remove automation attributes
        element.removeAttribute('data-automation-index');
        
        // Remove automation labels
        const labels = element.querySelectorAll('.automation-highlight-label');
        labels.forEach(label => label.remove());
      });
      
      // Remove all automation CSS styles
      const styles = document.querySelectorAll('style');
      styles.forEach(style => {
        if (style.textContent && (
            style.textContent.includes('automation-highlight') ||
            style.textContent.includes('data-automation-index') ||
            style.textContent.includes('--highlight-color')
        )) {
          style.remove();
        }
      });
      
      console.log('Automation highlights cleaned up');
    })();
  )");
  
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (frame) {
    frame->ExecuteScript(blink::WebScriptSource(cleanup_script));
    LOG(INFO) << "Kartik: Previous highlights cleanup completed";
  } else {
    LOG(ERROR) << "Kartik: Could not execute cleanup script - no frame available";
  }
}

}  // namespace automation