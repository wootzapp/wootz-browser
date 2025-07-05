/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/common/hash_utils.h"

#include <algorithm>
#include <string>

#include "base/check.h"
#include "base/containers/adapters.h"
#include "base/containers/span.h"
#include "base/strings/string_split.h"
#include "components/wootz_wallet/common/eth_abi_utils.h"
#include "components/wootz_wallet/common/hex_utils.h"
#include "third_party/bitcoin-core/src/src/crypto/ripemd160.h"
#include "third_party/ethash/src/include/ethash/keccak.h"
#include "crypto/sha2.h"

namespace wootz_wallet {
namespace {

std::array<uint8_t, 64> ConcatArrays(const std::array<uint8_t, 32>& arr1,
                                     const std::array<uint8_t, 32>& arr2) {
  std::array<uint8_t, 64> result;
  std::copy(arr1.begin(), arr1.end(), result.begin());
  std::copy(arr2.begin(), arr2.end(), result.begin() + 32);
  return result;
}

}  // namespace

std::string KeccakHash(const std::string& input, bool to_hex) {
  std::vector<uint8_t> bytes(input.begin(), input.end());
  std::vector<uint8_t> result = KeccakHash(bytes);
  std::string result_str(result.begin(), result.end());
  return to_hex ? ToHex(result_str) : result_str;
}

std::vector<uint8_t> KeccakHash(const std::vector<uint8_t>& input) {
  auto hash = ethash_keccak256(input.data(), input.size());
  return std::vector<uint8_t>(hash.bytes, hash.bytes + 32);
}

eth_abi::Bytes32 KeccakHashBytes32(base::span<const uint8_t> input) {
  auto hash = ethash_keccak256(input.data(), input.size());
  eth_abi::Bytes32 result;
  static_assert(sizeof(result) == sizeof(hash.bytes));
  std::copy(hash.bytes, hash.bytes + sizeof(hash.bytes), result.begin());
  return result;
}

std::string GetFunctionHash(const std::string& input) {
  std::string result = KeccakHash(input);
  return result.substr(0, std::min(static_cast<size_t>(10), result.length()));
}

eth_abi::Bytes4 GetFunctionHashBytes4(const std::string& input) {
  auto full_hash = KeccakHashBytes32(base::as_bytes(base::span<const char>(input.data(), input.size())));
  eth_abi::Bytes4 bytes_result;
  std::copy(full_hash.begin(), full_hash.begin() + 4, bytes_result.begin());
  return bytes_result;
}

eth_abi::Bytes32 Namehash(const std::string& name) {
  eth_abi::Bytes32 hash = {};
  std::vector<std::string> labels =
      SplitString(name, ".", base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);

  for (const auto& label : base::Reversed(labels)) {
    auto label_hash = KeccakHashBytes32(base::as_bytes(base::span<const char>(label.data(), label.size())));
    hash = KeccakHashBytes32(ConcatArrays(hash, label_hash));
  }
  return hash;
}

SHA256HashArray DoubleSHA256Hash(base::span<const uint8_t> input) {
  return crypto::SHA256Hash(crypto::SHA256Hash(input));
}

std::vector<uint8_t> Hash160(base::span<const uint8_t> input) {
  std::vector<uint8_t> result(CRIPEMD160::OUTPUT_SIZE);

  std::array<uint8_t, crypto::kSHA256Length> sha256hash =
      crypto::SHA256Hash(input);
  DCHECK(!sha256hash.empty());

  CRIPEMD160()
      .Write(sha256hash.data(), sha256hash.size())
      .Finalize(result.data());

  return result;
}

}  // namespace wootz_wallet