/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <optional>
#include <string>
#include <vector>

#include "base/values.h"
#include "components/demo_wallet/common/solana_wallet.mojom.h"

namespace solana_wallet {

struct SolanaAccountMeta {
  SolanaAccountMeta(const std::string& pubkey,
                    std::optional<uint8_t> address_table_lookup_index,
                    bool is_signer,
                    bool is_writable);
  ~SolanaAccountMeta();

  SolanaAccountMeta(const SolanaAccountMeta&);
  bool operator==(const SolanaAccountMeta&) const;

  mojom::SolanaAccountMetaPtr ToMojomSolanaAccountMeta() const;
  base::Value::Dict ToValue() const;

  static void FromMojomSolanaAccountMetas(
      const std::vector<mojom::SolanaAccountMetaPtr>& mojom_account_metas,
      std::vector<SolanaAccountMeta>* account_metas);
  static std::optional<SolanaAccountMeta> FromValue(
      const base::Value::Dict& value);

  std::string pubkey;
  std::optional<uint8_t> address_table_lookup_index;
  bool is_signer;
  bool is_writable;
};

}  // namespace solana_wallet

#endif
