/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/ethereum_keyring.h"

#include <optional>

#include "base/base64.h"
#include "base/strings/string_number_conversions.h"
#include "components/wootz_wallet/browser/eth_transaction.h"
#include "components/wootz_wallet/common/eth_address.h"
#include "components/wootz_wallet/common/hash_utils.h"

#include <iomanip>

namespace wootz_wallet {

namespace {

// Get the 32 byte message hash
std::vector<uint8_t> GetMessageHash(const std::vector<uint8_t>& message) {
  std::string prefix("\x19");
  prefix += std::string("Ethereum Signed Message:\n" +
                        base::NumberToString(message.size()));
  std::vector<uint8_t> hash_input(prefix.begin(), prefix.end());
  hash_input.insert(hash_input.end(), message.begin(), message.end());
  return wootz_wallet::KeccakHash(hash_input);
}

}  // namespace

EthereumKeyring::EthereumKeyring(base::span<const uint8_t> seed)
    : Secp256k1HDKeyring(seed, GetRootPath(mojom::KeyringId::kDefault)) {}

// static
bool EthereumKeyring::RecoverAddress(const std::vector<uint8_t>& message,
                                     const std::vector<uint8_t>& signature,
                                     std::string* address) {
  CHECK(address);
  // A compact ECDSA signature (recovery id byte + 64 bytes).
  if (signature.size() != kCompactSignatureSize + 1) {
    return false;
  }

  std::vector<uint8_t> signature_only = signature;
  uint8_t v = signature_only.back();
  if (v < 27) {
    VLOG(1) << "v should be >= 27";
    return false;
  }

  // v = chain_id ? recid + chain_id * 2 + 35 : recid + 27;
  // So recid = v - 27 when chain_id is 0
  uint8_t recid = v - 27;
  signature_only.pop_back();
  std::vector<uint8_t> hash = GetMessageHash(message);

  // Public keys (in scripts) are given as 04 <x> <y> where x and y are 32
  // byte big-endian integers representing the coordinates of a point on the
  // curve or in compressed form given as <sign> <x> where <sign> is 0x02 if
  // y is even and 0x03 if y is odd.
  HDKey key;
  std::vector<uint8_t> public_key =
      key.RecoverCompact(false, hash, signature_only, recid);
  if (public_key.size() != 65) {
    VLOG(1) << "public key should be 65 bytes";
    return false;
  }

  uint8_t first_byte = *public_key.begin();
  public_key.erase(public_key.begin());
  if (first_byte != 4) {
    VLOG(1) << "First byte of public key should be 4";
    return false;
  }

  EthAddress addr = EthAddress::FromPublicKey(public_key);
  *address = addr.ToChecksumAddress();
  return true;
}

std::vector<uint8_t> EthereumKeyring::SignMessage(
    const std::string& address,
    const std::vector<uint8_t>& message,
    uint256_t chain_id,
    bool is_eip712) {

  LOG(ERROR) << "JANGID: SignMessage started for address: " << address;
  LOG(ERROR) << "JANGID: Message size: " << message.size();
  LOG(ERROR) << "JANGID: Is EIP712: " << (is_eip712 ? "Yes" : "No");
  
  HDKey* hd_key = GetHDKeyFromAddress(address);
  if (!hd_key) {
    LOG(ERROR) << "JANGID: Failed to get HDKey for address: " << address;
    return std::vector<uint8_t>();
  }
  
LOG(ERROR) << "JANGID: Successfully retrieved HDKey for address: " << address;

  LOG(ERROR) << "JANGID: Successfully retrieved HDKey " << hd_key;
  
  // Log HDKey details (using only available methods)
  LOG(ERROR) << "JANGID: HDKey details:";
  LOG(ERROR) << "  - Public key (first 4 bytes): " 
             << base::HexEncode(hd_key->GetPublicKeyBytes().data(), 10);
  LOG(ERROR) << "  - Private key (first 4 bytes): " 
             << base::HexEncode(hd_key->GetPrivateKeyBytes().data(), 10);

  std::vector<uint8_t> hash;
  if (!is_eip712) {
    LOG(ERROR) << "JANGID: Calculating message hash for non-EIP712 message";
    hash = GetMessageHash(message);
  } else {
    LOG(ERROR) << "JANGID: Processing EIP712 message";
    // eip712 hash is Keccak
    if (message.size() != 32) {
      LOG(ERROR) << "JANGID: Invalid EIP712 message size: " << message.size();
      return std::vector<uint8_t>();
    }

    hash = message;
  }
  LOG(ERROR) << "JANGID: Hash size: " << hash.size();
  
  LOG(ERROR) << "JANGID: Message size: " << message.size();
  LOG(ERROR) << "JANGID: Message (hex): " << base::HexEncode(message.data(), message.size());

  // ... (later in the function)

  LOG(ERROR) << "JANGID: Hash size: " << hash.size();
  LOG(ERROR) << "JANGID: Hash (hex): " << base::HexEncode(hash.data(), hash.size());

  int recid;
  LOG(ERROR) << "JANGID: Signing message with HDKey";
  std::vector<uint8_t> signature = hd_key->SignCompact(hash, &recid);
  LOG(ERROR) << "JANGID: Signature size: " << signature.size();
  LOG(ERROR) << "JANGID: Recovery ID: " << recid;

  // Convert signature to hex string
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (const auto& byte : signature) {
    ss << std::setw(2) << static_cast<int>(byte);
  }
  LOG(ERROR) << "JANGID: Signature (hex): " << ss.str();

  uint8_t v =
      static_cast<uint8_t>(chain_id ? recid + chain_id * 2 + 35 : recid + 27);
  LOG(ERROR) << "JANGID: Calculated v: " << static_cast<int>(v);
  signature.push_back(v);

  // Convert final signature (including v) to hex string
  ss.str("");
  ss.clear();
  for (const auto& byte : signature) {
    ss << std::setw(2) << static_cast<int>(byte);
  }
  LOG(ERROR) << "JANGID: Final signature (hex): " << ss.str();

  LOG(ERROR) << "JANGID: Final signature size: " << signature.size();
  LOG(ERROR) << "JANGID: SignMessage completed successfully";

  return signature;
}

void EthereumKeyring::SignTransaction(const std::string& address,
                                      EthTransaction* tx,
                                      uint256_t chain_id) {
  HDKey* hd_key = GetHDKeyFromAddress(address);
  if (!hd_key || !tx) {
    return;
  }

  const std::vector<uint8_t> message = tx->GetMessageToSign(chain_id);
  int recid;
  const std::vector<uint8_t> signature = hd_key->SignCompact(message, &recid);
  tx->ProcessSignature(signature, recid, chain_id);
}

std::string EthereumKeyring::GetAddressInternal(const HDKey& hd_key) const {
  const std::vector<uint8_t> public_key = hd_key.GetUncompressedPublicKey();
  // trim the header byte 0x04
  const std::vector<uint8_t> pubkey_no_header(public_key.begin() + 1,
                                              public_key.end());
  EthAddress addr = EthAddress::FromPublicKey(pubkey_no_header);

  // TODO(darkdh): chain id op code
  return addr.ToChecksumAddress();
}

bool EthereumKeyring::GetPublicKeyFromX25519_XSalsa20_Poly1305(
    const std::string& address,
    std::string* key) {
  HDKey* hd_key = GetHDKeyFromAddress(address);
  if (!hd_key) {
    return false;
  }
  const std::vector<uint8_t> public_key =
      hd_key->GetPublicKeyFromX25519_XSalsa20_Poly1305();
  if (public_key.empty()) {
    return false;
  }
  *key = base::Base64Encode(public_key);
  return true;
}

std::optional<std::vector<uint8_t>>
EthereumKeyring::DecryptCipherFromX25519_XSalsa20_Poly1305(
    const std::string& version,
    const std::vector<uint8_t>& nonce,
    const std::vector<uint8_t>& ephemeral_public_key,
    const std::vector<uint8_t>& ciphertext,
    const std::string& address) {
  HDKey* hd_key = GetHDKeyFromAddress(address);
  if (!hd_key) {
    return std::nullopt;
  }
  return hd_key->DecryptCipherFromX25519_XSalsa20_Poly1305(
      version, nonce, ephemeral_public_key, ciphertext);
}

std::string EthereumKeyring::EncodePrivateKeyForExport(
    const std::string& address) {
  HDKey* hd_key = GetHDKeyFromAddress(address);
  if (!hd_key) {
    return std::string();
  }

  return base::ToLowerASCII(base::HexEncode(hd_key->GetPrivateKeyBytes()));
}

std::unique_ptr<HDKey> EthereumKeyring::DeriveAccount(uint32_t index) const {
  // m/44'/60'/0'/0/{index}
  return root_->DeriveNormalChild(index);
}

}  // namespace wootz_wallet
