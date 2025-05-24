// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_ACTION_URL_DRIVER_H_
#define COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_ACTION_URL_DRIVER_H_

#include <map>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/action_url/android/action_url_handler_android.h"
#include "components/action_url/content/common/mojom/action_url_agent.mojom.h"
#include "components/action_url/content/common/mojom/action_url_driver.mojom.h"
#include "components/action_url/core/action_url_driver.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace content {
class RenderFrameHost;
class RenderProcessHost;
}  // namespace content

namespace action_url {

// There is one ContentActionUrlDriver per RenderFrameHost.
// The lifetime is managed by the ContentActionUrlDriverFactory.
class ContentActionUrlDriver final : public ActionUrlDriver,
                                     public action_url::mojom::ActionUrlDriver {
 public:
  ContentActionUrlDriver(content::RenderFrameHost* render_frame_host);

  ContentActionUrlDriver(const ContentActionUrlDriver&) = delete;
  ContentActionUrlDriver& operator=(const ContentActionUrlDriver&) = delete;

  ~ContentActionUrlDriver() override;

  void BindPendingReceiver(
      mojo::PendingAssociatedReceiver<action_url::mojom::ActionUrlDriver>
          pending_receiver);
  void DidNavigate();

  // ActionUrlDriver implementation.
  int GetId() const override;

  content::RenderFrameHost* render_frame_host() const {
    return render_frame_host_;
  }

 protected:
  // action_url::mojom::ActionUrlDriver:
  void AllAnchorsParsed(
      const std::vector<action_url::AnchorData>& anchors_data) override;
  void OnBlockDrawCompleted() override;
  void Reset() override;

 private:
  void ActionUrlFetched(action_url::AnchorData anchor, std::string acion_url, std::string tag);

  void ProcessActionUrl(GURL action_url, action_url::AnchorData anchor, std::string tag);
  void OnDownloadedJson(std::unique_ptr<network::SimpleURLLoader> url_loader,
                        action_url::AnchorData anchor,
                        GURL action_url,
                        std::string tag,
                        std::unique_ptr<std::string> response_body);

  const mojo::AssociatedRemote<action_url::mojom::ActionUrlAgent>&
  GetActionUrlAgent();

  const raw_ptr<content::RenderFrameHost> render_frame_host_;

  int id_;
  int handler_processing_remaining_;
  int action_block_draw_remaining_;
  bool requested_for_header_;

  mojo::AssociatedRemote<action_url::mojom::ActionUrlAgent> action_url_agent_;

  const mojo::AssociatedRemote<action_url::mojom::ActionUrlAgent>
      action_url_agent_unbound_;

  mojo::AssociatedReceiver<action_url::mojom::ActionUrlDriver>
      action_url_receiver_{this};

  // std::unique_ptr<ActionUrlHandlerAndroid> handler_;
  std::vector<std::unique_ptr<ActionUrlHandlerAndroid>> handlers_list_;

  base::WeakPtrFactory<ContentActionUrlDriver> weak_factory_{this};
};

}  // namespace action_url

#endif  // COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_ACTION_URL_DRIVER_H_
