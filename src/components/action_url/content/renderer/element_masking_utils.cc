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
  
  DVLOG(2) << "Added warning to input element";
}

void ElementMaskingUtils::MaskContentElement(const blink::WebElement& element) {
  // For other elements, mask the content
  const_cast<blink::WebElement&>(element).MaskSensitiveContent(
      blink::WebString::FromUTF8(kMaskText));
  
  DVLOG(2) << "Masked element content with: " << kMaskText;
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
  
  return false;
}

bool ElementMaskingUtils::HasMaskingStyles(const blink::WebElement& element) {
  blink::WebString style_attr = element.GetAttribute("style");
  if (style_attr.IsNull()) {
    return false;
  }
  
  std::string style_str = style_attr.Utf8();
  
  // Look for our specific warning styles
  return (style_str.find("border: 2px solid #ff4444") != std::string::npos ||
          style_str.find("background-color: #fff5f5") != std::string::npos);
}

}  // namespace sensitive_masking