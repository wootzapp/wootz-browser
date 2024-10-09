// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { TopTabNavObjectType } from '../constants/types'
import { getLocale } from '$web-common/locale'

export const TopNavOptions = (): TopTabNavObjectType[] => [
  {
    id: 'portfolio',
    name: getLocale('wootzWalletTopNavPortfolio')
  },
  {
    id: 'nfts',
    name: getLocale('wootzWalletTopNavNFTS')
  },
  {
    id: 'activity', // Transactions
    name: getLocale('wootzWalletActivity')
  },
  {
    id: 'accounts',
    name: getLocale('wootzWalletTopNavAccounts')
  },
  {
    id: 'market',
    name: getLocale('wootzWalletTopNavMarket')
  }
  // Temp commented out for MVP
  // {
  //   id: 'apps',
  //   name: getLocale('wootzWalletTopTabApps')
  // }
]

export const TOP_NAV_OPTIONS = TopNavOptions()
