// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_ANDROID_ANDROID_WALLET_PAGE_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_ANDROID_ANDROID_WALLET_PAGE_UI_H_

#include <memory>
#include <string>

#include "chrome/browser/ui/webui/wootz_wallet/android/android_wallet_page_handler.h"
#include "chrome/browser/ui/webui/wootz_wallet/common_handler/wallet_handler.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/webui/mojo_web_ui_controller.h"

class AndroidWalletPageUI : public ui::MojoWebUIController,
                            public wootz_wallet::mojom::PageHandlerFactory {
 public:
  explicit AndroidWalletPageUI(content::WebUI* web_ui, const GURL& url);
  AndroidWalletPageUI(const AndroidWalletPageUI&) = delete;
  AndroidWalletPageUI& operator=(const AndroidWalletPageUI&) = delete;
  ~AndroidWalletPageUI() override;

  // Instantiates the implementor of the mojom::PageHandlerFactory mojo
  // interface passing the pending receiver that will be internally bound.
  void BindInterface(
      mojo::PendingReceiver<wootz_wallet::mojom::PageHandlerFactory> receiver);
  WEB_UI_CONTROLLER_TYPE_DECL();

 private:
  // wootz_wallet::mojom::PageHandlerFactory:
  void CreatePageHandler(
      mojo::PendingReceiver<wootz_wallet::mojom::PageHandler> page_receiver,
      mojo::PendingReceiver<wootz_wallet::mojom::WalletHandler> wallet_receiver,
      mojo::PendingReceiver<wootz_wallet::mojom::JsonRpcService>
          json_rpc_service,
      mojo::PendingReceiver<wootz_wallet::mojom::BitcoinWalletService>
          bitcoin_wallet_service_receiver,
      mojo::PendingReceiver<wootz_wallet::mojom::ZCashWalletService>
          zcash_wallet_service_receiver,
      mojo::PendingReceiver<wootz_wallet::mojom::SwapService> swap_service,
      mojo::PendingReceiver<wootz_wallet::mojom::AssetRatioService>
          asset_ratio_service,
      mojo::PendingReceiver<wootz_wallet::mojom::KeyringService>
          keyring_service,
      mojo::PendingReceiver<wootz_wallet::mojom::BlockchainRegistry>
          blockchain_registry,
      mojo::PendingReceiver<wootz_wallet::mojom::TxService> tx_service,
      mojo::PendingReceiver<wootz_wallet::mojom::EthTxManagerProxy>
          eth_tx_manager_proxy,
      mojo::PendingReceiver<wootz_wallet::mojom::SolanaTxManagerProxy>
          solana_tx_manager_proxy,
      mojo::PendingReceiver<wootz_wallet::mojom::FilTxManagerProxy>
          filecoin_tx_manager_proxy,
      mojo::PendingReceiver<wootz_wallet::mojom::WootzWalletService>
          wootz_wallet_service,
      mojo::PendingReceiver<wootz_wallet::mojom::WootzWalletP3A>
          wootz_wallet_p3a,
      mojo::PendingReceiver<wootz_wallet::mojom::IpfsService>
          ipfs_service_receiver,
      mojo::PendingReceiver<wootz_wallet::mojom::MeldIntegrationService>
          meld_integration_service) override;

  std::unique_ptr<AndroidWalletPageHandler> page_handler_;
  std::unique_ptr<wootz_wallet::WalletHandler> wallet_handler_;

  mojo::Receiver<wootz_wallet::mojom::PageHandlerFactory>
      page_factory_receiver_{this};
};

#endif  // CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_ANDROID_ANDROID_WALLET_PAGE_UI_H_
