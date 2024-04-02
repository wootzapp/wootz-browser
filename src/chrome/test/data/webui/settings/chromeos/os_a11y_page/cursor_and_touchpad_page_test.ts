// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://os-settings/lazy_load.js';

import {SettingsCursorAndTouchpadPageElement} from 'chrome://os-settings/lazy_load.js';
import {CrSettingsPrefs, DevicePageBrowserProxyImpl, Router, routes, SettingsDropdownMenuElement, SettingsPrefsElement, SettingsToggleButtonElement} from 'chrome://os-settings/os_settings.js';
import {assert} from 'chrome://resources/js/assert.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {flush} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {assertEquals, assertFalse, assertNotEquals, assertNull, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {waitAfterNextRender, waitBeforeNextRender} from 'chrome://webui-test/polymer_test_util.js';
import {eventToPromise, isVisible} from 'chrome://webui-test/test_util.js';

import {TestDevicePageBrowserProxy} from '../device_page/test_device_page_browser_proxy.js';

const DEFAULT_BLACK_CURSOR_COLOR = 0;
const RED_CURSOR_COLOR = 0xd93025;

/**
 * Possible control types for settings.
 */
enum ControlType {
  DROPDOWN = 'dropdown',
  TOGGLE = 'toggle',
}

suite('<settings-cursor-and-touchpad-page>', () => {
  let page: SettingsCursorAndTouchpadPageElement;
  let deviceBrowserProxy: TestDevicePageBrowserProxy;
  let prefElement: SettingsPrefsElement;

  async function initPage() {
    prefElement = document.createElement('settings-prefs');
    document.body.appendChild(prefElement);

    await CrSettingsPrefs.initialized;
    page = document.createElement('settings-cursor-and-touchpad-page');
    page.prefs = prefElement.prefs;
    document.body.appendChild(page);
    flush();
  }

  setup(() => {
    deviceBrowserProxy = new TestDevicePageBrowserProxy();
    deviceBrowserProxy.hasMouse = true;
    deviceBrowserProxy.hasTouchpad = true;
    deviceBrowserProxy.hasPointingStick = false;
    DevicePageBrowserProxyImpl.setInstanceForTesting(deviceBrowserProxy);

    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    Router.getInstance().navigateTo(routes.A11Y_CURSOR_AND_TOUCHPAD);
  });

  teardown(() => {
    page.remove();
    prefElement.remove();
    Router.getInstance().resetRouteForTesting();
  });

  test('cursor color prefs and dropdown synced', async () => {
    await initPage();

    // Make sure cursor color dropdown is black, matching default pref state.
    const cursorColorDropdown =
        page.shadowRoot!.querySelector<SettingsDropdownMenuElement>(
            '#cursorColorDropdown');
    assert(cursorColorDropdown);
    await waitAfterNextRender(cursorColorDropdown);
    const cursorColorSelectElement =
        cursorColorDropdown.shadowRoot!.querySelector('select');
    assert(cursorColorSelectElement);
    assertEquals(
        String(DEFAULT_BLACK_CURSOR_COLOR), cursorColorSelectElement.value);

    // Turn cursor color to red, and verify pref is also red.
    cursorColorSelectElement.value = String(RED_CURSOR_COLOR);
    cursorColorSelectElement.dispatchEvent(new CustomEvent('change'));
    const cursorColorPref = page.getPref('settings.a11y.cursor_color');
    const cursorColorEnabledPref =
        page.getPref('settings.a11y.cursor_color_enabled');
    assertEquals(RED_CURSOR_COLOR, cursorColorPref.value);
    assertTrue(cursorColorEnabledPref.value);

    // Turn cursor color back to default, and verify pref is also default.
    cursorColorSelectElement.value = String(DEFAULT_BLACK_CURSOR_COLOR);
    cursorColorSelectElement.dispatchEvent(new CustomEvent('change'));
    assertEquals(DEFAULT_BLACK_CURSOR_COLOR, cursorColorPref.value);
    assertFalse(cursorColorEnabledPref.value);
  });

  test(
      'should focus pointerSubpageButton button when returning from Pointers subpage',
      async () => {
        const selector = '#pointerSubpageButton';
        const route = routes.POINTERS;
        await initPage();
        flush();
        const router = Router.getInstance();

        const subpageButton =
            page.shadowRoot!.querySelector<HTMLElement>(selector);
        assert(subpageButton);

        subpageButton.click();
        assertEquals(route, router.currentRoute);
        assertNotEquals(
            subpageButton, page.shadowRoot!.activeElement,
            `${selector} should not be focused`);

        const popStateEventPromise = eventToPromise('popstate', window);
        router.navigateToPreviousRoute();
        await popStateEventPromise;
        await waitAfterNextRender(page);

        assertEquals(routes.A11Y_CURSOR_AND_TOUCHPAD, router.currentRoute);
        assertEquals(
            subpageButton, page.shadowRoot!.activeElement,
            `${selector} should be focused`);
      });

  test('Pointers row only visible if mouse/touchpad present', async () => {
    await initPage();
    const row =
        page.shadowRoot!.querySelector<HTMLElement>('#pointerSubpageButton');
    assert(row);
    assertFalse(row.hidden);

    // Has touchpad, doesn't have mouse ==> not hidden.
    deviceBrowserProxy.hasMouse = false;
    assertFalse(row.hidden);

    // Doesn't have either ==> hidden.
    deviceBrowserProxy.hasTouchpad = false;
    assertTrue(row.hidden);

    // Has mouse, doesn't have touchpad ==> not hidden.
    deviceBrowserProxy.hasMouse = true;
    assertFalse(row.hidden);

    // Has both ==> not hidden.
    deviceBrowserProxy.hasTouchpad = true;
    assertFalse(row.hidden);
  });

  test('tablet mode buttons visible', async () => {
    loadTimeData.overrideValues({
      isKioskModeActive: false,
      showTabletModeShelfNavigationButtonsSettings: true,
    });
    await initPage();
    flush();

    assertTrue(isVisible(page.shadowRoot!.querySelector(
        '#shelfNavigationButtonsEnabledControl')));
  });

  test('toggle tablet mode buttons', async () => {
    loadTimeData.overrideValues({
      isKioskModeActive: false,
      showTabletModeShelfNavigationButtonsSettings: true,
    });
    await initPage();
    flush();

    const navButtonsToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#shelfNavigationButtonsEnabledControl');
    assert(navButtonsToggle);
    assertTrue(isVisible(navButtonsToggle));
    // The default pref value is false.
    assertFalse(navButtonsToggle.checked);

    // Clicking the toggle should update the toggle checked value, and the
    // backing preference.
    navButtonsToggle.click();
    flush();

    assertTrue(navButtonsToggle.checked);
    assertFalse(navButtonsToggle.disabled);
    assertTrue(
        page.prefs.settings.a11y.tablet_mode_shelf_nav_buttons_enabled.value);

    navButtonsToggle.click();
    flush();

    assertFalse(navButtonsToggle.checked);
    assertFalse(navButtonsToggle.disabled);
    assertFalse(
        page.prefs.settings.a11y.tablet_mode_shelf_nav_buttons_enabled.value);
  });

  test('tablet mode buttons toggle disabled with spoken feedback', async () => {
    loadTimeData.overrideValues({
      isKioskModeActive: false,
      showTabletModeShelfNavigationButtonsSettings: true,
    });

    await initPage();
    flush();

    // Enable spoken feedback (ChromeVox).
    page.setPrefValue('settings.accessibility', true);

    const navButtonsToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#shelfNavigationButtonsEnabledControl');
    assert(navButtonsToggle);
    assertTrue(isVisible(navButtonsToggle));

    // If spoken feedback is enabled, the shelf nav buttons toggle should be
    // disabled and checked.
    assertTrue(navButtonsToggle.disabled);
    assertTrue(navButtonsToggle.checked);

    // Clicking the toggle should have no effect.
    navButtonsToggle.click();
    flush();

    assertTrue(navButtonsToggle.disabled);
    assertTrue(navButtonsToggle.checked);
    assertFalse(
        page.prefs.settings.a11y.tablet_mode_shelf_nav_buttons_enabled.value);

    // The toggle should be enabled if the spoken feedback gets disabled.
    page.set('prefs.settings.accessibility.value', false);
    flush();

    assertFalse(!!navButtonsToggle.disabled);
    assertFalse(navButtonsToggle.checked);
    assertFalse(
        page.prefs.settings.a11y.tablet_mode_shelf_nav_buttons_enabled.value);

    // Clicking the toggle should update the backing pref.
    navButtonsToggle.click();
    flush();

    assertFalse(!!navButtonsToggle.disabled);
    assertTrue(navButtonsToggle.checked);
    assertTrue(
        page.prefs.settings.a11y.tablet_mode_shelf_nav_buttons_enabled.value);
  });

  test('some parts are hidden in kiosk mode', async () => {
    loadTimeData.overrideValues({
      isKioskModeActive: true,
      showTabletModeShelfNavigationButtonsSettings: true,
    });
    await initPage();
    // Add mouse and touchpad to show some hidden settings.
    deviceBrowserProxy.hasMouse = true;
    deviceBrowserProxy.hasTouchpad = true;
    flush();

    // Shelf navigation buttons are not shown in kiosk mode, even if
    // showTabletModeShelfNavigationButtonsSettings is true.
    assertFalse(isVisible(page.shadowRoot!.querySelector(
        '#shelfNavigationButtonsEnabledControl')));

    const subpageLinks = page.root!.querySelectorAll('cr-link-row');
    subpageLinks.forEach(subpageLink => assertFalse(isVisible(subpageLink)));
  });

  test('large cursor options appear when large cursor enabled', async () => {
    await initPage();
    const largeCursorToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#largeCursorEnabledControl');
    assert(largeCursorToggle);
    const largeCursorSizeSlider =
        page.shadowRoot!.querySelector('#largeCursorSizeSlider');
    assertFalse(isVisible(largeCursorSizeSlider));
    assertTrue(isVisible(largeCursorToggle));
    assertFalse(largeCursorToggle.checked);
    assertFalse(page.prefs.settings.a11y.large_cursor_enabled.value);
    largeCursorToggle.click();

    await waitBeforeNextRender(page);
    flush();
    assertTrue(largeCursorToggle.checked);
    assertTrue(page.prefs.settings.a11y.large_cursor_enabled.value);
    assertTrue(isVisible(largeCursorSizeSlider));
  });

  const settingsControls = [
    {
      id: 'autoClickToggle',
      prefKey: 'settings.a11y.autoclick',
      defaultValue: false,
      alternateValue: true,
      type: ControlType.TOGGLE,
    },
    {
      id: 'delayBeforeClickDropdown',
      prefKey: 'settings.a11y.autoclick_delay_ms',
      defaultValue: 1000,
      alternateValue: 2000,
      type: ControlType.DROPDOWN,
    },
    {
      id: 'autoClickStabilizePositionToggle',
      prefKey: 'settings.a11y.autoclick_stabilize_position',
      defaultValue: false,
      alternateValue: true,
      type: ControlType.TOGGLE,
    },
    {
      id: 'autoclickMovementThresholdDropdown',
      prefKey: 'settings.a11y.autoclick_movement_threshold',
      defaultValue: 20,
      alternateValue: 5,
      type: ControlType.DROPDOWN,
    },
    {
      id: 'autoClickRevertToLeftClickToggle',
      prefKey: 'settings.a11y.autoclick_revert_to_left_click',
      defaultValue: true,
      alternateValue: false,
      type: ControlType.TOGGLE,
    },
  ];

  settingsControls.forEach(control => {
    const {id, prefKey, defaultValue, alternateValue, type} = control;

    test(`Autoclick ${type} ${id} syncs to Pref: ${prefKey}`, async () => {
      await initPage();

      // Ensure control exists.
      const control = page.shadowRoot!.querySelector<HTMLElement>(`#${id}`);
      assert(control);

      // Ensure pref is set to the default value.
      let pref = page.getPref(prefKey);
      assertEquals(defaultValue, pref.value);

      // Update control to alternate value.
      switch (type) {
        case ControlType.TOGGLE:
          control.click();
          break;
        case ControlType.DROPDOWN:
          await waitAfterNextRender(control);
          const controlElement = control.shadowRoot!.querySelector('select');
          assert(controlElement);
          controlElement.value = String(alternateValue);
          controlElement.dispatchEvent(new CustomEvent('change'));
          break;
      }

      // Ensure pref is set to the alternate value.
      pref = page.getPref(prefKey);
      assertEquals(alternateValue, pref.value);
    });
  });

  test(
      'cursor highlight pref enabled when cursor highlight enabled',
      async () => {
        await initPage();
        const cursorHighlightToggle =
            page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
                '#cursorHighlightToggle');
        assert(cursorHighlightToggle);
        assertTrue(isVisible(cursorHighlightToggle));
        assertFalse(cursorHighlightToggle.checked);
        assertFalse(page.prefs.settings.a11y.cursor_highlight.value);
        cursorHighlightToggle.click();

        await waitBeforeNextRender(page);
        flush();
        assertTrue(cursorHighlightToggle.checked);
        assertTrue(page.prefs.settings.a11y.cursor_highlight.value);
      });

  test(
      'face gaze feature does not show if the feature flag is disabled',
      async () => {
        loadTimeData.overrideValues({
          isAccessibilityFaceGazeEnabled: false,
        });

        await initPage();
        const faceGazeToggle =
            page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
                '#faceGazeToggle');
        assertEquals(null, faceGazeToggle);
      });

  test(
      'face tracking feature shows if the feature flag is enabled',
      async () => {
        loadTimeData.overrideValues({
          isAccessibilityFaceGazeEnabled: true,
        });

        await initPage();
        const faceGazeToggle =
            page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
                '#faceGazeToggle');
        assert(faceGazeToggle);
        assertTrue(isVisible(faceGazeToggle));

        const faceGazeCursorControlButton =
            page.shadowRoot!.querySelector('#faceGazeCursorControlButton');
        assert(faceGazeCursorControlButton);
        assertFalse(isVisible(faceGazeCursorControlButton));

        const faceGazeFacialExpressionsButton =
            page.shadowRoot!.querySelector('#faceGazeFacialExpressionsButton');
        assert(faceGazeFacialExpressionsButton);
        assertFalse(isVisible(faceGazeFacialExpressionsButton));

        assertFalse(faceGazeToggle.checked);
        assertFalse(page.prefs.settings.a11y.face_gaze.enabled.value);
        faceGazeToggle.click();

        await waitBeforeNextRender(page);
        flush();
        assertTrue(faceGazeToggle.checked);
        assertTrue(page.prefs.settings.a11y.face_gaze.enabled.value);
        assertTrue(isVisible(faceGazeCursorControlButton));
        assertTrue(isVisible(faceGazeFacialExpressionsButton));
      });

  test('Mouse keys feature disabled.', async () => {
    await initPage();

    if (loadTimeData.getBoolean('isAccessibilityMouseKeysEnabled')) {
      // Skip if the flag is enabled.
      return;
    }

    // Toggle shouldn't be available if flag is disabled.
    const enableMouseKeysToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#enableMouseKeys');
    assertNull(enableMouseKeysToggle);
  });

  test('Mouse keys: Ctrl to disable', async () => {
    await initPage();

    if (!loadTimeData.getBoolean('isAccessibilityMouseKeysEnabled')) {
      // Skip if the flag isn't enabled.
      return;
    }

    // If the flag is enabled, check that the UI works.
    assertFalse(page.prefs.settings.a11y.mouse_keys.enabled.value);

    const enableMouseKeysToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#enableMouseKeys');
    assert(enableMouseKeysToggle);
    assertTrue(isVisible(enableMouseKeysToggle));

    enableMouseKeysToggle.click();
    await waitBeforeNextRender(page);
    flush();

    assertTrue(page.prefs.settings.a11y.mouse_keys.enabled.value);

    // kAccessibilityMouseKeysShortcutToPauseEnabled
    assertTrue(page.prefs.settings.a11y.mouse_keys.ctrl_to_pause_enabled.value);
    const enableMouseKeysShortcutToPauseEnabledToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#enableMouseKeysShortcutToPause');
    assert(enableMouseKeysShortcutToPauseEnabledToggle);
    assertTrue(isVisible(enableMouseKeysShortcutToPauseEnabledToggle));

    enableMouseKeysShortcutToPauseEnabledToggle.click();
    await waitBeforeNextRender(page);
    flush();

    assertFalse(
        page.prefs.settings.a11y.mouse_keys.ctrl_to_pause_enabled.value);
  });

  test('Moust keys: Disable in text fields', async () => {
    await initPage();

    if (!loadTimeData.getBoolean('isAccessibilityMouseKeysEnabled')) {
      // Skip if the flag isn't enabled.
      return;
    }

    // If the flag is enabled, check that the UI works.
    assertFalse(page.prefs.settings.a11y.mouse_keys.enabled.value);

    const enableMouseKeysToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#enableMouseKeys');
    assert(enableMouseKeysToggle);
    assertTrue(isVisible(enableMouseKeysToggle));

    enableMouseKeysToggle.click();
    await waitBeforeNextRender(page);
    flush();

    assertTrue(page.prefs.settings.a11y.mouse_keys.enabled.value);

    // kAccessibilityMouseKeysDisableInTextFields
    assertTrue(
        page.prefs.settings.a11y.mouse_keys.disable_in_text_fields.value);
    const enableMouseKeysDisableInTextFieldsToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#enableMouseKeysDisableInTextFields');
    assert(enableMouseKeysDisableInTextFieldsToggle);
    assertTrue(isVisible(enableMouseKeysDisableInTextFieldsToggle));

    enableMouseKeysDisableInTextFieldsToggle.click();
    await waitBeforeNextRender(page);
    flush();

    assertFalse(
        page.prefs.settings.a11y.mouse_keys.disable_in_text_fields.value);
  });

  test('Mouse keys: Dominant Hand', async () => {
    await initPage();

    if (!loadTimeData.getBoolean('isAccessibilityMouseKeysEnabled')) {
      // Skip if the flag isn't enabled.
      return;
    }
    // If the flag is enabled, check that the UI works.
    assertFalse(page.prefs.settings.a11y.mouse_keys.enabled.value);

    const enableMouseKeysToggle =
        page.shadowRoot!.querySelector<SettingsToggleButtonElement>(
            '#enableMouseKeys');
    assert(enableMouseKeysToggle);
    assertTrue(isVisible(enableMouseKeysToggle));

    enableMouseKeysToggle.click();
    await waitBeforeNextRender(page);
    flush();

    assertTrue(page.prefs.settings.a11y.mouse_keys.enabled.value);

    // kAccessibilityMouseKeysDominantHand
    // Ensure control exists.
    const control =
        page.shadowRoot!.querySelector<HTMLElement>(`#mouseKeysDominantHand`);
    assert(control);

    // Ensure pref is set to the default value.
    let pref = page.getPref('settings.a11y.mouse_keys.dominant_hand');
    assertEquals(pref.value, 0);

    // Update control to alternate value.
    await waitAfterNextRender(control);
    const controlElement = control.shadowRoot!.querySelector('select');
    assert(controlElement);
    controlElement.value = String(1);
    controlElement.dispatchEvent(new CustomEvent('change'));

    // Ensure pref is set to the alternate value.
    pref = page.getPref('settings.a11y.mouse_keys.dominant_hand');
    assertEquals(pref.value, 1);
  });

});
