// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {PageCallbackRouter, PageHandlerFactory, PageHandlerRemote} from './throttling.mojom-webui.js';

export class ThrottlingBrowserProxy {
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

  setCustomNetworkConditions(offline: boolean, latency: number, downloadThroughput: number, uploadThroughput: number,packetLoss:number, packetQueueLength:number, packetReordering:boolean) {
    return this.handler.setCustomNetworkConditions({offline,latency, downloadThroughput, uploadThroughput,packetLoss, packetQueueLength, packetReordering});
  }

  static getInstance(): ThrottlingBrowserProxy {
    return instance || (instance = new ThrottlingBrowserProxy());
  }

  getCallbackRouter(): PageCallbackRouter {
    return this.callbackRouter;
  }
}

let instance: ThrottlingBrowserProxy|null = null;