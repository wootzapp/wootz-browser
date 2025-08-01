// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/renderer/sensitive_element_mask_agent.h"

#include <algorithm>
#include <regex>
#include "base/logging.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_form_element.h"
#include "third_party/blink/public/web/web_input_element.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace sensitive_masking {

SensitiveElementMaskAgent::SensitiveElementMaskAgent(content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame) {
  DVLOG(1) << "Creating SensitiveElementMaskAgent for RenderFrame";
  
  // Register as sensitive element client
  if (render_frame && render_frame->GetWebFrame()) {
    render_frame->GetWebFrame()->SetSensitiveElementClient(this);
    DVLOG(1) << "Registered as sensitive element client";
  } else {
    LOG(ERROR) << "Cannot register sensitive element client - invalid render_frame or web_frame";
  }
  
  // Register Mojo interface with safety checks
  if (render_frame && render_frame->GetAssociatedInterfaceRegistry()) {
    render_frame->GetAssociatedInterfaceRegistry()->AddInterface<sensitive_masking::mojom::SensitiveElementMaskingDriver>(
        base::BindRepeating(&SensitiveElementMaskAgent::BindReceiver, weak_factory_.GetWeakPtr()));
    DVLOG(1) << "Mojo interface registered successfully";
  } else {
    LOG(ERROR) << "Cannot register Mojo interface - invalid render_frame or registry";
  }
}

SensitiveElementMaskAgent::~SensitiveElementMaskAgent() {
  // Reset Mojo receiver to prevent use-after-free
  if (receiver_.is_bound()) {
    receiver_.reset();
  }
  
  // Stop any pending timers
  retry_timer_.Stop();
  
  // Unregister client
  if (render_frame() && render_frame()->GetWebFrame()) {
    render_frame()->GetWebFrame()->SetSensitiveElementClient(nullptr);
  }
}

void SensitiveElementMaskAgent::OnDestruct() {
  // Clean up before destruction
  if (receiver_.is_bound()) {
    receiver_.reset();
  }
  retry_timer_.Stop();
  
  delete this;
}

void SensitiveElementMaskAgent::DidCommitProvisionalLoad(ui::PageTransition transition) {
  // Reset state on navigation
  retry_count_ = 0;
  retry_timer_.Stop();
  sensitive_selectors_.clear();
}

void SensitiveElementMaskAgent::DidFinishLoad() {
  // Mask elements on page load
  MaskStaticElements();
}

int SensitiveElementMaskAgent::MaskElementsWithSelectors() {
  if (!render_frame() || !render_frame()->GetWebFrame()) {
    LOG(ERROR) << "No render frame available";
    return 0;
  }

  blink::WebDocument document = render_frame()->GetWebFrame()->GetDocument();
  if (document.IsNull()) {
    LOG(ERROR) << "Document is null, cannot mask elements";
    return 0;
  }

  if (sensitive_selectors_.empty()) {
    DVLOG(2) << "No selectors configured, skipping masking";
    return 0;
  }

  // Set flag to prevent recursion during masking
  currently_masking_ = true;

  int masked_count = 0;
  
  // Iterate through all our selectors and find matching elements
  for (const auto& selector : sensitive_selectors_) {
    DVLOG(2) << "Looking for elements matching selector: " << selector;
    
    // Use QuerySelectorAll to find all elements matching this selector
    blink::WebVector<blink::WebElement> elements = document.QuerySelectorAll(blink::WebString::FromUTF8(selector));
    
    DVLOG(2) << "Found " << elements.size() << " elements for selector: " << selector;
    
    // Mask each found element
    for (const auto& element : elements) {
      if (!element.IsNull()) {
        // Skip if already masked to avoid re-processing
        std::string already_masked = element.GetAttribute("data-sensitive-masked").Utf8();
        if (already_masked == "true") {
          continue;
        }
        
        MaskElement(element);
        masked_count++;
        DVLOG(2) << "Masked element with selector: " << selector;
      }
    }
  }
  
  DVLOG(1) << "Finished masking elements, total masked: " << masked_count;
  
  // Reset flag to allow future masking operations
  currently_masking_ = false;
  
  return masked_count;
}

