// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import WalletApiProxy from '../common/wallet_api_proxy'
import { WootzWallet } from '../constants/types'

export class WalletPanelApiProxy extends WalletApiProxy {
  panelHandler = new WootzWallet.PanelHandlerRemote()

  constructor() {
    super()

    const factory = WootzWallet.PanelHandlerFactory.getRemote()
    factory.createPanelHandler(
      this.panelHandler.$.bindNewPipeAndPassReceiver(),
      this.walletHandler.$.bindNewPipeAndPassReceiver(),
      this.jsonRpcService.$.bindNewPipeAndPassReceiver(),
      this.bitcoinWalletService.$.bindNewPipeAndPassReceiver(),
      this.zcashWalletService.$.bindNewPipeAndPassReceiver(),
      this.swapService.$.bindNewPipeAndPassReceiver(),
      this.simulationService.$.bindNewPipeAndPassReceiver(),
      this.assetRatioService.$.bindNewPipeAndPassReceiver(),
      this.keyringService.$.bindNewPipeAndPassReceiver(),
      this.blockchainRegistry.$.bindNewPipeAndPassReceiver(),
      this.txService.$.bindNewPipeAndPassReceiver(),
      this.ethTxManagerProxy.$.bindNewPipeAndPassReceiver(),
      this.solanaTxManagerProxy.$.bindNewPipeAndPassReceiver(),
      this.filTxManagerProxy.$.bindNewPipeAndPassReceiver(),
      this.wootzWalletService.$.bindNewPipeAndPassReceiver(),
      this.wootzWalletP3A.$.bindNewPipeAndPassReceiver(),
      this.wootzWalletIpfsService.$.bindNewPipeAndPassReceiver(),
      this.meldIntegrationService.$.bindNewPipeAndPassReceiver()
    )
  }
}

let walletPanelApiProxyInstance: WalletPanelApiProxy

export default function getWalletPanelApiProxy() {
  if (!walletPanelApiProxyInstance) {
    walletPanelApiProxyInstance = new WalletPanelApiProxy()
  }
  return walletPanelApiProxyInstance
}
