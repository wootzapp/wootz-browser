// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/top_chrome/webui_contents_preload_manager.h"

#include "base/feature_list.h"
#include "base/memory/memory_pressure_monitor.h"
#include "base/memory/weak_ptr.h"
#include "base/no_destructor.h"
#include "chrome/browser/task_manager/web_contents_tags.h"
#include "chrome/browser/ui/prefs/prefs_tab_helper.h"
#include "chrome/browser/ui/ui_features.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/generated_resources.h"
#include "components/keyed_service/content/browser_context_keyed_service_shutdown_notifier_factory.h"
#include "components/keyed_service/core/keyed_service_shutdown_notifier.h"
#include "content/public/browser/navigation_controller.h"
#include "ui/base/models/menu_model.h"
#include "ui/webui/mojo_bubble_web_ui_controller.h"

namespace {

// This factory is used to get notification for the browser context shutdown.
class BrowserContextShutdownNotifierFactory
    : public BrowserContextKeyedServiceShutdownNotifierFactory {
 public:
  BrowserContextShutdownNotifierFactory(
      const BrowserContextShutdownNotifierFactory&) = delete;
  BrowserContextShutdownNotifierFactory& operator=(
      const BrowserContextShutdownNotifierFactory&) = delete;

  static BrowserContextShutdownNotifierFactory* GetInstance() {
    static base::NoDestructor<BrowserContextShutdownNotifierFactory> s_factory;
    return s_factory.get();
  }

 private:
  friend class base::NoDestructor<BrowserContextShutdownNotifierFactory>;
  BrowserContextShutdownNotifierFactory()
      : BrowserContextKeyedServiceShutdownNotifierFactory(
            "WebUIContentsPreloadManager") {}
};

bool IsFeatureEnabled() {
  return base::FeatureList::IsEnabled(features::kPreloadTopChromeWebUI);
}

content::WebContents::CreateParams GetWebContentsCreateParams(
    const GURL& webui_url,
    content::BrowserContext* browser_context) {
  content::WebContents::CreateParams create_params(browser_context);
  // Set it to visible so that the resources are immediately loaded.
  create_params.initially_hidden = !IsFeatureEnabled();
  create_params.site_instance =
      content::SiteInstance::CreateForURL(browser_context, webui_url);

  return create_params;
}

content::WebUIController* GetWebUIController(
    content::WebContents* web_contents) {
  if (!web_contents) {
    return nullptr;
  }

  content::WebUI* webui = web_contents->GetWebUI();
  if (!webui) {
    return nullptr;
  }

  return webui->GetController();
}

}  // namespace

// Currently we preloads Tab Search. In practice, this also benefits other
// WebUIs. This is likely due to reused render processes that increase cache
// hits and reduce re-creation of common structs.
const char* const WebUIContentsPreloadManager::kPreloadedWebUIURL =
    chrome::kChromeUITabSearchURL;

// A stub WebUI page embdeder that captures the ready-to-show signal.
class WebUIContentsPreloadManager::WebUIControllerEmbedderStub final
    : public ui::MojoBubbleWebUIController::Embedder {
 public:
  WebUIControllerEmbedderStub() = default;
  ~WebUIControllerEmbedderStub() = default;

  // ui::MojoBubbleWebUIController::Embedder:
  void CloseUI() override {}
  void ShowContextMenu(gfx::Point point,
                       std::unique_ptr<ui::MenuModel> menu_model) override {}
  void HideContextMenu() override {}
  void ShowUI() override { is_ready_to_show_ = true; }

  // Attach this stub as the embedder of `web_contents`, assuming that the
  // contents is not yet ready to be shown.
  void AttachTo(content::WebContents* web_contents) {
    CHECK_NE(web_contents, nullptr);
    content::WebUIController* webui_controller =
        GetWebUIController(web_contents);
    if (!webui_controller) {
      return;
    }
    // TODO(40168622): Add type check. This is currently not possible because a
    // WebUIController subclass does not retain its parent class' type info.
    auto* bubble_controller =
        static_cast<ui::MojoBubbleWebUIController*>(webui_controller);
    bubble_controller->set_embedder(this->GetWeakPtr());
    web_contents_ = web_contents;
    is_ready_to_show_ = false;
  }

  // Detach from the previously attached `web_contents`, returns true if the
  // contents is ready to be shown.
  bool Detach() {
    content::WebUIController* webui_controller =
        GetWebUIController(web_contents_);
    if (!webui_controller) {
      return false;
    }

    auto* bubble_controller =
        static_cast<ui::MojoBubbleWebUIController*>(webui_controller);
    bubble_controller->set_embedder(nullptr);
    web_contents_ = nullptr;

    return is_ready_to_show_;
  }

  base::WeakPtr<WebUIControllerEmbedderStub> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 private:
  raw_ptr<content::WebContents> web_contents_ = nullptr;
  bool is_ready_to_show_ = false;
  base::WeakPtrFactory<WebUIControllerEmbedderStub> weak_ptr_factory_{this};
};

