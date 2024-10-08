// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_ZCASH_ZCASH_GET_TRANSPARENT_UTXOS_CONTEXT_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_ZCASH_ZCASH_GET_TRANSPARENT_UTXOS_CONTEXT_H_

#include <string>

#include "base/functional/callback.h"
#include "components/wootz_wallet/browser/zcash/zcash_wallet_service.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

class ZCashGetTransparentUtxosContext
    : public base::RefCountedThreadSafe<ZCashGetTransparentUtxosContext> {
 public:
  using GetUtxosCallback = ZCashWalletService::GetUtxosCallback;

  std::set<std::string> addresses;
  ZCashWalletService::UtxoMap utxos;
  std::optional<std::string> error;
  GetUtxosCallback callback;

  bool ShouldRespond() { return callback && (error || addresses.empty()); }

  void SetError(const std::string& error_string) { error = error_string; }

 protected:
  friend class ZCashWalletService;

  friend class base::RefCountedThreadSafe<ZCashGetTransparentUtxosContext>;
  ZCashGetTransparentUtxosContext();
  virtual ~ZCashGetTransparentUtxosContext();
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_ZCASH_ZCASH_GET_TRANSPARENT_UTXOS_CONTEXT_H_
