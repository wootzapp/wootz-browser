// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { assertNotReached } from 'chrome://resources/js/assert.js'

// types
import { WootzWallet, SerializableSolanaTxData } from '../constants/types'

export const emptyNetwork: WootzWallet.NetworkInfo = {
  chainId: '',
  chainName: '',
  activeRpcEndpointIndex: 0,
  rpcEndpoints: [],
  blockExplorerUrls: [],
  iconUrls: [],
  symbol: '',
  symbolName: '',
  decimals: 0,
  coin: WootzWallet.CoinType.ETH,
  supportedKeyrings: []
}

export const getNetworkInfo = (
  chainId: string,
  coin: WootzWallet.CoinType,
  list: WootzWallet.NetworkInfo[]
) => {
  for (let it of list) {
    if (it.chainId === chainId && it.coin === coin) {
      return it
    }
  }
  return emptyNetwork
}

export const networkSupportsAccount = (
  network: Pick<WootzWallet.NetworkInfo, 'coin' | 'supportedKeyrings'>,
  accountId: WootzWallet.AccountId
) => {
  return (
    network.coin === accountId.coin &&
    network.supportedKeyrings.includes(accountId.keyringId)
  )
}

export const filterNetworksForAccount = (
  networks: WootzWallet.NetworkInfo[],
  accountId: WootzWallet.AccountId
): WootzWallet.NetworkInfo[] => {
  if (!networks) {
    return []
  }
  return networks.filter((network) =>
    networkSupportsAccount(network, accountId)
  )
}

export const getTokensNetwork = (
  networks: WootzWallet.NetworkInfo[],
  token: WootzWallet.BlockchainToken
): WootzWallet.NetworkInfo => {
  if (!networks) {
    return emptyNetwork
  }

  const network = networks.filter((n) => n.chainId === token.chainId)
  if (network.length > 1) {
    return (
      network?.find(
        (n) => n.symbol.toLowerCase() === token.symbol.toLowerCase()
      ) ?? emptyNetwork
    )
  }

  return network[0] ?? emptyNetwork
}

export type TxDataPresence = {
  ethTxData?: Partial<WootzWallet.TxDataUnion['ethTxData']> | undefined
  ethTxData1559?: Partial<WootzWallet.TxDataUnion['ethTxData1559']> | undefined
  solanaTxData?:
    | Partial<WootzWallet.TxDataUnion['solanaTxData']>
    | SerializableSolanaTxData
    | undefined
  filTxData?: Partial<WootzWallet.TxDataUnion['filTxData']> | undefined
  btcTxData?: Partial<WootzWallet.TxDataUnion['btcTxData']> | undefined
  zecTxData?: Partial<WootzWallet.TxDataUnion['zecTxData']> | undefined
}

export const getCoinFromTxDataUnion = <T extends TxDataPresence>(
  txDataUnion: T
): WootzWallet.CoinType => {
  if (txDataUnion.ethTxData || txDataUnion.ethTxData1559) {
    return WootzWallet.CoinType.ETH
  }
  if (txDataUnion.filTxData) {
    return WootzWallet.CoinType.FIL
  }
  if (txDataUnion.solanaTxData) {
    return WootzWallet.CoinType.SOL
  }
  if (txDataUnion.btcTxData) {
    return WootzWallet.CoinType.BTC
  }
  if (txDataUnion.zecTxData) {
    return WootzWallet.CoinType.ZEC
  }

  assertNotReached('Unknown transaction coin')
}

export const reduceNetworkDisplayName = (name?: string) => {
  if (!name) {
    return ''
  }
  return name.split(' ')[0]
}
