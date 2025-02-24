// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/replace_element/content/browser/content_replace_element_driver_factory.h"

#include <utility>

#include "components/replace_element/content/browser/content_replace_element_driver.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

namespace replace_element {

ContentReplaceElementDriverFactory::ContentReplaceElementDriverFactory(
    content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<ContentReplaceElementDriverFactory>(
          *web_contents) {}

ContentReplaceElementDriverFactory::~ContentReplaceElementDriverFactory() = default;

// static
void ContentReplaceElementDriverFactory::BindReplaceElementDriver(
    mojo::PendingAssociatedReceiver<replace_element::mojom::ReplaceElementDriver>
        pending_receiver,
    content::RenderFrameHost* render_frame_host) {
  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);

  if (!web_contents) {
    return;
  }

  // This is called by a Mojo registry for associated interfaces, which should
  // never attempt to bind interfaces for RenderFrameHosts with non-live
  // RenderFrames.
  CHECK(render_frame_host->IsRenderFrameLive());

  ContentReplaceElementDriverFactory* factory =
      ContentReplaceElementDriverFactory::FromWebContents(web_contents);
  if (!factory) {
    return;
  }

  factory->GetDriverForFrame(render_frame_host)
      ->BindPendingReceiver(std::move(pending_receiver));
}

ContentReplaceElementDriver* ContentReplaceElementDriverFactory::GetDriverForFrame(
    content::RenderFrameHost* render_frame_host) {
  DCHECK_EQ(web_contents(),
            content::WebContents::FromRenderFrameHost(render_frame_host));

  // A RenderFrameHost without a live RenderFrame will never call
  // RenderFrameDeleted(), and the corresponding driver would never be cleaned
  // up.
  if (!render_frame_host->IsRenderFrameLive()) {
    return nullptr;
  }

  // try_emplace() will return an iterator to the driver corresponding to
  // `render_frame_host`, creating a new one if `render_frame_host` is not
  // already a key in the map.
  auto [it, inserted] =
      frame_driver_map_.try_emplace(render_frame_host,
                                    // Args passed to the ContentReplaceElementDriver
                                    // constructor if none exists for
                                    // `render_frame_host` yet.
                                    render_frame_host);
  return &it->second;
}

void ContentReplaceElementDriverFactory::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  frame_driver_map_.erase(render_frame_host);
}

void ContentReplaceElementDriverFactory::WebContentsDestroyed() {
  web_contents()->RemoveUserData(UserDataKey());
  // Do not add code - `this` is now destroyed.
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ContentReplaceElementDriverFactory);

}  // namespace replace_element