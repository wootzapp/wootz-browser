// Copyright (c) 2020 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { createRoot } from 'react-dom/client'
import { Provider } from 'react-redux'
import { initLocale } from 'brave-ui'
import { BrowserRouter } from 'react-router-dom'

// assets
import faveiconUrl from '../assets/svg-icons/brave-icon.svg'

// utils
import { loadTimeData } from '../../common/loadTimeData'
import {
  runLocalStorageMigrations //
} from '../common/constants/local-storage-keys'

// redux
import * as WalletActions from '../common/actions/wallet_actions'
import { store } from './store'

// components
import WootzCoreThemeProvider from '../../common/WootzCoreThemeProvider'
import Container from './container'

// style
import walletDarkTheme from '../theme/wallet-dark'
import walletLightTheme from '../theme/wallet-light'
import 'emptykit.css'

import { setIconBasePath } from '@brave/leo/react/icon'
setIconBasePath('chrome://resources/wootz-icons')

function App() {
  React.useEffect(() => {
    /** Sets FAVICON for Wallet Pages */
    let link = document.querySelector("link[rel~='icon']") as HTMLLinkElement
    if (!link) {
      link = document.createElement('link')
      link.rel = 'icon'
      document.getElementsByTagName('head')[0].appendChild(link)
    }
    link.href = faveiconUrl
  }, [])

  React.useEffect(() => {
    runLocalStorageMigrations()
  }, [])

  return (
    <Provider store={store}>
      <BrowserRouter>
        <WootzCoreThemeProvider
          dark={walletDarkTheme}
          light={walletLightTheme}
        >
          <Container />
        </WootzCoreThemeProvider>
      </BrowserRouter>
    </Provider>
  )
}

function initialize() {
  initLocale(loadTimeData.data_)
  const root = createRoot(document.getElementById('root')!)
  root.render(<App />)
  store.dispatch(WalletActions.initialize())
}

document.addEventListener('DOMContentLoaded', initialize)