using MakeContentsResult = WebUIContentsPreloadManager::MakeContentsResult;

MakeContentsResult::MakeContentsResult() = default;
MakeContentsResult::~MakeContentsResult() = default;
MakeContentsResult& MakeContentsResult::operator=(MakeContentsResult&&) =
    default;

WebUIContentsPreloadManager::WebUIContentsPreloadManager()
    : preload_mode_(
          static_cast<PreloadMode>(features::kPreloadTopChromeWebUIMode.Get())),
      webui_controller_embedder_stub_(
          std::make_unique<WebUIControllerEmbedderStub>()) {}

WebUIContentsPreloadManager::~WebUIContentsPreloadManager() = default;

// static
WebUIContentsPreloadManager* WebUIContentsPreloadManager::GetInstance() {
  static base::NoDestructor<WebUIContentsPreloadManager> s_instance;
  return s_instance.get();
}

// static
void WebUIContentsPreloadManager::EnsureFactoryBuilt() {
  // Ensure that the shutdown notifier factory is built.
  // The profile service's dependency manager requires the service factory
  // be registered at an early stage of browser lifetime.
  BrowserContextShutdownNotifierFactory::GetInstance();
}

void WebUIContentsPreloadManager::WarmupForBrowserContext(
    content::BrowserContext* browser_context) {
  if (preload_mode_ == PreloadMode::kPreloadOnMakeContents) {
    return;
  }

  CHECK_EQ(preload_mode_, PreloadMode::kPreloadOnWarmup);
  PreloadForBrowserContext(browser_context);
}

void WebUIContentsPreloadManager::PreloadForBrowserContextForTesting(
    content::BrowserContext* browser_context) {
  PreloadForBrowserContext(browser_context);
}

void WebUIContentsPreloadManager::PreloadForBrowserContext(
    content::BrowserContext* browser_context) {
  if (!ShouldPreloadForBrowserContext(browser_context)) {
    return;
  }

  preloaded_web_contents_ = CreateNewContents(browser_context);
  ObserveBrowserContextShutdown();
}

MakeContentsResult WebUIContentsPreloadManager::MakeContents(
    const GURL& webui_url,
    content::BrowserContext* browser_context) {
  std::unique_ptr<content::WebContents> web_contents_ret;
  bool is_ready_to_show = false;
  // Use preloaded contents if requested the same WebUI under the same browser
  // context. Navigating to or from a blank page is also allowed.
  // TODO(325836830): allow navigations between WebUIs.
  if (preloaded_web_contents_ &&
      preloaded_web_contents_->GetBrowserContext() == browser_context &&
      (preloaded_web_contents_->GetURL().host() == webui_url.host() ||
       preloaded_web_contents_->GetURL().IsAboutBlank() ||
       webui_url.IsAboutBlank())) {
    // Redirect if requested a different URL.
    if (preloaded_web_contents_->GetURL().host() != webui_url.host()) {
      LoadURLForContents(preloaded_web_contents_.get(), webui_url);
    }
    web_contents_ret = std::move(preloaded_web_contents_);
    is_ready_to_show = webui_controller_embedder_stub_->Detach();
    StopObserveBrowserContextShutdown();
  } else {
    web_contents_ret = CreateNewContents(browser_context, webui_url);
    is_ready_to_show = false;
  }

  if (ShouldPreloadForBrowserContext(browser_context)) {
    // Preloads a new contents.
    preloaded_web_contents_ = CreateNewContents(browser_context);
    webui_controller_embedder_stub_->AttachTo(preloaded_web_contents_.get());
    ObserveBrowserContextShutdown();
  }

  task_manager::WebContentsTags::ClearTag(web_contents_ret.get());

  MakeContentsResult result;
  result.web_contents = std::move(web_contents_ret);
  result.is_ready_to_show = is_ready_to_show;
  return result;
}

