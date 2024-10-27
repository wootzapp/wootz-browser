// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "chrome/browser/ui/webui/wootz_wallet/android/android_wallet_page_ui.h"

#include <utility>

#include "base/command_line.h"
#include "chrome/browser/wootz_wallet/asset_ratio_service_factory.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_ipfs_service_factory.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_provider_delegate_impl_helper.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "chrome/browser/wootz_wallet/meld_integration_service_factory.h"
#include "chrome/browser/wootz_wallet/swap_service_factory.h"
#include "chrome/browser/ui/webui/wootz_wallet/wallet_common_ui.h"
#include "chrome/browser/ui/webui/wootz_webui_source.h"
#include "components/wootz_wallet/browser/blockchain_registry.h"
#include "components/wootz_wallet/browser/wootz_wallet_constants.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet_page/resources/grit/wootz_wallet_deposit_page_generated_map.h"
#include "components/wootz_wallet_page/resources/grit/wootz_wallet_fund_wallet_page_generated_map.h"
#include "components/wootz_wallet_page/resources/grit/wootz_wallet_page_generated_map.h"
#include "components/wootz_wallet_page/resources/grit/wootz_wallet_send_page_generated_map.h"
#include "components/wootz_wallet_page/resources/grit/wootz_wallet_swap_page_generated_map.h"
#include "components/constants/webui_url_constants.h"
#include "components/l10n/common/localization_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "components/grit/wootz_components_resources.h"
#include "components/grit/wootz_components_strings.h"
#include "content/public/browser/url_data_source.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/common/url_constants.h"

AndroidWalletPageUI::AndroidWalletPageUI(content::WebUI* web_ui,
                                         const GURL& url)
    : ui::MojoWebUIController(web_ui,
                              true /* Needed for webui browser tests */) {
  auto* profile = Profile::FromWebUI(web_ui);
  content::WebUIDataSource* source =
      content::WebUIDataSource::CreateAndAdd(profile, kWalletPageHost);
  web_ui->AddRequestableScheme(content::kChromeUIUntrustedScheme);

  for (const auto& str : wootz_wallet::kLocalizedStrings) {
    std::u16string l10n_str =
        wootz_l10n::GetLocalizedResourceUTF16String(str.id);
    source->AddString(str.name, l10n_str);
  }

  // Add required resources.
  if (url.host() == kWalletPageHost) {
    webui::SetupWebUIDataSource(source,
                                base::make_span(kWootzWalletPageGenerated,
                                                kWootzWalletPageGeneratedSize),
                                IDR_WALLET_PAGE_HTML);
  } else {
    NOTREACHED_IN_MIGRATION()
        << "Failed to find page resources for:" << url.path();
  }

  source->AddBoolean("isAndroid", true);
  source->AddString("wootzWalletLedgerBridgeUrl", kUntrustedLedgerURL);
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FrameSrc,
      std::string("frame-src ") + kUntrustedTrezorURL + " " +
          kUntrustedLedgerURL + " " + kUntrustedNftURL + " " +
          kUntrustedLineChartURL + " " + kUntrustedMarketURL + ";");
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ImgSrc,
      base::JoinString(
          {"img-src", "'self'", "chrome://resources",
           "chrome://erc-token-images", base::StrCat({"data:", ";"})},
          " "));

  source->DisableTrustedTypesCSP();
  
    source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ConnectSrc, "connect-src *;");
      
  source->AddString("wootzWalletTrezorBridgeUrl", kUntrustedTrezorURL);
  source->AddString("wootzWalletNftBridgeUrl", kUntrustedNftURL);
  source->AddString("wootzWalletLineChartBridgeUrl", kUntrustedLineChartURL);
  source->AddString("wootzWalletMarketUiBridgeUrl", kUntrustedMarketURL);
  source->AddBoolean(wootz_wallet::mojom::kP3ACountTestNetworksLoadTimeKey,
                     base::CommandLine::ForCurrentProcess()->HasSwitch(
                         wootz_wallet::mojom::kP3ACountTestNetworksSwitch));

  wootz_wallet::AddBlockchainTokenImageSource(profile);
}

AndroidWalletPageUI::~AndroidWalletPageUI() = default;

void AndroidWalletPageUI::BindInterface(
    mojo::PendingReceiver<wootz_wallet::mojom::PageHandlerFactory> receiver) {
  page_factory_receiver_.reset();
  page_factory_receiver_.Bind(std::move(receiver));
}

WEB_UI_CONTROLLER_TYPE_IMPL(AndroidWalletPageUI)

void AndroidWalletPageUI::CreatePageHandler(
    mojo::PendingReceiver<wootz_wallet::mojom::PageHandler> page_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::WalletHandler> wallet_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::JsonRpcService>
        json_rpc_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::BitcoinWalletService>
        bitcoin_wallet_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::ZCashWalletService>
        zcash_wallet_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::SwapService>
        swap_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::AssetRatioService>
        asset_ratio_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::KeyringService>
        keyring_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::BlockchainRegistry>
        blockchain_registry_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::TxService> tx_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::EthTxManagerProxy>
        eth_tx_manager_proxy_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::SolanaTxManagerProxy>
        solana_tx_manager_proxy_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::FilTxManagerProxy>
        filecoin_tx_manager_proxy_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::WootzWalletService>
        wootz_wallet_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::WootzWalletP3A>
        wootz_wallet_p3a_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::IpfsService>
        ipfs_service_receiver,
    mojo::PendingReceiver<wootz_wallet::mojom::MeldIntegrationService>
        meld_integration_service) {
  auto* profile = Profile::FromWebUI(web_ui());
  DCHECK(profile);
  page_handler_ = std::make_unique<AndroidWalletPageHandler>(
      std::move(page_receiver), profile, this);
  wallet_handler_ = std::make_unique<wootz_wallet::WalletHandler>(
      std::move(wallet_receiver), profile);

  if (auto* wallet_service =
          wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
              profile)) {
    wallet_service->Bind(std::move(wootz_wallet_service_receiver));
    wallet_service->Bind(std::move(json_rpc_service_receiver));
    wallet_service->Bind(std::move(bitcoin_wallet_service_receiver));
    wallet_service->Bind(std::move(zcash_wallet_service_receiver));
    wallet_service->Bind(std::move(keyring_service_receiver));
    wallet_service->Bind(std::move(tx_service_receiver));
    wallet_service->Bind(std::move(eth_tx_manager_proxy_receiver));
    wallet_service->Bind(std::move(solana_tx_manager_proxy_receiver));
    wallet_service->Bind(std::move(filecoin_tx_manager_proxy_receiver));
    wallet_service->Bind(std::move(wootz_wallet_p3a_receiver));
  }

  wootz_wallet::SwapServiceFactory::BindForContext(
      profile, std::move(swap_service_receiver));
  wootz_wallet::AssetRatioServiceFactory::BindForContext(
      profile, std::move(asset_ratio_service_receiver));
  wootz_wallet::MeldIntegrationServiceFactory::BindForContext(
      profile, std::move(meld_integration_service));

  auto* blockchain_registry = wootz_wallet::BlockchainRegistry::GetInstance();
  if (blockchain_registry) {
    blockchain_registry->Bind(std::move(blockchain_registry_receiver));
  }
  wootz_wallet::WalletInteractionDetected(web_ui()->GetWebContents());
}
