// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import { WootzWallet } from '../../constants/types'

export const mockOriginInfo: WootzWallet.OriginInfo = {
  originSpec: 'https://with_a_really_looooooong_site_name.fixme.uniswap.org',
  eTldPlusOne: 'uniswap.org'
}

export const mockUniswapOriginInfo: WootzWallet.OriginInfo = {
  originSpec: 'https://app.uniswap.org',
  eTldPlusOne: 'uniswap.org'
}

export const mockWootzWalletOrigin: WootzWallet.OriginInfo = {
  originSpec: 'wootzapp://wallet',
  eTldPlusOne: 'wallet'
}
