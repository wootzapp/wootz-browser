import {PageCallbackRouter, PageHandlerFactory, PageHandlerRemote} from './solana_wallet.mojom-webui.js';

export class HelloWalletBrowserProxy {
  private handler: PageHandlerRemote;
  private callbackRouter: PageCallbackRouter;

  constructor() {
    this.callbackRouter = new PageCallbackRouter();
    this.handler = new PageHandlerRemote();
    const factory = PageHandlerFactory.getRemote();
    factory.createPageHandler(
        this.callbackRouter.$.bindNewPipeAndPassRemote(),
        this.handler.$.bindNewPipeAndPassReceiver());
  } 
  static getInstance(): HelloWalletBrowserProxy {
    return instance || (instance = new HelloWalletBrowserProxy());
  }
  static setInstance(obj: HelloWalletBrowserProxy) {
    instance = obj;
  }
}

let instance: HelloWalletBrowserProxy|null = null;