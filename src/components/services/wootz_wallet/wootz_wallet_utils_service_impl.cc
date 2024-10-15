/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/services/wootz_wallet/wootz_wallet_utils_service_impl.h"

#include <memory>
#include <utility>

#include "components/services/wootz_wallet/zcash/zcash_decoder.h"

namespace wootz_wallet {

WootzWalletUtilsServiceImpl::WootzWalletUtilsServiceImpl(
    mojo::PendingReceiver<mojom::WootzWalletUtilsService> receiver)
    : receiver_(this, std::move(receiver)) {}

WootzWalletUtilsServiceImpl::~WootzWalletUtilsServiceImpl() = default;

void WootzWalletUtilsServiceImpl::CreateZCashDecoderService(
    mojo::PendingAssociatedReceiver<zcash::mojom::ZCashDecoder>
        zcash_decoder_receiver) {
  if (!instance_) {
    instance_ = mojo::MakeSelfOwnedAssociatedReceiver(
        std::make_unique<ZCashDecoder>(), std::move(zcash_decoder_receiver));
  }
}

}  // namespace wootz_wallet
