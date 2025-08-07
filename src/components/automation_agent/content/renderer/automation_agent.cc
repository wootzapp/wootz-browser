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

  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame) {
    LOG(ERROR) << "No frame available in GetPageState";
    auto result = mojom::PageStateResult::New();
    result->success = false;
    result->error_message = "No frame available";
    std::move(callback).Run(std::move(result));
    return;
  }

  blink::WebDocument document = frame->GetDocument();
  CleanupPreviousHighlights(document);

  auto page_state = std::make_unique<base::Value::Dict>();
  base::Value::List elements_list;

  auto page_context = AnalyzePageContext(document, frame);
  page_state->Set("pageContext", std::move(page_context));

  auto viewport_info = AnalyzeViewport(frame);
  page_state->Set("viewport", std::move(viewport_info));

  blink::WebElementCollection elements = 
      document.GetElementsByHTMLTagName(blink::WebString::FromUTF8("*"));

  int element_count = 0;
  int interactive_elements = 0;
  int highlight_index = 0;

  std::set<blink::WebElement> highlighted_elements;
  std::vector<std::pair<blink::WebElement, int>> indexed_elements;
  base::Value::Dict element_categories;
  int form_elements = 0, nav_elements = 0, content_elements = 0, action_elements = 0;

  const int kMaxElements = 2000;

  for (auto element = elements.FirstItem(); 
       !element.IsNull() && element_count < kMaxElements; 
       element = elements.NextItem()) {
    element_count++;
    
    if (!include_hidden && !IsElementVisible(element)) {
      continue;
    }

    if (!IsElementInViewport(element)) {
      continue;
    }

    bool is_interactive = IsElementInteractive(element);

    std::string element_category = CategorizeElementForAI(element);

    if (!is_interactive && !debug_mode && element_category.empty()) {
      continue;
    }

    bool should_highlight = false;
    if (is_interactive) {
      interactive_elements++;
      
      if (element_category == "form") form_elements++;
      else if (element_category == "navigation") nav_elements++;
      else if (element_category == "action") action_elements++;
      else content_elements++;

      bool ancestor_highlighted = IsAncestorHighlighted(element, highlighted_elements);
      
      if (!ancestor_highlighted) {
        should_highlight = true;
      } else if (IsElementDistinctInteraction(element)) {
        should_highlight = true;
      } else {
        continue;
      }
    }

    if (!should_highlight && !debug_mode) {
      continue;
    }

    base::Value::Dict element_info;
    element_info.Set("tagName", element.TagName().Utf8());
    
    element_info.Set("xpath", GetElementXPath(element));
    element_info.Set("selector", GenerateElementSelector(element));
    
    if (should_highlight) {
      element_info.Set("index", highlight_index);
      element_info.Set("category", element_category);
      element_info.Set("purpose", GetElementPurpose(element));
      
      indexed_elements.push_back(std::make_pair(element, highlight_index));
      highlighted_elements.insert(element);
      highlight_index++;
    }
    
    if (debug_mode) {
      element_info.Set("isVisible", IsElementVisible(element));
      element_info.Set("isInteractive", is_interactive);
    }
    
    base::Value::Dict attributes;
    const char* ai_attrs[] = {"id", "class", "name", "type", "href", "data-testid", 
                              "aria-label", "role", "placeholder", "title"};
    for (const char* attr : ai_attrs) {
      blink::WebString attr_name = blink::WebString::FromUTF8(attr);
      if (element.HasAttribute(attr_name)) {
        attributes.Set(attr, element.GetAttribute(attr_name).Utf8());
      }
    }
    
    if (!attributes.empty()) {
      element_info.Set("attributes", std::move(attributes));
    }

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

  element_categories.Set("form", form_elements);
  element_categories.Set("navigation", nav_elements);
  element_categories.Set("action", action_elements);
  element_categories.Set("content", content_elements);
  page_state->Set("elementCategories", std::move(element_categories));

  auto capabilities = AnalyzePageCapabilities(document);
  page_state->Set("capabilities", std::move(capabilities));

  // Always inject index attributes for element targeting
  InjectElementIndexes(document, indexed_elements);

  // Only add visual highlighting in debug mode
  if (debug_mode) {
    InjectVisualHighlightCSS(document, indexed_elements);
  }

  page_state->Set("elements", std::move(elements_list));
  page_state->Set("url", document.Url().GetString().Utf8());
  page_state->Set("title", document.Title().Utf8());

  std::string json_string;
  base::JSONWriter::Write(*page_state, &json_string);

  auto result = mojom::PageStateResult::New();
  result->success = true;
  result->error_message = "";
  result->page_state.insert({"page_data", json_string});

  std::move(callback).Run(std::move(result));
}

