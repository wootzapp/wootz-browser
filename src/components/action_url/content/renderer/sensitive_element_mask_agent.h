// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_RENDERER_SENSITIVE_ELEMENT_MASK_AGENT_H_
#define COMPONENTS_ACTION_URL_CONTENT_RENDERER_SENSITIVE_ELEMENT_MASK_AGENT_H_

#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"
#include "components/action_url/content/common/mojom/sensitive_element_masking.mojom.h"
#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_sensitive_element_client.h"
#include "ui/base/page_transition_types.h"

namespace sensitive_masking {

// Main agent responsible for detecting and masking sensitive elements in web pages.
// 
// This class integrates with Blink's rendering pipeline to:
// - Detect dynamically added sensitive elements via WebSensitiveElementClient
// - Receive masking commands from browser process via Mojo
// - Apply appropriate masking/warning to sensitive form fields and content
// - Prevent infinite loops through depth limiting and element tracking
//
// The agent uses specialized utility classes for pattern matching and element
// manipulation to maintain clean separation of concerns.
class SensitiveElementMaskAgent final 
    : public content::RenderFrameObserver,
      public blink::WebSensitiveElementClient,
      public sensitive_masking::mojom::SensitiveElementMaskingDriver {
 public:
  explicit SensitiveElementMaskAgent(content::RenderFrame* render_frame);
  ~SensitiveElementMaskAgent() override;

  // Disable copy and assignment
  SensitiveElementMaskAgent(const SensitiveElementMaskAgent&) = delete;
  SensitiveElementMaskAgent& operator=(const SensitiveElementMaskAgent&) = delete;

  // content::RenderFrameObserver implementation
  void OnDestruct() override;
  void DidCommitProvisionalLoad(ui::PageTransition transition) override;
  void DidFinishLoad() override;

  // blink::WebSensitiveElementClient implementation
  void DidAddSensitiveElementDynamically(const blink::WebElement& element) override;

  // sensitive_masking::mojom::SensitiveElementMaskingDriver implementation
  void UpdateMaskingSelectors(const std::vector<std::string>& selectors) override;
  void SetMaskingEnabled(bool enabled) override;

  // Binds this agent to receive Mojo messages from browser process
  void BindReceiver(mojo::PendingAssociatedReceiver<
      sensitive_masking::mojom::SensitiveElementMaskingDriver> receiver);

 private:
  // Core masking operations
  void MaskStaticElements();
  int MaskElementsWithSelectors();
  void ProcessDynamicElement(const blink::WebElement& element);

  // Loop prevention and safety checks
  bool ShouldProcessElement(const blink::WebElement& element) const;
  bool IsWithinProcessingLimits() const;

  // Retry mechanism for delayed content
  void ScheduleRetryMasking();
  void OnRetryMasking();

  // State management
  std::vector<std::string> sensitive_selectors_;
  bool masking_enabled_ = true;
  bool currently_masking_ = false;
  
  // Loop prevention
  int dynamic_masking_depth_ = 0;
  static constexpr int kMaxDynamicMaskingDepth = 3;

  // Retry mechanism
  base::OneShotTimer retry_timer_;
  int retry_count_ = 0;
  static constexpr int kMaxRetries = 5;
  static constexpr base::TimeDelta kRetryDelay = base::Milliseconds(1000);

  // Mojo communication
  mojo::AssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver> 
      receiver_{this};
  
  // Must be last member for safe async operations
  base::WeakPtrFactory<SensitiveElementMaskAgent> weak_factory_{this};
};

}  // namespace sensitive_masking

#endif  // COMPONENTS_ACTION_URL_CONTENT_RENDERER_SENSITIVE_ELEMENT_MASK_AGENT_H_ 