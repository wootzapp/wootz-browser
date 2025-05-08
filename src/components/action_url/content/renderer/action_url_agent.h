// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_RENDERER_ACTION_URL_AGENT_H_
#define COMPONENTS_ACTION_URL_CONTENT_RENDERER_ACTION_URL_AGENT_H_

#include <map>

#include "base/android/action_spec_json.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "components/action_url/content/common/mojom/action_url_agent.mojom.h"
#include "components/action_url/content/common/mojom/action_url_driver.mojom.h"
#include "components/action_url/core/anchor_data.h"
#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/web/web_anchor_client.h"
#include "third_party/blink/public/web/web_anchor_element.h"

using blink::WebAnchorElement;
namespace action_url {

class ActionUrlAgent : public content::RenderFrameObserver,
                       public blink::WebAnchorClient,
                       public mojom::ActionUrlAgent {
 public:
  static constexpr base::TimeDelta kAnchorSeenThrottle =
      base::Milliseconds(1000);
  ActionUrlAgent(content::RenderFrame* render_frame,
                 blink::AssociatedInterfaceRegistry* registry);

  ActionUrlAgent(const ActionUrlAgent&) = delete;
  ActionUrlAgent& operator=(const ActionUrlAgent&) = delete;

  ~ActionUrlAgent() override;

  void BindPendingReceiver(
      mojo::PendingAssociatedReceiver<mojom::ActionUrlAgent> pending_receiver);

  mojom::ActionUrlDriver& GetActionUrlDriver();

  void Reset();

  // mojom::ActionUrlAgent:
  void SetUpHeader() override;
  void ReplaceUrL(const std::string& json,
                  const AnchorData& anchor,
                  const GURL& action_url,
                  const std::string& tag) override;
  void SetUpScriptBlock() override;

 protected:
  // blink::WebAnchorClient:
  // Signals from blink that a anchor element is added dynamically,
  void DidAddAnchorElementDynamically(const blink::WebElement&) override;

  // content::RenderFrameObserver:
  void DidCommitProvisionalLoad(ui::PageTransition transition) override;
  void DidCreateDocumentElement() override;
  void DidDispatchDOMContentLoadedEvent() override;
  void DidFinishLoad() override;
  void OnDestruct() override;

 private:
  void WaitTillDynamicChangeTimer(base::OneShotTimer& timer);
  void FindAnchorElementsOnPage(bool is_dynamic);
  std::unique_ptr<AnchorData> GetAnchorDataFromWebAnchor(
      const WebAnchorElement& anchor_element);

  base::ActionSpecJson ParseJson(const std::string& json_str,
                                 const GURL& action_url,
                                 const std::string& tag);
  void OnJsonParsed(WebAnchorElement anchor, base::ActionSpecJson action_spec);
  void ActionBlockDrawCompleted();

  std::map<uint64_t, WebAnchorElement> renderer_anchor_cache_;

  blink::WebVector<WebAnchorElement> buffer_for_anchors_;
  base::Lock buffer_mu_;

  base::OneShotTimer process_anchor_after_dynamic_change_timer_;

  bool is_dom_content_loaded_ = false;
  int action_block_counter_ = 1;

  mojo::AssociatedReceiver<mojom::ActionUrlAgent> receiver_{this};
  mojo::AssociatedRemote<mojom::ActionUrlDriver> action_url_driver_;

  base::WeakPtrFactory<ActionUrlAgent> weak_ptr_factory_{this};
};

}  // namespace action_url

#endif  // COMPONENTS_ACTION_URL_CONTENT_RENDERER_ACTION_URL_AGENT_H_
