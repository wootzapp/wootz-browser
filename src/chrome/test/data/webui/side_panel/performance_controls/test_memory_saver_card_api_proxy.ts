// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import type {MemorySaverCardApiProxy} from 'chrome://performance-side-panel.top-chrome/memory_saver_card_api_proxy.js';
import {MemorySaverCardCallbackRouter} from 'chrome://performance-side-panel.top-chrome/performance.mojom-webui.js';
import {TestBrowserProxy} from 'chrome://webui-test/test_browser_proxy.js';


export class TestMemorySaverCardApiProxy extends TestBrowserProxy implements
    MemorySaverCardApiProxy {
  private callbackRouter: MemorySaverCardCallbackRouter =
      new MemorySaverCardCallbackRouter();

  constructor() {
    super([]);
  }

  getCallbackRouter() {
    return this.callbackRouter;
  }
}
