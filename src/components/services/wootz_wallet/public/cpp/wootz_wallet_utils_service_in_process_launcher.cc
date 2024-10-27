/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_SERVICES_WOOTZ_WALLET_CONTENT_WOOTZ_WALLET_UTILS_SERVICE_LAUNCHER_H_
#define COMPONENTS_SERVICES_WOOTZ_WALLET_CONTENT_WOOTZ_WALLET_UTILS_SERVICE_LAUNCHER_H_

#include "components/services/wootz_wallet/public/cpp/wootz_wallet_utils_service_in_process_launcher.h"

#include <memory>
#include <utility>

#include "components/services/wootz_wallet/wootz_wallet_utils_service_impl.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"

namespace wootz_wallet {

void LaunchInProcessWootzWalletUtilsService(
    mojo::PendingReceiver<mojom::WootzWalletUtilsService> receiver) {
  mojo::MakeSelfOwnedReceiver(
      std::make_unique<WootzWalletUtilsServiceImpl>(
          mojo::PendingReceiver<mojom::WootzWalletUtilsService>()),
      std::move(receiver));
}

}  // namespace wootz_wallet

#endif  // COMPONENTS_SERVICES_WOOTZ_WALLET_CONTENT_WOOTZ_WALLET_UTILS_SERVICE_LAUNCHER_H_
