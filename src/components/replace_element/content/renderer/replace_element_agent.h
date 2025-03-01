// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_REPLACE_ELEMENT_CONTENT_RENDERER_REPLACE_ELEMENT_AGENT_H_
#define COMPONENTS_REPLACE_ELEMENT_CONTENT_RENDERER_REPLACE_ELEMENT_AGENT_H_

#include "components/replace_element/content/common/mojom/replace_element_agent.mojom.h"
#include "components/replace_element/content/common/mojom/replace_element_driver.mojom.h"
#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace replace_element {

class ReplaceElementAgent : public content::RenderFrameObserver,
                       public mojom::ReplaceElementAgent {
 public:
  ReplaceElementAgent(content::RenderFrame* render_frame,
                 blink::AssociatedInterfaceRegistry* registry);

  ReplaceElementAgent(const ReplaceElementAgent&) = delete;
  ReplaceElementAgent& operator=(const ReplaceElementAgent&) = delete;

  ~ReplaceElementAgent() override;

  void BindPendingReceiver(
      mojo::PendingAssociatedReceiver<mojom::ReplaceElementAgent> pending_receiver);

  // mojom::ReplaceElementAgent:
  void ReplaceElement(const std::string& element, const std::string& json_data) override;

  mojom::ReplaceElementDriver& GetReplaceElementDriver();

 protected:
  // content::RenderFrameObserver:
  void DidDispatchDOMContentLoadedEvent() override;
  void OnDestruct() override;

 private:
  mojo::AssociatedReceiver<mojom::ReplaceElementAgent> receiver_{this};
  mojo::AssociatedRemote<mojom::ReplaceElementDriver> replace_element_driver_;
};

}  // namespace replace_element

#endif  // COMPONENTS_REPLACE_ELEMENT_CONTENT_RENDERER_REPLACE_ELEMENT_AGENT_H_