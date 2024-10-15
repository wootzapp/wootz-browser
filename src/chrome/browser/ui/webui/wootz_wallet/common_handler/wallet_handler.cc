// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "chrome/browser/ui/webui/wootz_wallet/common_handler/wallet_handler.h"

#include <utility>

#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/browser/keyring_service.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/common_utils.h"
#include "chrome/browser/profiles/profile.h"

namespace wootz_wallet {

WalletHandler::WalletHandler(
    mojo::PendingReceiver<mojom::WalletHandler> receiver,
    Profile* profile)
    : receiver_(this, std::move(receiver)),
      wootz_wallet_service_(
          WootzWalletServiceFactory::GetServiceForContext(profile)) {}

WalletHandler::~WalletHandler() = default;

// TODO(apaymyshev): this is the only method in WalletHandler. Should be merged
// into WootzWalletService.
void WalletHandler::GetWalletInfo(GetWalletInfoCallback callback) {
  if (!wootz_wallet_service_) {
    std::move(callback).Run(nullptr);
    return;
  }

  auto* keyring_service = wootz_wallet_service_->keyring_service();

  std::move(callback).Run(mojom::WalletInfo::New(
      keyring_service->IsWalletCreatedSync(), keyring_service->IsLockedSync(),
      keyring_service->IsWalletBackedUpSync(), IsBitcoinEnabled(),
      IsBitcoinImportEnabled(), IsZCashEnabled(), IsAnkrBalancesEnabled(),
      IsTransactionSimulationsEnabled()));
}

}  // namespace wootz_wallet
