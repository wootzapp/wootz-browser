// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_WALLET_VERSION_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WALLET_VERSION_UI_H_

#include "build/build_config.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/web_ui_data_source.h"

class WalletUIConfig : public WebUIConfig {
 public:
  WalletUIConfig()
      : WebUIConfig(content::kChromeUIScheme, kChromeUIWalletHost) {}
  ~WalletUIConfig() override = default;

  std::unique_ptr<content::WebUIController> CreateWebUIController(
      content::WebUI* web_ui) override {
    return std::make_unique<WalletUI>(web_ui);
  }
};
