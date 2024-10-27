/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_WALLET_COMMON_UI_H_
#define CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_WALLET_COMMON_UI_H_

#include <stdint.h>

class Profile;
class Browser;

namespace url {
class Origin;
}  // namespace url

namespace content {
class WebContents;
}  // namespace content

namespace wootz_wallet {

void AddBlockchainTokenImageSource(Profile* profile);

bool IsWootzWalletOrigin(const url::Origin& origin);

content::WebContents* GetWebContentsFromTabId(Browser** browser,
                                              int32_t tab_id);
content::WebContents* GetActiveWebContents();

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_WALLET_COMMON_UI_H_