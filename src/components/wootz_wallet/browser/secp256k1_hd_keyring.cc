/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/secp256k1_hd_keyring.h"

#include <utility>

#include "base/containers/contains.h"

namespace wootz_wallet {

Secp256k1HDKeyring::Secp256k1HDKeyring(base::span<const uint8_t> seed,
                                       const std::string& hd_path)
    : root_(ConstructRootHDKey(seed, hd_path)) {}

Secp256k1HDKeyring::~Secp256k1HDKeyring() = default;

// static
std::unique_ptr<HDKey> Secp256k1HDKeyring::ConstructRootHDKey(
    base::span<const uint8_t> seed,
    const std::string& hd_path) {
  if (!seed.empty()) {
    if (auto master_key = HDKey::GenerateFromSeed(seed)) {
      return master_key->DeriveChildFromPath(hd_path);
    }
  }
  return nullptr;
}

std::string Secp256k1HDKeyring::GetDiscoveryAddress(size_t index) const {
  if (auto key = DeriveAccount(index)) {
    return GetAddressInternal(*key);
  }
  return std::string();
}

std::vector<std::string> Secp256k1HDKeyring::GetHDAccountsForTesting() const {
  std::vector<std::string> addresses;
  for (auto& acc : accounts_) {
    addresses.push_back(GetAddressInternal(*acc));
  }
  return addresses;
}

std::vector<std::string> Secp256k1HDKeyring::GetImportedAccountsForTesting()
    const {
  std::vector<std::string> addresses;
  for (auto& acc : imported_accounts_) {
    addresses.push_back(GetAddressInternal(*acc.second));
  }
  return addresses;
}

bool Secp256k1HDKeyring::RemoveImportedAccount(const std::string& address) {
  return imported_accounts_.erase(address) != 0;
}

std::optional<AddedAccountInfo> Secp256k1HDKeyring::AddNewHDAccount() {
  if (!root_) {
    return std::nullopt;
  }

  auto new_acc_index = static_cast<uint32_t>(accounts_.size());
  auto new_account = DeriveAccount(new_acc_index);
  if (!new_account) {
    return std::nullopt;
  }
  auto& added_account = accounts_.emplace_back(std::move(new_account));
  return AddedAccountInfo{new_acc_index, GetAddressInternal(*added_account)};
}

void Secp256k1HDKeyring::RemoveLastHDAccount() {
  CHECK(!accounts_.empty());
  accounts_.pop_back();
}

std::string Secp256k1HDKeyring::ImportAccount(
    const std::vector<uint8_t>& private_key) {
  std::unique_ptr<HDKey> hd_key = HDKey::GenerateFromPrivateKey(private_key);
  if (!hd_key) {
    return std::string();
  }

  const std::string address = GetAddressInternal(*hd_key);

  if (base::Contains(imported_accounts_, address)) {
    return std::string();
  }

  if (base::ranges::any_of(accounts_, [&](auto& acc) {
        return GetAddressInternal(*acc) == address;
      })) {
    return std::string();
  }

  imported_accounts_[address] = std::move(hd_key);
  return address;
}

HDKey* Secp256k1HDKeyring::GetHDKeyFromAddress(const std::string& address) {
  LOG(ERROR) << "JANGID: GetHDKeyFromAddress started for address: " << address;

  // Check imported accounts
  LOG(ERROR) << "JANGID: Checking imported accounts...";
  const auto imported_accounts_iter = imported_accounts_.find(address);
  if (imported_accounts_iter != imported_accounts_.end()) {
    LOG(ERROR) << "JANGID: HDKey found in imported accounts";
    HDKey* hd_key = imported_accounts_iter->second.get();
    LOG(ERROR) << "JANGID: Imported HDKey pointer: " << hd_key;
    if (hd_key) {
      LOG(ERROR) << "JANGID: Imported HDKey public key: ";
    } else {
      LOG(ERROR) << "JANGID: Imported HDKey is null";
    }
    return hd_key;
  }
  LOG(ERROR) << "JANGID: Address not found in imported accounts";

  // Check HD accounts
  LOG(ERROR) << "JANGID: Checking HD accounts...";
  for (size_t i = 0; i < accounts_.size(); ++i) {
    auto& acc = accounts_[i];
    std::string derived_address = GetAddressInternal(*acc);
    LOG(ERROR) << "JANGID: Checking HD account " << i << ", derived address: " << derived_address;
    
    if (derived_address == address) {
      LOG(ERROR) << "JANGID: HDKey found in HD accounts at index " << i;
      HDKey* hd_key = acc.get();
      LOG(ERROR) << "JANGID: HD account HDKey pointer: " << hd_key;
      if (hd_key) {
        LOG(ERROR) << "JANGID: HD account HDKey public key: ";
      } else {
        LOG(ERROR) << "JANGID: HD account HDKey is null";
      }
      return hd_key;
    }
  }

  LOG(ERROR) << "JANGID: HDKey not found for address: " << address;
  return nullptr;
}

}  // namespace wootz_wallet
