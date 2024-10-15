/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_IMPORT_KEYRING_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_IMPORT_KEYRING_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "components/wootz_wallet/browser/bitcoin/bitcoin_base_keyring.h"
#include "components/wootz_wallet/browser/internal/hd_key.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

class BitcoinImportKeyring : public BitcoinBaseKeyring {
 public:
  explicit BitcoinImportKeyring(bool testnet);
  ~BitcoinImportKeyring() override;
  BitcoinImportKeyring(const BitcoinImportKeyring&) = delete;
  BitcoinImportKeyring& operator=(const BitcoinImportKeyring&) = delete;

  bool AddAccount(uint32_t account, const std::string& payload);
  bool RemoveAccount(uint32_t account);

  // BitcoinBaseKeyring:
  mojom::BitcoinAddressPtr GetAddress(
      uint32_t account,
      const mojom::BitcoinKeyId& key_id) override;
  std::optional<std::vector<uint8_t>> GetPubkey(
      uint32_t account,
      const mojom::BitcoinKeyId& key_id) override;
  std::optional<std::vector<uint8_t>> SignMessage(
      uint32_t account,
      const mojom::BitcoinKeyId& key_id,
      base::span<const uint8_t, 32> message) override;

 private:
  std::map<uint32_t, std::unique_ptr<HDKey>> accounts_;
  HDKey* GetAccountByIndex(uint32_t account);

  std::unique_ptr<HDKey> DeriveKey(uint32_t account,
                                   const mojom::BitcoinKeyId& key_id);

  bool testnet_ = false;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_BITCOIN_BITCOIN_IMPORT_KEYRING_H_
