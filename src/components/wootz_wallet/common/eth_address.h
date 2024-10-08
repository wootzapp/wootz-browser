/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_COMMON_ETH_ADDRESS_H_
#define COMPONENTS_WOOTZ_WALLET_COMMON_ETH_ADDRESS_H_

#include <string>
#include <vector>

#include "base/containers/span.h"
#include "components/wootz_wallet/common/wootz_wallet_types.h"

namespace wootz_wallet {

inline constexpr size_t kEthAddressLength = 20u;

class EthAddress {
 public:
  // public key must be uncompressed and no header byte so its length is 64
  // bytes
  static EthAddress FromPublicKey(const std::vector<uint8_t>& public_key);
  // input should be a valid address with 20 bytes hex representation starting
  // with 0x
  static EthAddress FromHex(const std::string& input);
  static EthAddress FromBytes(base::span<const uint8_t> bytes);
  static EthAddress ZeroAddress();
  static bool IsValidAddress(const std::string& input);
  static std::optional<std::string> ToEip1191ChecksumAddress(
      const std::string& address,
      const std::string& chain_id);

  EthAddress();
  EthAddress(const EthAddress& other);
  ~EthAddress();
  bool operator==(const EthAddress& other) const;
  bool operator!=(const EthAddress& other) const;

  bool IsEmpty() const;
  bool IsValid() const;
  bool IsZeroAddress() const;
  const std::vector<uint8_t>& bytes() const { return bytes_; }

  std::string ToHex() const;
  // EIP55 + EIP1191
  std::string ToChecksumAddress(uint256_t eip1191_chaincode = 0) const;

 private:
  explicit EthAddress(std::vector<uint8_t> bytes);
  explicit EthAddress(base::span<const uint8_t> bytes);

  std::vector<uint8_t> bytes_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_COMMON_ETH_ADDRESS_H_
