/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_NFT_NFT_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_NFT_NFT_UI_H_

#include <memory>

#include "content/public/browser/web_ui.h"
#include "content/public/browser/webui_config.h"
#include "content/public/common/url_constants.h"
#include "ui/webui/untrusted_web_ui_controller.h"

namespace nft {

class UntrustedNftUI : public ui::UntrustedWebUIController {
 public:
  explicit UntrustedNftUI(content::WebUI* web_ui);
  UntrustedNftUI(const UntrustedNftUI&) = delete;
  UntrustedNftUI& operator=(const UntrustedNftUI&) = delete;
  ~UntrustedNftUI() override;
};

class UntrustedNftUIConfig : public content::WebUIConfig {
 public:
  UntrustedNftUIConfig();
  ~UntrustedNftUIConfig() override = default;

  std::unique_ptr<content::WebUIController> CreateWebUIController(
      content::WebUI* web_ui,
      const GURL& url) override;
};

}  // namespace nft

#endif  // CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_NFT_NFT_UI_H_
