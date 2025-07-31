// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/renderer/sensitive_element_mask_agent.h"

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
  LOG(INFO) << "AMIT: Creating SensitiveElementMaskAgent for RenderFrame";
  
  // Register as sensitive element client
  if (render_frame && render_frame->GetWebFrame()) {
    render_frame->GetWebFrame()->SetSensitiveElementClient(this);
    LOG(INFO) << "AMIT: Registered as sensitive element client";
  } else {
    LOG(ERROR) << "AMIT: Cannot register sensitive element client - invalid render_frame or web_frame";
  }
  
  // Register Mojo interface with safety checks
  if (render_frame && render_frame->GetAssociatedInterfaceRegistry()) {
    LOG(INFO) << "AMIT: Registering Mojo interface";
    render_frame->GetAssociatedInterfaceRegistry()->AddInterface<sensitive_masking::mojom::SensitiveElementMaskingDriver>(
        base::BindRepeating(&SensitiveElementMaskAgent::BindReceiver, weak_factory_.GetWeakPtr()));
    LOG(INFO) << "AMIT: Mojo interface registered successfully";
  } else {
    LOG(ERROR) << "AMIT: Cannot register Mojo interface - invalid render_frame or registry";
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
    LOG(ERROR) << "AMIT: No render frame available";
    return 0;
  }

  blink::WebDocument document = render_frame()->GetWebFrame()->GetDocument();
  if (document.IsNull()) {
    LOG(ERROR) << "AMIT: Document is null, cannot mask elements";
    return 0;
  }

  if (sensitive_selectors_.empty()) {
    LOG(INFO) << "AMIT: No selectors configured, skipping masking";
    return 0;
  }

  // Set flag to prevent recursion during masking
  currently_masking_ = true;

  int masked_count = 0;
  
  // Iterate through all our selectors and find matching elements
  for (const auto& selector : sensitive_selectors_) {
    LOG(INFO) << "AMIT: Looking for elements matching selector: " << selector;
    
    // Use QuerySelectorAll to find all elements matching this selector
    blink::WebVector<blink::WebElement> elements = document.QuerySelectorAll(blink::WebString::FromUTF8(selector));
    
    LOG(INFO) << "AMIT: Found " << elements.size() << " elements for selector: " << selector;
    
    // Log if no elements found for debugging
    if (elements.size() == 0) {
      LOG(INFO) << "AMIT: No elements found for selector: " << selector;
    }
    
    // Mask each found element
    for (const auto& element : elements) {
      if (!element.IsNull()) {
        // Skip if already masked to avoid re-processing
        std::string already_masked = element.GetAttribute("data-sensitive-masked").Utf8();
        if (already_masked == "true") {
          LOG(INFO) << "AMIT: Skipping already masked element";
          continue;
        }
        
        MaskElement(element);
        masked_count++;
        LOG(INFO) << "AMIT: Masked element with selector: " << selector;
      }
    }
  }
  
  LOG(INFO) << "AMIT: Finished masking elements, total masked: " << masked_count;
  
  // Reset flag to allow future masking operations
  currently_masking_ = false;
  
  return masked_count;
}

