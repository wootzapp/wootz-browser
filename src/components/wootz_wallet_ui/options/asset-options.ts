// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// magics
import { SKIP_PRICE_LOOKUP_COINGECKO_ID } from '../common/constants/magics'

// types
import { WootzWallet, SupportedTestNetworks } from '../constants/types'

// options
import { AllNetworksOption } from './network-filter-options'

// icons
import {
  ArbIcon,
  AuroraIcon,
  BaseIcon,
  CeloIcon,
  FtmIcon,
  NeonIcon,
  OpIcon,
  AVAXIconUrl,
  BNBIconUrl,
  BTCIconUrl,
  ETHIconUrl,
  FILECOINIconUrl,
  MATICIconUrl,
  SOLIconUrl,
  ZECIconUrl
} from '../assets/network_token_icons/network_token_icons'

export const getNetworkLogo = (chainId: string, symbol: string): string => {
  if (chainId === WootzWallet.AURORA_MAINNET_CHAIN_ID) return AuroraIcon
  if (chainId === WootzWallet.OPTIMISM_MAINNET_CHAIN_ID) return OpIcon
  if (chainId === WootzWallet.POLYGON_MAINNET_CHAIN_ID) return MATICIconUrl
  if (chainId === WootzWallet.BNB_SMART_CHAIN_MAINNET_CHAIN_ID)
    return BNBIconUrl
  if (chainId === WootzWallet.AVALANCHE_MAINNET_CHAIN_ID) return AVAXIconUrl
  if (chainId === WootzWallet.FANTOM_MAINNET_CHAIN_ID) return FtmIcon
  if (chainId === WootzWallet.CELO_MAINNET_CHAIN_ID) return CeloIcon
  if (chainId === WootzWallet.ARBITRUM_MAINNET_CHAIN_ID) return ArbIcon
  if (chainId === WootzWallet.NEON_EVM_MAINNET_CHAIN_ID) return NeonIcon
  if (chainId === WootzWallet.BASE_MAINNET_CHAIN_ID) return BaseIcon
  if (chainId === AllNetworksOption.chainId)
    return AllNetworksOption.iconUrls[0]

  switch (symbol.toUpperCase()) {
    case 'SOL':
      return SOLIconUrl
    case 'ETH':
      return ETHIconUrl
    case 'FIL':
      return FILECOINIconUrl
    case 'BTC':
      return BTCIconUrl
    case 'ZEC':
      return ZECIconUrl
  }

  return ''
}

export const makeNativeAssetLogo = (symbol: string, chainId: string) => {
  return getNetworkLogo(
    symbol.toUpperCase() === 'ETH' ? WootzWallet.MAINNET_CHAIN_ID : chainId,
    symbol
  )
}

type UndefinedIf<R, T> = T extends undefined ? undefined : R
export const makeNetworkAsset = <
  T extends WootzWallet.NetworkInfo | undefined | null
>(
  network: T
): UndefinedIf<WootzWallet.BlockchainToken, T> => {
  if (!network) {
    return undefined as UndefinedIf<WootzWallet.BlockchainToken, T>
  }

  return {
    contractAddress: '',
    name: network.symbolName,
    symbol: network.symbol,
    logo: makeNativeAssetLogo(network.symbol, network.chainId),
    isErc20: false,
    isErc721: false,
    isErc1155: false,
    isNft: false,
    isSpam: false,
    decimals: network.decimals,
    visible: true,
    tokenId: '',
    coingeckoId:
      // skip getting prices of known testnet tokens
      SupportedTestNetworks.includes(network.chainId)
        ? SKIP_PRICE_LOOKUP_COINGECKO_ID
        : '',
    chainId: network.chainId,
    coin: network.coin
  } as UndefinedIf<WootzWallet.BlockchainToken, T>
}
