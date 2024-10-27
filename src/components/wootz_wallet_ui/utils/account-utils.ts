// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { assertNotReached } from 'chrome://resources/js/assert.js'
import { getLocale } from '../../common/locale'

// types
import {
  WootzWallet,
  BitcoinMainnetKeyringIds,
  BitcoinTestnetKeyringIds
} from '../constants/types'

// constants
import registry from '../common/constants/registry'

// utils
import { reduceAddress } from './reduce-address'
import { EntityState } from '@reduxjs/toolkit'

export const sortAccountsByName = (accounts: WootzWallet.AccountInfo[]) => {
  return [...accounts].sort(function (
    a: WootzWallet.AccountInfo,
    b: WootzWallet.AccountInfo
  ) {
    if (a.name < b.name) {
      return -1
    }

    if (a.name > b.name) {
      return 1
    }

    return 0
  })
}

export const groupAccountsById = (
  accounts: WootzWallet.AccountInfo[],
  key: string
) => {
  return accounts.reduce<Record<string, WootzWallet.AccountInfo[]>>(
    (result, obj) => {
      ;(result[obj[key]] = result[obj[key]] || []).push(obj)
      return result
    },
    {}
  )
}

export const findAccountByUniqueKey = <
  T extends { accountId: { uniqueKey: string } }
>(
  accounts: T[],
  uniqueKey: string | undefined
): T | undefined => {
  if (!uniqueKey) {
    return
  }

  return accounts.find((account) => uniqueKey === account.accountId.uniqueKey)
}

export const entityIdFromAccountId = (
  accountId: Pick<WootzWallet.AccountId, 'uniqueKey'>
) => {
  return accountId.uniqueKey
}

export const findAccountByAddress = (
  address: string,
  accounts: EntityState<WootzWallet.AccountInfo> | undefined
): WootzWallet.AccountInfo | undefined => {
  if (!address || !accounts) return undefined
  for (const id of accounts.ids) {
    if (
      accounts.entities[id]?.address.toLowerCase() === address.toLowerCase()
    ) {
      return accounts.entities[id]
    }
  }
  return undefined
}

export const findAccountByAccountId = (
  accountId: Pick<WootzWallet.AccountId, 'uniqueKey'>,
  accounts: EntityState<WootzWallet.AccountInfo> | undefined
): WootzWallet.AccountInfo | undefined => {
  if (!accounts) {
    return undefined
  }

  return accounts.entities[entityIdFromAccountId(accountId)]
}

export const getAddressLabel = (
  address: string,
  accounts?: EntityState<WootzWallet.AccountInfo>
): string => {
  if (!accounts) {
    return registry[address.toLowerCase()] ?? reduceAddress(address)
  }
  return (
    registry[address.toLowerCase()] ??
    findAccountByAddress(address, accounts)?.name ??
    reduceAddress(address)
  )
}

export const getAccountLabel = (
  accountId: WootzWallet.AccountId,
  accounts: EntityState<WootzWallet.AccountInfo>
): string => {
  return (
    findAccountByAccountId(accountId, accounts)?.name ??
    reduceAddress(accountId.address)
  )
}

export function isHardwareAccount(account: WootzWallet.AccountId) {
  return account.kind === WootzWallet.AccountKind.kHardware
}

export const keyringIdForNewAccount = (
  coin: WootzWallet.CoinType,
  chainId?: string | undefined
) => {
  if (coin === WootzWallet.CoinType.ETH) {
    return WootzWallet.KeyringId.kDefault
  }

  if (coin === WootzWallet.CoinType.SOL) {
    return WootzWallet.KeyringId.kSolana
  }

  if (coin === WootzWallet.CoinType.FIL) {
    if (
      chainId === WootzWallet.FILECOIN_MAINNET ||
      chainId === WootzWallet.LOCALHOST_CHAIN_ID
    ) {
      return WootzWallet.KeyringId.kFilecoin
    }
    if (chainId === WootzWallet.FILECOIN_TESTNET) {
      return WootzWallet.KeyringId.kFilecoinTestnet
    }
  }

  if (coin === WootzWallet.CoinType.BTC) {
    if (chainId === WootzWallet.BITCOIN_MAINNET) {
      return WootzWallet.KeyringId.kBitcoin84
    }
    if (chainId === WootzWallet.BITCOIN_TESTNET) {
      return WootzWallet.KeyringId.kBitcoin84Testnet
    }
  }

  if (coin === WootzWallet.CoinType.ZEC) {
    if (chainId === WootzWallet.Z_CASH_MAINNET) {
      return WootzWallet.KeyringId.kZCashMainnet
    }
    if (chainId === WootzWallet.Z_CASH_TESTNET) {
      return WootzWallet.KeyringId.kZCashTestnet
    }
  }

  assertNotReached(`Unknown coin ${coin} and chainId ${chainId}`)
}

