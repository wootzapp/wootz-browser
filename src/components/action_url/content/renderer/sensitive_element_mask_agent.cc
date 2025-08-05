// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/renderer/sensitive_element_mask_agent.h"

#include "base/auto_reset.h"
#include "base/logging.h"
#include "components/action_url/content/renderer/css_selector_matcher.h"
#include "components/action_url/content/renderer/element_masking_utils.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"

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
  
  // Clear loop prevention state
  dynamic_masking_depth_ = 0;
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
    LOG(INFO) << "[Renderer][Masking] No selectors configured, skipping masking";
    return 0;
  }

  LOG(INFO) << "[Renderer][Masking] Starting to mask elements with " << sensitive_selectors_.size() << " selectors";

  // Set flag to prevent recursion during masking
  currently_masking_ = true;

  int masked_count = 0;
  
  // Iterate through all our selectors and find matching elements
  for (const auto& selector : sensitive_selectors_) {
    LOG(INFO) << "[Renderer][Masking] Looking for elements matching selector: " << selector;
    
    // Use QuerySelectorAll to find all elements matching this selector
    blink::WebVector<blink::WebElement> elements = document.QuerySelectorAll(blink::WebString::FromUTF8(selector));
    
    LOG(INFO) << "[Renderer][Masking] Found " << elements.size() << " elements for selector: " << selector;
    
    // Mask each found element
    for (const auto& element : elements) {
      if (!element.IsNull()) {
        // Skip if already masked to avoid re-processing
        if (ElementMaskingUtils::IsElementCreatedByMasking(element)) {
          continue;
        }
        
        ElementMaskingUtils::MaskElement(element);
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
  // Early validation
  if (!masking_enabled_ || !ElementMaskingUtils::IsElementValid(element) || 
      sensitive_selectors_.empty()) {
    return;
  }
  
  // Safety checks and loop prevention
  if (!ShouldProcessElement(element) || !IsWithinProcessingLimits()) {
    return;
  }
  
  // Process with safety guards
  base::AutoReset<int> depth_guard(&dynamic_masking_depth_, dynamic_masking_depth_ + 1);
  base::AutoReset<bool> masking_guard(&currently_masking_, true);
  
  ProcessDynamicElement(element);
}

void SensitiveElementMaskAgent::ProcessDynamicElement(const blink::WebElement& element) {
  DVLOG(2) << "Processing dynamic element: " << element.TagName().Utf8();
  
  // Validate frame and document
  if (!render_frame() || !render_frame()->GetWebFrame()) {
    return;
  }
  
  blink::WebDocument document = render_frame()->GetWebFrame()->GetDocument();
  if (document.IsNull()) {
    return;
  }
  
  // Check each selector with optimized matching
  for (const auto& selector : sensitive_selectors_) {
    if (selector.empty()) {
      continue;
    }
    
    // Try fast pattern matching first
    if (CssSelectorMatcher::DoesElementMatchSelector(element, selector)) {
      DVLOG(2) << "Element matches selector via pattern matching: " << selector;
      ElementMaskingUtils::MaskElement(element);
      return;
    }
    
    // Fallback to QuerySelectorAll for complex selectors
    DVLOG(3) << "Checking element against complex selector: " << selector;
    blink::WebVector<blink::WebElement> matching_elements = 
        document.QuerySelectorAll(blink::WebString::FromUTF8(selector));
    
    for (const auto& matching_element : matching_elements) {
      if (!matching_element.IsNull() && matching_element.Equals(element)) {
        DVLOG(2) << "Element matches selector via QuerySelectorAll: " << selector;
        ElementMaskingUtils::MaskElement(element);
        return;
      }
    }
  }
}

void SensitiveElementMaskAgent::UpdateMaskingSelectors(const std::vector<std::string>& selectors) {
  LOG(INFO) << "[Renderer][Masking] UpdateMaskingSelectors called with " << selectors.size() << " selectors";
  for (const auto& selector : selectors) {
    LOG(INFO) << "[Renderer][Masking] Received selector: " << selector;
  }
  
  // Validate and filter selectors for security
  sensitive_selectors_ = CssSelectorMatcher::ValidateSelectors(selectors);
  LOG(INFO) << "[Renderer][Masking] After validation, " << sensitive_selectors_.size() << " selectors remain";
  for (const auto& selector : sensitive_selectors_) {
    LOG(INFO) << "[Renderer][Masking] Valid selector: " << selector;
  }
  
  // Reset retry count for new selectors
  retry_count_ = 0;
  retry_timer_.Stop(); // Cancel any pending retry
  
  int masked_count = 0;
  
  // Re-mask all elements with new selectors
  LOG(INFO) << "[Renderer][Masking] Masking enabled: " << masking_enabled_;
  if (masking_enabled_) {
    LOG(INFO) << "[Renderer][Masking] Re-masking all elements with current selectors";
    masked_count = MaskElementsWithSelectors();
    
    // If no elements were masked, start retry mechanism
    if (masked_count == 0 && !sensitive_selectors_.empty()) {
      DVLOG(1) << "No elements masked via Mojo, starting retry mechanism";
      ScheduleRetryMasking();
    }
  }
  
  LOG(INFO) << "[Renderer][Masking] UpdateMaskingSelectors complete, masked " << masked_count << " elements";
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
      [](base::WeakPtr<SensitiveElementMaskAgent> self) {
        if (self) {
          LOG(WARNING) << "SensitiveElementMaskAgent Mojo receiver disconnected";
        }
      }, weak_factory_.GetWeakPtr()));
  
  DVLOG(1) << "Mojo receiver bound successfully";
}

bool SensitiveElementMaskAgent::ShouldProcessElement(const blink::WebElement& element) const {
  // Check if element was created by our masking process
  if (ElementMaskingUtils::IsElementCreatedByMasking(element)) {
    DVLOG(3) << "Element created by masking process, skipping";
    return false;
  }
  
  // Check if we're currently in a masking operation
  if (currently_masking_) {
    DVLOG(3) << "Currently masking, skipping dynamic element";
    return false;
  }
  
  return true;
}

bool SensitiveElementMaskAgent::IsWithinProcessingLimits() const {
  if (dynamic_masking_depth_ >= kMaxDynamicMaskingDepth) {
    DVLOG(1) << "Maximum dynamic masking depth reached, skipping element";
    return false;
  }
  
  return true;
}

}  // namespace sensitive_masking 