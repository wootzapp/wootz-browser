// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { createRoot } from 'react-dom/client'
import { Provider } from 'react-redux'
import { BrowserRouter } from 'react-router-dom'

import { initLocale } from 'brave-ui'

// Style
import walletDarkTheme from '../../../../theme/wallet-dark'
import walletLightTheme from '../../../../theme/wallet-light'
import 'emptykit.css'

// Utils
import { loadTimeData } from '../../../../../common/loadTimeData'

// Redux
import { store } from '../../../store'
import * as WalletActions from '../../../../common/actions/wallet_actions'

// Components
import {
  // eslint-disable-next-line import/no-named-default
  default as WootzCoreThemeProvider
} from '../../../../../common/WootzCoreThemeProvider'
import { FundWalletScreen } from '../../fund-wallet/fund-wallet'

// Resources
// import { setIconBasePath } from '@brave/leo/react/icon'
// setIconBasePath('chrome://resources/wootz-icons')

export function AndroidFundWalletApp() {
  return (
    <Provider store={store}>
      <BrowserRouter>
        <WootzCoreThemeProvider
          dark={walletDarkTheme}
          light={walletLightTheme}
        >
          <FundWalletScreen isAndroid={true} />
        </WootzCoreThemeProvider>
      </BrowserRouter>
    </Provider>
  )
}

function initialize() {
  initLocale(loadTimeData.data_)
  store.dispatch(WalletActions.initialize())
  const root = createRoot(document.getElementById('root')!)
  root.render(<AndroidFundWalletApp />)
}

document.addEventListener('DOMContentLoaded', initialize)
