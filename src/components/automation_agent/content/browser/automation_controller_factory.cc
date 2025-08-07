#include "components/automation_agent/content/browser/automation_controller_factory.h"

#include <utility>

#include "components/automation_agent/content/browser/automation_controller.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

namespace automation {

AutomationControllerFactory::AutomationControllerFactory(
    content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<AutomationControllerFactory>(*web_contents) {
  // Any initialization if needed
}

AutomationControllerFactory::~AutomationControllerFactory() = default;

// static
void AutomationControllerFactory::BindAutomationDriver(
    mojo::PendingAssociatedReceiver<mojom::AutomationDriver> pending_receiver,
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

  AutomationControllerFactory* factory =
      AutomationControllerFactory::FromWebContents(web_contents);
  if (!factory) {
    return;
  }

  // Get the driver and bind the receiver
  if (auto* driver = factory->GetDriverForFrame(render_frame_host)) {
    driver->BindPendingReceiver(std::move(pending_receiver));
  }
}

AutomationController* AutomationControllerFactory::GetDriverForFrame(
    content::RenderFrameHost* render_frame_host) {
  DCHECK_EQ(web_contents(),
            content::WebContents::FromRenderFrameHost(render_frame_host));

  // A RenderFrameHost without a live RenderFrame will never call
  // RenderFrameDeleted(), and the corresponding driver would never be cleaned
  // up.
  if (!render_frame_host->IsRenderFrameLive()) {
    return nullptr;
  }

  auto it = frame_driver_map_.find(render_frame_host);
  if (it == frame_driver_map_.end()) {
    // Create a new controller if one doesn't exist
    auto [inserted_it, success] = frame_driver_map_.emplace(
        render_frame_host,
        std::make_unique<AutomationController>(render_frame_host));
    it = inserted_it;
  }
  return it->second.get();
}

void AutomationControllerFactory::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  frame_driver_map_.erase(render_frame_host);
}

void AutomationControllerFactory::WebContentsDestroyed() {
  web_contents()->RemoveUserData(UserDataKey());
  // Do not add code - `this` is now destroyed.
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(AutomationControllerFactory);

}  // namespace automation