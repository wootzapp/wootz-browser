/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/eth_tx_state_manager.h"

#include <optional>

#include "base/logging.h"
#include "base/values.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/eip1559_transaction.h"
#include "components/wootz_wallet/browser/eip2930_transaction.h"
#include "components/wootz_wallet/browser/eth_tx_meta.h"
#include "components/wootz_wallet/browser/tx_meta.h"

namespace wootz_wallet {

EthTxStateManager::EthTxStateManager(
    PrefService* prefs,
    TxStorageDelegate* delegate,
    AccountResolverDelegate* account_resolver_delegate)
    : TxStateManager(prefs, delegate, account_resolver_delegate) {}

EthTxStateManager::~EthTxStateManager() = default;

std::unique_ptr<EthTxMeta> EthTxStateManager::GetEthTx(const std::string& id) {
  LOG(ERROR) << "jangid_sign: GetEthTx called with ID: " << id;
  
  auto tx = TxStateManager::GetTx(id);
  if (!tx) {
    LOG(ERROR) << "jangid_sign: No transaction found for ID: " << id;
    return nullptr;
  }
  
  LOG(ERROR) << "jangid_sign: Successfully retrieved transaction for ID: " << id;
  return std::unique_ptr<EthTxMeta>{static_cast<EthTxMeta*>(tx.release())};
}

std::unique_ptr<EthTxMeta> EthTxStateManager::ValueToEthTxMeta(
    const base::Value::Dict& value) {
  return std::unique_ptr<EthTxMeta>{
      static_cast<EthTxMeta*>(ValueToTxMeta(value).release())};
}

mojom::CoinType EthTxStateManager::GetCoinType() const {
  return mojom::CoinType::ETH;
}

std::unique_ptr<TxMeta> EthTxStateManager::ValueToTxMeta(
    const base::Value::Dict& value) {
  LOG(INFO) << "Starting ValueToTxMeta conversion";
  LOG(INFO) << "Input value dict: " << value;

  std::unique_ptr<EthTxMeta> meta = std::make_unique<EthTxMeta>();

  if (!ValueToBaseTxMeta(value, meta.get())) {
    LOG(ERROR) << "Failed to convert base tx meta";
    return nullptr;
  }

  const base::Value::Dict* tx_receipt = value.FindDict("tx_receipt");
  if (!tx_receipt) {
    LOG(ERROR) << "tx_receipt not found in value dict";
    return nullptr;
  }
  LOG(INFO) << "Found tx_receipt: " << *tx_receipt;

  std::optional<TransactionReceipt> tx_receipt_from_value =
      ValueToTransactionReceipt(*tx_receipt);
  if (!tx_receipt_from_value) {
    LOG(ERROR) << "Failed to convert tx_receipt to TransactionReceipt";
    return nullptr;
  }
  meta->set_tx_receipt(*tx_receipt_from_value);

  const base::Value::Dict* tx = value.FindDict("tx");
  if (!tx) {
    LOG(ERROR) << "tx not found in value dict";
    return nullptr;
  }
  LOG(INFO) << "Found tx: " << *tx;

  std::optional<bool> sign_only = value.FindBool("sign_only");
  if (sign_only) {
    LOG(INFO) << "Found sign_only: " << *sign_only;
    meta->set_sign_only(*sign_only);
  } else {
    LOG(INFO) << "sign_only not found in value dict";
  }

  std::optional<int> type = tx->FindInt("type");
  if (!type) {
    LOG(ERROR) << "tx type not found in tx dict";
    return nullptr;
  }
  LOG(INFO) << "Transaction type: " << *type;

  switch (static_cast<uint8_t>(*type)) {
    case 0: {
      LOG(INFO) << "Processing EthTransaction (type 0)";
      std::optional<EthTransaction> tx_from_value =
          EthTransaction::FromValue(*tx);
      if (!tx_from_value) {
        LOG(ERROR) << "Failed to convert to EthTransaction";
        return nullptr;
      }
      meta->set_tx(std::make_unique<EthTransaction>(*tx_from_value));
      break;
    }
    case 1: {
      LOG(INFO) << "Processing Eip2930Transaction (type 1)";
      std::optional<Eip2930Transaction> tx_from_value =
          Eip2930Transaction::FromValue(*tx);
      if (!tx_from_value) {
        LOG(ERROR) << "Failed to convert to Eip2930Transaction";
        return nullptr;
      }
      meta->set_tx(std::make_unique<Eip2930Transaction>(*tx_from_value));
      break;
    }
    case 2: {
      LOG(INFO) << "Processing Eip1559Transaction (type 2)";
      std::optional<Eip1559Transaction> tx_from_value =
          Eip1559Transaction::FromValue(*tx);
      if (!tx_from_value) {
        LOG(ERROR) << "Failed to convert to Eip1559Transaction";
        return nullptr;
      }
      meta->set_tx(std::make_unique<Eip1559Transaction>(*tx_from_value));
      break;
    }
    default:
      LOG(ERROR) << "Unsupported transaction type: " << *type;
      break;
  }

  LOG(INFO) << "Successfully completed ValueToTxMeta conversion";
  return meta;
}

}  // namespace wootz_wallet
