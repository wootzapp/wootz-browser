/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_SCOPED_TXS_UPDATE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_SCOPED_TXS_UPDATE_H_

#include "base/memory/raw_ptr.h"
#include "base/values.h"

namespace wootz_wallet {

class TxStorageDelegate;

class ScopedTxsUpdate {
 public:
  explicit ScopedTxsUpdate(TxStorageDelegate* delegate);
  ScopedTxsUpdate(const ScopedTxsUpdate&) = delete;
  ScopedTxsUpdate& operator=(const ScopedTxsUpdate&) = delete;
  virtual ~ScopedTxsUpdate();

  base::Value::Dict& Get();

  base::Value::Dict& operator*() { return Get(); }

  base::Value::Dict* operator->() { return &Get(); }

 private:
  raw_ptr<TxStorageDelegate> delegate_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_SCOPED_TXS_UPDATE_H_
