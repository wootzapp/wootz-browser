/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/ui/webui/wootz_wallet/nft/nft_ui.h"

#include <string>

#include "components/wootz_wallet/browser/wootz_wallet_constants.h"
#include "components/constants/webui_url_constants.h"
#include "components/l10n/common/localization_util.h"
#include "components/nft_display/resources/grit/nft_display_generated_map.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "chrome/grit/browser_resources.h"
#include "chrome/grit/generated_resources.h"
#include "components/grit/wootz_components_resources.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"
#include "ui/resources/grit/webui_resources.h"

namespace nft {

UntrustedNftUI::UntrustedNftUI(content::WebUI* web_ui)
    : ui::UntrustedWebUIController(web_ui) {
  auto* untrusted_source = content::WebUIDataSource::CreateAndAdd(
      web_ui->GetWebContents()->GetBrowserContext(), kUntrustedNftURL);

  for (const auto& str : wootz_wallet::kLocalizedStrings) {
    std::u16string l10n_str =
        wootz_l10n::GetLocalizedResourceUTF16String(str.id);
    untrusted_source->AddString(str.name, l10n_str);
  }

  untrusted_source->SetDefaultResource(IDR_WOOTZ_WALLET_NFT_DISPLAY_HTML);
  untrusted_source->AddResourcePaths(
      base::make_span(kNftDisplayGenerated, kNftDisplayGeneratedSize));
  untrusted_source->AddFrameAncestor(GURL(kWootzUIWalletPageURL));
  untrusted_source->AddFrameAncestor(GURL(kWootzUIWalletPanelURL));
  webui::SetupWebUIDataSource(
      untrusted_source,
      base::make_span(kNftDisplayGenerated, kNftDisplayGeneratedSize),
      IDR_WOOTZ_WALLET_NFT_DISPLAY_HTML);
  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      std::string("script-src 'self' chrome-untrusted://resources;"));
  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::StyleSrc,
      std::string("style-src 'self' 'unsafe-inline';"));
  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FontSrc,
      std::string("font-src 'self' data:;"));
  untrusted_source->AddResourcePath("load_time_data_deprecated.js",
                                    IDR_WEBUI_JS_LOAD_TIME_DATA_DEPRECATED_JS);
  untrusted_source->UseStringsJs();
  untrusted_source->AddString("wootzWalletNftBridgeUrl", kUntrustedNftURL);
  untrusted_source->AddString("wootzWalletTrezorBridgeUrl",
                              kUntrustedTrezorURL);
  untrusted_source->AddString("wootzWalletLedgerBridgeUrl",
                              kUntrustedLedgerURL);
  untrusted_source->AddString("wootzWalletMarketUiBridgeUrl",
                              kUntrustedMarketURL);
  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ImgSrc,
      std::string("img-src 'self' https: data:;"));
}

UntrustedNftUI::~UntrustedNftUI() = default;

std::unique_ptr<content::WebUIController>
UntrustedNftUIConfig::CreateWebUIController(content::WebUI* web_ui,
                                            const GURL& url) {
  return std::make_unique<UntrustedNftUI>(web_ui);
}

UntrustedNftUIConfig::UntrustedNftUIConfig()
    : WebUIConfig(content::kChromeUIUntrustedScheme, kUntrustedNftHost) {}

}  // namespace nft
