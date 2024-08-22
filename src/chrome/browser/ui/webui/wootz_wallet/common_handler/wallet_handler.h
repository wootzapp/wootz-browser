// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_COMMON_HANDLER_WALLET_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_COMMON_HANDLER_WALLET_HANDLER_H_

#include "base/memory/raw_ptr.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"

class Profile;

namespace wootz_wallet {

class WootzWalletService;

class WalletHandler : public mojom::WalletHandler {
 public:
  WalletHandler(mojo::PendingReceiver<mojom::WalletHandler> receiver,
                Profile* profile);

  WalletHandler(const WalletHandler&) = delete;
  WalletHandler& operator=(const WalletHandler&) = delete;
  ~WalletHandler() override;

  // mojom::WalletHandler:
  void GetWalletInfo(GetWalletInfoCallback) override;

 private:
  mojo::Receiver<mojom::WalletHandler> receiver_;

  const raw_ptr<WootzWalletService> wootz_wallet_service_ = nullptr;
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_UI_WEBUI_WOOTZ_WALLET_COMMON_HANDLER_WALLET_HANDLER_H_
