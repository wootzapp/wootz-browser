// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

;(function () {
  if (!window.wootzEthereum) {
    return
  }
  var EventEmitter = require('events')
  var WootzWeb3ProviderEventEmitter = new EventEmitter()
  $Object.defineProperties(window.wootzEthereum, {
    on: {
      value: WootzWeb3ProviderEventEmitter.on,
      writable: false
    },
    emit: {
      value: WootzWeb3ProviderEventEmitter.emit,
      writable: false
    },
    removeListener: {
      value: WootzWeb3ProviderEventEmitter.removeListener,
      writable: false
    },
    removeAllListeners: {
      value: WootzWeb3ProviderEventEmitter.removeAllListeners,
      writable: false
    }
  })
})()
