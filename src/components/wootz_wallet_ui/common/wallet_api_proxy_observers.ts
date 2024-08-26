// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// types & constants
import { WootzWallet } from '../constants/types'
import { Store } from './async/types'

// redux
import * as WalletActions from './actions/wallet_actions'
import { walletApi } from './slices/api.slice'

// utils
import { makeSerializableTransaction } from '../utils/model-serialization-utils'
import { getCoinFromTxDataUnion } from '../utils/network-utils'
import { getHasPendingRequests } from '../utils/api-utils'

export function makeWootzWalletServiceTokenObserver(store: Store) {
  const wootzWalletServiceTokenObserverReceiver =
    new WootzWallet.WootzWalletServiceTokenObserverReceiver({
      onTokenAdded(token) {
        store.dispatch(
          walletApi.endpoints.invalidateUserTokensRegistry.initiate()
        )
        store.dispatch(WalletActions.refreshNetworksAndTokens())
        // re-parse transactions with new coins list
        store.dispatch(
          walletApi.endpoints.invalidateTransactionsCache.initiate()
        )
      },
      onTokenRemoved(token) {
        store.dispatch(
          walletApi.endpoints.invalidateUserTokensRegistry.initiate()
        )
        store.dispatch(WalletActions.refreshNetworksAndTokens())
        // re-parse transactions with new coins list
        store.dispatch(
          walletApi.endpoints.invalidateTransactionsCache.initiate()
        )
      }
    })
  return wootzWalletServiceTokenObserverReceiver
}

export function makeJsonRpcServiceObserver(store: Store) {
  const jsonRpcServiceObserverReceiver =
    new WootzWallet.JsonRpcServiceObserverReceiver({
      chainChangedEvent: function (chainId, coin, origin) {
        store.dispatch(walletApi.endpoints.invalidateSelectedChain.initiate())
      },
      onAddEthereumChainRequestCompleted: function (chainId, error) {
        // update add/switch chain requests query data
        store.dispatch(
          walletApi.util.invalidateTags([
            'PendingAddChainRequests',
            'PendingSwitchChainRequests'
          ])
        )
      }
    })
  return jsonRpcServiceObserverReceiver
}

export function makeKeyringServiceObserver(store: Store) {
  const keyringServiceObserverReceiver =
    new WootzWallet.KeyringServiceObserverReceiver({
      walletCreated: function () {
        store.dispatch(WalletActions.walletCreated())
      },
      walletRestored: function () {
        store.dispatch(WalletActions.walletRestored())
      },
      walletReset: function () {
        store.dispatch(WalletActions.walletReset())
      },
      locked: function () {
        store.dispatch(WalletActions.locked())
      },
      unlocked: function () {
        store.dispatch(WalletActions.unlocked())
      },
      backedUp: function () {
        store.dispatch(WalletActions.backedUp())
      },
      accountsChanged: function () {
        store.dispatch(walletApi.endpoints.invalidateAccountInfos.initiate())
        store.dispatch(walletApi.endpoints.invalidateSelectedAccount.initiate())
      },
      accountsAdded: function () {
        store.dispatch(walletApi.endpoints.invalidateAccountInfos.initiate())
        store.dispatch(walletApi.endpoints.invalidateSelectedAccount.initiate())
      },
      autoLockMinutesChanged: function () {
        store.dispatch(WalletActions.autoLockMinutesChanged())
      },
      selectedWalletAccountChanged: function (
        account: WootzWallet.AccountInfo
      ) {
        store.dispatch(walletApi.endpoints.invalidateSelectedAccount.initiate())
      },
      selectedDappAccountChanged: function (
        coin: WootzWallet.CoinType,
        account: WootzWallet.AccountInfo | null
      ) {
        // TODO: Handle this event.
      }
    })
  return keyringServiceObserverReceiver
}

