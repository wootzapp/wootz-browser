// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import { WootzWallet, CreateAccountOptionsType } from '../constants/types'
import { getLocale } from '../../common/locale'
import { getNetworkLogo } from './asset-options'

export const CreateAccountOptions = (options: {
  isBitcoinEnabled: boolean
  isZCashEnabled: boolean
}): CreateAccountOptionsType[] => {
  let accounts = [
    {
      description: getLocale('wootzWalletCreateAccountEthereumDescription'),
      name: 'Ethereum',
      coin: WootzWallet.CoinType.ETH,
      icon: getNetworkLogo(WootzWallet.MAINNET_CHAIN_ID, 'ETH'),
      chainIcons: ['eth-color', 'matic-color', 'op-color', 'aurora-color']
    }
  ]
  accounts.push({
    description: getLocale('wootzWalletCreateAccountSolanaDescription'),
    name: 'Solana',
    coin: WootzWallet.CoinType.SOL,
    icon: getNetworkLogo(WootzWallet.SOLANA_MAINNET, 'SOL'),
    chainIcons: ['sol-color']
  })

  accounts.push({
    description: getLocale('wootzWalletCreateAccountFilecoinDescription'),
    name: 'Filecoin',
    coin: WootzWallet.CoinType.FIL,
    icon: getNetworkLogo(WootzWallet.FILECOIN_MAINNET, 'FIL'),
    chainIcons: ['filecoin-color']
  })

  if (options.isBitcoinEnabled) {
    accounts.push({
      description: getLocale('wootzWalletCreateAccountBitcoinDescription'),
      name: 'Bitcoin',
      coin: WootzWallet.CoinType.BTC,
      icon: getNetworkLogo(WootzWallet.BITCOIN_MAINNET, 'BTC'),
      chainIcons: []
    })
  }
  if (options.isZCashEnabled) {
    accounts.push({
      description: getLocale('wootzWalletCreateAccountZCashDescription'),
      name: 'ZCash',
      coin: WootzWallet.CoinType.ZEC,
      icon: getNetworkLogo(WootzWallet.Z_CASH_MAINNET, 'ZEC'),
      chainIcons: []
    })
  }
  return accounts
}
