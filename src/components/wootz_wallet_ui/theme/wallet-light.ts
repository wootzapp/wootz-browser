/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import ITheme from 'wootz-ui/theme/theme-interface'
import IThemeWallet from './wallet-theme'
import defaultTheme from 'wootz-ui/theme/wootz-default'
import colors from 'wootz-ui/theme/colors'

const walletLightTheme: ITheme & IThemeWallet = {
  ...defaultTheme,
  name: 'Wallet Light',
  color: {
    ...defaultTheme.color,
    text: colors.neutral900,
    panelBackground: '#F9F9FD',
    panelBackgroundSecondary: colors.neutral000,
    outlineColor: 'rgba(255,80,0,0.2)'
  }
}

export default walletLightTheme