void SensitiveElementMaskAgent::MaskStaticElements() {
  LOG(INFO) << "AMIT: Masking static elements on page load";
  
  if (!masking_enabled_) {
    LOG(INFO) << "AMIT: Masking is disabled, skipping";
    return;
  }

  // Use the common masking logic
  int masked_count = MaskElementsWithSelectors();
  LOG(INFO) << "AMIT: Static masking complete, masked " << masked_count << " elements";
  
  // If no elements were masked and we have retries left, schedule a retry
  if (masked_count == 0 && retry_count_ < kMaxRetries && !sensitive_selectors_.empty()) {
    LOG(INFO) << "AMIT: No elements masked, scheduling retry " << (retry_count_ + 1) << "/" << kMaxRetries;
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
  
  LOG(INFO) << "AMIT: Element added dynamically: " << tag_name_str;
  
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
    
    LOG(INFO) << "AMIT: Checking dynamic element against selector: " << selector;
    blink::WebVector<blink::WebElement> matching_elements = document.QuerySelectorAll(blink::WebString::FromUTF8(selector));
    LOG(INFO) << "AMIT: QuerySelectorAll found " << matching_elements.size() << " elements for selector: " << selector;
    
    // Check if the dynamically added element is in the results
    for (const auto& matching_element : matching_elements) {
      if (!matching_element.IsNull() && matching_element.Equals(element)) {
        LOG(INFO) << "AMIT: Dynamic element matches selector: " << selector;
        MaskElement(element);
        return; // Found a match, no need to check other selectors
      }
    }
    
    // Debug: Let's also try a different approach - check if the element matches the selector directly
    LOG(INFO) << "AMIT: Element didn't match via QuerySelectorAll comparison for selector: " << selector;
    LOG(INFO) << "AMIT: Element tag: " << element.TagName().Utf8() 
              << ", id: '" << element.GetAttribute("id").Utf8() 
              << "', class: '" << element.GetAttribute("class").Utf8() 
              << "', data-type: '" << element.GetAttribute("data-type").Utf8() << "'";
    
    // Try manual matching for simple cases to see if that works
    if (DoesElementMatchSelector(element, selector)) {
      LOG(INFO) << "AMIT: Element matches via manual matching! Masking now.";
      MaskElement(element);
      return;
    }
    
    LOG(INFO) << "AMIT: Element doesn't match selector '" << selector << "' via any method";
  }
}



void SensitiveElementMaskAgent::MaskElement(const blink::WebElement& element) const {
  // Safety check - ensure element is valid
  if (element.IsNull()) {
    LOG(ERROR) << "AMIT: Cannot mask null element";
    return;
  }
  
  LOG(INFO) << "AMIT: Masking element: " << element.TagName().Utf8();
  
  // Additional safety check - ensure we can access tag name
  blink::WebString tag_name = element.TagName();
  if (tag_name.IsNull() || tag_name.IsEmpty()) {
    LOG(ERROR) << "AMIT: Element has invalid tag name";
    return;
  }
  
  // Mark element as processed to avoid infinite loops
  const_cast<blink::WebElement&>(element).SetAttribute("data-sensitive-masked", "true");
  
  // For input elements, show warning instead of masking
  if (element.HasHTMLTagName(blink::WebString::FromUTF8("input"))) {
    const_cast<blink::WebElement&>(element).ShowInputWarning();
    LOG(INFO) << "AMIT: Added warning to input element";
  } else {
    // For other elements, use standard masking
    const_cast<blink::WebElement&>(element).MaskSensitiveContent(blink::WebString::FromUTF8("XXX"));
    LOG(INFO) << "AMIT: Masked element with XXX";
  }
}

