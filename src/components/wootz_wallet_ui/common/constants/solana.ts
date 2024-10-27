// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

// types
import { WootzWallet } from '../../constants/types'

/**
 * There are 1-billion lamports in one SOL
 */
export const LAMPORTS_PER_SOL = 1000000000

export const SolanaTransactionTypes = [
  WootzWallet.TransactionType.SolanaSystemTransfer,
  WootzWallet.TransactionType.SolanaSPLTokenTransfer,
  WootzWallet.TransactionType
    .SolanaSPLTokenTransferWithAssociatedTokenAccountCreation,
  WootzWallet.TransactionType.SolanaDappSignTransaction,
  WootzWallet.TransactionType.SolanaDappSignAndSendTransaction,
  WootzWallet.TransactionType.SolanaSwap,
  WootzWallet.TransactionType.SolanaCompressedNftTransfer
]
