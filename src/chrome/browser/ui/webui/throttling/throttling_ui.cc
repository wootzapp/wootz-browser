// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/throttling/throttling_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/throttling/throttling_page_handler.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/throttling_resources.h"
#include "chrome/grit/throttling_resources_map.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"

ThrottlingUI::ThrottlingUI(content::WebUI* web_ui)
    : MojoWebUIController(web_ui, /*enable_chrome_send=*/true) {
  profile_ = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIThrottlingHost);
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kThrottlingResources,
                      kThrottlingResourcesSize),
      IDR_THROTTLING_THROTTLING_HTML);
}

ThrottlingUI::~ThrottlingUI() = default;

void ThrottlingUI::BindInterface(
    mojo::PendingReceiver<throttling::mojom::PageHandlerFactory>
        receiver) {
  throttling_page_factory_receiver_.reset();
  throttling_page_factory_receiver_.Bind(std::move(receiver));
}

void ThrottlingUI::CreatePageHandler(
    mojo::PendingRemote<throttling::mojom::Page> page,
    mojo::PendingReceiver<throttling::mojom::PageHandler>
        receiver) {
  throttling_page_handler_ =
      std::make_unique<ThrottlingPageHandler>(
          std::move(receiver), std::move(page), profile_);
}

WEB_UI_CONTROLLER_TYPE_IMPL(ThrottlingUI)