bool SensitiveElementMaskAgent::DoesElementMatchSelector(const blink::WebElement& element, const std::string& selector) const {
  if (selector.empty() || element.IsNull()) {
    LOG(INFO) << "AMIT: Manual matching failed - empty selector or null element";
    return false;
  }
  
  LOG(INFO) << "AMIT: Manual matching - checking element " << element.TagName().Utf8() << " against selector: " << selector;
  
  // Handle simple tag selectors
  if (selector.find_first_of("#.[:]") == std::string::npos) {
    // Pure tag selector like "input", "div", "span"
    bool matches = element.HasHTMLTagName(blink::WebString::FromUTF8(selector));
    LOG(INFO) << "AMIT: Tag selector '" << selector << "' matches: " << (matches ? "YES" : "NO");
    return matches;
  }
  
  // Handle ID selectors
  if (selector.starts_with("#")) {
    std::string target_id = selector.substr(1);
    std::string element_id = element.GetAttribute("id").Utf8();
    bool matches = (element_id == target_id);
    LOG(INFO) << "AMIT: ID selector '" << selector << "' (target: '" << target_id << "', element: '" << element_id << "') matches: " << (matches ? "YES" : "NO");
    return matches;
  }
  
  // Handle class selectors
  if (selector.starts_with(".")) {
    std::string target_class = selector.substr(1);
    std::string element_class = element.GetAttribute("class").Utf8();
    bool matches = (element_class.find(target_class) != std::string::npos);
    LOG(INFO) << "AMIT: Class selector '" << selector << "' (target: '" << target_class << "', element: '" << element_class << "') matches: " << (matches ? "YES" : "NO");
    return matches;
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
  LOG(INFO) << "AMIT: Complex selector '" << selector << "' - cannot handle manually";
  return false;
}

void SensitiveElementMaskAgent::UpdateMaskingSelectors(const std::vector<std::string>& selectors) {
  LOG(INFO) << "AMIT: Updating masking selectors via Mojo, got " << selectors.size() << " new selectors";
  
  sensitive_selectors_ = selectors;
  
  // Reset retry count for new selectors
  retry_count_ = 0;
  retry_timer_.Stop(); // Cancel any pending retry
  
  for (const auto& selector : selectors) {
    LOG(INFO) << "AMIT: New selector: " << selector;
  }
  
  int masked_count = 0;
  
  // Re-mask all elements with new selectors
  if (masking_enabled_) {
    LOG(INFO) << "AMIT: Re-masking all elements with current selectors";
    masked_count = MaskElementsWithSelectors();
    
    // If no elements were masked, start retry mechanism
    if (masked_count == 0 && !sensitive_selectors_.empty()) {
      LOG(INFO) << "AMIT: No elements masked via Mojo, starting retry mechanism";
      ScheduleRetryMasking();
    }
  }
  
  LOG(INFO) << "AMIT: UpdateMaskingSelectors complete, masked " << masked_count << " elements";
  
  // No callback to call - simplified like replace_element
}

void SensitiveElementMaskAgent::SetMaskingEnabled(bool enabled) {
  LOG(INFO) << "AMIT: Setting masking enabled via Mojo: " << enabled;
  masking_enabled_ = enabled;
  
  if (enabled) {
    int masked_count = MaskElementsWithSelectors();
    LOG(INFO) << "AMIT: Re-enabled masking, masked " << masked_count << " elements";
  }
  // Note: We don't unmask when disabled - that would be more complex
  // and might not be desired behavior
}

void SensitiveElementMaskAgent::ScheduleRetryMasking() {
  retry_count_++;
  LOG(INFO) << "AMIT: Scheduling retry masking in " << kRetryDelay.InMilliseconds() << "ms (attempt " << retry_count_ << "/" << kMaxRetries << ")";
  
  retry_timer_.Start(
    FROM_HERE,
    kRetryDelay,
    base::BindOnce(&SensitiveElementMaskAgent::OnRetryMasking, base::Unretained(this))
  );
}

void SensitiveElementMaskAgent::OnRetryMasking() {
  LOG(INFO) << "AMIT: Retry masking attempt " << retry_count_ << "/" << kMaxRetries;
  
  if (!masking_enabled_) {
    LOG(INFO) << "AMIT: Masking disabled, cancelling retry";
    return;
  }
  
  int masked_count = MaskElementsWithSelectors();
  LOG(INFO) << "AMIT: Retry masking complete, masked " << masked_count << " elements";
  
  // If still no elements masked and we have retries left, schedule another retry
  if (masked_count == 0 && retry_count_ < kMaxRetries && !sensitive_selectors_.empty()) {
    LOG(INFO) << "AMIT: Still no elements masked, scheduling another retry";
    ScheduleRetryMasking();
  } else if (masked_count > 0) {
    LOG(INFO) << "AMIT: Retry successful! Found and masked " << masked_count << " elements";
    retry_count_ = 0; // Reset for future use
  } else {
    LOG(INFO) << "AMIT: Max retries reached, giving up on masking";
    retry_count_ = 0; // Reset for future use
  }
}



void SensitiveElementMaskAgent::BindReceiver(mojo::PendingAssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver> receiver) {
  LOG(INFO) << "AMIT: Binding Mojo receiver for SensitiveElementMaskingDriver";
  
  // Reset existing binding if any to prevent crashes
  if (receiver_.is_bound()) {
    LOG(INFO) << "AMIT: Receiver already bound, resetting before rebinding";
    receiver_.reset();
  }
  
  // Bind with weak pointer for safety
  receiver_.Bind(std::move(receiver));
  
  // Set up disconnect handler to detect issues
  receiver_.set_disconnect_handler(base::BindOnce(
      [](SensitiveElementMaskAgent* self) {
        LOG(WARNING) << "AMIT: SensitiveElementMaskAgent Mojo receiver disconnected";
      }, base::Unretained(this)));
  
  LOG(INFO) << "AMIT: Mojo receiver bound successfully";
}

}  // namespace sensitive_masking 