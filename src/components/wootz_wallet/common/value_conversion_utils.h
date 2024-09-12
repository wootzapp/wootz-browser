/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_VALUE_CONVERSION_UTILS_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_VALUE_CONVERSION_UTILS_H_

#include <optional>
#include <string>
#include <vector>

#include "base/values.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "url/origin.h"

namespace wootz_wallet {

std::optional<std::string> ExtractChainIdFromValue(
    const base::Value::Dict* dict);
base::Value::Dict NetworkInfoToValue(const mojom::NetworkInfo& info);
mojom::NetworkInfoPtr ValueToNetworkInfo(const base::Value& value);
mojom::NetworkInfoPtr ParseEip3085Payload(const base::Value& value);
base::Value::List PermissionRequestResponseToValue(
    const url::Origin& origin,
    const std::vector<std::string>& accounts);

mojom::BlockchainTokenPtr ValueToBlockchainToken(
    const base::Value::Dict& value);
base::Value::Dict BlockchainTokenToValue(
    const mojom::BlockchainTokenPtr& token);

// Returns index of the first URL to use that:
// 1. Has no variables in it like ${INFURA_API_KEY}
// 2. Is HTTP or HTTPS
// Otherwise returns 0.
int GetFirstValidChainURLIndex(const std::vector<GURL>& chain_urls);

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_VALUE_CONVERSION_UTILS_H_
