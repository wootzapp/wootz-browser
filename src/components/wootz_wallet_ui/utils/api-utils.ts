// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { mapLimit } from 'async'
import { Store } from 'redux'

// actions
import { PanelActions } from '../panel/actions'

// types
import type WalletApiProxy from '../common/wallet_api_proxy'
import { WootzWallet, SupportedCoinTypes } from '../constants/types'

// utils
import getAPIProxy from '../common/async/bridge'

export function handleEndpointError(
  endpointName: string,
  friendlyMessage: string,
  error: any
) {
  const message = `${friendlyMessage}: ${error?.message || error}`
  console.log(`error in: ${endpointName || 'endpoint'}: ${message}`)
  console.error(error)
  return {
    error: friendlyMessage
  }
}

export async function getEnabledCoinTypes(api: WalletApiProxy) {
  const { isBitcoinEnabled, isZCashEnabled } = (
    await api.walletHandler.getWalletInfo()
  ).walletInfo

  // Get All Networks
  return SupportedCoinTypes.filter((coin) => {
    return (
      coin === WootzWallet.CoinType.FIL ||
      coin === WootzWallet.CoinType.SOL ||
      (coin === WootzWallet.CoinType.BTC && isBitcoinEnabled) ||
      (coin === WootzWallet.CoinType.ZEC && isZCashEnabled) ||
      coin === WootzWallet.CoinType.ETH
    )
  })
}

export async function getVisibleNetworksList(api: WalletApiProxy) {
  const { jsonRpcService } = api

  const enabledCoinTypes = await getEnabledCoinTypes(api)
  const { networks: allNetworks } = await jsonRpcService.getAllNetworks()

  const networks = (
    await mapLimit(enabledCoinTypes, 10, async (coin: number) => {
      const { chainIds: hiddenChainIds } =
        await jsonRpcService.getHiddenNetworks(coin)
      return allNetworks.filter((n) => !hiddenChainIds.includes(n.chainId))
    })
  ).flat(1)

  return networks
}

export function navigateToConnectHardwareWallet(
  panelHandler: WootzWallet.PanelHandlerRemote,
  store: Pick<Store, 'dispatch' | 'getState'>
) {
  panelHandler.setCloseOnDeactivate(false)

  const selectedPanel: string | undefined =
    store.getState()?.panel?.selectedPanel

  if (selectedPanel === 'connectHardwareWallet') {
    return
  }

  store.dispatch(PanelActions.navigateTo('connectHardwareWallet'))
  store.dispatch(PanelActions.setHardwareWalletInteractionError(undefined))
}

export const getHasPendingRequests = async () => {
  const { wootzWalletService, jsonRpcService, txService } = getAPIProxy()

  const { count: pendingTxsCount } =
    await txService.getPendingTransactionsCount()
  if (pendingTxsCount > 0) {
    return true
  }

  const { requests: signAllTxsRequests } =
    await wootzWalletService.getPendingSignAllTransactionsRequests()
  if (signAllTxsRequests.length) {
    return true
  }

  const { requests: signMessageRequests } =
    await wootzWalletService.getPendingSignMessageRequests()
  if (signMessageRequests.length) {
    return true
  }

  const { requests: signTxRequests } =
    await wootzWalletService.getPendingSignTransactionRequests()
  if (signTxRequests.length) {
    return true
  }

  const { requests: addTokenRequests } =
    await wootzWalletService.getPendingAddSuggestTokenRequests()
  if (addTokenRequests.length) {
    return true
  }

  const { requests: decryptRequests } =
    await wootzWalletService.getPendingDecryptRequests()
  if (decryptRequests.length) {
    return true
  }

  const { requests: publicKeyRequests } =
    await wootzWalletService.getPendingGetEncryptionPublicKeyRequests()
  if (publicKeyRequests.length) {
    return true
  }

  const { requests: addChainRequests } =
    await jsonRpcService.getPendingAddChainRequests()
  if (addChainRequests.length) {
    return true
  }

  const { requests: switchChainRequests } =
    await jsonRpcService.getPendingSwitchChainRequests()
  if (switchChainRequests.length) {
    return true
  }

  return false
}
