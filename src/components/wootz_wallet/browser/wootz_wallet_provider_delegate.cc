/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/wootz_wallet_provider_delegate.h"

namespace wootz_wallet {

bool WootzWalletProviderDelegate::IsSolanaAccountConnected(
    const std::string& account) {
  return false;
}

}  // namespace wootz_wallet
