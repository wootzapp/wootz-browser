// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome-untrusted://lens/lens_overlay_app.js';

import {BrowserProxyImpl} from 'chrome-untrusted://lens/browser_proxy.js';
import type {LensOverlayAppElement} from 'chrome-untrusted://lens/lens_overlay_app.js';
import {assertFalse, assertTrue} from 'chrome-untrusted://webui-test/chai_assert.js';
import {waitBeforeNextRender} from 'chrome-untrusted://webui-test/polymer_test_util.js';

import {TestLensOverlayBrowserProxy} from './test_overlay_browser_proxy.js';

suite('OverlayInfoButton', () => {
  let testBrowserProxy: TestLensOverlayBrowserProxy;
  let lensOverlayElement: LensOverlayAppElement;

  setup(() => {
    // Resetting the HTML needs to be the first thing we do in setup to
    // guarantee that any singleton instances don't change while any UI is still
    // attached to the DOM.
    document.body.innerHTML = window.trustedTypes!.emptyHTML;

    testBrowserProxy = new TestLensOverlayBrowserProxy();
    BrowserProxyImpl.setInstance(testBrowserProxy);

    lensOverlayElement = document.createElement('lens-overlay-app');
    document.body.appendChild(lensOverlayElement);
    return waitBeforeNextRender(lensOverlayElement);
  });

  test('verify clicking info button calls browser proxy', async () => {
    lensOverlayElement.$.infoButton.dispatchEvent(new MouseEvent('click', {
      button: 1,
      altKey: false,
      ctrlKey: true,
      metaKey: false,
      shiftKey: true,
    }));
    const clickModifiers =
        await testBrowserProxy.handler.whenCalled('infoRequestedByOverlay');
    assertTrue(clickModifiers.middleButton);
    assertFalse(clickModifiers.altKey);
    assertTrue(clickModifiers.ctrlKey);
    assertFalse(clickModifiers.metaKey);
    assertTrue(clickModifiers.shiftKey);
  });
});
