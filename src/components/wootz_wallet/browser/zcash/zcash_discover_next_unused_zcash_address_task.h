// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_ZCASH_ZCASH_DISCOVER_NEXT_UNUSED_ZCASH_ADDRESS_TASK_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_ZCASH_ZCASH_DISCOVER_NEXT_UNUSED_ZCASH_ADDRESS_TASK_H_

#include "components/wootz_wallet/browser/zcash/zcash_wallet_service.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

class ZCashDiscoverNextUnusedZCashAddressTask
    : public base::RefCounted<ZCashDiscoverNextUnusedZCashAddressTask> {
 public:
  void ScheduleWorkOnTask();

 private:
  friend class ZCashWalletService;
  friend class base::RefCounted<ZCashDiscoverNextUnusedZCashAddressTask>;

  ZCashDiscoverNextUnusedZCashAddressTask(
      base::WeakPtr<ZCashWalletService> zcash_wallet_service,
      mojom::AccountIdPtr account_id,
      mojom::ZCashAddressPtr start_address,
      ZCashWalletService::DiscoverNextUnusedAddressCallback callback);
  virtual ~ZCashDiscoverNextUnusedZCashAddressTask();

  mojom::ZCashAddressPtr GetNextAddress(const mojom::ZCashAddressPtr& address);

  void WorkOnTask();
  void OnGetIsKnownAddress(base::expected<bool, std::string> stats);
  void OnGetLastBlock(
      base::expected<zcash::mojom::BlockIDPtr, std::string> result);

  base::WeakPtr<ZCashWalletService> zcash_wallet_service_;
  mojom::AccountIdPtr account_id_;
  mojom::ZCashAddressPtr start_address_;
  mojom::ZCashAddressPtr current_address_;
  mojom::ZCashAddressPtr result_;
  std::optional<uint64_t> block_end_;
  std::optional<std::string> error_;
  ZCashWalletService::DiscoverNextUnusedAddressCallback callback_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_ZCASH_ZCASH_DISCOVER_NEXT_UNUSED_ZCASH_ADDRESS_TASK_H_
