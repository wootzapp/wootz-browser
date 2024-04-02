// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_HELLO_WALLET_HELLO_WALLET_UI_H_
#define CHROME_BROWSER_UI_WEBUI_HELLO_WALLET_HELLO_WALLET_UI_H_

#include "base/memory/raw_ptr.h"
#include "components/demo_wallet/common/solana_wallet.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "ui/webui/mojo_web_ui_controller.h"

class Profile;
class HelloWalletPageHandler;

// The WebUI controller for chrome://segmentation-internals.
class HelloWalletUI : public ui::MojoWebUIController,
                      public solana_wallet::mojom::PageHandlerFactory {
 public:
  explicit HelloWalletUI(content::WebUI* web_ui);
  ~HelloWalletUI() override;

  HelloWalletUI(const HelloWalletUI&) = delete;
  HelloWalletUI& operator=(const HelloWalletUI&) = delete;

  void BindInterface(
      mojo::PendingReceiver<solana_wallet::mojom::PageHandlerFactory> receiver);

 private:
  // solana_wallet::mojom::PageHandlerFactory impls.
  void CreatePageHandler(
      mojo::PendingRemote<solana_wallet::mojom::Page> page,
      mojo::PendingReceiver<solana_wallet::mojom::PageHandler> receiver)
      override;

  raw_ptr<Profile> profile_;
  std::unique_ptr<HelloWalletPageHandler> hello_wallet_page_handler_;
  mojo::Receiver<solana_wallet::mojom::PageHandlerFactory>
      hello_wallet_page_factory_receiver_{this};

  WEB_UI_CONTROLLER_TYPE_DECL();
};

#endif  // CHROME_BROWSER_UI_WEBUI_HELLO_WALLET_HELLO_WALLET_UI_H_
