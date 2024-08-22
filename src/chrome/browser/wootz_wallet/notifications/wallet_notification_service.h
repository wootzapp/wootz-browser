/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_NOTIFICATIONS_WALLET_NOTIFICATION_SERVICE_H_
#define CHROME_BROWSER_WOOTZ_WALLET_NOTIFICATIONS_WALLET_NOTIFICATION_SERVICE_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/keyed_service/core/keyed_service.h"
#include "mojo/public/cpp/bindings/receiver.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace wootz_wallet {
class WootzWalletService;

class WalletNotificationService : public KeyedService,
                                  public mojom::TxServiceObserver {
 public:
  WalletNotificationService(WootzWalletService* wootz_wallet_service,
                            content::BrowserContext* context);
  ~WalletNotificationService() override;
  WalletNotificationService(const WalletNotificationService&) = delete;
  WalletNotificationService operator=(const WalletNotificationService&) =
      delete;

  // mojom::TxServiceObserver
  void OnNewUnapprovedTx(mojom::TransactionInfoPtr tx_info) override {}
  void OnUnapprovedTxUpdated(mojom::TransactionInfoPtr tx_info) override {}
  void OnTransactionStatusChanged(mojom::TransactionInfoPtr tx_info) override;
  void OnTxServiceReset() override {}

 private:
  friend class WalletNotificationServiceUnitTest;

  bool ShouldDisplayUserNotification(mojom::TransactionStatus status);
  void DisplayUserNotification(mojom::TransactionStatus status,
                               const std::string& address,
                               const std::string& tx_id);

  raw_ptr<content::BrowserContext> context_;
  mojo::Receiver<mojom::TxServiceObserver> tx_observer_receiver_{this};
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_NOTIFICATIONS_WALLET_NOTIFICATION_SERVICE_H_
