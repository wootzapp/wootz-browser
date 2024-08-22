/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { getLocale } from '$web-common/locale'
import { WootzWallet, BuyOption } from '../constants/types'

import RampIcon from '../assets/svg-icons/ramp-icon.svg'
import SardineIconLight from '../assets/svg-icons/sardine-logo-light.svg'
import SardineIconDark from '../assets/svg-icons/sardine-logo-dark.svg'
import TransakIcon from '../assets/svg-icons/transak-logo.svg'
import StripeIcon from '../assets/svg-icons/stripe-logo.svg'
import CoinbaseIcon from '../assets/svg-icons/coinbase-logo.svg'
import { isStripeSupported } from '../utils/asset-utils'

function getBuyOptions(): BuyOption[] {
  const buyOptions = [
    {
      id: WootzWallet.OnRampProvider.kRamp,
      actionText: getLocale('wootzWalletBuyWithRamp'),
      icon: RampIcon,
      name: getLocale('wootzWalletBuyRampNetworkName'),
      description: getLocale('wootzWalletBuyRampDescription')
    },
    {
      id: WootzWallet.OnRampProvider.kTransak,
      actionText: getLocale('wootzWalletBuyWithTransak'),
      icon: TransakIcon,
      name: getLocale('wootzWalletBuyTransakName'),
      description: getLocale('wootzWalletBuyTransakDescription')
    },
    {
      id: WootzWallet.OnRampProvider.kSardine,
      actionText: getLocale('wootzWalletBuyWithSardine'),
      icon: window.matchMedia('(prefers-color-scheme: dark)').matches
        ? SardineIconDark
        : SardineIconLight,
      name: getLocale('wootzWalletBuySardineName'),
      description: getLocale('wootzWalletBuySardineDescription')
    },
    {
      id: WootzWallet.OnRampProvider.kCoinbase,
      actionText: getLocale('wootzWalletBuyWithCoinbase'),
      icon: CoinbaseIcon,
      name: getLocale('wootzWalletBuyCoinbaseName'),
      description: getLocale('wootzWalletBuyCoinbaseDescription')
    }
  ]

  if (isStripeSupported()) {
    buyOptions.push({
      id: WootzWallet.OnRampProvider.kStripe,
      actionText: getLocale('wootzWalletBuyWithStripe'),
      icon: StripeIcon,
      name: getLocale('wootzWalletBuyStripeName'),
      description: getLocale('wootzWalletBuyStripeDescription')
    })
  }

  return buyOptions
}

export const BuyOptions = getBuyOptions()
