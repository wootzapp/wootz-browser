// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import {
  NativeAssetBalanceRegistry,
  TokenBalanceRegistry
} from '../common/constants/mocks'
import { WootzWallet, RewardsExternalWallet } from './types'

export interface WalletApiDataOverrides {
  selectedCoin?: WootzWallet.CoinType
  selectedAccountId?: WootzWallet.AccountId
  chainIdsForCoins?: Record<WootzWallet.CoinType, string>
  networks?: WootzWallet.NetworkInfo[]
  defaultBaseCurrency?: string
  transactionInfos?: WootzWallet.TransactionInfo[]
  blockchainTokens?: WootzWallet.BlockchainToken[]
  userAssets?: WootzWallet.BlockchainToken[]
  accountInfos?: WootzWallet.AccountInfo[]
  nativeBalanceRegistry?: NativeAssetBalanceRegistry
  tokenBalanceRegistry?: TokenBalanceRegistry
  simulationOptInStatus?: WootzWallet.BlowfishOptInStatus
  evmSimulationResponse?: WootzWallet.EVMSimulationResponse | null
  svmSimulationResponse?: WootzWallet.SolanaSimulationResponse | null
  signTransactionRequests?: WootzWallet.SignTransactionRequest[]
  signAllTransactionsRequests?: WootzWallet.SignAllTransactionsRequest[]
}

export type WootzRewardsProxyOverrides = Partial<{
  rewardsEnabled: boolean
  balance: number
  externalWallet: RewardsExternalWallet | null
}>
