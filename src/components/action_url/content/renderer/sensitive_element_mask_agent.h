// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_RENDERER_SENSITIVE_ELEMENT_MASK_AGENT_H_
#define COMPONENTS_ACTION_URL_CONTENT_RENDERER_SENSITIVE_ELEMENT_MASK_AGENT_H_

#include <vector>
#include <string>

#include "base/timer/timer.h"
#include "base/memory/weak_ptr.h"
#include "content/public/renderer/render_frame_observer.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_node.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_sensitive_element_client.h"
#include "ui/base/page_transition_types.h"
#include "components/action_url/content/common/mojom/sensitive_element_masking.mojom.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace sensitive_masking {

class SensitiveElementMaskAgent : public content::RenderFrameObserver,
                                  public blink::WebSensitiveElementClient,
                                  public sensitive_masking::mojom::SensitiveElementMaskingDriver {
 public:
  explicit SensitiveElementMaskAgent(content::RenderFrame* render_frame);
  ~SensitiveElementMaskAgent() override;

  // RenderFrameObserver implementation
  void OnDestruct() override;
  void DidCommitProvisionalLoad(ui::PageTransition transition) override;
  void DidFinishLoad() override;

  // WebSensitiveElementClient implementation
  void DidAddSensitiveElementDynamically(const blink::WebElement& element) override;

  // SensitiveElementMaskingDriver Mojo interface
  void UpdateMaskingSelectors(const std::vector<std::string>& selectors) override;
  void SetMaskingEnabled(bool enabled) override;

  // Bind this agent to receive Mojo messages
  void BindReceiver(mojo::PendingAssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver> receiver);

 private:
  // Dynamic list of sensitive selectors (updated from extension)
  std::vector<std::string> sensitive_selectors_;

  bool masking_enabled_ = true;
  bool currently_masking_ = false;

  // Mask elements on page load
  void MaskStaticElements();
  
  // Check if element should be masked

  
  // Mask a single element
  void MaskElement(const blink::WebElement& element) const;
  
  // Helper to check if an element matches a CSS selector manually
  bool DoesElementMatchSelector(const blink::WebElement& element, const std::string& selector) const;
  
  // Security validation for CSS selectors
  bool IsValidCSSSelector(const std::string& selector) const;

  // Mask all elements matching current selectors and return count
  int MaskElementsWithSelectors();
  
  // Retry mechanism for delayed content
  void ScheduleRetryMasking();
  void OnRetryMasking();
  base::OneShotTimer retry_timer_;
  int retry_count_ = 0;
  static constexpr int kMaxRetries = 5;
  static constexpr base::TimeDelta kRetryDelay = base::Milliseconds(1000);

  mojo::AssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver> receiver_{this};
  
  // WeakPtr factory for safe async operations (must be last member)
  base::WeakPtrFactory<SensitiveElementMaskAgent> weak_factory_{this};
};

}  // namespace sensitive_masking

#endif  // COMPONENTS_ACTION_URL_CONTENT_RENDERER_SENSITIVE_ELEMENT_MASK_AGENT_H_ 