// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_REPLACE_ELEMENT_CONTENT_BROWSER_CONTENT_REPLACE_ELEMENT_DRIVER_H_
#define COMPONENTS_REPLACE_ELEMENT_CONTENT_BROWSER_CONTENT_REPLACE_ELEMENT_DRIVER_H_

#include <map>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/replace_element/content/common/mojom/replace_element_agent.mojom.h"
#include "components/replace_element/content/common/mojom/replace_element_driver.mojom.h"
#include "components/replace_element/core/replace_element_driver.h"
#include "content/public/browser/render_frame_host.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace content {
class RenderFrameHost;
}

namespace replace_element {

// There is one ContentReplaceElementDriver per RenderFrameHost.
// The lifetime is managed by the ContentReplaceElementDriverFactory.
class ContentReplaceElementDriver final : public ReplaceElementDriver,
                                     public replace_element::mojom::ReplaceElementDriver {
 public:
  ContentReplaceElementDriver(content::RenderFrameHost* render_frame_host);

  ContentReplaceElementDriver(const ContentReplaceElementDriver&) = delete;
  ContentReplaceElementDriver& operator=(const ContentReplaceElementDriver&) = delete;

  ~ContentReplaceElementDriver() override;

  void BindPendingReceiver(
      mojo::PendingAssociatedReceiver<replace_element::mojom::ReplaceElementDriver>
          pending_receiver);
  void DidNavigate();

  // ReplaceElementDriver implementation.
  int GetId() const override;
  void ReplaceElement(std::string element, std::string json_data) override;

  content::RenderFrameHost* render_frame_host() const {
    return render_frame_host_;
  }

 protected:
  // replace_element::mojom::ReplaceElementDriver:
  void CallToDriver() override;

 private:
  const mojo::AssociatedRemote<replace_element::mojom::ReplaceElementAgent>&
  GetReplaceElementAgent();

  const raw_ptr<content::RenderFrameHost> render_frame_host_;

  int id_;

  mojo::AssociatedRemote<replace_element::mojom::ReplaceElementAgent> replace_element_agent_;

  const mojo::AssociatedRemote<replace_element::mojom::ReplaceElementAgent>
      replace_element_agent_unbound_;

  mojo::AssociatedReceiver<replace_element::mojom::ReplaceElementDriver>
      replace_element_receiver_{this};

  base::WeakPtrFactory<ContentReplaceElementDriver> weak_factory_{this};
};

}  // namespace replace_element

#endif  // COMPONENTS_REPLACE_ELEMENT_CONTENT_BROWSER_CONTENT_REPLACE_ELEMENT_DRIVER_H_