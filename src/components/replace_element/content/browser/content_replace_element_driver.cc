// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/replace_element/content/browser/content_replace_element_driver.h"

#include <utility>

#include "components/replace_element/content/browser/content_replace_element_driver_factory.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"

namespace replace_element {

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

ContentReplaceElementDriver::ContentReplaceElementDriver(
    content::RenderFrameHost* render_frame_host)
    : render_frame_host_(render_frame_host) {
  static unsigned next_free_id = 0;
  id_ = next_free_id++;

  render_frame_host_->GetRemoteAssociatedInterfaces()->GetInterface(
      &replace_element_agent_);
}

ContentReplaceElementDriver::~ContentReplaceElementDriver() = default;

void ContentReplaceElementDriver::BindPendingReceiver(
    mojo::PendingAssociatedReceiver<replace_element::mojom::ReplaceElementDriver>
        pending_receiver) {
  if (IsRenderFrameHostSupported(render_frame_host_)) {
    replace_element_receiver_.Bind(std::move(pending_receiver));
  }
}

void ContentReplaceElementDriver::DidNavigate() {
  if (!IsRenderFrameHostSupported(render_frame_host_)) {
    replace_element_receiver_.reset();
  }
}

int ContentReplaceElementDriver::GetId() const {
  return id_;
}

void ContentReplaceElementDriver::ReplaceElement(std::string element, std::string json_data) {
  LOG(INFO) << "ElementReplacer:: browser side; Propagating replace element call to renderer";
  if (const auto& agent = GetReplaceElementAgent()) {
    agent->ReplaceElement(element, json_data);
  }
}

void ContentReplaceElementDriver::CallToDriver() {
  LOG(INFO) << "ElementReplacer; I am in browser process.";

}

const mojo::AssociatedRemote<replace_element::mojom::ReplaceElementAgent>&
ContentReplaceElementDriver::GetReplaceElementAgent() {
  CHECK_NE(render_frame_host_->GetLifecycleState(),
           content::RenderFrameHost::LifecycleState::kPendingCommit);

  return IsRenderFrameHostSupported(render_frame_host_)
             ? replace_element_agent_
             : replace_element_agent_unbound_;
}

}  // namespace replace_element