std::string AutomationAgent::GetElementXPath(const blink::WebElement& element) {
  if (element.IsNull()) return "";
  
  std::vector<std::string> path;
  blink::WebNode current = element;
  
  while (!current.IsNull() && current.IsElementNode()) {  
    blink::WebElement current_element = current.To<blink::WebElement>();
    std::string tag = current_element.TagName().Utf8();
    std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
    
    // Stop at body or html
    if (tag == "body" || tag == "html") {
      path.insert(path.begin(), tag);
      break;
    }
    
    int position = 1;
    blink::WebNode sibling = current_element.PreviousSibling();
    while (!sibling.IsNull()) {
      if (sibling.IsElementNode()) {  
        blink::WebElement sibling_element = sibling.To<blink::WebElement>();
        if (sibling_element.TagName().Utf8() == current_element.TagName().Utf8()) {
          position++;
        }
      }
      sibling = sibling.PreviousSibling();
    }
    
    std::string element_path = tag + "[" + std::to_string(position) + "]";
    path.insert(path.begin(), element_path);
    
    current = current.ParentNode();
  }
  
  std::string xpath = "/";
  for (size_t i = 0; i < path.size(); ++i) {
    if (i > 0) xpath += "/";
    xpath += path[i];
  }
  
  return xpath;
}

base::Value::Dict AutomationAgent::AnalyzePageContext(const blink::WebDocument& document, blink::WebLocalFrame* frame) {
  base::Value::Dict context;
  
  std::string url = document.Url().GetString().Utf8();
  std::string title = document.Title().Utf8();
  
  std::string page_type = "general";
  if (url.find("/login") != std::string::npos || url.find("/signin") != std::string::npos) {
    page_type = "authentication";
  } else if (url.find("/search") != std::string::npos) {
    page_type = "search";
  } else if (url.find("/checkout") != std::string::npos || url.find("/cart") != std::string::npos) {
    page_type = "commerce";
  } else if (document.QuerySelector(blink::WebString::FromUTF8("form"))) {
    page_type = "form";
  }
  
  bool has_login_form = !!(
    document.QuerySelector(blink::WebString::FromUTF8("input[type='password']")) ||
    document.QuerySelector(blink::WebString::FromUTF8("input[name*='password']"))
  );
  
  bool has_user_menu = !!(
    document.QuerySelector(blink::WebString::FromUTF8("[aria-label*='menu']")) ||
    document.QuerySelector(blink::WebString::FromUTF8("[aria-label*='account']")) ||
    document.QuerySelector(blink::WebString::FromUTF8("[data-testid*='user']"))
  );
  
  context.Set("pageType", page_type);
  context.Set("hasLoginForm", has_login_form);
  context.Set("hasUserMenu", has_user_menu);
  context.Set("isLoggedIn", has_user_menu && !has_login_form);
  
  return context;
}

base::Value::Dict AutomationAgent::AnalyzeViewport(blink::WebLocalFrame* frame) {
  base::Value::Dict viewport;
  
  blink::WebView* web_view = frame->View();
  if (web_view) {
    gfx::SizeF viewport_size = web_view->VisualViewportSize();
    
    // Handle background web contents which might have 0 dimensions
    float width = viewport_size.width();
    float height = viewport_size.height();
    
    // For background web contents, use default dimensions if viewport is 0
    if (width <= 0 || height <= 0) {
      width = 360;  // Default mobile width (common Android phone width)
      height = 640;  // Default mobile height (common Android phone height)
    }
    
    bool is_mobile_width = width <= 768;
    bool is_tablet_width = width > 768 && width <= 1024;
    bool is_portrait = height > width;
    
    viewport.Set("width", static_cast<int>(width));
    viewport.Set("height", static_cast<int>(height));
    viewport.Set("isMobileWidth", is_mobile_width);
    viewport.Set("isTabletWidth", is_tablet_width);
    viewport.Set("isPortrait", is_portrait);
    viewport.Set("deviceType", is_mobile_width ? "mobile" : is_tablet_width ? "tablet" : "desktop");
    viewport.Set("aspectRatio", height > 0 ? width / height : 1.0);  // Prevent division by zero
  }
  
  return viewport;
}

