/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/tx_state_manager.h"

#include <optional>
#include <utility>

#include "base/json/values_util.h"
#include "base/values.h"
#include "components/wootz_wallet/browser/account_resolver_delegate.h"
#include "components/wootz_wallet/browser/wootz_wallet_constants.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/pref_names.h"
#include "components/wootz_wallet/browser/scoped_txs_update.h"
#include "components/wootz_wallet/browser/solana_message.h"
#include "components/wootz_wallet/browser/tx_meta.h"
#include "components/wootz_wallet/browser/tx_storage_delegate.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "url/origin.h"

namespace wootz_wallet {

namespace {

constexpr size_t kMaxConfirmedTxNum = 500;
constexpr size_t kMaxRejectedTxNum = 500;

}  // namespace

bool TxStateManager::ValueToBaseTxMeta(const base::Value::Dict& value,
                                       TxMeta* meta) {
  LOG(ERROR) << "JANGID_SIGN: Starting ValueToBaseTxMeta conversion";

  const std::string* id = value.FindString("id");
  if (!id) {
    LOG(ERROR) << "JANGID_SIGN: Missing transaction ID";
    return false;
  }
  LOG(ERROR) << "JANGID_SIGN: Processing transaction ID: " << *id;
  meta->set_id(*id);

  std::optional<int> status = value.FindInt("status");
  if (!status) {
    LOG(ERROR) << "JANGID_SIGN: Missing transaction status";
    return false;
  }
  LOG(ERROR) << "JANGID_SIGN: Transaction status: " << *status;
  meta->set_status(static_cast<mojom::TransactionStatus>(*status));

  const std::string* from_account_id = value.FindString("from_account_id");
  const std::string* from_address = value.FindString("from");
  LOG(ERROR) << "JANGID_SIGN: From account_id: " << (from_account_id ? *from_account_id : "null");
  LOG(ERROR) << "JANGID_SIGN: From address: " << (from_address ? *from_address : "null");
  
  auto account_id = account_resolver_delegate_->ResolveAccountId(
      from_account_id, from_address);
  if (!account_id) {
    LOG(ERROR) << "JANGID_SIGN: Failed to resolve account ID";
    return false;
  }
  meta->set_from(std::move(account_id));

  const base::Value* created_time = value.Find("created_time");
  if (!created_time) {
    return false;
  }
  std::optional<base::Time> created_time_from_value =
      base::ValueToTime(created_time);
  if (!created_time_from_value) {
    return false;
  }
  meta->set_created_time(*created_time_from_value);

  const base::Value* submitted_time = value.Find("submitted_time");
  if (!submitted_time) {
    return false;
  }
  std::optional<base::Time> submitted_time_from_value =
      base::ValueToTime(submitted_time);
  if (!submitted_time_from_value) {
    return false;
  }
  meta->set_submitted_time(*submitted_time_from_value);

  const base::Value* confirmed_time = value.Find("confirmed_time");
  if (!confirmed_time) {
    return false;
  }
  std::optional<base::Time> confirmed_time_from_value =
      base::ValueToTime(confirmed_time);
  if (!confirmed_time_from_value) {
    return false;
  }
  meta->set_confirmed_time(*confirmed_time_from_value);

  const std::string* tx_hash = value.FindString("tx_hash");
  if (!tx_hash) {
    LOG(ERROR) << "JANGID_SIGN: Missing transaction hash";
    return false;
  }
  LOG(ERROR) << "JANGID_SIGN: Transaction hash: " << *tx_hash;
  meta->set_tx_hash(*tx_hash);

  const std::string* origin_spec = value.FindString("origin");
  // That's ok not to have origin.
  if (origin_spec) {
    LOG(ERROR) << "JANGID_SIGN: Transaction origin: " << *origin_spec;
    meta->set_origin(url::Origin::Create(GURL(*origin_spec)));
    DCHECK(!meta->origin()->opaque());
  } else {
    LOG(ERROR) << "JANGID_SIGN: No origin specified";
  }

  const auto coin_int = value.FindInt("coin");
  if (!coin_int) {
    LOG(ERROR) << "JANGID_SIGN: Missing coin type";
    return false;
  }
  const auto coin = static_cast<mojom::CoinType>(coin_int.value());
  LOG(ERROR) << "JANGID_SIGN: Coin type: " << coin_int.value();
  
  if (!mojom::IsKnownEnumValue(coin)) {
    LOG(ERROR) << "JANGID_SIGN: Unknown coin type";
    return false;
  }
  if (coin != meta->GetCoinType()) {
    LOG(ERROR) << "JANGID_SIGN: Coin type mismatch. Expected: " 
               << meta->GetCoinType() << ", Got: " << coin;
    return false;
  }

  const auto* chain_id_string = value.FindString("chain_id");
  if (!chain_id_string) {
    LOG(ERROR) << "JANGID_SIGN: Missing chain ID";
    return false;
  }
  LOG(ERROR) << "JANGID_SIGN: Chain ID: " << *chain_id_string;
  meta->set_chain_id(*chain_id_string);

  LOG(ERROR) << "JANGID_SIGN: Successfully converted transaction metadata";
  return true;
}

TxStateManager::TxStateManager(
    PrefService* prefs,
    TxStorageDelegate* delegate,
    AccountResolverDelegate* account_resolver_delegate)
    : prefs_(prefs),
      delegate_(delegate),
      account_resolver_delegate_(account_resolver_delegate),
      weak_factory_(this) {
  DCHECK(delegate);
}

TxStateManager::~TxStateManager() = default;

bool TxStateManager::AddOrUpdateTx(const TxMeta& meta) {
  LOG(ERROR) << "JANGID_SIGN: Entering AddOrUpdateTx";
  LOG(ERROR) << "JANGID_SIGN: Transaction ID: " << meta.id();
  LOG(ERROR) << "JANGID_SIGN: Chain ID: " << meta.chain_id();
  LOG(ERROR) << "JANGID_SIGN: From address: " << meta.from()->address;
  LOG(ERROR) << "JANGID_SIGN: Transaction status: " << static_cast<int>(meta.status());

  DCHECK(meta.from());
  DCHECK_EQ(GetCoinType(), meta.GetCoinType());

  if (!delegate_->IsInitialized()) {
    LOG(ERROR) << "JANGID_SIGN: Delegate not initialized, cannot add/update transaction";
    return false;
  }

  bool is_add = false;
  {
    LOG(ERROR) << "JANGID_SIGN: Starting transaction update";
    ScopedTxsUpdate update(delegate_);
    is_add = update->Find(meta.id()) == nullptr;
    LOG(ERROR) << "JANGID_SIGN: Operation type: " << (is_add ? "Add new" : "Update existing");
    
    update->Set(meta.id(), meta.ToValue());
    LOG(ERROR) << "JANGID_SIGN: Transaction data saved to state";
  }

  if (!is_add) {
    LOG(ERROR) << "JANGID_SIGN: Notifying observers of transaction status change";
    for (auto& observer : observers_) {
      observer.OnTransactionStatusChanged(meta.ToTransactionInfo());
    }
  } else {
    LOG(ERROR) << "JANGID_SIGN: Notifying observers of new unapproved transaction";
    for (auto& observer : observers_) {
      observer.OnNewUnapprovedTx(meta.ToTransactionInfo());
    }

    // We only keep most recent 1k confirmed plus rejected tx metas per network
    LOG(ERROR) << "JANGID_SIGN: Checking if transaction cleanup needed";
    LOG(ERROR) << "JANGID_SIGN: Max confirmed transactions: " << kMaxConfirmedTxNum;
    LOG(ERROR) << "JANGID_SIGN: Max rejected transactions: " << kMaxRejectedTxNum;
    
    RetireTxByStatus(meta.chain_id(), mojom::TransactionStatus::Confirmed,
                     kMaxConfirmedTxNum);
    RetireTxByStatus(meta.chain_id(), mojom::TransactionStatus::Rejected,
                     kMaxRejectedTxNum);
    LOG(ERROR) << "JANGID_SIGN: Transaction cleanup completed";
  }

  LOG(ERROR) << "JANGID_SIGN: Successfully added/updated transaction";
  return true;
}

std::unique_ptr<TxMeta> TxStateManager::GetTx(const std::string& meta_id) {
  LOG(ERROR) << "JANGID_SIGN: Looking up transaction with ID: " << meta_id;

  if (!delegate_->IsInitialized()) {
    LOG(ERROR) << "JANGID_SIGN: Storage delegate not initialized";
    return nullptr;
  }

  const auto& txs = delegate_->GetTxs();
  LOG(ERROR) << "JANGID_SIGN: Total transactions in storage: " << txs.size();

  // Debug print available transaction IDs
  LOG(ERROR) << "JANGID_SIGN: Available transaction IDs:";
  for (const auto it : txs) {
    LOG(ERROR) << "  - " << it.first;
  }

  const base::Value::Dict* value = txs.FindDict(meta_id);
  if (!value) {
    LOG(ERROR) << "JANGID_SIGN: Transaction not found in storage";
    return nullptr;
  }

  // Print raw transaction data
  LOG(ERROR) << "JANGID_SIGN: Found raw transaction data:";
  for (const auto it : *value) {
    LOG(ERROR) << "  " << it.first << ": " << it.second;
  }

  auto tx_meta = ValueToTxMeta(*value);
  if (!tx_meta) {
    LOG(ERROR) << "JANGID_SIGN: Failed to convert transaction data to TxMeta";
    return nullptr;
  }

  // Print converted transaction details
  LOG(ERROR) << "JANGID_SIGN: Successfully converted transaction:";
  LOG(ERROR) << "  - ID: " << tx_meta->id();
  LOG(ERROR) << "  - Status: " << static_cast<int>(tx_meta->status());
  LOG(ERROR) << "  - From: " << tx_meta->from()->address;
  LOG(ERROR) << "  - Chain ID: " << tx_meta->chain_id();

  return tx_meta;
}

bool TxStateManager::DeleteTx(const std::string& meta_id) {
  if (!delegate_->IsInitialized()) {
    return false;
  }
  {
    ScopedTxsUpdate update(delegate_);
    update->Remove(meta_id);
  }
  return true;
}

std::vector<std::unique_ptr<TxMeta>> TxStateManager::GetTransactionsByStatus(
    const std::optional<std::string>& chain_id,
    const std::optional<mojom::TransactionStatus>& status,
    const mojom::AccountIdPtr& from) {
  DCHECK(from);
  return GetTransactionsByStatus(chain_id, status,
                                 std::make_optional(from.Clone()));
}

std::vector<std::unique_ptr<TxMeta>> TxStateManager::GetTransactionsByStatus(
    const std::optional<std::string>& chain_id,
    const std::optional<mojom::TransactionStatus>& status,
    const std::optional<mojom::AccountIdPtr>& from) {
  std::vector<std::unique_ptr<TxMeta>> result;
  
  LOG(ERROR) << "JANGID_SIGN: Getting transactions with filters:";
  LOG(ERROR) << "JANGID_SIGN: Chain ID filter: " << (chain_id.has_value() ? *chain_id : "none");
  LOG(ERROR) << "JANGID_SIGN: Status filter: " << (status.has_value() ? static_cast<int>(*status) : -1);

  if (!delegate_->IsInitialized()) {
    LOG(ERROR) << "JANGID_SIGN: Delegate not initialized, returning empty result";
    return result;
  }

  const auto& txs = delegate_->GetTxs();
  LOG(ERROR) << "JANGID_SIGN: Total transactions found: " << txs.size();

  // Debug print all transactions
  for (const auto it : txs) {
    LOG(ERROR) << "JANGID_SIGN: Transaction ID: " << it.first;
    auto* meta_dict = it.second.GetIfDict();
    if (!meta_dict) {
      LOG(ERROR) << "JANGID_SIGN: Invalid transaction data format for ID: " << it.first;
      continue;
    }

    // Print key transaction details
    const std::string* tx_id = meta_dict->FindString("id");
    const std::string* tx_hash = meta_dict->FindString("tx_hash");
    std::optional<int> tx_status = meta_dict->FindInt("status");
    
    LOG(ERROR) << "JANGID_SIGN: Details for transaction " << it.first << ":";
    LOG(ERROR) << "  - ID: " << (tx_id ? *tx_id : "missing");
    LOG(ERROR) << "  - Hash: " << (tx_hash ? *tx_hash : "missing");
    LOG(ERROR) << "  - Status: " << (tx_status.has_value() ? *tx_status : -1);

    std::unique_ptr<TxMeta> meta = ValueToTxMeta(*meta_dict);
    if (!meta) {
      LOG(ERROR) << "JANGID_SIGN: Failed to convert transaction data for ID: " << it.first;
      continue;
    }
    if (meta->from()->coin != GetCoinType()) {
      continue;
    }
    bool should_include = true;
    if (chain_id.has_value() && meta->chain_id() != *chain_id) {
      continue;
      LOG(ERROR) << "JANGID_SIGN: Filtering out transaction due to chain_id mismatch";
      should_include = false;
    }
    if (status.has_value() && meta->status() != *status) {
      continue;
      LOG(ERROR) << "JANGID_SIGN: Filtering out transaction due to status mismatch";
      should_include = false;
    }
    if (from.has_value() && meta->from() != *from) {
      continue;
      LOG(ERROR) << "JANGID_SIGN: Filtering out transaction due to from address mismatch";
      should_include = false;
    }

    if (should_include) {
      LOG(ERROR) << "JANGID_SIGN: Including transaction in results: " << it.first;
      result.push_back(std::move(meta));
    }
  }

  LOG(ERROR) << "JANGID_SIGN: Returning " << result.size() << " filtered transactions";
  return result;
}

void TxStateManager::RetireTxByStatus(const std::string& chain_id,
                                      mojom::TransactionStatus status,
                                      size_t max_num) {
  if (no_retire_for_testing_) {
    return;
  }

  if (status != mojom::TransactionStatus::Confirmed &&
      status != mojom::TransactionStatus::Rejected) {
    return;
  }
  auto tx_metas = GetTransactionsByStatus(chain_id, status, std::nullopt);
  if (tx_metas.size() > max_num) {
    TxMeta* oldest_meta = nullptr;
    for (const auto& tx_meta : tx_metas) {
      if (!oldest_meta) {
        oldest_meta = tx_meta.get();
      } else {
        if (tx_meta->status() == mojom::TransactionStatus::Confirmed &&
            tx_meta->confirmed_time() < oldest_meta->confirmed_time()) {
          oldest_meta = tx_meta.get();
        } else if (tx_meta->status() == mojom::TransactionStatus::Rejected &&
                   tx_meta->created_time() < oldest_meta->created_time()) {
          oldest_meta = tx_meta.get();
        }
      }
    }
    DCHECK(oldest_meta);
    DeleteTx(oldest_meta->id());
  }
}

void TxStateManager::AddObserver(TxStateManager::Observer* observer) {
  observers_.AddObserver(observer);
}

void TxStateManager::RemoveObserver(TxStateManager::Observer* observer) {
  observers_.RemoveObserver(observer);
}

void TxStateManager::SetNoRetireForTesting(bool no_retire) {
  no_retire_for_testing_ = no_retire;
}

}  // namespace wootz_wallet
