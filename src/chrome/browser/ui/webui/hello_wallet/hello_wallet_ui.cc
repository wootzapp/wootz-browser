// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/solana_wallet/solana_wallet_ui.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/hello_wallet/hello_wallet_page_handler.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/common/webui_url_constants.h"
#include "chrome/grit/hello_wallet_resources.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"

HelloWalletUI::HelloWalletUI(content::WebUI* web_ui)
    : MojoWebUIController(web_ui, /*enable_chrome_send=*/true) {
  profile_ = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(),
      chrome::kChromeUIHelloWalletHost);
  webui::SetupWebUIDataSource(
      source,
      base::make_span(kHelloWalletResources,
                      kHelloWalletResourcesSize),
      IDR_HELLO_WALLET_HELLO_WALLET_HTML);
}

HelloWalletUI::~HelloWalletUI() = default;

void HelloWalletUI::BindInterface(
    mojo::PendingReceiver<solana_wallet::mojom::PageHandlerFactory>
        receiver) {
  hello_wallet_page_factory_receiver_.reset();
  hello_wallet_page_factory_receiver_.Bind(std::move(receiver));
}

void HelloWalletUI::CreatePageHandler(
    mojo::PendingRemote<solana_wallet::mojom::Page> page,
    mojo::PendingReceiver<solana_wallet::mojom::PageHandler>
        receiver) {
  hello_wallet_page_handler_ =
      std::make_unique<HelloWalletPageHandler>(
          std::move(receiver), std::move(page), profile_);
}

WEB_UI_CONTROLLER_TYPE_IMPL(HelloWalletUI)