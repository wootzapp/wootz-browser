/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_SERVICES_WOOTZ_WALLET_WOOTZ_WALLET_UTILS_SERVICE_IMPL_H_
#define COMPONENTS_SERVICES_WOOTZ_WALLET_WOOTZ_WALLET_UTILS_SERVICE_IMPL_H_

#include "components/services/wootz_wallet/public/mojom/wootz_wallet_utils_service.mojom.h"
#include "components/services/wootz_wallet/public/mojom/zcash_decoder.mojom.h"
#include "mojo/public/cpp/bindings/pending_associated_remote.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/self_owned_associated_receiver.h"

namespace wootz_wallet {

class WootzWalletUtilsServiceImpl : public mojom::WootzWalletUtilsService {
 public:
  explicit WootzWalletUtilsServiceImpl(
      mojo::PendingReceiver<mojom::WootzWalletUtilsService> receiver);

  ~WootzWalletUtilsServiceImpl() override;

  WootzWalletUtilsServiceImpl(const WootzWalletUtilsServiceImpl&) = delete;
  WootzWalletUtilsServiceImpl& operator=(const WootzWalletUtilsServiceImpl&) =
      delete;

  void CreateZCashDecoderService(
      mojo::PendingAssociatedReceiver<zcash::mojom::ZCashDecoder>
          zcash_decoder_receiver) override;

 private:
  mojo::Receiver<mojom::WootzWalletUtilsService> receiver_;
  mojo::SelfOwnedAssociatedReceiverRef<zcash::mojom::ZCashDecoder> instance_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_SERVICES_WOOTZ_WALLET_WOOTZ_WALLET_UTILS_SERVICE_IMPL_H_
