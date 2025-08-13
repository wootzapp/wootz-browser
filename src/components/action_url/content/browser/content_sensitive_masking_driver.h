// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_SENSITIVE_MASKING_DRIVER_H_
#define COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_SENSITIVE_MASKING_DRIVER_H_

#include <map>
#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/action_url/content/common/mojom/sensitive_element_masking.mojom.h"
#include "content/public/browser/render_frame_host.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace content {
class RenderFrameHost;
}

namespace sensitive_masking {

// Define callback type for extension API (since Mojo interface no longer has callback)
using UpdateMaskingSelectorsCallback = base::OnceCallback<void(int32_t masked_count)>;

// There is one ContentSensitiveMaskingDriver per RenderFrameHost.
// The lifetime is managed by the ContentSensitiveMaskingDriverFactory.
class ContentSensitiveMaskingDriver final : public sensitive_masking::mojom::SensitiveElementMaskingDriver {
 public:
  ContentSensitiveMaskingDriver(content::RenderFrameHost* render_frame_host);

  ContentSensitiveMaskingDriver(const ContentSensitiveMaskingDriver&) = delete;
  ContentSensitiveMaskingDriver& operator=(const ContentSensitiveMaskingDriver&) = delete;

  ~ContentSensitiveMaskingDriver() override;

  void BindPendingReceiver(
      mojo::PendingAssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver>
          pending_receiver);
  void DidNavigate();

  // Returns driver id which is unique in the current tab.
  int GetId() const;
  void UpdateMaskingSelectorsDirectly(const std::vector<std::string>& selectors, 
                                     UpdateMaskingSelectorsCallback callback);

  content::RenderFrameHost* render_frame_host() const {
    return render_frame_host_;
  }

 protected:
  // sensitive_masking::mojom::SensitiveElementMaskingDriver:
  void UpdateMaskingSelectors(const std::vector<std::string>& selectors) override;
  void SetMaskingEnabled(bool enabled) override;

 private:
  const raw_ptr<content::RenderFrameHost> render_frame_host_;

  int id_;

  mojo::AssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver>
      sensitive_masking_receiver_{this};
  
  // Remote to communicate with renderer-side agent
  mojo::AssociatedRemote<sensitive_masking::mojom::SensitiveElementMaskingDriver>
      renderer_remote_;

  base::WeakPtrFactory<ContentSensitiveMaskingDriver> weak_factory_{this};
};

}  // namespace sensitive_masking

#endif  // COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_SENSITIVE_MASKING_DRIVER_H_ 