std::string AutomationAgent::CategorizeElementForAI(const blink::WebElement& element) {
  if (element.IsNull()) return "";
  
  std::string tag = element.TagName().Utf8();
  std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
  
  // Form elements
  if (tag == "input" || tag == "textarea" || tag == "select" || 
      element.GetAttribute(blink::WebString::FromUTF8("contenteditable")).Utf8() == "true") { 
    return "form";
  }
  
  // Navigation elements
  if (tag == "a" || 
      element.GetAttribute(blink::WebString::FromUTF8("role")).Utf8() == "link" || 
      element.GetAttribute(blink::WebString::FromUTF8("role")).Utf8() == "menuitem") { 
    return "navigation";
  }
  
  // Action elements
  if (tag == "button" || 
      element.GetAttribute(blink::WebString::FromUTF8("role")).Utf8() == "button" || 
      element.GetAttribute(blink::WebString::FromUTF8("type")).Utf8() == "submit") { 
    return "action";
  }
  
  // Check for interactive divs/spans (common in modern web apps)
  if ((tag == "div" || tag == "span") && 
      (element.HasAttribute(blink::WebString::FromUTF8("onclick")) ||  // Fix: Use HasAttribute
       element.GetAttribute(blink::WebString::FromUTF8("role")).Utf8() == "button" || 
       element.GetAttribute(blink::WebString::FromUTF8("tabindex")).Utf8() != "")) { 
    return "action";
  }
  
  return "content";
}

std::string AutomationAgent::GetElementPurpose(const blink::WebElement& element) {
  if (element.IsNull()) return "unknown";
  
  std::string text = element.TextContent().Utf8();
  std::string aria_label = element.GetAttribute(blink::WebString::FromUTF8("aria-label")).Utf8(); 
  std::string placeholder = element.GetAttribute(blink::WebString::FromUTF8("placeholder")).Utf8(); 
  std::string type = element.GetAttribute(blink::WebString::FromUTF8("type")).Utf8(); 
  
  std::transform(text.begin(), text.end(), text.begin(), ::tolower);
  std::transform(aria_label.begin(), aria_label.end(), aria_label.begin(), ::tolower);
  std::transform(placeholder.begin(), placeholder.end(), placeholder.begin(), ::tolower);
  
  // Authentication patterns
  if (text.find("login") != std::string::npos || text.find("sign in") != std::string::npos ||
      aria_label.find("login") != std::string::npos || type == "password") {
    return "authentication";
  }
  
  // Search patterns
  if (text.find("search") != std::string::npos || placeholder.find("search") != std::string::npos ||
      aria_label.find("search") != std::string::npos) {
    return "search";
  }
  
  // Submit/action patterns
  if (text.find("submit") != std::string::npos || text.find("send") != std::string::npos ||
      text.find("save") != std::string::npos || type == "submit") {
    return "submit";
  }
  
  // Navigation patterns
  if (text.find("home") != std::string::npos || text.find("back") != std::string::npos ||
      text.find("next") != std::string::npos || text.find("menu") != std::string::npos) {
    return "navigation";
  }
  
  return "interaction";
}

base::Value::Dict AutomationAgent::AnalyzePageCapabilities(const blink::WebDocument& document) {
  base::Value::Dict capabilities;
  
  // Check for various capabilities
  bool can_login = !!(document.QuerySelector(blink::WebString::FromUTF8("input[type='password']")));
  bool can_search = !!(document.QuerySelector(blink::WebString::FromUTF8("input[type='search']")) ||
                      document.QuerySelector(blink::WebString::FromUTF8("[role='searchbox']")) ||
                      document.QuerySelector(blink::WebString::FromUTF8("input[placeholder*='search' i]")));
  bool has_forms = !!(document.QuerySelector(blink::WebString::FromUTF8("form")));
  bool has_file_upload = !!(document.QuerySelector(blink::WebString::FromUTF8("input[type='file']")));
  bool has_media = !!(document.QuerySelector(blink::WebString::FromUTF8("video, audio")));
  
  capabilities.Set("canLogin", can_login);
  capabilities.Set("canSearch", can_search);
  capabilities.Set("hasForms", has_forms);
  capabilities.Set("hasFileUpload", has_file_upload);
  capabilities.Set("hasMedia", has_media);
  capabilities.Set("isInteractive", can_login || can_search || has_forms);
  
  return capabilities;
}