GURL WebUIContentsPreloadManager::GetPreloadedURLForTesting() const {
  return GURL(kPreloadedWebUIURL);
}

void WebUIContentsPreloadManager::DisableNavigationForTesting() {
  is_navigation_disabled_for_test_ = true;
}

std::unique_ptr<content::WebContents>
WebUIContentsPreloadManager::CreateNewContents(
    content::BrowserContext* browser_context,
    GURL url) {
  std::unique_ptr<content::WebContents> web_contents =
      content::WebContents::Create(
          GetWebContentsCreateParams(url, browser_context));

  // Propagates user prefs to web contents.
  // This is needed by, for example, text selection color on ChromeOS.
  PrefsTabHelper::CreateForWebContents(web_contents.get());

  task_manager::WebContentsTags::CreateForToolContents(
      web_contents.get(), IDS_TASK_MANAGER_PRELOADED_RENDERER_FOR_UI);

  LoadURLForContents(web_contents.get(), url);

  return web_contents;
}

void WebUIContentsPreloadManager::LoadURLForContents(
    content::WebContents* web_contents,
    GURL url) {
  if (is_navigation_disabled_for_test_) {
    return;
  }

  web_contents->GetController().LoadURL(url, content::Referrer(),
                                        ui::PAGE_TRANSITION_AUTO_TOPLEVEL,
                                        std::string());
}

bool WebUIContentsPreloadManager::ShouldPreloadForBrowserContext(
    content::BrowserContext* browser_context) const {
  // Don't preload if the feature is disabled.
  if (!IsFeatureEnabled()) {
    return false;
  }

  // Don't preload if already preloaded for this `browser_context`.
  if (preloaded_web_contents_ &&
      preloaded_web_contents_->GetBrowserContext() == browser_context) {
    return false;
  }

  // Don't preload if under heavy memory pressure.
  const auto* memory_monitor = base::MemoryPressureMonitor::Get();
  if (memory_monitor && memory_monitor->GetCurrentPressureLevel() >=
                            base::MemoryPressureMonitor::MemoryPressureLevel::
                                MEMORY_PRESSURE_LEVEL_MODERATE) {
    return false;
  }

  return true;
}

void WebUIContentsPreloadManager::ObserveBrowserContextShutdown() {
  CHECK(preloaded_web_contents_);
  // Cleans up the preloaded contents on browser context shutdown.
  content::BrowserContext* browser_context =
      preloaded_web_contents_->GetBrowserContext();
  browser_context_shutdown_subscription_ =
      BrowserContextShutdownNotifierFactory::GetInstance()
          ->Get(browser_context)
          ->Subscribe(base::BindRepeating(
              &WebUIContentsPreloadManager::OnBrowserContextShutdown,
              base::Unretained(this), browser_context));
}

void WebUIContentsPreloadManager::StopObserveBrowserContextShutdown() {
  browser_context_shutdown_subscription_ = {};
}

void WebUIContentsPreloadManager::OnBrowserContextShutdown(
    content::BrowserContext* browser_context) {
  if (!preloaded_web_contents_) {
    return;
  }

  webui_controller_embedder_stub_->Detach();
  CHECK_EQ(preloaded_web_contents_->GetBrowserContext(), browser_context);
  preloaded_web_contents_.reset();
}
