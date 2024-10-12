// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import type {CrToastManagerElement, DownloadsItemElement} from 'chrome://downloads/downloads.js';
import {BrowserProxy, DangerType, IconLoaderImpl, loadTimeData, SafeBrowsingState, State, TailoredWarningType} from 'chrome://downloads/downloads.js';
import {stringToMojoString16, stringToMojoUrl} from 'chrome://resources/js/mojo_type_util.js';
import {flush} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {assertEquals, assertFalse, assertNotReached, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {eventToPromise, isVisible} from 'chrome://webui-test/test_util.js';

import {createDownload, TestDownloadsProxy, TestIconLoader} from './test_support.js';

suite('item tests', function() {
  let item: DownloadsItemElement;
  let testDownloadsProxy: TestDownloadsProxy;
  let testIconLoader: TestIconLoader;
  let toastManager: CrToastManagerElement;

  setup(function() {
    document.body.innerHTML = window.trustedTypes!.emptyHTML;

    testDownloadsProxy = new TestDownloadsProxy();

    BrowserProxy.setInstance(testDownloadsProxy);

    testIconLoader = new TestIconLoader();
    IconLoaderImpl.setInstance(testIconLoader);

    item = document.createElement('downloads-item');
    document.body.appendChild(item);

    toastManager = document.createElement('cr-toast-manager');
    document.body.appendChild(toastManager);
  });

  test('dangerous downloads aren\'t linkable', () => {
    item.set('data', createDownload({
               dangerType: DangerType.kDangerousFile,
               fileExternallyRemoved: false,
               hideDate: true,
               state: State.kDangerous,
               url: stringToMojoUrl('http://evil.com'),
             }));
    flush();

    assertFalse(isVisible(item.$['file-link']));
    assertFalse(item.$.url.hasAttribute('href'));
    assertFalse(item.$['file-link'].hasAttribute('href'));
  });

  test('downloads without original url in data aren\'t linkable', () => {
    const displayUrl = 'https://test.test';
    item.set('data', createDownload({
               hideDate: false,
               state: State.kComplete,
               url: undefined,
               displayUrl: stringToMojoString16(displayUrl),
             }));
    flush();

    assertFalse(item.$.url.hasAttribute('href'));
    assertFalse(item.$['file-link'].hasAttribute('href'));
    assertEquals(displayUrl, item.$.url.text);
  });

  test('failed deep scans aren\'t linkable', () => {
    item.set('data', createDownload({
               dangerType: DangerType.kDeepScannedFailed,
               fileExternallyRemoved: false,
               hideDate: true,
               state: State.kComplete,
               url: stringToMojoUrl('http://evil.com'),
             }));
    flush();

    assertFalse(isVisible(item.$['file-link']));
    assertFalse(item.$.url.hasAttribute('href'));
  });

  test('url display string is a link to the original url', () => {
    const url = 'https://' +
        'a'.repeat(1000) + '.com/document.pdf';
    const displayUrl = 'https://' +
        '啊'.repeat(1000) + '.com/document.pdf';
    item.set('data', createDownload({
               hideDate: false,
               state: State.kComplete,
               url: stringToMojoUrl(url),
               displayUrl: stringToMojoString16(displayUrl),
             }));
    flush();

    assertEquals(url, item.$.url.href);
    assertEquals(url, item.$['file-link'].href);
    assertEquals(displayUrl, item.$.url.text);
  });

  test('icon loads successfully', async () => {
    testIconLoader.setShouldIconsLoad(true);
    item.set('data', createDownload({filePath: 'unique1', hideDate: false}));
    const loadedPath = await testIconLoader.whenCalled('loadIcon');
    assertEquals(loadedPath, 'unique1');
    flush();
    assertFalse(item.getFileIcon().hidden);
  });

  test('icon fails to load', async () => {
    testIconLoader.setShouldIconsLoad(false);
    item.set('data', createDownload({filePath: 'unique2', hideDate: false}));
    item.set('data', createDownload({hideDate: false}));
    const loadedPath = await testIconLoader.whenCalled('loadIcon');
    assertEquals(loadedPath, 'unique2');
    flush();
    assertTrue(item.getFileIcon().hidden);
  });

  test('icon overridden by danger type', async () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: false});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    testIconLoader.setShouldIconsLoad(true);
    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               dangerType: DangerType.kSensitiveContentBlock,
             }));

    assertEquals('cr:error', item.shadowRoot!.querySelector('iron-icon')!.icon);
    assertTrue(item.$['file-icon'].hidden);

    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               dangerType: DangerType.kBlockedTooLarge,
             }));

    assertEquals('cr:error', item.shadowRoot!.querySelector('iron-icon')!.icon);
    assertTrue(item.$['file-icon'].hidden);

    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               dangerType: DangerType.kBlockedPasswordProtected,
             }));

    assertEquals('cr:error', item.shadowRoot!.querySelector('iron-icon')!.icon);
    assertTrue(item.$['file-icon'].hidden);

    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               dangerType: DangerType.kDeepScannedFailed,
             }));

    assertEquals('cr:info', item.shadowRoot!.querySelector('iron-icon')!.icon);
    assertTrue(item.$['file-icon'].hidden);
  });

  test(
      'icon overridden by display type for improvedDownloadWarningsUX',
      async () => {
        loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
        const item = document.createElement('downloads-item');
        document.body.innerHTML = window.trustedTypes!.emptyHTML;
        document.body.appendChild(item);
        testIconLoader.setShouldIconsLoad(true);
        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kSensitiveContentBlock,
                 }));

        assertEquals(
            'cr:error', item.shadowRoot!.querySelector('iron-icon')!.icon);
        assertTrue(item.$['file-icon'].hidden);
        assertEquals(
            'red',
            item.shadowRoot!.querySelector('iron-icon')!.getAttribute(
                'icon-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   isInsecure: true,
                 }));

        assertEquals(
            'cr:warning', item.shadowRoot!.querySelector('iron-icon')!.icon);
        assertTrue(item.$['file-icon'].hidden);
        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('iron-icon')!.getAttribute(
                'icon-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousFile,
                   safeBrowsingState: SafeBrowsingState.kNoSafeBrowsing,
                 }));

        assertEquals(
            'cr:warning', item.shadowRoot!.querySelector('iron-icon')!.icon);
        assertTrue(item.$['file-icon'].hidden);
        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('iron-icon')!.getAttribute(
                'icon-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousFile,
                   safeBrowsingState: SafeBrowsingState.kEnhancedProtection,
                   hasSafeBrowsingVerdict: true,
                 }));

        assertEquals(
            'cr:warning', item.shadowRoot!.querySelector('iron-icon')!.icon);
        assertTrue(item.$['file-icon'].hidden);
        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('iron-icon')!.getAttribute(
                'icon-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousFile,
                   safeBrowsingState: SafeBrowsingState.kStandardProtection,
                   hasSafeBrowsingVerdict: false,
                 }));

        assertEquals(
            'cr:warning', item.shadowRoot!.querySelector('iron-icon')!.icon);
        assertTrue(item.$['file-icon'].hidden);
        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('iron-icon')!.getAttribute(
                'icon-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDeepScannedFailed,
                 }));

        assertEquals(
            'cr:warning', item.shadowRoot!.querySelector('iron-icon')!.icon);
        assertTrue(item.$['file-icon'].hidden);
        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('iron-icon')!.getAttribute(
                'icon-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousUrl,
                 }));

        assertEquals(
            'downloads:dangerous',
            item.shadowRoot!.querySelector('iron-icon')!.icon);
        assertTrue(item.$['file-icon'].hidden);
        assertEquals(
            'red',
            item.shadowRoot!.querySelector('iron-icon')!.getAttribute(
                'icon-color'));
      });

  test(
      'description color set by display type for improvedDownloadWarningsUX',
      async () => {
        loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
        const item = document.createElement('downloads-item');
        document.body.innerHTML = window.trustedTypes!.emptyHTML;
        document.body.appendChild(item);

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kSensitiveContentBlock,
                 }));

        assertEquals(
            'red',
            item.shadowRoot!.querySelector('.description')!.getAttribute(
                'description-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   isInsecure: true,
                 }));

        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('.description')!.getAttribute(
                'description-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousFile,
                   safeBrowsingState: SafeBrowsingState.kNoSafeBrowsing,
                 }));

        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('.description')!.getAttribute(
                'description-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousFile,
                   safeBrowsingState: SafeBrowsingState.kEnhancedProtection,
                   hasSafeBrowsingVerdict: true,
                 }));

        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('.description')!.getAttribute(
                'description-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousFile,
                   safeBrowsingState: SafeBrowsingState.kStandardProtection,
                   hasSafeBrowsingVerdict: false,
                 }));

        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('.description')!.getAttribute(
                'description-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDeepScannedFailed,
                 }));

        assertEquals(
            'grey',
            item.shadowRoot!.querySelector('.description')!.getAttribute(
                'description-color'));

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   dangerType: DangerType.kDangerousUrl,
                 }));

        assertEquals(
            'red',
            item.shadowRoot!.querySelector('.description')!.getAttribute(
                'description-color'));
      });

  test('description text overridden by tailored warning type', () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);

    function assertDescriptionText(expected: string) {
      assertEquals(
          expected,
          item.shadowRoot!.querySelector('.description:not([hidden])')!
              .textContent!.trim());
    }

    // Suspicious archive
    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               state: State.kDangerous,
               dangerType: DangerType.kUncommonContent,
               tailoredWarningType: TailoredWarningType.kSuspiciousArchive,
             }));
    assertDescriptionText(
        loadTimeData.getString('dangerUncommonSuspiciousArchiveDesc'));

    // Cookie theft without account
    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               state: State.kDangerous,
               dangerType: DangerType.kCookieTheft,
               tailoredWarningType: TailoredWarningType.kCookieTheft,
             }));
    assertDescriptionText(loadTimeData.getString('dangerDownloadCookieTheft'));

    // Cookie theft with account
    item.set(
        'data', createDownload({
          filePath: 'unique1',
          hideDate: false,
          state: State.kDangerous,
          dangerType: DangerType.kCookieTheft,
          tailoredWarningType: TailoredWarningType.kCookieTheftWithAccountInfo,
          accountEmail: 'alice@gmail.com',
        }));
    assertDescriptionText(loadTimeData.getStringF(
        'dangerDownloadCookieTheftAndAccountDesc', 'alice@gmail.com'));

    // Cookie theft with empty account
    item.set(
        'data', createDownload({
          filePath: 'unique1',
          hideDate: false,
          state: State.kDangerous,
          dangerType: DangerType.kCookieTheft,
          tailoredWarningType: TailoredWarningType.kCookieTheftWithAccountInfo,
        }));
    assertDescriptionText(loadTimeData.getString('dangerDownloadCookieTheft'));
  });

  test(
      'icon aria-hidden determined by display type for improvedDownloadWarningsUX',
      () => {
        loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
        const item = document.createElement('downloads-item');
        document.body.innerHTML = window.trustedTypes!.emptyHTML;
        document.body.appendChild(item);
        testIconLoader.setShouldIconsLoad(true);

        const iconWrapper = item.shadowRoot!.querySelector('.icon-wrapper');

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                   isInsecure: true,
                 }));
        flush();
        assertTrue(!!iconWrapper);
        assertEquals('false', iconWrapper.ariaHidden);

        item.set('data', createDownload({
                   dangerType: DangerType.kDangerousFile,
                   hideDate: true,
                   state: State.kDangerous,
                 }));
        flush();
        assertTrue(!!iconWrapper);
        assertEquals('false', iconWrapper.ariaHidden);

        item.set('data', createDownload({
                   filePath: 'unique1',
                   hideDate: false,
                 }));
        flush();
        assertTrue(!!iconWrapper);
        assertEquals('true', iconWrapper!.ariaHidden);
      });

  test('save dangerous click fires event for dangerous', () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               state: State.kDangerous,
               dangerType: DangerType.kDangerousUrl,
             }));
    flush();
    const whenFired = eventToPromise('save-dangerous-click', item);
    item.getMoreActionsButton().click();
    const saveDangerousButton =
        item.shadowRoot!.querySelector<HTMLElement>('#save-dangerous');
    assertTrue(!!saveDangerousButton);
    assertTrue(isVisible(saveDangerousButton));
    saveDangerousButton.click();
    flush();
    return whenFired;
  });

  test('save dangerous click does not fire event for suspicious', async () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({
               id: 'itemId',
               filePath: 'unique1',
               hideDate: false,
               state: State.kDangerous,
               // Uncommon content is suspicious, not dangerous, so no dialog
               // event should be fired.
               dangerType: DangerType.kUncommonContent,
             }));
    flush();

    // The event should never be fired.
    item.addEventListener('save-dangerous-click', () => {
      assertNotReached('Unexpected event fired');
    });

    item.getMoreActionsButton().click();
    const saveDangerousButton =
        item.shadowRoot!.querySelector<HTMLElement>('#save-dangerous');
    assertTrue(!!saveDangerousButton);
    assertTrue(isVisible(saveDangerousButton));
    saveDangerousButton.click();
    flush();
    // The mojo handler is called directly, no event for the dialog is fired.
    const id = await testDownloadsProxy.handler.whenCalled(
        'saveSuspiciousRequiringGesture');
    assertEquals('itemId', id);
  });

  test('deep scan dropdown buttons shown on correct state', () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               state: State.kPromptForScanning,
             }));
    flush();
    item.getMoreActionsButton().click();
    assertTrue(
        isVisible(item.shadowRoot!.querySelector<HTMLElement>('#deep-scan')));
    assertTrue(isVisible(
        item.shadowRoot!.querySelector<HTMLElement>('#bypass-deep-scan')));
  });

  test('local decryption scan icon and text', () => {
    loadTimeData.overrideValues({'improvedDownloadWarningsUX': true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               state: State.kPromptForLocalPasswordScanning,
             }));
    flush();

    const icon = item.shadowRoot!.querySelector(
        'iron-icon[icon="cr:warning"][icon-color=grey]');
    assertTrue(!!icon);

    assertEquals(
        loadTimeData.getString('controlLocalPasswordScan'),
        item.shadowRoot!.querySelector<HTMLElement>(
                            '#deepScan')!.textContent!.trim());
  });

  test('open anyway dropdown button shown on failed deep scan', () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({
               filePath: 'unique1',
               hideDate: false,
               state: State.kComplete,
               dangerType: DangerType.kDeepScannedFailed,
             }));
    flush();
    item.getMoreActionsButton().click();
    assertTrue(
        isVisible(item.shadowRoot!.querySelector<HTMLElement>('#open-anyway')));
  });

  test('undo is shown in toast', () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({hideDate: false}));
    flush();
    toastManager = document.createElement('cr-toast-manager');
    document.body.appendChild(toastManager);
    toastManager.show('', /* hideSlotted= */ true);
    assertTrue(toastManager.slottedHidden);
    item.getMoreActionsButton().click();
    const removeButton = item.shadowRoot!.querySelector<HTMLElement>('#remove');
    assertTrue(!!removeButton);
    removeButton!.click();
    assertFalse(toastManager.slottedHidden);
    assertFalse(item.getMoreActionsMenu().open);
  });

  test('undo is not shown in toast when item is dangerous', () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({hideDate: false, isDangerous: true}));
    flush();
    toastManager = document.createElement('cr-toast-manager');
    document.body.appendChild(toastManager);
    toastManager.show('', /* hideSlotted= */ false);
    assertFalse(toastManager.slottedHidden);
    item.getMoreActionsButton().click();
    const removeButton =
        item.shadowRoot!.querySelector<HTMLElement>('#discard-dangerous');
    assertTrue(!!removeButton);
    removeButton.click();
    assertTrue(toastManager.slottedHidden);
    assertFalse(item.getMoreActionsMenu().open);
  });

  test('undo is not shown in toast when item is insecure', () => {
    loadTimeData.overrideValues({improvedDownloadWarningsUX: true});
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.set('data', createDownload({hideDate: false, isInsecure: true}));
    flush();
    toastManager = document.createElement('cr-toast-manager');
    document.body.appendChild(toastManager);
    toastManager.show('', /* hideSlotted= */ false);
    assertFalse(toastManager.slottedHidden);
    item.getMoreActionsButton().click();
    const removeButton =
        item.shadowRoot!.querySelector<HTMLElement>('#discard-dangerous');
    assertTrue(!!removeButton);
    removeButton.click();
    assertTrue(toastManager.slottedHidden);
    assertFalse(item.getMoreActionsMenu().open);
  });

  // <if expr="_google_chrome">
  test('ESBDownloadRowPromoShownAndClicked', async () => {
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.showEsbPromotion = true;
    item.set('data', createDownload({
               dangerType: DangerType.kDangerousFile,
               fileExternallyRemoved: false,
               hideDate: true,
               state: State.kDangerous,
               isDangerous: true,
               url: stringToMojoUrl('http://evil.com'),
             }));
    flush();
    const esbPromo =
        item.shadowRoot!.querySelector<HTMLElement>('#esb-download-row-promo');
    assertTrue(!!esbPromo);
    assertTrue(isVisible(esbPromo));
    esbPromo.click();
    await testDownloadsProxy.handler.whenCalled('openEsbSettings');
  });

  test('ESBDownloadRowPromoNotShown', () => {
    const item = document.createElement('downloads-item');
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    document.body.appendChild(item);
    item.showEsbPromotion = false;
    item.set('data', createDownload({
               dangerType: DangerType.kDangerousFile,
               fileExternallyRemoved: false,
               hideDate: true,
               state: State.kDangerous,
               isDangerous: true,
               url: stringToMojoUrl('http://evil.com'),
             }));
    flush();
    const esbPromo = item.shadowRoot!.querySelector('#esb-download-row-promo');
    assertFalse(isVisible(esbPromo));
  });
  // </if>
});
