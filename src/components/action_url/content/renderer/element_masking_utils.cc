// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/renderer/element_masking_utils.h"

#include "base/logging.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_document.h"

namespace sensitive_masking {

void ElementMaskingUtils::MaskElement(const blink::WebElement& element) {
  if (!IsElementValid(element)) {
    LOG(ERROR) << "Cannot mask invalid element";
    return;
  }
  
  DVLOG(2) << "Masking element: " << element.TagName().Utf8();
  
  // Mark element as processed first to prevent infinite loops
  MarkElementAsMasked(element);
  
  // Apply appropriate masking based on element type
  if (element.HasHTMLTagName(blink::WebString::FromUTF8("input"))) {
    MaskInputElement(element);
  } else {
    MaskContentElement(element);
  }
}

bool ElementMaskingUtils::IsElementCreatedByMasking(const blink::WebElement& element) {
  if (!IsElementValid(element)) {
    return false;
  }
  
  // Check for masking attributes
  if (HasMaskingAttributes(element)) {
    return true;
  }
  
  // Check for masking-specific styles
  if (HasMaskingStyles(element)) {
    return true;
  }
  
  return false;
}

bool ElementMaskingUtils::IsElementValid(const blink::WebElement& element) {
  if (element.IsNull()) {
    return false;
  }
  
  blink::WebString tag_name = element.TagName();
  if (tag_name.IsNull() || tag_name.IsEmpty()) {
    return false;
  }
  
  return true;
}

void ElementMaskingUtils::MaskInputElement(const blink::WebElement& element) {
  // For input elements, show warning instead of masking content
  const_cast<blink::WebElement&>(element).ShowInputWarning(
    blink::WebString::FromUTF8(kWarningText));
  
  // Add warning attribute for detection
  const_cast<blink::WebElement&>(element).SetAttribute(
      kWarningAttribute, "true");
  
  DVLOG(2) << "Added clean warning below input element";
}

void ElementMaskingUtils::MaskContentElement(const blink::WebElement& element) {
  // Create enhanced masked container with toggle functionality
  CreateMaskedContainer(element);
  
  DVLOG(2) << "Created masked container with toggle for element: " << element.TagName().Utf8();
}

void ElementMaskingUtils::MarkElementAsMasked(const blink::WebElement& element) {
  const_cast<blink::WebElement&>(element).SetAttribute(
      kMaskedAttribute, "true");
}

bool ElementMaskingUtils::HasMaskingAttributes(const blink::WebElement& element) {
  // Check for our masking marker attribute
  blink::WebString data_masked = element.GetAttribute(kMaskedAttribute);
  if (!data_masked.IsNull() && data_masked.Utf8() == "true") {
    return true;
  }
  
  // Check for warning marker attribute
  blink::WebString data_warning = element.GetAttribute(kWarningAttribute);
  if (!data_warning.IsNull() && data_warning.Utf8() == "true") {
    return true;
  }
  
  // Check for masked container attribute
  blink::WebString data_container = element.GetAttribute(kMaskedContainerAttr);
  if (!data_container.IsNull() && data_container.Utf8() == "true") {
    return true;
  }
  
  // Check for toggle button attribute
  blink::WebString data_toggle = element.GetAttribute(kToggleButtonAttr);
  if (!data_toggle.IsNull() && data_toggle.Utf8() == "true") {
    return true;
  }
  
  // Check for warning container attribute
  blink::WebString data_warning_container = element.GetAttribute(kWarningContainerAttr);
  if (!data_warning_container.IsNull() && data_warning_container.Utf8() == "true") {
    return true;
  }
  
  return false;
}

bool ElementMaskingUtils::HasMaskingStyles(const blink::WebElement& element) {
  blink::WebString style_attr = element.GetAttribute("style");
  if (style_attr.IsNull()) {
    return false;
  }
  
  std::string style_str = style_attr.Utf8();
  
  // Look for our masking styles, universal toggle button styles, and input warning styles
  return (style_str.find("box-shadow: inset 1px 0 0 rgba(255, 149, 0, 0.1)") != std::string::npos ||
          style_str.find("color: #86868b") != std::string::npos ||
          style_str.find("font-family: -apple-system") != std::string::npos ||
          style_str.find("background: linear-gradient(135deg, #ff6b6b 0%, #ff8e53 100%)") != std::string::npos ||
          style_str.find("position: fixed") != std::string::npos ||
          style_str.find("top: 20px") != std::string::npos ||
          style_str.find("font-size: 9px") != std::string::npos ||
          style_str.find("color: #999") != std::string::npos);
}

void ElementMaskingUtils::CreateMaskedContainer(const blink::WebElement& element) {
  // Store original content in attribute for later retrieval
  blink::WebString original_content = element.InnerHTML();
  const_cast<blink::WebElement&>(element).SetAttribute(kOriginalContentAttr, original_content);
  
  // Create clean masked UI with hidden original content for universal toggle
  std::string masked_html = R"(
    <div data-masked-container="true" style="display: inline-block;">
      <div class="masked-display" style="
        display: inline-flex; 
        align-items: center; 
        gap: 6px; 
        background: linear-gradient(90deg, #f0f0f0 0%, #e8e8e8 100%); 
        border: 1px solid #ddd; 
        border-radius: 6px; 
        padding: 6px 12px; 
        font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; 
        font-size: 13px; 
        color: #666; 
        box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        outline: none !important;
        background-color: transparent !important;
      ">
        <span style="font-weight: 500;">🔒 Sensitive information</span>
      </div>
      <div class="original-content" style="display: none;">)" + original_content.Utf8() + R"(</div>
    </div>
  )";
  
  // Replace element content with masked UI
  const_cast<blink::WebElement&>(element).SetInnerHTML(
      blink::WebString::FromUTF8(masked_html));
  
  // Inject universal toggle button (only once per document)
  blink::WebDocument document = element.GetDocument();
  if (!document.IsNull()) {
    blink::WebElement existing_controller = document.GetElementById("universal-mask-toggle");
    if (existing_controller.IsNull()) {
      // Create button with inline toggle logic to avoid script injection issues
      std::string button_html = 
        "<div id=\"universal-mask-toggle\" style=\""
        "position: fixed; bottom: 20px; right: 20px; z-index: 9999; "
        "opacity: 0.8; transition: opacity 0.3s ease;\" "
        "ontouchstart=\"this.style.opacity='1'\" ontouchend=\"this.style.opacity='0.8'\">"
        "<button id=\"mask-toggle-btn\" ontouchstart=\""
        "var containers = document.querySelectorAll('[data-masked-container]');"
        "var button = this;"
        "var state = button.getAttribute('data-state') || 'hidden';"
        "containers.forEach(function(container) {"
        "var maskedDiv = container.querySelector('.masked-display');"
        "var originalDiv = container.querySelector('.original-content');"
        "if (state === 'hidden') {"
        "if (maskedDiv) maskedDiv.style.display = 'none';"
        "if (originalDiv) originalDiv.style.display = 'inline-block';"
        "} else {"
        "if (maskedDiv) maskedDiv.style.display = 'inline-flex';"
        "if (originalDiv) originalDiv.style.display = 'none';"
        "}});"
        "if (state === 'hidden') {"
        "button.setAttribute('data-state', 'visible'); button.innerHTML = 'Hide';"
        "} else {"
        "button.setAttribute('data-state', 'hidden'); button.innerHTML = 'View';"
        "}\" style=\""
        "background: #ff6b6b; "
        "color: white; border: none; border-radius: 8px; "
        "padding: 12px 16px; font-size: 14px; font-weight: 600; "
        "box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2); "
        "transition: all 0.2s ease; "
        "font-family: -apple-system, BlinkMacSystemFont, system-ui, sans-serif; "
        "min-width: 70px; min-height: 40px; touch-action: manipulation;"
        "data-state=\"hidden\">"
        "View</button></div>";
      
      // Inject button directly
      blink::WebElement body = document.Body();
      if (!body.IsNull()) {
        std::string current_body = body.InnerHTML().Utf8();
        current_body += button_html;
        const_cast<blink::WebElement&>(body).SetInnerHTML(
            blink::WebString::FromUTF8(current_body));
      }
    }
  }
  
  DVLOG(2) << "Created clean masked container with universal toggle capability";
}

void ElementMaskingUtils::CreateInputWarning(const blink::WebElement& element) {
  // Add warning attribute for identification
  const_cast<blink::WebElement&>(element).SetAttribute(
      "data-warning-text", blink::WebString::FromUTF8(kWarningText));
  
  // Create a small text warning below the input field
  blink::WebDocument document = element.GetDocument();
  if (!document.IsNull()) {
    // Check if we already have a warning text element
    blink::WebElement existing_warning = document.GetElementById("input-warning-text");
    if (existing_warning.IsNull()) {
      std::string warning_text_html = 
        "<div id=\"input-warning-text\" style=\""
        "font-size: 9px; color: #999; margin-top: 2px; "
        "font-family: -apple-system, BlinkMacSystemFont, system-ui, sans-serif;\">"
        "Sensitive information</div>";
      
      // Append to document body
      blink::WebElement body = document.Body();
      if (!body.IsNull()) {
        std::string current_body = body.InnerHTML().Utf8();
        current_body += warning_text_html;
        const_cast<blink::WebElement&>(body).SetInnerHTML(
            blink::WebString::FromUTF8(current_body));
      }
    }
  }
  
  DVLOG(2) << "Added small text warning below input element";
}

void ElementMaskingUtils::ToggleMaskedVisibility(const blink::WebElement& toggle_button) {
  // This method can be called from C++ if needed
  // The actual toggle logic is handled by the JavaScript onclick handler
  DVLOG(2) << "Toggle visibility called";
}

blink::WebElement ElementMaskingUtils::FindMaskedContainer(const blink::WebElement& element) {
  // Check if this element itself is a masked container
  if (!element.GetAttribute(kMaskedContainerAttr).IsNull()) {
    return element;
  }
  
  // For now, return null element if not found directly
  // The innerHTML approach makes DOM traversal less necessary
  return blink::WebElement();
}

}  // namespace sensitive_masking