void AutomationAgent::PerformAction(
    const std::string& action,
    const base::flat_map<std::string, std::string>& params,
    PerformActionCallback callback) {

  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame) {
    std::move(callback).Run(false);
    return;
  }

  blink::WebDocument document = frame->GetDocument();
  bool success = false;

  // Handle scroll action first (doesn't need target element)
  if (action == "scroll") {
    
    auto direction_it = params.find("direction");
    if (direction_it == params.end()) {
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
      
      // Get current scroll position
      gfx::PointF current_offset = frame->GetScrollOffset();
      
      // Calculate new scroll position based on direction
      gfx::PointF new_offset = current_offset;
      
      if (direction == "down") {
        new_offset.set_y(current_offset.y() + amount);
        success = true;
      } else if (direction == "up") {
        new_offset.set_y(std::max(0.0f, current_offset.y() - amount));
        success = true;
      } else if (direction == "right") {
        new_offset.set_x(current_offset.x() + amount);
        success = true;
      } else if (direction == "left") {
        new_offset.set_x(std::max(0.0f, current_offset.x() - amount));
        success = true;
      } else {
        success = false;
      }
      
      if (success) {
        frame->SetScrollOffset(new_offset);
      }
    }
    
    std::move(callback).Run(success);
    return;
  }

  // For other actions, find target element
  blink::WebElement target_element;
  auto index_it = params.find("index");
  auto selector_it = params.find("selector");
  
  if (index_it != params.end()) {
    int target_index = std::stoi(index_it->second);
    std::string index_selector = "[data-automation-index='" + std::to_string(target_index) + "']";
    target_element = document.QuerySelector(blink::WebString::FromUTF8(index_selector));
  } else if (selector_it != params.end()) {
    target_element = document.QuerySelector(blink::WebString::FromUTF8(selector_it->second));
  }

  if (target_element.IsNull()) {
    std::move(callback).Run(false);
    return;
  }

  if (action == "click") {
    target_element.SimulateClick();
    success = true;
  } 
  else if (action == "fill") {
    auto text_it = params.find("text");
    if (text_it != params.end()) {
      if (target_element.IsEditable()) {
        target_element.PasteText(blink::WebString::FromUTF8(text_it->second), true);
        success = true;
      } else {
        LOG(ERROR) << "Element is not editable for fill action";
      }
    } else {
      LOG(ERROR) << "Missing text parameter for fill action";
    }
  }
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
        return true;
      }
    }
  }

  // Check for elements with specific types
  if (tag == "input") {
    blink::WebString type_attr = element.GetAttribute(blink::WebString::FromUTF8("type"));
    std::string type = type_attr.Utf8();
    // All input types are potentially interactive
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
      return true;
    }
  }

  // Check for div/span elements that might be custom buttons
  if (tag == "div" || tag == "span") {
    // Check if it has pointer cursor or click handlers
    if (cursor == "pointer" || 
        element.HasAttribute(blink::WebString::FromUTF8("onclick")) ||
        element.HasAttribute(blink::WebString::FromUTF8("role"))) {
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

  gfx::SizeF viewport_size_f = web_view->VisualViewportSize();
  
  // For background web contents, if viewport is 0, consider all elements in viewport
  if (viewport_size_f.width() <= 0 || viewport_size_f.height() <= 0) {
    return true;
  }
  
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

  return in_viewport;
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
  
  static const std::vector<std::string> colors = {
    "#FF0000", "#00FF00", "#0000FF", "#FFA500", "#800080", "#008080",
    "#FF69B4", "#4B0082", "#FF4500", "#2E8B57", "#DC143C", "#4682B4"
  };
  
  // Build CSS that targets elements by their XPath or other stable selectors
  // instead of modifying DOM attributes
  std::string all_css = R"(
    .automation-highlight {
      outline: 2px solid var(--highlight-color) !important;
      outline-offset: 1px !important;
      background: var(--highlight-bg) !important;
      position: relative !important;
    }
    
    .automation-highlight::after {
      content: attr(data-index) !important;
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

  // Add element-specific styles using CSS attribute selectors
  for (auto& pair : indexed_elements) {
    int index = pair.second;
    int color_index = index % colors.size();
    std::string base_color = colors[color_index];
    std::string bg_color = base_color + "1A";
    
    all_css += "[data-automation-index='" + std::to_string(index) + "'] { ";
    all_css += "--highlight-color: " + base_color + " !important; ";
    all_css += "--highlight-bg: " + bg_color + " !important; ";
    all_css += "}\n";
  }

  document.InsertStyleSheet(blink::WebString::FromUTF8(all_css));
  
  // Apply attributes without using JavaScript - this still modifies DOM but minimally
  for (auto& pair : indexed_elements) {
    blink::WebElement element = pair.first;
    int index = pair.second;
    
    std::string existing_class = element.GetAttribute("class").Utf8();
    std::string new_class = existing_class + " automation-highlight";
    
    element.SetAttribute("class", blink::WebString::FromUTF8(new_class));
    element.SetAttribute("data-automation-index", blink::WebString::FromUTF8(std::to_string(index))); // For CSS content
  }
}

void AutomationAgent::InjectElementIndexes(blink::WebDocument& document, 
                                          std::vector<std::pair<blink::WebElement, int>>& indexed_elements) {
  
  // Set index attributes on elements (always needed for PerformAction)
  for (auto& pair : indexed_elements) {
    blink::WebElement element = pair.first;
    int index = pair.second;
    
    element.SetAttribute("data-automation-index", blink::WebString::FromUTF8(std::to_string(index)));
  }
}

void AutomationAgent::InjectVisualHighlightCSS(blink::WebDocument& document, 
                                              std::vector<std::pair<blink::WebElement, int>>& indexed_elements) {
  
  static const std::vector<std::string> colors = {
    "#FF0000", "#00FF00", "#0000FF", "#FFA500", "#800080", "#008080",
    "#FF69B4", "#4B0082", "#FF4500", "#2E8B57", "#DC143C", "#4682B4"
  };
  
  std::string all_css = R"(
    .automation-highlight {
      outline: 2px solid var(--highlight-color) !important;
      outline-offset: 1px !important;
      background: var(--highlight-bg) !important;
      position: relative !important;
    }
    
    .automation-highlight::after {
      content: attr(data-index) !important;
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

  // Add element-specific styles using CSS attribute selectors
  for (auto& pair : indexed_elements) {
    int index = pair.second;
    int color_index = index % colors.size();
    std::string base_color = colors[color_index];
    std::string bg_color = base_color + "1A";
    
    all_css += "[data-automation-index='" + std::to_string(index) + "'] { ";
    all_css += "--highlight-color: " + base_color + " !important; ";
    all_css += "--highlight-bg: " + bg_color + " !important; ";
    all_css += "}\n";
  }

  document.InsertStyleSheet(blink::WebString::FromUTF8(all_css));
  
  // Apply visual highlighting classes and data-index for CSS content
  for (auto& pair : indexed_elements) {
    blink::WebElement element = pair.first;
    int index = pair.second;
    
    std::string existing_class = element.GetAttribute("class").Utf8();
    std::string new_class = existing_class + " automation-highlight";
    
    element.SetAttribute("class", blink::WebString::FromUTF8(new_class));
    element.SetAttribute("data-index", blink::WebString::FromUTF8(std::to_string(index))); // For CSS content
  }
}

void AutomationAgent::CleanupPreviousHighlights(blink::WebDocument& document) {
  
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
    LOG(INFO) << "Previous highlights cleanup completed";
  } else {
    LOG(ERROR) << "Could not execute cleanup script - no frame available";
  }
}

bool AutomationAgent::IsElementDistinctInteraction(const blink::WebElement& element) {
  if (element.IsNull())
    return false;

  std::string tag = element.TagName().Utf8();
  std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);

  // Define elements that are always distinct interactions
  static const std::set<std::string> distinct_tags = {
    "a", "button", "input", "select", "textarea", "details", "summary", "label", "option"
  };

  if (distinct_tags.count(tag)) {
    return true;
  }

  // Check for interactive roles
  blink::WebString role_attr = element.GetAttribute(blink::WebString::FromUTF8("role"));
  if (!role_attr.IsEmpty()) {
    std::string role = role_attr.Utf8();
    static const std::set<std::string> distinct_roles = {
      "button", "link", "menuitem", "menuitemradio", "menuitemcheckbox",
      "radio", "checkbox", "tab", "switch", "slider", "spinbutton",
      "combobox", "searchbox", "textbox", "listbox", "option", "scrollbar"
    };
    
    if (distinct_roles.count(role)) {
      return true;
    }
  }

  // Check for contenteditable
  if (element.IsContentEditable() || 
      element.GetAttribute(blink::WebString::FromUTF8("contenteditable")).Utf8() == "true") {
    return true;
  }

  // Check for testing/automation attributes
  if (element.HasAttribute(blink::WebString::FromUTF8("data-testid")) ||
      element.HasAttribute(blink::WebString::FromUTF8("data-cy")) ||
      element.HasAttribute(blink::WebString::FromUTF8("data-test"))) {
    return true;
  }

  // Check for explicit onclick handler
  if (element.HasAttribute(blink::WebString::FromUTF8("onclick"))) {
    return true;
  }

  return false;
}

bool AutomationAgent::IsAncestorHighlighted(const blink::WebElement& element,
                                          const std::set<blink::WebElement>& highlighted_elements) {
  blink::WebNode current = element.ParentNode();  // Use ParentNode() instead
  
  while (!current.IsNull()) {
    if (current.IsElementNode()) {
      blink::WebElement current_element = current.To<blink::WebElement>();
      if (highlighted_elements.count(current_element) > 0) {
        return true;
      }
    }
    current = current.ParentNode();
  }
  
  return false;
}

std::string AutomationAgent::GenerateElementSelector(
  const blink::WebElement& element) {
if (element.IsNull()) {
  return "";
}

// Try ID first (most specific and unique)
blink::WebString id = element.GetAttribute(blink::WebString::FromUTF8("id"));
if (!id.IsEmpty()) {
  return "#" + id.Utf8();
}

// Try data-testid (great for automation)
blink::WebString testid =
    element.GetAttribute(blink::WebString::FromUTF8("data-testid"));
if (!testid.IsEmpty()) {
  return "[data-testid=\"" + testid.Utf8() + "\"]";
}

// Try aria-label for accessibility (good for buttons/interactive elements)
blink::WebString aria_label =
    element.GetAttribute(blink::WebString::FromUTF8("aria-label"));
if (!aria_label.IsEmpty()) {
  return "[aria-label=\"" + aria_label.Utf8() + "\"]";
}

// Try name for form elements
blink::WebString name =
    element.GetAttribute(blink::WebString::FromUTF8("name"));
if (!name.IsEmpty()) {
  return "[name=\"" + name.Utf8() + "\"]";
}

std::string tag = element.TagName().Utf8();
std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);

// For links, try href
if (tag == "a") {
  blink::WebString href =
      element.GetAttribute(blink::WebString::FromUTF8("href"));
  if (!href.IsEmpty()) {
    return "a[href=\"" + href.Utf8() + "\"]";
  }
}

// For inputs, include type
if (tag == "input") {
  blink::WebString type =
      element.GetAttribute(blink::WebString::FromUTF8("type"));
  if (!type.IsEmpty()) {
    return "input[type=\"" + type.Utf8() + "\"]";
  }
}

// Try role attribute
blink::WebString role =
    element.GetAttribute(blink::WebString::FromUTF8("role"));
if (!role.IsEmpty()) {
  return "[role=\"" + role.Utf8() + "\"]";
}

// Try class-based selector (but make it more specific)
blink::WebString class_attr =
    element.GetAttribute(blink::WebString::FromUTF8("class"));
if (!class_attr.IsEmpty()) {
  std::string classes = class_attr.Utf8();

  // Get first class
  size_t space_pos = classes.find(' ');
  std::string first_class = (space_pos != std::string::npos)
                                ? classes.substr(0, space_pos)
                                : classes;

  if (!first_class.empty()) {
    // Make it more specific by combining with tag
    return tag + "." + first_class;
  }
}

// For custom elements (like ytm-*), use the tag name directly
if (tag.find("-") != std::string::npos) {
  return tag;
}

// Last resort: try nth-child selector for better uniqueness
int position = 1;
blink::WebNode sibling = element.PreviousSibling();
while (!sibling.IsNull()) {
  if (sibling.IsElementNode()) {
    blink::WebElement sibling_element = sibling.To<blink::WebElement>();
    if (sibling_element.TagName().Utf8() == element.TagName().Utf8()) {
      position++;
    }
  }
  sibling = sibling.PreviousSibling();
}

return tag + ":nth-child(" + std::to_string(position) + ")";
}

}  // namespace automation