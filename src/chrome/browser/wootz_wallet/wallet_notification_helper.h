/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_WALLET_NOTIFICATION_HELPER_H_
#define CHROME_BROWSER_WOOTZ_WALLET_WALLET_NOTIFICATION_HELPER_H_

namespace content {
class BrowserContext;
}  // namespace content

namespace wootz_wallet {

class TxService;

void RegisterWalletNotificationService(content::BrowserContext* context,
                                       TxService* service);

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_WALLET_NOTIFICATION_HELPER_H_
