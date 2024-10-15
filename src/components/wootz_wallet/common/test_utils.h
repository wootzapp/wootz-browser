/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_TEST_UTILS_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_TEST_UTILS_H_

#include <string>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"

namespace wootz_wallet {

bool AllCoinsTested();

inline constexpr char kHttpURL[] = "http://bad.com/";
inline constexpr char kHttpLocalhostURL[] = "http://localhost:8080/";

mojom::NetworkInfo GetTestNetworkInfo1(
    const std::string& chain_id = "chain_id",
    mojom::CoinType coin = mojom::CoinType::ETH);
mojom::NetworkInfo GetTestNetworkInfo2(
    const std::string& chain_id = "chain_id2",
    mojom::CoinType coin = mojom::CoinType::ETH);
mojom::NetworkInfo GetTestNetworkInfoWithHttpURL(
    const std::string& chain_id = "http_url",
    mojom::CoinType coin = mojom::CoinType::ETH);

// Matcher to check equality of two mojo structs. Matcher needs copyable value
// which is not possible for some mojo types, so wrapping it with RefCounted.
template <typename T>
auto EqualsMojo(const T& value) {
  return testing::Truly(
      [value = base::MakeRefCounted<base::RefCountedData<T>>(value.Clone())](
          const T& candidate) { return mojo::Equals(candidate, value->data); });
}

namespace mojom {

// These are pretty printers for gmock expect/assert failures.
void PrintTo(const BitcoinAddressPtr& address, ::std::ostream* os);
void PrintTo(const BlockchainTokenPtr& token, ::std::ostream* os);
void PrintTo(const BitcoinBalancePtr& balance, ::std::ostream* os);
void PrintTo(const BitcoinKeyId& key_id, ::std::ostream* os);

}  // namespace mojom

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_TEST_UTILS_H_
