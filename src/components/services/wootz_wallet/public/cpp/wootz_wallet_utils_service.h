/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_SERVICES_WOOTZ_WALLET_PUBLIC_CPP_WOOTZ_WALLET_UTILS_SERVICE_H_
#define COMPONENTS_SERVICES_WOOTZ_WALLET_PUBLIC_CPP_WOOTZ_WALLET_UTILS_SERVICE_H_

#include "components/services/wootz_wallet/public/mojom/wootz_wallet_utils_service.mojom.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace wootz_wallet {

/**
 * Launches and communicates mojom::WootzWalletUtilsService in separate process.
 */
class WootzWalletUtilsService {
 public:
  WootzWalletUtilsService();
  ~WootzWalletUtilsService();
  WootzWalletUtilsService(const WootzWalletUtilsService&) = delete;
  WootzWalletUtilsService& operator=(const WootzWalletUtilsService&) = delete;

  // Creates decoder in wootz wallet utils process and provides handles.
  void CreateZCashDecoder(
      mojo::PendingAssociatedReceiver<zcash::mojom::ZCashDecoder> receiver);

  static WootzWalletUtilsService* GetInstance();

 private:
  void MaybeLaunchService();

  mojo::Remote<mojom::WootzWalletUtilsService> wootz_wallet_utils_service_;

  base::WeakPtrFactory<WootzWalletUtilsService> weak_ptr_factory_{this};
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_SERVICES_WOOTZ_WALLET_PUBLIC_CPP_WOOTZ_WALLET_UTILS_SERVICE_H_
