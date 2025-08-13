// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/browser/content_sensitive_masking_driver.h"

#include <utility>

#include "components/action_url/content/browser/content_sensitive_masking_driver_factory.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace sensitive_masking {

namespace {

bool IsRenderFrameHostSupported(content::RenderFrameHost* rfh) {
  if (rfh->GetLifecycleState() ==
      content::RenderFrameHost::LifecycleState::kPendingCommit) {
    return true;
  }

  if (rfh->GetLifecycleState() ==
      content::RenderFrameHost::LifecycleState::kPrerendering) {
    return false;
  }
  return true;
}

}  // namespace

ContentSensitiveMaskingDriver::ContentSensitiveMaskingDriver(
    content::RenderFrameHost* render_frame_host)
    : render_frame_host_(render_frame_host) {
  static unsigned next_free_id = 0;
  id_ = next_free_id++;
  
  // Establish connection to renderer-side agent
  render_frame_host_->GetRemoteAssociatedInterfaces()->GetInterface(&renderer_remote_);
}

ContentSensitiveMaskingDriver::~ContentSensitiveMaskingDriver() = default;

void ContentSensitiveMaskingDriver::BindPendingReceiver(
    mojo::PendingAssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver>
        pending_receiver) {
  if (IsRenderFrameHostSupported(render_frame_host_)) {
    sensitive_masking_receiver_.Bind(std::move(pending_receiver));
  }
}

void ContentSensitiveMaskingDriver::DidNavigate() {
  if (!IsRenderFrameHostSupported(render_frame_host_)) {
    sensitive_masking_receiver_.reset();
  }
}

int ContentSensitiveMaskingDriver::GetId() const {
  return id_;
}

void ContentSensitiveMaskingDriver::UpdateMaskingSelectorsDirectly(
    const std::vector<std::string>& selectors, 
    UpdateMaskingSelectorsCallback callback) {
  DVLOG(1) << "Received selectors from extension API, forwarding to renderer";
  
  // Check if we can forward to renderer
  if (renderer_remote_.is_bound()) {
    // Forward to renderer-side agent via Mojo
    renderer_remote_->UpdateMaskingSelectors(selectors);
    
    // Return number of selectors sent to indicate success
    std::move(callback).Run(static_cast<int32_t>(selectors.size()));
  } else {
    LOG(ERROR) << "Renderer remote not bound, cannot forward selectors";
    // Return 0 to indicate failure
    std::move(callback).Run(0);
  }
}

void ContentSensitiveMaskingDriver::UpdateMaskingSelectors(
    const std::vector<std::string>& selectors) {
  DVLOG(1) << "Browser-side UpdateMaskingSelectors called with " 
            << selectors.size() << " selectors, forwarding to renderer";
  
  // This method is called internally, error handling done in UpdateMaskingSelectorsDirectly
  if (renderer_remote_.is_bound()) {
    renderer_remote_->UpdateMaskingSelectors(selectors);
  }
}

void ContentSensitiveMaskingDriver::SetMaskingEnabled(bool enabled) {
  DVLOG(1) << "Setting masking enabled: " << enabled;
  
  // Forward to renderer-side agent via Mojo
  if (renderer_remote_.is_bound()) {
    renderer_remote_->SetMaskingEnabled(enabled);
  } else {
    LOG(ERROR) << "Renderer remote not bound, cannot forward masking enabled";
  }
}

}  // namespace sensitive_masking 