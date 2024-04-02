// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import type {ExtensionsHatsBrowserProxy} from 'chrome://extensions/extensions.js';
import {TestBrowserProxy} from 'chrome://webui-test/test_browser_proxy.js';

export class TestExtensionsHatsBrowserProxy extends TestBrowserProxy implements
    ExtensionsHatsBrowserProxy {
  constructor() {
    super([
      'panelShown',
      'extensionKeptAction',
      'extensionRemovedAction',
      'nonTriggerExtensionRemovedAction',
      'removeAllAction',
    ]);
  }

  panelShown() {
    this.methodCalled('panelShown');
  }

  extensionKeptAction() {
    this.methodCalled('extensionKeptAction');
  }

  extensionRemovedAction() {
    this.methodCalled('extensionRemovedAction');
  }

  nonTriggerExtensionRemovedAction() {
    this.methodCalled('nonTriggerExtensionRemovedAction');
  }

  removeAllAction(numberOfExtensionsRemoved: number) {
    this.methodCalled('removeAllAction', [numberOfExtensionsRemoved]);
  }
}