void SensitiveElementMaskAgent::MaskStaticElements() {
  DVLOG(1) << "Masking static elements on page load";
  
  if (!masking_enabled_) {
    DVLOG(1) << "Masking is disabled, skipping";
    return;
  }

  // Use the common masking logic
  int masked_count = MaskElementsWithSelectors();
  DVLOG(1) << "Static masking complete, masked " << masked_count << " elements";
  
  // If no elements were masked and we have retries left, schedule a retry
  if (masked_count == 0 && retry_count_ < kMaxRetries && !sensitive_selectors_.empty()) {
    DVLOG(1) << "No elements masked, scheduling retry " << (retry_count_ + 1) << "/" << kMaxRetries;
    ScheduleRetryMasking();
  }
}

void SensitiveElementMaskAgent::DidAddSensitiveElementDynamically(const blink::WebElement& element) {
  // Skip if masking is disabled
  if (!masking_enabled_) {
    return;
  }
  
  // Skip if element is null or we have no selectors
  if (element.IsNull() || sensitive_selectors_.empty()) {
    return;
  }
  
  // Prevent recursion during masking operations
  if (currently_masking_) {
    return;
  }
  
  // Skip elements that are likely created by our own masking process
  // to avoid infinite loops
  blink::WebString tag_name = element.TagName();
  if (tag_name.IsNull() || tag_name.IsEmpty()) {
    return; // Invalid element
  }
  
  std::string tag_name_str = tag_name.Utf8();
  if (tag_name_str == "DIV" || tag_name_str == "STRONG" || tag_name_str == "SPAN") {
    blink::WebString data_masked = element.GetAttribute("data-sensitive-masked");
    if (!data_masked.IsNull() && data_masked.Utf8() == "true") {
      return; // Skip elements we've already processed
    }
  }
  
  DVLOG(2) << "Element added dynamically: " << tag_name_str;
  
  // Check if this element matches our sensitive selectors using QuerySelectorAll
  // This is more reliable than manual matching for complex selectors
  if (!render_frame() || !render_frame()->GetWebFrame()) {
    return;
  }
  
  blink::WebDocument document = render_frame()->GetWebFrame()->GetDocument();
  if (document.IsNull()) {
    return;
  }
  
  // Check each selector to see if this element matches
  for (const auto& selector : sensitive_selectors_) {
    // Skip empty selectors
    if (selector.empty()) {
      continue;
    }
    
    DVLOG(3) << "Checking dynamic element against selector: " << selector;
    blink::WebVector<blink::WebElement> matching_elements = document.QuerySelectorAll(blink::WebString::FromUTF8(selector));
    
    // Check if the dynamically added element is in the results
    for (const auto& matching_element : matching_elements) {
      if (!matching_element.IsNull() && matching_element.Equals(element)) {
        DVLOG(2) << "Dynamic element matches selector: " << selector;
        MaskElement(element);
        return; // Found a match, no need to check other selectors
      }
    }
    
    // Try manual matching for simple cases as fallback
    if (DoesElementMatchSelector(element, selector)) {
      DVLOG(2) << "Element matches via manual matching, masking now";
      MaskElement(element);
      return;
    }
  }
}

void SensitiveElementMaskAgent::MaskElement(const blink::WebElement& element) const {
  // Safety check - ensure element is valid
  if (element.IsNull()) {
    LOG(ERROR) << "Cannot mask null element";
    return;
  }
  
  DVLOG(2) << "Masking element: " << element.TagName().Utf8();
  
  // Additional safety check - ensure we can access tag name
  blink::WebString tag_name = element.TagName();
  if (tag_name.IsNull() || tag_name.IsEmpty()) {
    LOG(ERROR) << "Element has invalid tag name";
    return;
  }
  
  // Mark element as processed to avoid infinite loops
  const_cast<blink::WebElement&>(element).SetAttribute("data-sensitive-masked", "true");
  
  // For input elements, show warning instead of masking
  if (element.HasHTMLTagName(blink::WebString::FromUTF8("input"))) {
    const_cast<blink::WebElement&>(element).ShowInputWarning();
    DVLOG(2) << "Added warning to input element";
  } else {
    // For other elements, use standard masking
    const_cast<blink::WebElement&>(element).MaskSensitiveContent(blink::WebString::FromUTF8("XXX"));
    DVLOG(2) << "Masked element with XXX";
  }
}

