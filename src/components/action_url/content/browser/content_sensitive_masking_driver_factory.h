// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_SENSITIVE_MASKING_DRIVER_FACTORY_H_
#define COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_SENSITIVE_MASKING_DRIVER_FACTORY_H_

#include <map>

#include "base/memory/raw_ptr.h"
#include "components/action_url/content/common/mojom/sensitive_element_masking.mojom.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace content {
class WebContents;
}

namespace sensitive_masking {

class ContentSensitiveMaskingDriver;

// Creates and owns ContentSensitiveMaskingDriver. There is one
// factory per WebContents, and one driver per RenderFrameHost.
class ContentSensitiveMaskingDriverFactory
    : public content::WebContentsObserver,
      public content::WebContentsUserData<ContentSensitiveMaskingDriverFactory> {
 public:
  ContentSensitiveMaskingDriverFactory(const ContentSensitiveMaskingDriverFactory&) = delete;
  ContentSensitiveMaskingDriverFactory& operator=(
      const ContentSensitiveMaskingDriverFactory&) = delete;

  ~ContentSensitiveMaskingDriverFactory() override;

  static void BindSensitiveMaskingDriver(
      mojo::PendingAssociatedReceiver<sensitive_masking::mojom::SensitiveElementMaskingDriver>
          pending_receiver,
      content::RenderFrameHost* render_frame_host);

  // Note that this may return null if the RenderFrameHost does not have a
  // live RenderFrame (e.g. it represents a crashed RenderFrameHost).
  ContentSensitiveMaskingDriver* GetDriverForFrame(
      content::RenderFrameHost* render_frame_host);

 private:
  friend class content::WebContentsUserData<ContentSensitiveMaskingDriverFactory>;

  ContentSensitiveMaskingDriverFactory(content::WebContents* web_contents);

  // content::WebContentsObserver:
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void WebContentsDestroyed() override;

  std::map<content::RenderFrameHost*, ContentSensitiveMaskingDriver> frame_driver_map_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace sensitive_masking

#endif  // COMPONENTS_ACTION_URL_CONTENT_BROWSER_CONTENT_SENSITIVE_MASKING_DRIVER_FACTORY_H_ 