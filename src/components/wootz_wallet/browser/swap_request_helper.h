/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_SWAP_REQUEST_HELPER_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_SWAP_REQUEST_HELPER_H_

#include <optional>
#include <string>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

namespace jupiter {
std::optional<std::string> EncodeTransactionParams(
    const mojom::JupiterTransactionParams& params);
}  // namespace jupiter

namespace lifi {
std::optional<std::string> EncodeQuoteParams(
    mojom::SwapQuoteParamsPtr params,
    const std::optional<std::string>& fee_param);

std::optional<std::string> EncodeTransactionParams(mojom::LiFiStepPtr step);
}  // namespace lifi

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_SWAP_REQUEST_HELPER_H_
