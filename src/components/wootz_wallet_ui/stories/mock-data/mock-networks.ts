// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { WootzWallet } from '../../constants/types'
import {
  ETHIconUrl,
  FILECOINIconUrl,
  SOLIconUrl,
  BTCIconUrl
} from '../../assets/network_token_icons/network_token_icons'

export const mockEthMainnet: WootzWallet.NetworkInfo = {
  activeRpcEndpointIndex: 0,
  blockExplorerUrls: ['https://etherscan.io', 'https://etherchain.org'],
  chainId: WootzWallet.MAINNET_CHAIN_ID,
  chainName: 'Ethereum Mainnet',
  coin: WootzWallet.CoinType.ETH,
  supportedKeyrings: [WootzWallet.KeyringId.kDefault],
  decimals: 18,
  iconUrls: [ETHIconUrl],
  rpcEndpoints: [{ url: 'https://mainnet.infura.io/v3/' }],
  symbol: 'ETH',
  symbolName: 'Ethereum'
}

export const mockSepolia: WootzWallet.NetworkInfo = {
  activeRpcEndpointIndex: 0,
  blockExplorerUrls: ['https://sepolia.etherscan.io'],
  chainId: WootzWallet.SEPOLIA_CHAIN_ID,
  chainName: 'Sepolia Test Network',
  coin: 60,
  supportedKeyrings: [WootzWallet.KeyringId.kDefault],
  decimals: 18,
  iconUrls: [ETHIconUrl],
  rpcEndpoints: [{ url: 'https://sepolia-infura.wootz.com' }],
  symbol: 'ETH',
  symbolName: 'Ethereum'
}

export const mockEthLocalhost: WootzWallet.NetworkInfo = {
  activeRpcEndpointIndex: 0,
  blockExplorerUrls: ['http://localhost:7545/'],
  chainId: WootzWallet.LOCALHOST_CHAIN_ID,
  chainName: 'Localhost',
  coin: 60,
  supportedKeyrings: [WootzWallet.KeyringId.kDefault],
  decimals: 18,
  iconUrls: [ETHIconUrl],
  rpcEndpoints: [{ url: 'http://localhost:7545/' }],
  symbol: 'ETH',
  symbolName: 'Ethereum'
}

export const mockFilecoinMainnetNetwork: WootzWallet.NetworkInfo = {
  chainId: 'f',
  chainName: 'Filecoin Mainnet',
  activeRpcEndpointIndex: 0,
  rpcEndpoints: [{ url: 'https://calibration.node.glif.io/rpc/v0' }],
  blockExplorerUrls: ['https://filscan.io/tipset/message-detail'],
  symbol: 'FIL',
  symbolName: 'Filecoin',
  decimals: 18,
  iconUrls: [FILECOINIconUrl],
  coin: WootzWallet.CoinType.FIL,
  supportedKeyrings: [WootzWallet.KeyringId.kFilecoin],
}

export const mockFilecoinTestnetNetwork: WootzWallet.NetworkInfo = {
  chainId: 't',
  chainName: 'Filecoin Testnet',
  activeRpcEndpointIndex: 0,
  rpcEndpoints: [{ url: 'https://solana-mainnet.wallet.wootz.com' }],
  blockExplorerUrls: ['https://calibration.filscan.io/tipset/message-detail'],
  symbol: 'FIL',
  symbolName: 'Filecoin',
  decimals: 18,
  iconUrls: [FILECOINIconUrl],
  coin: WootzWallet.CoinType.FIL,
  supportedKeyrings: [WootzWallet.KeyringId.kFilecoinTestnet],
}

export const mockSolanaMainnetNetwork: WootzWallet.NetworkInfo = {
  activeRpcEndpointIndex: 0,
  blockExplorerUrls: ['https://explorer.solana.com'],
  chainId: '0x65',
  chainName: 'Solana Mainnet Beta',
  coin: WootzWallet.CoinType.SOL,
  supportedKeyrings: [WootzWallet.KeyringId.kSolana],
  decimals: 9,
  iconUrls: [SOLIconUrl],
  rpcEndpoints: [{ url: 'https://api.testnet.solana.com' }],
  symbol: 'SOL',
  symbolName: 'Solana'
}

export const mockSolanaTestnetNetwork: WootzWallet.NetworkInfo = {
  chainId: '0x66',
  chainName: 'Solana Testnet',
  activeRpcEndpointIndex: 0,
  rpcEndpoints: [{ url: 'https://api.testnet.solana.com' }],
  blockExplorerUrls: ['https://explorer.solana.com?cluster=testnet'],
  symbol: 'SOL',
  symbolName: 'Solana',
  decimals: 9,
  iconUrls: [SOLIconUrl],
  coin: WootzWallet.CoinType.SOL,
  supportedKeyrings: [WootzWallet.KeyringId.kSolana],
}

export const mockBitcoinMainnet: WootzWallet.NetworkInfo = {
  activeRpcEndpointIndex: 0,
  blockExplorerUrls: ['https://bitcoin.explorer'],
  chainId: 'bitcoin_mainnet',
  chainName: 'Bitcoin Mainnet',
  coin: WootzWallet.CoinType.BTC,
  supportedKeyrings: [WootzWallet.KeyringId.kBitcoin84],
  decimals: 8,
  iconUrls: [BTCIconUrl],
  rpcEndpoints: [{ url: 'https://bitcoin.rpc' }],
  symbol: 'BTC',
  symbolName: 'Bitcoin'
}

export const mockBitcoinTestnet: WootzWallet.NetworkInfo = {
  activeRpcEndpointIndex: 0,
  blockExplorerUrls: ['https://bitcoin.explorer'],
  chainId: 'bitcoin_testnet',
  chainName: 'Bitcoin Testnet',
  coin: WootzWallet.CoinType.BTC,
  supportedKeyrings: [WootzWallet.KeyringId.kBitcoin84Testnet],
  decimals: 8,
  iconUrls: [BTCIconUrl],
  rpcEndpoints: [{ url: 'https://bitcoin.rpc/test' }],
  symbol: 'BTC',
  symbolName: 'Bitcoin'
}

export const mockNetworks: WootzWallet.NetworkInfo[] = [
  mockEthMainnet,
  mockSepolia,
  mockFilecoinMainnetNetwork,
  mockFilecoinTestnetNetwork,
  mockSolanaMainnetNetwork,
  mockSolanaTestnetNetwork,
  mockEthLocalhost,
  mockBitcoinMainnet
]