export function makeTxServiceObserver(store: Store) {
  const txServiceManagerObserverReceiver =
    new WootzWallet.TxServiceObserverReceiver({
      onNewUnapprovedTx: function (txInfo) {
        store.dispatch(
          walletApi.endpoints.newUnapprovedTxAdded.initiate(
            makeSerializableTransaction(txInfo)
          )
        )
      },
      onUnapprovedTxUpdated: function (txInfo) {
        store.dispatch(
          walletApi.endpoints.unapprovedTxUpdated.initiate(
            makeSerializableTransaction(txInfo)
          )
        )
      },
      onTransactionStatusChanged: (txInfo) => {
        store.dispatch(
          walletApi.endpoints.transactionStatusChanged.initiate({
            chainId: txInfo.chainId,
            coinType: getCoinFromTxDataUnion(txInfo.txDataUnion),
            fromAccountId: txInfo.fromAccountId,
            id: txInfo.id,
            txStatus: txInfo.txStatus
          })
        )

        // close then panel UI if there are no more pending transactions
        if (
          [
            WootzWallet.TransactionStatus.Submitted,
            WootzWallet.TransactionStatus.Signed,
            WootzWallet.TransactionStatus.Rejected,
            WootzWallet.TransactionStatus.Approved
          ].includes(txInfo.txStatus)
        ) {
          const state = store.getState()
          if (
            state.panel && // run only in panel
            state.ui.selectedPendingTransactionId === undefined &&
            (state.panel?.selectedPanel === 'approveTransaction' ||
              txInfo.txStatus === WootzWallet.TransactionStatus.Rejected)
          ) {
            getHasPendingRequests().then((hasPendingRequests) => {
              if (!hasPendingRequests) {
                store.dispatch(walletApi.endpoints.closePanelUI.initiate())
              }
            })
          }
        }
      },
      onTxServiceReset: function () {}
    })
  return txServiceManagerObserverReceiver
}

export function makeWootzWalletServiceObserver(store: Store) {
  let lastKnownActiveOrigin: WootzWallet.OriginInfo
  const wootzWalletServiceObserverReceiver =
    new WootzWallet.WootzWalletServiceObserverReceiver({
      onActiveOriginChanged: function (originInfo) {
        // check that the origin has changed from the stored values
        // in any way before dispatching the update action
        if (
          lastKnownActiveOrigin &&
          lastKnownActiveOrigin.eTldPlusOne === originInfo.eTldPlusOne &&
          lastKnownActiveOrigin.originSpec === originInfo.originSpec
        ) {
          return
        }
        lastKnownActiveOrigin = originInfo
        store.dispatch(walletApi.util.invalidateTags(['ActiveOrigin']))
      },
      onDefaultEthereumWalletChanged: function (defaultWallet) {
        store.dispatch(
          walletApi.util.invalidateTags([
            'DefaultEthWallet',
            'IsMetaMaskInstalled'
          ])
        )
      },
      onDefaultSolanaWalletChanged: function (defaultWallet) {
        store.dispatch(walletApi.util.invalidateTags(['DefaultSolWallet']))
      },
      onDefaultBaseCurrencyChanged: function (currency) {
        store.dispatch(WalletActions.defaultBaseCurrencyChanged({ currency }))
      },
      onDefaultBaseCryptocurrencyChanged: function (cryptocurrency) {
        store.dispatch(
          WalletActions.defaultBaseCryptocurrencyChanged({ cryptocurrency })
        )
      },
      onNetworkListChanged: function () {
        // FIXME(onyb): Due to a bug, the OnNetworkListChanged event is fired
        // merely upon switching to a custom network.
        //
        // Skipping balances refresh for now, until the bug is fixed.
        store.dispatch(WalletActions.refreshNetworksAndTokens())
      },
      onDiscoverAssetsStarted: function () {
        store.dispatch(WalletActions.setAssetAutoDiscoveryCompleted(false))
      },
      onDiscoverAssetsCompleted: function () {
        store.dispatch(
          walletApi.endpoints.invalidateUserTokensRegistry.initiate()
        )
        store.dispatch(WalletActions.setAssetAutoDiscoveryCompleted(true))
      },
      onResetWallet: function () {}
    })
  return wootzWalletServiceObserverReceiver
}