bool SensitiveElementMaskAgent::DoesElementMatchSelector(const blink::WebElement& element, const std::string& selector) const {
  if (selector.empty() || element.IsNull()) {
    return false;
  }
  
  // Handle simple tag selectors
  if (selector.find_first_of("#.[:]") == std::string::npos) {
    // Pure tag selector like "input", "div", "span"
    return element.HasHTMLTagName(blink::WebString::FromUTF8(selector));
  }
  
  // Handle ID selectors
  if (selector.starts_with("#")) {
    std::string target_id = selector.substr(1);
    std::string element_id = element.GetAttribute("id").Utf8();
    return (element_id == target_id);
  }
  
  // Handle class selectors
  if (selector.starts_with(".")) {
    std::string target_class = selector.substr(1);
    std::string element_class = element.GetAttribute("class").Utf8();
    return (element_class.find(target_class) != std::string::npos);
  }
  
  // Handle attribute selectors like span[data-type="employee-id"]
  if (selector.find('[') != std::string::npos && selector.find(']') != std::string::npos) {
    size_t tag_end = selector.find('[');
    std::string tag = selector.substr(0, tag_end);
    
    // Check tag first
    if (!tag.empty() && !element.HasHTMLTagName(blink::WebString::FromUTF8(tag))) {
      return false;
    }
    
    // Extract attribute part
    size_t attr_start = selector.find('[') + 1;
    size_t attr_end = selector.find(']');
    std::string attr_part = selector.substr(attr_start, attr_end - attr_start);
    
    // Handle attribute="value" format
    size_t equals_pos = attr_part.find('=');
    if (equals_pos != std::string::npos) {
      std::string attr_name = attr_part.substr(0, equals_pos);
      std::string attr_value = attr_part.substr(equals_pos + 1);
      
      // Remove quotes if present
      if (attr_value.front() == '"' && attr_value.back() == '"') {
        attr_value = attr_value.substr(1, attr_value.length() - 2);
      }
      
      std::string element_attr_value = element.GetAttribute(blink::WebString::FromUTF8(attr_name)).Utf8();
      return element_attr_value == attr_value;
    } else {
      // Handle attribute existence check like [data-type]
      std::string element_attr_value = element.GetAttribute(blink::WebString::FromUTF8(attr_part)).Utf8();
      return !element_attr_value.empty();
    }
  }
  
  // For complex selectors we can't handle manually, return false
  // (This will fall back to QuerySelectorAll approach)
  return false;
}

void SensitiveElementMaskAgent::UpdateMaskingSelectors(const std::vector<std::string>& selectors) {
  DVLOG(1) << "Updating masking selectors via Mojo, got " << selectors.size() << " new selectors";
  
  // Validate and filter selectors for security
  std::vector<std::string> validated_selectors;
  for (const auto& selector : selectors) {
    if (IsValidCSSSelector(selector)) {
      validated_selectors.push_back(selector);
    } else {
      LOG(WARNING) << "Rejecting invalid CSS selector: " << selector;
    }
  }
  
  sensitive_selectors_ = validated_selectors;
  
  // Reset retry count for new selectors
  retry_count_ = 0;
  retry_timer_.Stop(); // Cancel any pending retry
  
  int masked_count = 0;
  
  // Re-mask all elements with new selectors
  if (masking_enabled_) {
    DVLOG(1) << "Re-masking all elements with current selectors";
    masked_count = MaskElementsWithSelectors();
    
    // If no elements were masked, start retry mechanism
    if (masked_count == 0 && !sensitive_selectors_.empty()) {
      DVLOG(1) << "No elements masked via Mojo, starting retry mechanism";
      ScheduleRetryMasking();
    }
  }
  
  DVLOG(1) << "UpdateMaskingSelectors complete, masked " << masked_count << " elements";
}

