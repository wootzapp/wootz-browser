// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import { WootzWallet, NetworkFilterType } from '../constants/types'
import { getLocale } from '../../common/locale'
import AllNetworksIcon from '../assets/svg-icons/all-networks-icon.svg'
import { getNetworkId } from '../common/slices/entities/network.entity'

export const AllNetworksOption: WootzWallet.NetworkInfo = {
  blockExplorerUrls: [],
  chainId: 'all',
  chainName: getLocale('wootzWalletNetworkFilterAll'),
  coin: 0,
  decimals: 0,
  iconUrls: [AllNetworksIcon],
  activeRpcEndpointIndex: 0,
  rpcEndpoints: [],
  symbol: 'all',
  symbolName: 'all',
  supportedKeyrings: []
}

export const AllNetworksOptionNetworkId = getNetworkId(AllNetworksOption)

export const AllNetworksOptionDefault: NetworkFilterType = {
  chainId: 'all',
  coin: 0
}

export const SupportedTopLevelChainIds = [
  WootzWallet.MAINNET_CHAIN_ID,
  WootzWallet.SOLANA_MAINNET,
  WootzWallet.FILECOIN_MAINNET,
  WootzWallet.BITCOIN_MAINNET,
  WootzWallet.Z_CASH_MAINNET
]
