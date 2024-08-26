// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { WootzWallet } from '../constants/types'

export class WalletApiProxy {
  walletHandler = new WootzWallet.WalletHandlerRemote()
  jsonRpcService = new WootzWallet.JsonRpcServiceRemote()
  bitcoinWalletService = new WootzWallet.BitcoinWalletServiceRemote()
  zcashWalletService = new WootzWallet.ZCashWalletServiceRemote()
  swapService = new WootzWallet.SwapServiceRemote()
  simulationService = new WootzWallet.SimulationServiceRemote()
  assetRatioService = new WootzWallet.AssetRatioServiceRemote()
  meldIntegrationService = new WootzWallet.MeldIntegrationServiceRemote()
  keyringService = new WootzWallet.KeyringServiceRemote()
  blockchainRegistry = new WootzWallet.BlockchainRegistryRemote()
  txService = new WootzWallet.TxServiceRemote()
  ethTxManagerProxy = new WootzWallet.EthTxManagerProxyRemote()
  solanaTxManagerProxy = new WootzWallet.SolanaTxManagerProxyRemote()
  filTxManagerProxy = new WootzWallet.FilTxManagerProxyRemote()
  wootzWalletService = new WootzWallet.WootzWalletServiceRemote()
  wootzWalletP3A = new WootzWallet.WootzWalletP3ARemote()
  wootzWalletIpfsService = new WootzWallet.IpfsServiceRemote()

  addJsonRpcServiceObserver(
    observer: WootzWallet.JsonRpcServiceObserverReceiver
  ) {
    this.jsonRpcService.addObserver(observer.$.bindNewPipeAndPassRemote())
  }

  addKeyringServiceObserver(
    observer: WootzWallet.KeyringServiceObserverReceiver
  ) {
    this.keyringService.addObserver(observer.$.bindNewPipeAndPassRemote())
  }

  addTxServiceObserver(observer: WootzWallet.TxServiceObserverReceiver) {
    this.txService.addObserver(observer.$.bindNewPipeAndPassRemote())
  }

  addWootzWalletServiceObserver(
    observer: WootzWallet.WootzWalletServiceObserverReceiver
  ) {
    this.wootzWalletService.addObserver(observer.$.bindNewPipeAndPassRemote())
  }

  addWootzWalletServiceTokenObserver(
    observer: WootzWallet.WootzWalletServiceTokenObserverReceiver
  ) {
    this.wootzWalletService.addTokenObserver(
      observer.$.bindNewPipeAndPassRemote()
    )
  }
}

export default WalletApiProxy