void SensitiveElementMaskAgent::SetMaskingEnabled(bool enabled) {
  DVLOG(1) << "Setting masking enabled via Mojo: " << enabled;
  masking_enabled_ = enabled;
  
  if (enabled) {
    int masked_count = MaskElementsWithSelectors();
    DVLOG(1) << "Re-enabled masking, masked " << masked_count << " elements";
  }
  // Note: We don't unmask when disabled - that would be more complex
  // and might not be desired behavior
}

void SensitiveElementMaskAgent::ScheduleRetryMasking() {
  retry_count_++;
  DVLOG(1) << "Scheduling retry masking in " << kRetryDelay.InMilliseconds() << "ms (attempt " << retry_count_ << "/" << kMaxRetries << ")";
  
  retry_timer_.Start(
    FROM_HERE,
    kRetryDelay,
    base::BindOnce(&SensitiveElementMaskAgent::OnRetryMasking, weak_factory_.GetWeakPtr())
  );
}

void SensitiveElementMaskAgent::OnRetryMasking() {
  DVLOG(1) << "Retry masking attempt " << retry_count_ << "/" << kMaxRetries;
  
  if (!masking_enabled_) {
    DVLOG(1) << "Masking disabled, cancelling retry";
    return;
  }
  
  int masked_count = MaskElementsWithSelectors();
  DVLOG(1) << "Retry masking complete, masked " << masked_count << " elements";
  
  // If still no elements masked and we have retries left, schedule another retry
  if (masked_count == 0 && retry_count_ < kMaxRetries && !sensitive_selectors_.empty()) {
    DVLOG(1) << "Still no elements masked, scheduling another retry";
    ScheduleRetryMasking();
  } else if (masked_count > 0) {
    DVLOG(1) << "Retry successful! Found and masked " << masked_count << " elements";
    retry_count_ = 0; // Reset for future use
  } else {
    DVLOG(1) << "Max retries reached, giving up on masking";
    retry_count_ = 0; // Reset for future use
  }
}

void SensitiveElementMaskAgent::BindReceiver(mojo::PendingAssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver> receiver) {
  DVLOG(1) << "Binding Mojo receiver for SensitiveElementMaskingDriver";
  
  // Reset existing binding if any to prevent crashes
  if (receiver_.is_bound()) {
    DVLOG(1) << "Receiver already bound, resetting before rebinding";
    receiver_.reset();
  }
  
  // Bind with weak pointer for safety
  receiver_.Bind(std::move(receiver));
  
  // Set up disconnect handler to detect issues
  receiver_.set_disconnect_handler(base::BindOnce(
      [](SensitiveElementMaskAgent* self) {
        LOG(WARNING) << "SensitiveElementMaskAgent Mojo receiver disconnected";
      }, base::Unretained(this)));
  
  DVLOG(1) << "Mojo receiver bound successfully";
}

bool SensitiveElementMaskAgent::IsValidCSSSelector(const std::string& selector) const {
  // Basic security validation for CSS selectors
  if (selector.empty() || selector.length() > 200) {
    return false;
  }
  
  // Reject selectors with potentially dangerous content
  static const std::vector<std::string> dangerous_patterns = {
    "javascript:", "expression(", "eval(", "import", "@import", 
    "url(", "behavior:", "-moz-binding", "script", "<", ">", 
    "\"", "'", "\\", "/*", "*/"
  };
  
  std::string lower_selector = selector;
  std::transform(lower_selector.begin(), lower_selector.end(), 
                 lower_selector.begin(), ::tolower);
  
  for (const auto& pattern : dangerous_patterns) {
    if (lower_selector.find(pattern) != std::string::npos) {
      return false;
    }
  }
  
  // Allow only basic CSS selector characters
  static const std::regex allowed_pattern(R"(^[a-zA-Z0-9\-_\.\#\[\]="\s:,>+~\(\)]+$)");
  return std::regex_match(selector, allowed_pattern);
}

}  // namespace sensitive_masking 