const bitcoinTestnetKeyrings = [
  WootzWallet.KeyringId.kBitcoin84Testnet,
  WootzWallet.KeyringId.kBitcoinImportTestnet
]
const zcashTestnetKeyrings = [WootzWallet.KeyringId.kZCashTestnet]

export const getAccountTypeDescription = (accountId: WootzWallet.AccountId) => {
  switch (accountId.coin) {
    case WootzWallet.CoinType.ETH:
      return getLocale('wootzWalletETHAccountDescription')
    case WootzWallet.CoinType.SOL:
      return getLocale('wootzWalletSOLAccountDescription')
    case WootzWallet.CoinType.FIL:
      return getLocale('wootzWalletFILAccountDescription')
    case WootzWallet.CoinType.BTC:
      if (bitcoinTestnetKeyrings.includes(accountId.keyringId)) {
        return getLocale('wootzWalletBTCTestnetAccountDescription')
      }
      return getLocale('wootzWalletBTCMainnetAccountDescription')
    case WootzWallet.CoinType.ZEC:
      if (zcashTestnetKeyrings.includes(accountId.keyringId)) {
        return getLocale('wootzWalletZECTestnetAccountDescription')
      }
      return getLocale('wootzWalletZECAccountDescription')
    default:
      assertNotReached(`Unknown coin ${accountId.coin}`)
  }
}

export const isFVMAccount = (
  account: WootzWallet.AccountInfo,
  network: WootzWallet.NetworkInfo
) => {
  return (
    (network.chainId === WootzWallet.FILECOIN_ETHEREUM_MAINNET_CHAIN_ID &&
      account.accountId.keyringId === WootzWallet.KeyringId.kFilecoin) ||
    (network.chainId === WootzWallet.FILECOIN_ETHEREUM_TESTNET_CHAIN_ID &&
      account.accountId.keyringId === WootzWallet.KeyringId.kFilecoinTestnet)
  )
}

export const getAccountsForNetwork = (
  network: Pick<WootzWallet.NetworkInfo, 'chainId' | 'coin'>,
  accounts: WootzWallet.AccountInfo[]
) => {
  if (network.chainId === WootzWallet.BITCOIN_MAINNET) {
    return accounts.filter((account) =>
      BitcoinMainnetKeyringIds.includes(account.accountId.keyringId)
    )
  }
  if (network.chainId === WootzWallet.BITCOIN_TESTNET) {
    return accounts.filter((account) =>
      BitcoinTestnetKeyringIds.includes(account.accountId.keyringId)
    )
  }
  if (network.chainId === WootzWallet.Z_CASH_MAINNET) {
    return accounts.filter(
      (account) =>
        account.accountId.keyringId === WootzWallet.KeyringId.kZCashMainnet
    )
  }
  if (network.chainId === WootzWallet.Z_CASH_TESTNET) {
    return accounts.filter(
      (account) =>
        account.accountId.keyringId === WootzWallet.KeyringId.kZCashTestnet
    )
  }
  if (network.chainId === WootzWallet.FILECOIN_MAINNET) {
    return accounts.filter(
      (account) =>
        account.accountId.keyringId === WootzWallet.KeyringId.kFilecoin
    )
  }
  if (network.chainId === WootzWallet.FILECOIN_TESTNET) {
    return accounts.filter(
      (account) =>
        account.accountId.keyringId === WootzWallet.KeyringId.kFilecoinTestnet
    )
  }
  return accounts.filter((account) => account.accountId.coin === network.coin)
}
