/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/eth_topics_builder.h"

#include <utility>

#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/common/hash_utils.h"
#include "components/wootz_wallet/common/hex_utils.h"

namespace wootz_wallet {

bool MakeAssetDiscoveryTopics(
    const std::vector<std::string>& to_account_addresses,
    base::Value::List* topics) {
  // First topic matches full keccak hash of the erc20::Transfer event signature
  topics->Append(wootz_wallet::KeccakHash("Transfer(address,address,uint256)"));

  // Second topic matches everything (any from_address)
  topics->Append(base::Value());

  // Third topic matches any of the to_addresses
  base::Value::List to_address_topic;
  for (const auto& account_address : to_account_addresses) {
    std::string padded_address;
    if (!wootz_wallet::PadHexEncodedParameter(account_address,
                                              &padded_address)) {
      return false;
    }
    to_address_topic.Append(padded_address);
  }
  topics->Append(std::move(to_address_topic));
  return true;
}

}  // namespace wootz_wallet
