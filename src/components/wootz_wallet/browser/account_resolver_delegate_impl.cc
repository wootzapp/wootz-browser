/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/account_resolver_delegate_impl.h"

namespace wootz_wallet {

AccountResolverDelegateImpl::AccountResolverDelegateImpl(
    KeyringService* keyring_service)
    : keyring_service_(keyring_service) {}

mojom::AccountIdPtr AccountResolverDelegateImpl::ResolveAccountId(
    const std::string* from_account_id,
    const std::string* from_address) {
  const auto& accounts = keyring_service_->GetAllAccountInfos();
  LOG(ERROR) << "ResolveAccountId: Found " << accounts.size() << " accounts";

  if (from_account_id) {
    LOG(ERROR) << "Searching by account_id: " << *from_account_id;
    for (auto& account : accounts) {
      DCHECK(!account->account_id->unique_key.empty());
      if (account->account_id->unique_key == *from_account_id) {
        LOG(ERROR) << "Found matching account by ID: " << account->address;
        LOG(ERROR) << "Found matching unique_key: " << account->account_id->unique_key;
        return account->account_id->Clone();
      }
    }
    LOG(ERROR) << "No matching account found for ID: " << *from_account_id;
  } else if (from_address && !from_address->empty()) {
    LOG(ERROR) << "Searching by address: " << *from_address;
    for (auto& account : accounts) {
      if (base::EqualsCaseInsensitiveASCII(account->address, *from_address)) {
        LOG(ERROR) << "Found matching account by address";
        return account->account_id->Clone();
      }
    }
    LOG(ERROR) << "No matching account found for address: " << *from_address;
  }

  LOG(ERROR) << "No matching account found";
  return nullptr;
}

bool AccountResolverDelegateImpl::ValidateAccountId(
    const mojom::AccountIdPtr& account_id) {
  const auto& accounts = keyring_service_->GetAllAccountInfos();
  for (auto& account : accounts) {
    if (account->account_id == account_id) {
      return true;
    }
  }
  return false;
}

}  // namespace wootz_wallet
