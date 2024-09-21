// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { loadTimeData } from '../../common/loadTimeData'
import { WootzWallet } from '../constants/types'
import { isComponentInStorybook } from '../utils/string-utils'

const marketUiOrigin = loadTimeData.getString('wootzWalletMarketUiBridgeUrl')
export const wootzWalletPanelOrigin = 'wootzapp://wallet-panel.top-chrome'

// remove trailing /
export const wootzMarketUiOrigin = marketUiOrigin.endsWith('/')
  ? marketUiOrigin.slice(0, -1)
  : marketUiOrigin
export const wootzWalletOrigin = 'wootzapp://wallet'

export const enum MarketUiCommand {
  UpdateCoinMarkets = 'update-coin-markets',
  SelectCoinMarket = 'select-coin-market',
  SelectBuy = 'select-buy',
  SelectDeposit = 'select-deposit',
  UpdateTradableAssets = 'update-tradable-assets',
  UpdateBuyableAssets = 'update-buyable-assets',
  UpdateDepositableAssets = 'update-depositable-assets',
  UpdateIframeHeight = 'update-iframe-height'
}

export type MarketCommandMessage = {
  command: MarketUiCommand
}

export type UpdateCoinMarketMessage = MarketCommandMessage & {
  payload: {
    coins: WootzWallet.CoinMarket[]
    defaultFiatCurrency: string
  }
}

export type SelectCoinMarketMessage = MarketCommandMessage & {
  payload: WootzWallet.CoinMarket
}

export type SelectBuyMessage = MarketCommandMessage & {
  payload: WootzWallet.CoinMarket
}

export type SelectDepositMessage = MarketCommandMessage & {
  payload: WootzWallet.CoinMarket
}

export type UpdateBuyableAssetsMessage = MarketCommandMessage & {
  payload: WootzWallet.BlockchainToken[]
}

export type UpdateDepositableAssetsMessage = MarketCommandMessage & {
  payload: WootzWallet.BlockchainToken[]
}

export type UpdateIframeHeightMessage = MarketCommandMessage & {
  payload: number
}

export const sendMessageToMarketUiFrame = (
  targetWindow: Window | null,
  message: MarketCommandMessage
) => {
  if (targetWindow && !isComponentInStorybook()) {
    targetWindow.postMessage(message, wootzMarketUiOrigin)
  }
}

export const sendMessageToWalletUi = (
  targetWindow: Window | null,
  message: MarketCommandMessage,
  origin: string
) => {
  if (targetWindow) {
    targetWindow.postMessage(message, origin)
  }
}
