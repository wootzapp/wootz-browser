// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_ACTION_URL_DRIVER_FACTORY_H_
#define COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_ACTION_URL_DRIVER_FACTORY_H_

#include <map>

#include "base/memory/raw_ptr.h"
#include "components/action_url/content/common/mojom/action_url_driver.mojom.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace content {
class WebContents;
}

namespace action_url {

class ContentActionUrlDriver;

// Creates and owns ContentActionUrlDrivers. There is one
// factory per WebContents, and one driver per RenderFrameHost.
class ContentActionUrlDriverFactory
    : public content::WebContentsObserver,
      public content::WebContentsUserData<ContentActionUrlDriverFactory> {
 public:
  ContentActionUrlDriverFactory(const ContentActionUrlDriverFactory&) = delete;
  ContentActionUrlDriverFactory& operator=(
      const ContentActionUrlDriverFactory&) = delete;

  ~ContentActionUrlDriverFactory() override;

  static void BindActionUrlDriver(
      mojo::PendingAssociatedReceiver<action_url::mojom::ActionUrlDriver>
          pending_receiver,
      content::RenderFrameHost* render_frame_host);

  // Note that this may return null if the RenderFrameHost does not have a
  // live RenderFrame (e.g. it represents a crashed RenderFrameHost).
  ContentActionUrlDriver* GetDriverForFrame(
      content::RenderFrameHost* render_frame_host);

 private:
  friend class content::WebContentsUserData<ContentActionUrlDriverFactory>;

  ContentActionUrlDriverFactory(content::WebContents* web_contents);

  // content::WebContentsObserver:
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void WebContentsDestroyed() override;

  std::map<content::RenderFrameHost*, ContentActionUrlDriver> frame_driver_map_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace action_url

#endif  // COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_ACTION_URL_DRIVER_FACTORY_H_
