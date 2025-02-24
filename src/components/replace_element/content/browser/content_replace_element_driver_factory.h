// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_REPLACE_ELEMENT_CONTENT_BROWSER_CONTENT_REPLACE_ELEMENT_DRIVER_FACTORY_H_
#define COMPONENTS_REPLACE_ELEMENT_CONTENT_BROWSER_CONTENT_REPLACE_ELEMENT_DRIVER_FACTORY_H_

#include <map>

#include "base/memory/raw_ptr.h"
#include "components/replace_element/content/common/mojom/replace_element_driver.mojom.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace content {
class WebContents;
}

namespace replace_element {

class ContentReplaceElementDriver;

// Creates and owns ContentReplaceElementDriver. There is one
// factory per WebContents, and one driver per RenderFrameHost.
class ContentReplaceElementDriverFactory
    : public content::WebContentsObserver,
      public content::WebContentsUserData<ContentReplaceElementDriverFactory> {
 public:
  ContentReplaceElementDriverFactory(const ContentReplaceElementDriverFactory&) = delete;
  ContentReplaceElementDriverFactory& operator=(
      const ContentReplaceElementDriverFactory&) = delete;

  ~ContentReplaceElementDriverFactory() override;

  static void BindReplaceElementDriver(
      mojo::PendingAssociatedReceiver<replace_element::mojom::ReplaceElementDriver>
          pending_receiver,
      content::RenderFrameHost* render_frame_host);

  // Note that this may return null if the RenderFrameHost does not have a
  // live RenderFrame (e.g. it represents a crashed RenderFrameHost).
  ContentReplaceElementDriver* GetDriverForFrame(
      content::RenderFrameHost* render_frame_host);

 private:
  friend class content::WebContentsUserData<ContentReplaceElementDriverFactory>;

  ContentReplaceElementDriverFactory(content::WebContents* web_contents);

  // content::WebContentsObserver:
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void WebContentsDestroyed() override;

  std::map<content::RenderFrameHost*, ContentReplaceElementDriver> frame_driver_map_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace replace_element

#endif  // COMPONENTS_REPLACE_ELEMENT_CONTENT_BROWSER_CONTENT_REPLACE_ELEMENT_DRIVER_FACTORY_H_