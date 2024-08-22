// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

// types
import { WootzWallet, WalletState } from '../../constants/types'

// mocks
import { networkEntityAdapter } from '../../common/slices/entities/network.entity'

export const mockWalletState: WalletState = {
  addUserAssetError: false,
  hasInitialized: true,
  isBitcoinEnabled: true,
  isBitcoinImportEnabled: true,
  isZCashEnabled: true,
  isAnkrBalancesFeatureEnabled: false,
  allowedNewWalletAccountTypeNetworkIds: [
    networkEntityAdapter.selectId({
      chainId: WootzWallet.FILECOIN_MAINNET,
      coin: WootzWallet.CoinType.FIL
    }),
    networkEntityAdapter.selectId({
      chainId: WootzWallet.FILECOIN_TESTNET,
      coin: WootzWallet.CoinType.FIL
    }),
    networkEntityAdapter.selectId({
      chainId: WootzWallet.BITCOIN_MAINNET,
      coin: WootzWallet.CoinType.BTC
    }),
    networkEntityAdapter.selectId({
      chainId: WootzWallet.BITCOIN_TESTNET,
      coin: WootzWallet.CoinType.BTC
    }),
    networkEntityAdapter.selectId({
      chainId: WootzWallet.Z_CASH_MAINNET,
      coin: WootzWallet.CoinType.ZEC
    }),
    networkEntityAdapter.selectId({
      chainId: WootzWallet.Z_CASH_TESTNET,
      coin: WootzWallet.CoinType.ZEC
    }),
    networkEntityAdapter.selectId({
      chainId: WootzWallet.SOLANA_MAINNET,
      coin: WootzWallet.CoinType.SOL
    }),
    networkEntityAdapter.selectId({
      chainId: WootzWallet.MAINNET_CHAIN_ID,
      coin: WootzWallet.CoinType.ETH
    })
  ],
  isWalletCreated: false,
  isWalletLocked: false,
  passwordAttempts: 0,
  assetAutoDiscoveryCompleted: false,
  isRefreshingNetworksAndTokens: false
}
