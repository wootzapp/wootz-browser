/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_CONTEXT_UTILS_H_
#define CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_CONTEXT_UTILS_H_

namespace content {
class BrowserContext;
}

namespace wootz_wallet {

bool IsAllowedForContext(content::BrowserContext* context);

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_CONTEXT_UTILS_H_
