// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import {
  WootzWallet,
  SerializableTransactionInfo,
  SlippagePresetObjectType,
  TransactionProviderError
} from '../../constants/types'

export type UnapprovedTxUpdated = {
  txInfo: SerializableTransactionInfo
}

export type TransactionStatusChanged = {
  txInfo: SerializableTransactionInfo
}

export type SwapParamsPayloadType = {
  fromAsset: WootzWallet.BlockchainToken
  toAsset: WootzWallet.BlockchainToken
  fromAssetAmount?: string
  toAssetAmount?: string
  slippageTolerance: SlippagePresetObjectType
  accountAddress: string
  networkChainId: string
  full: boolean
}

export type UpdateUnapprovedTransactionGasFieldsType = {
  chainId: string
  txMetaId: string
  gasLimit: string
  gasPrice?: string
  maxPriorityFeePerGas?: string
  maxFeePerGas?: string
}

export type UpdateUnapprovedTransactionSpendAllowanceType = {
  chainId: string
  txMetaId: string
  spenderAddress: string
  allowance: string
}

export type UpdateUnapprovedTransactionNonceType = {
  chainId: string
  txMetaId: string
  nonce: string
}

export type DefaultEthereumWalletChanged = {
  defaultWallet: WootzWallet.DefaultWallet
}

export type DefaultSolanaWalletChanged = {
  defaultWallet: WootzWallet.DefaultWallet
}

export type DefaultBaseCurrencyChanged = {
  currency: string
}

export type DefaultBaseCryptocurrencyChanged = {
  cryptocurrency: string
}

export type SetTransactionProviderErrorType = {
  transactionId: string
  providerError: TransactionProviderError
}

export interface RetryTransactionPayload {
  chainId: string
  transactionId: string
  coinType: WootzWallet.CoinType
}

export type SpeedupTransactionPayload = RetryTransactionPayload
export type CancelTransactionPayload = RetryTransactionPayload
