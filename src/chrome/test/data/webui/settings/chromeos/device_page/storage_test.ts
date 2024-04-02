// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://os-settings/lazy_load.js';

import {SettingsStorageElement} from 'chrome://os-settings/lazy_load.js';
import {DevicePageBrowserProxyImpl, Router, routes, setDisplayApiForTesting, StorageSpaceState} from 'chrome://os-settings/os_settings.js';
import {webUIListenerCallback} from 'chrome://resources/js/cr.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {flush} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {assertEquals, assertFalse, assertNull, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {flushTasks} from 'chrome://webui-test/polymer_test_util.js';
import {isVisible} from 'chrome://webui-test/test_util.js';

import {FakeSystemDisplay} from '../fake_system_display.js';

import {getFakePrefs} from './device_page_test_util.js';
import {TestDevicePageBrowserProxy} from './test_device_page_browser_proxy.js';

suite('<settings-storage> for device page', () => {
  const isRevampWayfindingEnabled =
      loadTimeData.getBoolean('isRevampWayfindingEnabled');
  let storageSubpage: SettingsStorageElement;
  let fakeSystemDisplay: FakeSystemDisplay;
  let browserProxy: TestDevicePageBrowserProxy;

  setup(async () => {
    fakeSystemDisplay = new FakeSystemDisplay();
    setDisplayApiForTesting(fakeSystemDisplay);

    Router.getInstance().navigateTo(routes.STORAGE);

    browserProxy = new TestDevicePageBrowserProxy();
    DevicePageBrowserProxyImpl.setInstanceForTesting(browserProxy);
  });

  teardown(() => {
    Router.getInstance().resetRouteForTesting();
  });

  async function createStorageSubpage(): Promise<void> {
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    storageSubpage = document.createElement('settings-storage');
    storageSubpage.prefs = getFakePrefs();
    document.body.appendChild(storageSubpage);
    await flushTasks();

    storageSubpage['stopPeriodicUpdate_']();
  }

  /**
   * Simulate storage size stat callback.
   */
  function sendStorageSizeStat(
      usedSize: string, availableSize: string, usedRatio: number,
      spaceState: number): void {
    webUIListenerCallback('storage-size-stat-changed', {
      usedSize: usedSize,
      availableSize: availableSize,
      usedRatio: usedRatio,
      spaceState: spaceState,
    });
    flush();
  }

  function getStorageItemLabelFromId(id: string): string {
    const rowItem = storageSubpage.shadowRoot!.querySelector('#' + id);
    assertTrue(!!rowItem);
    const label = rowItem.shadowRoot!.querySelector<HTMLElement>('#label');
    assertTrue(!!label);
    return label.innerText;
  }

  function getStorageItemSubLabelFromId(id: string): string {
    const rowItem = storageSubpage.shadowRoot!.querySelector('#' + id);
    assertTrue(!!rowItem);
    const subLabel =
        rowItem.shadowRoot!.querySelector<HTMLElement>('#subLabel');
    assertTrue(!!subLabel);
    return subLabel.innerText;
  }

  test('storage stats size', async () => {
    await createStorageSubpage();

    // Low available storage space.
    sendStorageSizeStat('9.1 GB', '0.9 GB', 0.91, StorageSpaceState.LOW);
    assertEquals('91%', storageSubpage.$.inUseLabelArea.style.width);
    assertEquals('9%', storageSubpage.$.availableLabelArea.style.width);
    assertTrue(
        isVisible(storageSubpage.shadowRoot!.querySelector('#lowMessage')));
    assertFalse(isVisible(
        storageSubpage.shadowRoot!.querySelector('#criticallyLowMessage')));
    assertTrue(!!storageSubpage.shadowRoot!.querySelector('#bar.space-low'));
    assertNull(
        storageSubpage.shadowRoot!.querySelector('#bar.space-critically-low'));

    let inUseArea = storageSubpage.$.inUseLabelArea.querySelector<HTMLElement>(
        '.storage-size');
    assertTrue(!!inUseArea);
    assertEquals('9.1 GB', inUseArea.innerText);

    let availableArea =
        storageSubpage.$.availableLabelArea.querySelector<HTMLElement>(
            '.storage-size');
    assertTrue(!!availableArea);
    assertEquals('0.9 GB', availableArea.innerText);

    // Critically low available storage space.
    sendStorageSizeStat(
        '9.7 GB', '0.3 GB', 0.97, StorageSpaceState.CRITICALLY_LOW);
    assertEquals('97%', storageSubpage.$.inUseLabelArea.style.width);
    assertEquals('3%', storageSubpage.$.availableLabelArea.style.width);
    assertFalse(
        isVisible(storageSubpage.shadowRoot!.querySelector('#lowMessage')));
    assertTrue(isVisible(
        storageSubpage.shadowRoot!.querySelector('#criticallyLowMessage')));
    assertNull(storageSubpage.shadowRoot!.querySelector('#bar.space-low'));
    assertTrue(!!storageSubpage.shadowRoot!.querySelector(
        '#bar.space-critically-low'));

    inUseArea = storageSubpage.$.inUseLabelArea.querySelector<HTMLElement>(
        '.storage-size');
    assertTrue(!!inUseArea);
    assertEquals('9.7 GB', inUseArea.innerText);

    availableArea =
        storageSubpage.$.availableLabelArea.querySelector<HTMLElement>(
            '.storage-size');
    assertTrue(!!availableArea);
    assertEquals('0.3 GB', availableArea.innerText);

    // Normal storage usage.
    sendStorageSizeStat('2.5 GB', '7.5 GB', 0.25, StorageSpaceState.NORMAL);
    assertEquals('25%', storageSubpage.$.inUseLabelArea.style.width);
    assertEquals('75%', storageSubpage.$.availableLabelArea.style.width);
    assertFalse(
        isVisible(storageSubpage.shadowRoot!.querySelector('#lowMessage')));
    assertFalse(isVisible(
        storageSubpage.shadowRoot!.querySelector('#criticallyLowMessage')));
    assertNull(storageSubpage.shadowRoot!.querySelector('#bar.space-low'));
    assertNull(
        storageSubpage.shadowRoot!.querySelector('#bar.space-critically-low'));

    inUseArea = storageSubpage.$.inUseLabelArea.querySelector<HTMLElement>(
        '.storage-size');
    assertTrue(!!inUseArea);
    assertEquals('2.5 GB', inUseArea.innerText);

    availableArea =
        storageSubpage.$.availableLabelArea.querySelector<HTMLElement>(
            '.storage-size');
    assertTrue(!!availableArea);
    assertEquals('7.5 GB', availableArea.innerText);
  });

  test('system size', async () => {
    await createStorageSubpage();

    const systemSizeLabel =
        storageSubpage.shadowRoot!.querySelector<HTMLElement>(
            '#systemSizeLabel');
    assertTrue(!!systemSizeLabel);
    assertEquals('System', systemSizeLabel.innerText);

    let systemSizeSubLabel =
        storageSubpage.shadowRoot!.querySelector<HTMLElement>(
            '#systemSizeSubLabel');
    assertTrue(!!systemSizeSubLabel);
    assertEquals('Calculating…', systemSizeSubLabel.innerText);

    // Send system size callback.
    webUIListenerCallback('storage-system-size-changed', '8.4 GB');
    flush();
    systemSizeSubLabel = storageSubpage.shadowRoot!.querySelector<HTMLElement>(
        '#systemSizeSubLabel');
    assertTrue(!!systemSizeSubLabel);
    assertEquals('8.4 GB', systemSizeSubLabel.innerText);

    // In guest mode, the system row should be hidden.
    storageSubpage.set('isEphemeralUser_', true);
    flush();
    assertFalse(
        isVisible(storageSubpage.shadowRoot!.querySelector('#systemSize')));
  });

  test('apps extensions size', async () => {
    await createStorageSubpage();

    const expectedLabel =
        isRevampWayfindingEnabled ? 'Apps' : 'Apps and extensions';
    assertEquals(expectedLabel, getStorageItemLabelFromId('appsSize'));
    assertEquals('Calculating…', getStorageItemSubLabelFromId('appsSize'));

    // Send apps size callback.
    webUIListenerCallback('storage-apps-size-changed', '59.5 KB');
    flush();
    assertEquals('59.5 KB', getStorageItemSubLabelFromId('appsSize'));
  });

  test('other users size', async () => {
    await createStorageSubpage();

    // The other users row is visible by default, displaying
    // "calculating...".
    assertTrue(
        isVisible(storageSubpage.shadowRoot!.querySelector('#otherUsersSize')));
    assertEquals('Other users', getStorageItemLabelFromId('otherUsersSize'));
    assertEquals(
        'Calculating…', getStorageItemSubLabelFromId('otherUsersSize'));

    // Simulate absence of other users.
    webUIListenerCallback('storage-other-users-size-changed', '0 B', true);
    flush();
    assertFalse(
        isVisible(storageSubpage.shadowRoot!.querySelector('#otherUsersSize')));

    // Send other users callback with a size that is not null.
    webUIListenerCallback('storage-other-users-size-changed', '322 MB', false);
    flush();
    assertTrue(
        isVisible(storageSubpage.shadowRoot!.querySelector('#otherUsersSize')));
    assertEquals('322 MB', getStorageItemSubLabelFromId('otherUsersSize'));

    // If the user is in Guest mode, the row is not visible.
    storageSubpage.set('isEphemeralUser_', true);
    webUIListenerCallback('storage-other-users-size-changed', '322 MB', false);
    flush();
    assertFalse(
        isVisible(storageSubpage.shadowRoot!.querySelector('#otherUsersSize')));
  });

  test('drive offline size', async () => {
    interface Params {
      isDriveEnabled: boolean;
      isVisible: boolean;
    }

    async function assertDriveOfflineSizeVisibility(params: Params):
        Promise<void> {
      await createStorageSubpage();
      storageSubpage.set('prefs.gdata.disabled.value', !params.isDriveEnabled);
      await flushTasks();

      const expectedState = params.isVisible ? 'be visible' : 'not be visible';
      assertEquals(
          params.isVisible,
          isVisible(
              storageSubpage.shadowRoot!.querySelector('#driveOfflineSize')),
          `Expected #driveOfflineSize to ${expectedState} with params: ${
              JSON.stringify(params)}`);
    }

    await assertDriveOfflineSizeVisibility({
      isDriveEnabled: false,
      isVisible: false,
    });

    await assertDriveOfflineSizeVisibility({
      isDriveEnabled: true,
      isVisible: true,
    });
  });
});
