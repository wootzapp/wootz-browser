// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://customize-chrome-side-panel.top-chrome/wallpaper_search/wallpaper_search.js';
import 'chrome://customize-chrome-side-panel.top-chrome/strings.m.js';

import {CustomizeChromeAction} from 'chrome://customize-chrome-side-panel.top-chrome/common.js';
import type {CustomizeChromePageRemote} from 'chrome://customize-chrome-side-panel.top-chrome/customize_chrome.mojom-webui.js';
import {CustomizeChromeApiProxy} from 'chrome://customize-chrome-side-panel.top-chrome/customize_chrome_api_proxy.js';
import type {Descriptors, InspirationGroup, ResultDescriptors, WallpaperSearchClientRemote, WallpaperSearchHandlerInterface} from 'chrome://customize-chrome-side-panel.top-chrome/wallpaper_search.mojom-webui.js';
import {DescriptorDName, UserFeedback, WallpaperSearchClientCallbackRouter, WallpaperSearchHandlerRemote, WallpaperSearchStatus} from 'chrome://customize-chrome-side-panel.top-chrome/wallpaper_search.mojom-webui.js';
import type {CustomizeChromeCombobox} from 'chrome://customize-chrome-side-panel.top-chrome/wallpaper_search/combobox/customize_chrome_combobox.js';
import type {WallpaperSearchElement} from 'chrome://customize-chrome-side-panel.top-chrome/wallpaper_search/wallpaper_search.js';
import {DESCRIPTOR_D_VALUE} from 'chrome://customize-chrome-side-panel.top-chrome/wallpaper_search/wallpaper_search.js';
import {WallpaperSearchProxy} from 'chrome://customize-chrome-side-panel.top-chrome/wallpaper_search/wallpaper_search_proxy.js';
import {WindowProxy} from 'chrome://customize-chrome-side-panel.top-chrome/window_proxy.js';
import type {CrAutoImgElement} from 'chrome://resources/cr_elements/cr_auto_img/cr_auto_img.js';
import {CrFeedbackOption} from 'chrome://resources/cr_elements/cr_feedback_buttons/cr_feedback_buttons.js';
import type {CrIconButtonElement} from 'chrome://resources/cr_elements/cr_icon_button/cr_icon_button.js';
import {hexColorToSkColor} from 'chrome://resources/js/color_utils.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {PromiseResolver} from 'chrome://resources/js/promise_resolver.js';
import type {IronCollapseElement} from 'chrome://resources/polymer/v3_0/iron-collapse/iron-collapse.js';
import {assertDeepEquals, assertEquals, assertFalse, assertGE, assertNotEquals, assertTrue} from 'chrome://webui-test/chai_assert.js';
import type {MetricsTracker} from 'chrome://webui-test/metrics_test_support.js';
import {fakeMetricsPrivate} from 'chrome://webui-test/metrics_test_support.js';
import {flushTasks, waitAfterNextRender} from 'chrome://webui-test/polymer_test_util.js';
import type {TestMock} from 'chrome://webui-test/test_mock.js';
import {eventToPromise, isVisible, whenCheck} from 'chrome://webui-test/test_util.js';

import {$$, assertNotStyle, assertStyle, createBackgroundImage, createTheme, installMock} from '../test_support.js';

suite('WallpaperSearchTest', () => {
  let callbackRouterRemote: CustomizeChromePageRemote;
  let handler: TestMock<WallpaperSearchHandlerInterface>;
  let metrics: MetricsTracker;
  let wallpaperSearchCallbackRouterRemote: WallpaperSearchClientRemote;
  let wallpaperSearchElement: WallpaperSearchElement;
  let windowProxy: TestMock<WindowProxy>;

  async function createWallpaperSearchElement(
      descriptors: Descriptors|null = null,
      inspirationGroups: InspirationGroup[]|null =
          null): Promise<WallpaperSearchElement> {
    handler.setResultFor('getDescriptors', Promise.resolve({descriptors}));
    handler.setResultFor(
        'getInspirations', Promise.resolve({inspirationGroups}));
    wallpaperSearchElement =
        document.createElement('customize-chrome-wallpaper-search');
    document.body.appendChild(wallpaperSearchElement);
    return wallpaperSearchElement;
  }

  async function createWallpaperSearchElementWithDescriptors(
      inspirationGroups: InspirationGroup[]|null = null) {
    createWallpaperSearchElement(
        {
          descriptorA: [{category: 'foo', labels: ['bar', 'baz']}],
          descriptorB: [{label: 'foo', imagePath: 'bar.png'}],
          descriptorC: ['foo', 'bar', 'baz'],
        },
        inspirationGroups);
  }

  function updateCrFeedbackButtons(option: CrFeedbackOption) {
    wallpaperSearchElement.$.feedbackButtons.selectedOption = option;
    wallpaperSearchElement.$.feedbackButtons.dispatchEvent(
        new CustomEvent('selected-option-changed', {
          bubbles: true,
          composed: true,
          detail: {value: option},
        }));
  }

  setup(async () => {
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    windowProxy = installMock(WindowProxy);
    windowProxy.setResultFor('onLine', true);
    handler = installMock(
        WallpaperSearchHandlerRemote,
        (mock: WallpaperSearchHandlerRemote) =>
            WallpaperSearchProxy.setInstance(
                mock, new WallpaperSearchClientCallbackRouter()));
    wallpaperSearchCallbackRouterRemote =
        WallpaperSearchProxy.getInstance()
            .callbackRouter.$.bindNewPipeAndPassRemote();
    callbackRouterRemote = CustomizeChromeApiProxy.getInstance()
                               .callbackRouter.$.bindNewPipeAndPassRemote();
    metrics = fakeMetricsPrivate();
  });

  suite('Misc', () => {
    test('wallpaper search element added to side panel', () => {
      createWallpaperSearchElement();
      assertTrue(document.body.contains(wallpaperSearchElement));
    });

    test('clicking back button creates event', async () => {
      createWallpaperSearchElement();
      const eventPromise = eventToPromise('back-click', wallpaperSearchElement);
      wallpaperSearchElement.$.heading.getBackButton().click();
      const event = await eventPromise;
      assertTrue(!!event);
    });

    test('clicking learn more calls handler', async () => {
      createWallpaperSearchElement();
      const learnMoreLink =
          wallpaperSearchElement.shadowRoot!.querySelector<HTMLAnchorElement>(
              '#disclaimer a')!;
      const clickEvent = new Event('click', {cancelable: true});
      learnMoreLink.dispatchEvent(clickEvent);
      await handler.whenCalled('openHelpArticle');
      assertTrue(clickEvent.defaultPrevented);
    });

    test(
        'inspiration card is not shown if inspiration is disabled',
        async () => {
          loadTimeData.overrideValues(
              {wallpaperSearchInspirationCardEnabled: false});

          createWallpaperSearchElement();
          await flushTasks();

          assertEquals(0, handler.getCallCount('getInspirations'));
          assertFalse(!!wallpaperSearchElement.shadowRoot!.querySelector(
              '#inspirationCard'));
        });
  });

  suite('Descriptors', () => {
    test('descriptors are fetched from the backend', () => {
      createWallpaperSearchElement();
      assertEquals(1, handler.getCallCount('getDescriptors'));
    });

    test('descriptor menus populate correctly', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      assertEquals(
          1,
          wallpaperSearchElement.shadowRoot!
              .querySelector<CustomizeChromeCombobox>(
                  '#descriptorComboboxA')!.items.length);
      assertEquals(
          1,
          wallpaperSearchElement.shadowRoot!
              .querySelector<CustomizeChromeCombobox>(
                  '#descriptorComboboxB')!.items.length);
      assertEquals(
          3,
          wallpaperSearchElement.shadowRoot!
              .querySelector<CustomizeChromeCombobox>(
                  '#descriptorComboboxC')!.items.length);
      assertEquals(
          6,
          wallpaperSearchElement.shadowRoot!
              .querySelectorAll('#descriptorMenuD button')
              .length);
    });

    test('check marks one item in descriptorMenuD at a time', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      assertFalse(
          !!$$(wallpaperSearchElement, '#descriptorMenuD button [checked]'));

      $$<HTMLElement>(wallpaperSearchElement, '.default-color')!.click();

      let checkedMarkedColors =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '#descriptorMenuD button [checked]');
      assertEquals(1, checkedMarkedColors.length);
      assertEquals(
          checkedMarkedColors[0],
          $$(wallpaperSearchElement, '.default-color .color-check-mark'));
      assertEquals(checkedMarkedColors[0]!.parentElement!.title, 'Red');
      assertEquals(
          checkedMarkedColors[0]!.parentElement!.getAttribute('aria-current'),
          'true');

      wallpaperSearchElement.$.hueSlider.dispatchEvent(
          new Event('selected-hue-changed'));

      checkedMarkedColors = wallpaperSearchElement.shadowRoot!.querySelectorAll(
          '#descriptorMenuD button [checked]');
      assertEquals(1, checkedMarkedColors.length);
      assertEquals(
          checkedMarkedColors[0],
          $$(wallpaperSearchElement, '#customColorContainer [checked]'));
      assertEquals(
          checkedMarkedColors[0]!.parentElement!.title, 'Custom color');
      assertEquals(
          checkedMarkedColors[0]!.parentElement!.getAttribute('aria-current'),
          'true');
    });

    test('unselects colors', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      assertFalse(
          !!$$(wallpaperSearchElement, '#descriptorMenuD button [checked]'));

      $$<HTMLElement>(wallpaperSearchElement, '.default-color')!.click();
      let checkedMarkedColors =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '#descriptorMenuD button [checked]');
      assertEquals(1, checkedMarkedColors.length);

      // Clicking again should deselect it.
      $$<HTMLElement>(wallpaperSearchElement, '.default-color')!.click();
      checkedMarkedColors = wallpaperSearchElement.shadowRoot!.querySelectorAll(
          '#descriptorMenuD button [checked]');
      assertEquals(0, checkedMarkedColors.length);

      // Verify submitting does not send a color.
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
      wallpaperSearchElement.$.submitButton.click();
      await flushTasks();
      assertEquals(1, handler.getCallCount('getWallpaperSearchResults'));
      assertEquals(null, handler.getArgs('getWallpaperSearchResults')[0].color);
    });

    test('unselects hue', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();
      assertTrue(wallpaperSearchElement.$.deleteSelectedHueButton.hidden);

      // Select a hue and verify delete button becomes visible.
      wallpaperSearchElement.$.hueSlider.selectedHue = 10;
      wallpaperSearchElement.$.hueSlider.dispatchEvent(
          new Event('selected-hue-changed'));
      await flushTasks();
      assertFalse(wallpaperSearchElement.$.deleteSelectedHueButton.hidden);

      // Click on delete button.
      wallpaperSearchElement.$.deleteSelectedHueButton.click();
      await flushTasks();

      // Verify there are no checked colors.
      assertEquals(
          0,
          wallpaperSearchElement.shadowRoot!
              .querySelectorAll('#descriptorMenuD button [checked]')
              .length);

      // Verify submitting does not send a hue.
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
      wallpaperSearchElement.$.submitButton.click();
      await flushTasks();
      assertEquals(1, handler.getCallCount('getWallpaperSearchResults'));
      assertEquals(null, handler.getArgs('getWallpaperSearchResults')[0].color);
    });
  });

  suite('Search', () => {
    test('clicking search invokes backend', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
      wallpaperSearchElement.$.submitButton.click();

      assertEquals(1, handler.getCallCount('getWallpaperSearchResults'));
    });

    test('sends selected descriptor values to backend', async () => {
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve(
              {status: WallpaperSearchStatus.kOk, results: ['123', '456']}));
      createWallpaperSearchElement({
        descriptorA: [{category: 'foo', labels: ['bar', 'baz']}],
        descriptorB: [{label: 'foo', imagePath: 'bar.png'}],
        descriptorC: ['baz'],
      });
      await flushTasks();

      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxA')!.value = 'bar';
      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxB')!.value = 'foo';
      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxC')!.value = 'baz';
      $$<HTMLElement>(
          wallpaperSearchElement, '#descriptorMenuD button')!.click();
      wallpaperSearchElement.$.submitButton.click();

      assertEquals(1, handler.getCallCount('getWallpaperSearchResults'));
      const resultDescriptors: ResultDescriptors =
          handler.getArgs('getWallpaperSearchResults')[0];
      assertEquals('bar', resultDescriptors.subject);
      assertEquals('foo', resultDescriptors.style);
      assertEquals('baz', resultDescriptors.mood);
      const skColor = hexColorToSkColor(DESCRIPTOR_D_VALUE[0]!.hex);
      assertNotEquals(skColor, {value: 0});
      assertDeepEquals({color: skColor}, resultDescriptors.color);
    });

    test('sends hue to backend', async () => {
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve(
              {status: WallpaperSearchStatus.kOk, results: ['123', '456']}));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      $$<HTMLElement>(
          wallpaperSearchElement, '#descriptorMenuD button')!.click();

      wallpaperSearchElement.$.hueSlider.selectedHue = 10;
      wallpaperSearchElement.$.hueSlider.dispatchEvent(
          new Event('selected-hue-changed'));
      wallpaperSearchElement.$.submitButton.click();
      await flushTasks();

      assertEquals(1, handler.getCallCount('getWallpaperSearchResults'));
      assertDeepEquals(
          {hue: 10}, handler.getArgs('getWallpaperSearchResults')[0].color);
    });

    test(
        'selects random descriptor a if user does not select one', async () => {
          handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
            status: WallpaperSearchStatus.kOk,
            results: ['123', '456'],
          }));
          createWallpaperSearchElementWithDescriptors();
          await flushTasks();
          assertEquals(
              undefined, wallpaperSearchElement.$.descriptorComboboxA.value);

          wallpaperSearchElement.$.submitButton.click();
          await flushTasks();
          assertEquals(1, handler.getCallCount('getWallpaperSearchResults'));
          assertNotEquals(
              undefined, wallpaperSearchElement.$.descriptorComboboxA.value);
          assertNotEquals(
              undefined,
              handler.getArgs('getWallpaperSearchResults')[0].subject);
        });

    test('sends one descriptor value to the backend', async () => {
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
      createWallpaperSearchElement({
        descriptorA: [{category: 'foo', labels: ['bar']}],
        descriptorB: [{label: 'foo', imagePath: 'bar.png'}],
        descriptorC: ['baz'],
      });
      await flushTasks();

      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxA')!.value = 'bar';
      await flushTasks();
      wallpaperSearchElement.$.submitButton.click();

      assertEquals(1, handler.getCallCount('getWallpaperSearchResults'));
      const resultDescriptors: ResultDescriptors =
          handler.getArgs('getWallpaperSearchResults')[0];
      assertEquals('bar', resultDescriptors.subject);
      assertEquals(null, resultDescriptors.style);
      assertEquals(null, resultDescriptors.mood);
      assertEquals(null, resultDescriptors.color);
    });

    test('empty result shows no tiles', async () => {
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      assertTrue(!wallpaperSearchElement.shadowRoot!.querySelector('.tile'));
    });

    test('shows mix of filled and empty containers', async () => {
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [
          {image: '123', id: {high: 10, low: 1}},
          {image: '456', id: {high: 8, low: 2}},
        ],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      // There should always be 6 tiles total. Since there are 2 images in the
      // response, there should be 2 result tiles and the remaining 4 should be
      // empty.
      assertEquals(
          wallpaperSearchElement.$.wallpaperSearch.querySelectorAll('.tile')
              .length,
          6);
      assertEquals(
          wallpaperSearchElement.$.wallpaperSearch
              .querySelectorAll('.tile.result')
              .length,
          2);
      assertEquals(
          wallpaperSearchElement.$.wallpaperSearch
              .querySelectorAll('.tile.empty')
              .length,
          4);
    });

    test('handle result click', async () => {
      windowProxy.setResultFor('now', 321);
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      assertFalse(isVisible(wallpaperSearchElement.$.loading));
      wallpaperSearchElement.$.submitButton.click();
      assertTrue(isVisible(wallpaperSearchElement.$.loading));
      await waitAfterNextRender(wallpaperSearchElement);
      assertFalse(isVisible(wallpaperSearchElement.$.loading));

      assertGE(handler.getCallCount('setResultRenderTime'), 1);
      assertDeepEquals(
          [[{high: 10, low: 1}], 321],
          handler.getArgs('setResultRenderTime').at(-1));

      const result =
          $$(wallpaperSearchElement, '#wallpaperSearch .tile.result');
      assertTrue(!!result);
      (result as HTMLElement).click();
      assertEquals(
          1, handler.getCallCount('setBackgroundToWallpaperSearchResult'));
      assertEquals(
          10,
          handler.getArgs('setBackgroundToWallpaperSearchResult')[0][0].high);
      assertEquals(
          1, handler.getArgs('setBackgroundToWallpaperSearchResult')[0][0].low);
      assertEquals(
          321, handler.getArgs('setBackgroundToWallpaperSearchResult')[0][1]);
    });

    test('results reset between search results', async () => {
      const exampleResults = {
        results: [{image: '123', id: {high: 10, low: 1}}],
      };
      handler.setResultFor(
          'getWallpaperSearchResults', Promise.resolve(exampleResults));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      // Check that there are tiles.
      let result = $$(wallpaperSearchElement, '#wallpaperSearch .tile.result');
      assertTrue(!!result);

      // Create promise resolver so we have time between request and result to
      // test.
      const newResultsResolver = new PromiseResolver();
      handler.setResultFor(
          'getWallpaperSearchResults', newResultsResolver.promise);

      // Check that the previous tiles disappear after click until promise is
      // resolved, including the empty tiles.
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);
      result =
          $$(wallpaperSearchElement,
             '#wallpaperSearch .tile.result, #wallpaperSearch .tile.empty');
      assertFalse(!!result);
      newResultsResolver.resolve(exampleResults);
      await waitAfterNextRender(wallpaperSearchElement);
      result =
          $$(wallpaperSearchElement,
             '#wallpaperSearch .tile.result, #wallpaperSearch .tile.empty');
      assertTrue(!!result);
    });

    test('sizes loading tiles', async () => {
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      // Force a width on the element for more consistent testing.
      wallpaperSearchElement.style.display = 'block';
      wallpaperSearchElement.style.width = '300px';

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      // Assert that the svg takes the full width of the content area.
      const svg =
          wallpaperSearchElement.$.loading.querySelector<HTMLElement>('svg')!;
      const contentWidth = wallpaperSearchElement.$.wallpaperSearch.offsetWidth;
      assertTrue(contentWidth < 300);
      await whenCheck(
          svg, () => svg.getAttribute('width') === `${contentWidth}`);

      // Assert that loading tiles are sized the same as result tiles.
      const resultTile =
          wallpaperSearchElement.shadowRoot!.querySelector<HTMLElement>(
              '.tile.result')!;
      const rects = wallpaperSearchElement.$.loading.querySelectorAll('rect');
      rects.forEach((rect) => {
        // Offset width/height values are automatically rounded, so round the
        // rect's dimensions. The difference in decimal pixel values is
        // negligible.
        assertEquals(
            resultTile.offsetWidth,
            Math.round(Number(rect.getAttribute('width'))));
        assertEquals(
            resultTile.offsetHeight,
            Math.round(Number(rect.getAttribute('height'))));
      });
    });

    test('current theme is checked', async () => {
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [
          {
            image: '123',
            id: {high: BigInt(10), low: BigInt(1)},
            descriptors: null,
          },
          {
            image: '456',
            id: {high: BigInt(8), low: BigInt(2)},
            descriptors: null,
          },
        ],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      // Set a default theme.
      let theme = createTheme();
      callbackRouterRemote.setTheme(theme);
      await callbackRouterRemote.$.flushForTesting();
      await waitAfterNextRender(wallpaperSearchElement);

      // Populate results.
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      // There should be no checked tiles.
      assertFalse(!!$$(wallpaperSearchElement, '.tile [checked]'));

      // Set theme to the first tile.
      theme = createTheme();
      theme.backgroundImage = createBackgroundImage('');
      theme.backgroundImage.localBackgroundId = {
        high: BigInt(10),
        low: BigInt(1),
      };
      callbackRouterRemote.setTheme(theme);
      await callbackRouterRemote.$.flushForTesting();
      await waitAfterNextRender(wallpaperSearchElement);

      // The first result should be checked and be the only one checked.
      const firstResult = $$(wallpaperSearchElement, '.tile .image-check-mark');
      const checkedResults =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '.tile [checked]');
      assertEquals(checkedResults.length, 1);
      assertEquals(checkedResults[0], firstResult);
      assertEquals(
          checkedResults[0]!.parentElement!.getAttribute('aria-current'),
          'true');
    });

    test('labels results', async () => {
      loadTimeData.overrideValues({
        'wallpaperSearchResultLabel': 'Image $1 of $2',
        'wallpaperSearchResultLabelB': 'Image $1 of $2, $3',
        'wallpaperSearchResultLabelC': 'Image $1 of $2, $3',
        'wallpaperSearchResultLabelBC': 'Image $1 of $2, $3, $4',
      });
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [
          {image: '123', id: {high: 10, low: 1}},
          {image: '123', id: {high: 10, low: 1}},
        ],
      }));
      createWallpaperSearchElement({
        descriptorA: [{category: 'category', labels: ['Label A1', 'Label A2']}],
        descriptorB: [{label: 'Label B', imagePath: 'bar.png'}],
        descriptorC: ['Label C'],
      });
      await flushTasks();

      // Select only descriptor A.
      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxA')!.value = 'Label A1';
      await flushTasks();
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      function getAriaLabelOfTile(index: number): string|null {
        return wallpaperSearchElement.shadowRoot!
            .querySelectorAll('.tile')[index]!.ariaLabel;
      }

      assertEquals('Image 1 of Label A1', getAriaLabelOfTile(0));
      assertEquals('Image 2 of Label A1', getAriaLabelOfTile(1));

      // Select descriptor B.
      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxB')!.value = 'Label B';
      await flushTasks();
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);
      assertEquals('Image 1 of Label A1, Label B', getAriaLabelOfTile(0));
      assertEquals('Image 2 of Label A1, Label B', getAriaLabelOfTile(1));

      // Select descriptor C.
      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxC')!.value = 'Label C';
      await flushTasks();
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);
      assertEquals(
          'Image 1 of Label A1, Label B, Label C', getAriaLabelOfTile(0));
      assertEquals(
          'Image 2 of Label A1, Label B, Label C', getAriaLabelOfTile(1));

      // Recreate element to empty out descriptors. Select options for
      // descriptors A and C only.
      createWallpaperSearchElement({
        descriptorA: [{category: 'category', labels: ['Label A1', 'Label A2']}],
        descriptorB: [{label: 'Label B', imagePath: 'bar.png'}],
        descriptorC: ['Label C'],
      });
      await flushTasks();
      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxA')!.value = 'Label A1';
      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxC')!.value = 'Label C';
      await flushTasks();
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);
      assertEquals('Image 1 of Label A1, Label C', getAriaLabelOfTile(0));
      assertEquals('Image 2 of Label A1, Label C', getAriaLabelOfTile(1));
    });

    test('announces results', async () => {
      loadTimeData.overrideValues({
        'wallpaperSearchLoadingA11yMessage': 'Generating...',
        'wallpaperSearchSuccessA11yMessage': 'Generated $1 images',
      });
      const resultsResolver = new PromiseResolver();
      handler.setResultFor(
          'getWallpaperSearchResults', resultsResolver.promise);
      createWallpaperSearchElement({
        descriptorA: [{category: 'category', labels: ['Label A1', 'Label A2']}],
        descriptorB: [{label: 'Label B', imagePath: 'bar.png'}],
        descriptorC: ['Label C'],
      });
      await flushTasks();

      const loadingEventPromise =
          eventToPromise('cr-a11y-announcer-messages-sent', document.body);
      wallpaperSearchElement.$.submitButton.click();
      const loadingEvent = await loadingEventPromise;
      assertTrue(loadingEvent.detail.messages.includes('Generating...'));

      const successEventPromise =
          eventToPromise('cr-a11y-announcer-messages-sent', document.body);
      resultsResolver.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [
          {image: '123', id: {high: 10, low: 1}},
          {image: '123', id: {high: 10, low: 1}},
        ],
      });
      const successEvent = await successEventPromise;
      assertTrue(successEvent.detail.messages.includes('Generated 2 images'));
    });

    test('shows results from latest search request', async () => {
      windowProxy.setResultFor('now', 321);
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();
      assertFalse(isVisible(wallpaperSearchElement.$.loading));

      const resultsPromise1 = new PromiseResolver();
      handler.setResultFor(
          'getWallpaperSearchResults', resultsPromise1.promise);
      wallpaperSearchElement.$.submitButton.click();
      const resultsPromise2 = new PromiseResolver();
      handler.setResultFor(
          'getWallpaperSearchResults', resultsPromise2.promise);
      wallpaperSearchElement.$.submitButton.click();
      assertTrue(isVisible(wallpaperSearchElement.$.loading));
      resultsPromise1.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 9, low: 1}}],
      });
      await flushTasks();

      assertTrue(isVisible(wallpaperSearchElement.$.loading));
      assertFalse(isVisible($$(wallpaperSearchElement, '#error')!));

      resultsPromise2.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 7, low: 8}}],
      });
      await flushTasks();

      assertFalse(isVisible(wallpaperSearchElement.$.loading));
      assertGE(handler.getCallCount('getWallpaperSearchResults'), 2);
      assertTrue(!!$$(wallpaperSearchElement, '#wallpaperSearch .tile.result'));
      assertGE(handler.getCallCount('setResultRenderTime'), 1);
      assertDeepEquals(
          [[{high: 7, low: 8}], 321],
          handler.getArgs('setResultRenderTime').at(-1));
    });

    test('error status is ignored if there is another request', async () => {
      windowProxy.setResultFor('now', 321);
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();
      assertFalse(isVisible(wallpaperSearchElement.$.loading));

      const resultsPromise1 = new PromiseResolver();
      handler.setResultFor(
          'getWallpaperSearchResults', resultsPromise1.promise);
      wallpaperSearchElement.$.submitButton.click();
      const resultsPromise2 = new PromiseResolver();
      handler.setResultFor(
          'getWallpaperSearchResults', resultsPromise2.promise);
      wallpaperSearchElement.$.submitButton.click();
      assertTrue(isVisible(wallpaperSearchElement.$.loading));
      resultsPromise1.resolve(
          {status: WallpaperSearchStatus.kError, results: []});
      await flushTasks();

      assertTrue(isVisible(wallpaperSearchElement.$.loading));
      assertFalse(isVisible($$(wallpaperSearchElement, '#error')!));

      resultsPromise2.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      });
      await flushTasks();

      assertFalse(isVisible(wallpaperSearchElement.$.loading));
      assertGE(handler.getCallCount('getWallpaperSearchResults'), 2);
      assertTrue(!!$$(wallpaperSearchElement, '#wallpaperSearch .tile.result'));
      assertGE(handler.getCallCount('setResultRenderTime'), 1);
      assertDeepEquals(
          [[{high: 10, low: 1}], 321],
          handler.getArgs('setResultRenderTime').at(-1));
    });

    test('triggers hats survey on success', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      }));
      wallpaperSearchElement.$.submitButton.click();
      await flushTasks();
      assertEquals(1, handler.getCallCount('launchHatsSurvey'));
    });

    test('does not trigger hats survey on error', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kError, results: []}));
      wallpaperSearchElement.$.submitButton.click();
      await flushTasks();
      assertEquals(0, handler.getCallCount('launchHatsSurvey'));
    });
  });

  suite('History', () => {
    test('hide history card if history is empty', async () => {
      createWallpaperSearchElement();

      wallpaperSearchCallbackRouterRemote.setHistory([]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      assertTrue(!!wallpaperSearchElement.$.historyCard.hidden);
    });

    test('show history in history card', async () => {
      createWallpaperSearchElement();

      assertTrue(!!wallpaperSearchElement.$.historyCard.hidden);

      wallpaperSearchCallbackRouterRemote.setHistory([
        {
          image: '123',
          id: {high: BigInt(10), low: BigInt(1)},
          descriptors: null,
        },
        {
          image: '456',
          id: {high: BigInt(8), low: BigInt(2)},
          descriptors: null,
        },
      ]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      const historyTiles =
          wallpaperSearchElement.$.historyCard.querySelectorAll('.tile.result');
      const historyEmptyTiles =
          wallpaperSearchElement.$.historyCard.querySelectorAll('.tile.empty');
      assertFalse(!!wallpaperSearchElement.$.historyCard.hidden);
      assertEquals(historyTiles.length, 2);
      assertEquals(historyEmptyTiles.length, 4);
      assertEquals(
          (historyTiles[0]! as HTMLElement).getAttribute('aria-label'),
          'Recent AI theme 1');
      assertEquals(
          (historyTiles[1]! as HTMLElement).getAttribute('aria-label'),
          'Recent AI theme 2');
    });

    test('set history image on click', async () => {
      createWallpaperSearchElement();

      wallpaperSearchCallbackRouterRemote.setHistory([
        {
          image: '123',
          id: {high: BigInt(10), low: BigInt(1)},
          descriptors: {
            subject: 'foo',
            mood: 'bar',
            style: 'foobar',
            color: null,
          },
        },
        {
          image: '456',
          id: {high: BigInt(8), low: BigInt(2)},
          descriptors: null,
        },
      ]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      const historyTile =
          $$(wallpaperSearchElement, '#historyCard .tile.result');
      assertTrue(!!historyTile);
      (historyTile as HTMLElement).click();

      assertEquals(1, handler.getCallCount('setBackgroundToHistoryImage'));
      const args = handler.getArgs('setBackgroundToHistoryImage');
      assertEquals(BigInt(10), args[0][0].high);
      assertEquals(BigInt(1), args[0][0].low);
      assertEquals('foo', args[0][1].subject);
      assertEquals('bar', args[0][1].mood);
      assertEquals('foobar', args[0][1].style);
    });

    test('current history theme is checked', async () => {
      createWallpaperSearchElement();

      wallpaperSearchCallbackRouterRemote.setHistory([
        {
          image: '123',
          id: {high: BigInt(10), low: BigInt(1)},
          descriptors: null,
        },
        {
          image: '456',
          id: {high: BigInt(8), low: BigInt(2)},
          descriptors: null,
        },
      ]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      // There should be no checked tiles.
      assertFalse(!!$$(wallpaperSearchElement, '.tile [checked]'));

      // Set theme to the first tile.
      const theme = createTheme();
      theme.backgroundImage = createBackgroundImage('');
      theme.backgroundImage.localBackgroundId = {
        high: BigInt(10),
        low: BigInt(1),
      };
      callbackRouterRemote.setTheme(theme);
      await callbackRouterRemote.$.flushForTesting();

      // The first result should be checked and be the only one checked.
      const firstResult = $$(wallpaperSearchElement, '.tile .image-check-mark');
      const checkedResults =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '.tile [checked]');
      assertEquals(checkedResults.length, 1);
      assertEquals(checkedResults[0], firstResult);
      assertEquals(
          checkedResults[0]!.parentElement!.getAttribute('aria-current'),
          'true');
    });

    test('labels history', async () => {
      loadTimeData.overrideValues({
        'wallpaperSearchHistoryResultLabelNoDescriptor': 'Image $1',
        'wallpaperSearchHistoryResultLabel': 'Image $1 of $2',
        'wallpaperSearchHistoryResultLabelB': 'Image $1 of $2, $3',
        'wallpaperSearchHistoryResultLabelC': 'Image $1 of $2, $3',
        'wallpaperSearchHistoryResultLabelBC': 'Image $1 of $2, $3, $4',
      });
      createWallpaperSearchElement();

      wallpaperSearchCallbackRouterRemote.setHistory([
        {
          image: '123',
          id: {high: BigInt(10), low: BigInt(1)},
          descriptors: null,
        },
        {
          image: '456',
          id: {high: BigInt(8), low: BigInt(2)},
          descriptors: {
            subject: 'foo',
            style: null,
            mood: null,
            color: null,
          },
        },
        {
          image: '789',
          id: {high: BigInt(8), low: BigInt(3)},
          descriptors: {
            subject: 'foo',
            mood: 'bar',
            style: null,
            color: null,
          },
        },
        {
          image: '012',
          id: {high: BigInt(8), low: BigInt(4)},
          descriptors: {
            subject: 'foo',
            style: 'foobar',
            mood: null,
            color: null,
          },
        },
        {
          image: '345',
          id: {high: BigInt(10), low: BigInt(5)},
          descriptors: {
            subject: 'foo',
            mood: 'bar',
            style: 'foobar',
            color: null,
          },
        },
      ]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      const historyTiles =
          wallpaperSearchElement.$.historyCard.querySelectorAll('.tile.result');

      assertEquals(historyTiles.length, 5);
      assertEquals('Image 1', historyTiles[0]!.ariaLabel);
      assertEquals('Image 2 of foo', historyTiles[1]!.ariaLabel);
      assertEquals('Image 3 of foo, bar', historyTiles[2]!.ariaLabel);
      assertEquals('Image 4 of foo, foobar', historyTiles[3]!.ariaLabel);
      assertEquals('Image 5 of foo, foobar, bar', historyTiles[4]!.ariaLabel);
    });
  });

  suite('Error', () => {
    suite('Descriptors', () => {
      test('shows error ui for failed descriptor fetch', async () => {
        createWallpaperSearchElement(/*descriptors=*/ null);
        await flushTasks();

        assertNotStyle(
            $$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      test(
          'clicking generic cta for descriptors creates back-click event',
          async () => {
            loadTimeData.overrideValues(
                {genericErrorDescription: 'generic error'});
            createWallpaperSearchElement();
            await flushTasks();

            assertEquals(1, handler.getCallCount('getDescriptors'));
            assertNotStyle(
                $$(wallpaperSearchElement, '#error')!, 'display', 'none');
            assertEquals(
                $$<HTMLElement>(
                    wallpaperSearchElement, '#errorDescription')!.textContent,
                'generic error');
            assertStyle(
                $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display',
                'none');


            handler.setResultFor('getDescriptors', Promise.resolve({
              status: WallpaperSearchStatus.kOk,
              descriptors: {
                descriptorA: [{category: 'foo', labels: ['bar', 'baz']}],
                descriptorB: [{label: 'foo', imagePath: 'bar.png'}],
                descriptorC: ['foo', 'bar', 'baz'],
              },
            }));
            const eventPromise =
                eventToPromise('back-click', wallpaperSearchElement);
            $$<HTMLElement>(wallpaperSearchElement, '#errorCTA')!.click();
            const event = await eventPromise;

            assertTrue(!!event);
            assertEquals(1, handler.getCallCount('getDescriptors'));
          });

      test('shows history description for generic error', async () => {
        loadTimeData.overrideValues(
            {genericErrorDescriptionWithHistory: 'generic error with history'});
        createWallpaperSearchElement();

        wallpaperSearchCallbackRouterRemote.setHistory([
          {
            image: '123',
            id: {high: BigInt(10), low: BigInt(1)},
            descriptors: null,
          },
          {
            image: '456',
            id: {high: BigInt(8), low: BigInt(2)},
            descriptors: null,
          },
        ]);
        await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

        assertNotStyle(
            $$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertEquals(
            $$<HTMLElement>(
                wallpaperSearchElement, '#errorDescription')!.textContent,
            'generic error with history');
        assertStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      test('shows inspiration description for generic error', async () => {
        loadTimeData.overrideValues({
          wallpaperSearchInspirationCardEnabled: true,
          genericErrorDescriptionWithInspiration:
              'generic error with inspiration',
        });
        createWallpaperSearchElement(
            /*descriptors=*/ null, /*inspirationGroups=*/[
              {
                descriptors: {
                  subject: 'foobar',
                  style: null,
                  mood: null,
                  color: null,
                },
                inspirations: [
                  {
                    id: {high: BigInt(10), low: BigInt(1)},
                    description: 'Description',
                    backgroundUrl: {url: 'https://example.com/foo_1.png'},
                    thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                  },
                ],
              },
            ]);
        await flushTasks();

        assertNotStyle(
            $$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertEquals(
            $$<HTMLElement>(
                wallpaperSearchElement, '#errorDescription')!.textContent,
            'generic error with inspiration');
        assertStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      test(
          'shows inspiration and history description for generic error',
          async () => {
            loadTimeData.overrideValues({
              wallpaperSearchInspirationCardEnabled: true,
              genericErrorDescriptionWithHistoryAndInspiration:
                  'generic error with history and inspiration',
            });
            createWallpaperSearchElement(
                /*descriptors=*/ null, /*inspirationGroups=*/[
                  {
                    descriptors: {
                      subject: 'foobar',
                      style: null,
                      mood: null,
                      color: null,
                    },
                    inspirations: [
                      {
                        id: {high: BigInt(10), low: BigInt(1)},
                        description: 'Description',
                        backgroundUrl: {url: 'https://example.com/foo_1.png'},
                        thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                      },
                    ],
                  },
                ]);

            wallpaperSearchCallbackRouterRemote.setHistory([
              {
                image: '123',
                id: {high: BigInt(10), low: BigInt(1)},
                descriptors: null,
              },
              {
                image: '456',
                id: {high: BigInt(8), low: BigInt(2)},
                descriptors: null,
              },
            ]);
            await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

            assertNotStyle(
                $$(wallpaperSearchElement, '#error')!, 'display', 'none');
            assertEquals(
                $$<HTMLElement>(
                    wallpaperSearchElement, '#errorDescription')!.textContent,
                'generic error with history and inspiration');
            assertStyle(
                $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display',
                'none');
          });

      test(
          'clicking offline cta for descriptors creates back-click event',
          async () => {
            loadTimeData.overrideValues({offlineDescription: 'offline error'});
            windowProxy.setResultFor('onLine', false);
            createWallpaperSearchElement();
            await flushTasks();

            assertEquals(1, handler.getCallCount('getDescriptors'));
            assertNotStyle(
                $$(wallpaperSearchElement, '#error')!, 'display', 'none');
            assertEquals(
                $$<HTMLElement>(
                    wallpaperSearchElement, '#errorDescription')!.textContent,
                'offline error');
            assertStyle(
                $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display',
                'none');

            const eventPromise =
                eventToPromise('back-click', wallpaperSearchElement);
            $$<HTMLElement>(wallpaperSearchElement, '#errorCTA')!.click();
            const event = await eventPromise;

            assertTrue(!!event);
            assertEquals(1, handler.getCallCount('getDescriptors'));
          });

      test('shows history description for offline error', async () => {
        loadTimeData.overrideValues(
            {offlineDescriptionWithHistory: 'offline error with history'});
        createWallpaperSearchElement();

        windowProxy.setResultFor('onLine', false);
        wallpaperSearchCallbackRouterRemote.setHistory([
          {
            image: '123',
            id: {high: BigInt(10), low: BigInt(1)},
            descriptors: null,
          },
          {
            image: '456',
            id: {high: BigInt(8), low: BigInt(2)},
            descriptors: null,
          },
        ]);
        await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

        assertNotStyle(
            $$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertEquals(
            $$<HTMLElement>(
                wallpaperSearchElement, '#errorDescription')!.textContent,
            'offline error with history');
      });
    });

    suite('Search', () => {
      test('shows search ui if there are no errors', async () => {
        handler.setResultFor(
            'getWallpaperSearchResults',
            Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
        createWallpaperSearchElementWithDescriptors();
        await flushTasks();

        assertStyle($$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertNotStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');

        wallpaperSearchElement.$.submitButton.click();
        await waitAfterNextRender(wallpaperSearchElement);

        assertStyle($$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertNotStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      test('shows error ui if browser offline', async () => {
        loadTimeData.overrideValues({offlineDescription: 'offline error'});
        windowProxy.setResultFor('onLine', false);
        createWallpaperSearchElementWithDescriptors();
        await flushTasks();

        wallpaperSearchElement.$.submitButton.click();
        await waitAfterNextRender(wallpaperSearchElement);

        assertEquals(1, windowProxy.getCallCount('onLine'));
        assertNotStyle(
            $$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertEquals(
            $$<HTMLElement>(
                wallpaperSearchElement, '#errorDescription')!.textContent,
            'offline error');
        assertStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      test('checks if browser is back online', async () => {
        windowProxy.setResultFor('onLine', false);
        createWallpaperSearchElementWithDescriptors();
        await flushTasks();

        wallpaperSearchElement.$.submitButton.click();
        await waitAfterNextRender(wallpaperSearchElement);

        assertEquals(1, windowProxy.getCallCount('onLine'));
        windowProxy.setResultFor('onLine', true);

        $$<HTMLElement>(wallpaperSearchElement, '#errorCTA')!.click();
        await waitAfterNextRender(wallpaperSearchElement);

        assertEquals(1, windowProxy.getCallCount('onLine'));
        assertStyle($$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertNotStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      [[WallpaperSearchStatus.kError, 'generic error'],
       [WallpaperSearchStatus.kRequestThrottled, 'throttle error'],
       [WallpaperSearchStatus.kSignedOut, 'signed out error'],
      ].forEach(([status, description]) => {
        test(`shows correct error for status ${status}`, async () => {
          loadTimeData.overrideValues({
            genericErrorDescription: 'generic error',
            requestThrottledDescription: 'throttle error',
            signedOutDescription: 'signed out error',
          });
          handler.setResultFor(
              'getWallpaperSearchResults',
              Promise.resolve({status: status, results: []}));
          createWallpaperSearchElementWithDescriptors();
          await flushTasks();

          wallpaperSearchElement.$.submitButton.click();
          await waitAfterNextRender(wallpaperSearchElement);

          assertNotStyle(
              $$(wallpaperSearchElement, '#error')!, 'display', 'none');
          assertEquals(
              $$<HTMLElement>(
                  wallpaperSearchElement, '#errorDescription')!.textContent,
              description);
          assertStyle(
              $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display',
              'none');
        });
      });

      test(`shows generic error if there is history`, async () => {
        loadTimeData.overrideValues(
            {genericErrorDescriptionWithHistory: 'generic error with history'});
        handler.setResultFor(
            'getWallpaperSearchResults',
            Promise.resolve(
                {status: WallpaperSearchStatus.kError, results: []}));
        createWallpaperSearchElementWithDescriptors();
        await flushTasks();

        wallpaperSearchCallbackRouterRemote.setHistory([
          {
            image: '123',
            id: {high: BigInt(10), low: BigInt(1)},
            descriptors: null,
          },
          {
            image: '456',
            id: {high: BigInt(8), low: BigInt(2)},
            descriptors: null,
          },
        ]);
        await wallpaperSearchCallbackRouterRemote.$.flushForTesting();
        wallpaperSearchElement.$.submitButton.click();
        await waitAfterNextRender(wallpaperSearchElement);

        assertNotStyle(
            $$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertEquals(
            $$<HTMLElement>(
                wallpaperSearchElement, '#errorDescription')!.textContent,
            'generic error with history');
        assertStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      test(`shows generic error if there is inspiration`, async () => {
        loadTimeData.overrideValues({
          wallpaperSearchInspirationCardEnabled: true,
          genericErrorDescriptionWithInspiration:
              'generic error with inspiration',
        });
        handler.setResultFor(
            'getWallpaperSearchResults',
            Promise.resolve(
                {status: WallpaperSearchStatus.kError, results: []}));
        createWallpaperSearchElementWithDescriptors([{
          descriptors: {
            subject: 'foobar',
            style: null,
            mood: null,
            color: null,
          },
          inspirations: [
            {
              id: {high: BigInt(10), low: BigInt(1)},
              description: 'Description',
              backgroundUrl: {url: 'https://example.com/foo_1.png'},
              thumbnailUrl: {url: 'https://example.com/foo_2.png'},
            },
          ],
        }]);
        await flushTasks();

        wallpaperSearchElement.$.submitButton.click();
        await waitAfterNextRender(wallpaperSearchElement);

        assertNotStyle(
            $$(wallpaperSearchElement, '#error')!, 'display', 'none');
        assertEquals(
            $$<HTMLElement>(
                wallpaperSearchElement, '#errorDescription')!.textContent,
            'generic error with inspiration');
        assertStyle(
            $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display', 'none');
      });

      test(
          `shows generic error if there is history and inspiration`,
          async () => {
            loadTimeData.overrideValues({
              wallpaperSearchInspirationCardEnabled: true,
              genericErrorDescriptionWithHistoryAndInspiration:
                  'generic error with history and inspiration',
            });
            handler.setResultFor(
                'getWallpaperSearchResults',
                Promise.resolve(
                    {status: WallpaperSearchStatus.kError, results: []}));
            createWallpaperSearchElementWithDescriptors([{
              descriptors: {
                subject: 'foobar',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            }]);
            await flushTasks();

            wallpaperSearchCallbackRouterRemote.setHistory([
              {
                image: '123',
                id: {high: BigInt(10), low: BigInt(1)},
                descriptors: null,
              },
              {
                image: '456',
                id: {high: BigInt(8), low: BigInt(2)},
                descriptors: null,
              },
            ]);
            await wallpaperSearchCallbackRouterRemote.$.flushForTesting();
            wallpaperSearchElement.$.submitButton.click();
            await waitAfterNextRender(wallpaperSearchElement);

            assertNotStyle(
                $$(wallpaperSearchElement, '#error')!, 'display', 'none');
            assertEquals(
                $$<HTMLElement>(
                    wallpaperSearchElement, '#errorDescription')!.textContent,
                'generic error with history and inspiration');
            assertStyle(
                $$(wallpaperSearchElement, '#wallpaperSearch')!, 'display',
                'none');
          });
    });

    test('refocuses on search ui after error is resolved', async () => {
      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kError, results: []}));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      assertEquals(
          wallpaperSearchElement.$.wallpaperSearch,
          wallpaperSearchElement.shadowRoot!.activeElement);

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      assertEquals(
          wallpaperSearchElement.$.error,
          wallpaperSearchElement.shadowRoot!.activeElement);
      $$<HTMLElement>(wallpaperSearchElement, '#errorCTA')!.click();

      assertEquals(
          wallpaperSearchElement.$.wallpaperSearch,
          wallpaperSearchElement.shadowRoot!.activeElement);

      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      assertEquals(
          wallpaperSearchElement.$.wallpaperSearch,
          wallpaperSearchElement.shadowRoot!.activeElement);
    });
  });

  suite('Feedback', () => {
    test('shows feedback buttons and submits', async () => {
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();
      assertFalse(isVisible(wallpaperSearchElement.$.feedbackButtons));

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);
      assertTrue(isVisible(wallpaperSearchElement.$.feedbackButtons));

      // Mock interacting with the feedback buttons.
      updateCrFeedbackButtons(CrFeedbackOption.THUMBS_DOWN);
      let feedbackArgs = await handler.whenCalled('setUserFeedback');
      assertEquals(UserFeedback.kThumbsDown, feedbackArgs);
      handler.resetResolver('setUserFeedback');

      updateCrFeedbackButtons(CrFeedbackOption.THUMBS_UP);
      feedbackArgs = await handler.whenCalled('setUserFeedback');
      assertEquals(UserFeedback.kThumbsUp, feedbackArgs);
      handler.resetResolver('setUserFeedback');

      updateCrFeedbackButtons(CrFeedbackOption.UNSPECIFIED);
      feedbackArgs = await handler.whenCalled('setUserFeedback');
      assertEquals(UserFeedback.kUnspecified, feedbackArgs);
    });

    test('resets on new results', async () => {
      // First result.
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      updateCrFeedbackButtons(CrFeedbackOption.THUMBS_UP);
      await handler.whenCalled('setUserFeedback');
      handler.resetResolver('setUserFeedback');

      // New results.
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '321', id: {high: 10, low: 1}}],
      }));
      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      // Verify feedback option was reset, but this shouldn't call the back-end.
      assertEquals(
          CrFeedbackOption.UNSPECIFIED,
          wallpaperSearchElement.$.feedbackButtons.selectedOption);
      assertEquals(0, handler.getCallCount('setUserFeedback'));
    });
  });

  suite('Metrics', () => {
    test('clicking submit sets metric', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      handler.setResultFor(
          'getWallpaperSearchResults',
          Promise.resolve({status: WallpaperSearchStatus.kOk, results: []}));
      wallpaperSearchElement.$.submitButton.click();

      assertEquals(
          1, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_PROMPT_SUBMITTED));
    });

    test('clicking result tile sets metric', async () => {
      windowProxy.setResultFor('now', 321);
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      const result =
          $$(wallpaperSearchElement, '#wallpaperSearch .tile.result');
      assertTrue(!!result);
      (result as HTMLElement).click();
      assertEquals(
          2, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_RESULT_IMAGE_SELECTED));
    });

    test('clicking history tile sets metric', async () => {
      createWallpaperSearchElement();

      wallpaperSearchCallbackRouterRemote.setHistory([
        {
          image: '123',
          id: {high: BigInt(10), low: BigInt(1)},
          descriptors: null,
        },
        {
          image: '456',
          id: {high: BigInt(8), low: BigInt(2)},
          descriptors: null,
        },
      ]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      const historyTile =
          $$(wallpaperSearchElement, '#historyCard .tile.result');
      assertTrue(!!historyTile);
      (historyTile as HTMLElement).click();
      assertEquals(
          1, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_HISTORY_IMAGE_SELECTED));
    });

    test('clicking feedback buttons sets metric', async () => {
      handler.setResultFor('getWallpaperSearchResults', Promise.resolve({
        status: WallpaperSearchStatus.kOk,
        results: [{image: '123', id: {high: 10, low: 1}}],
      }));
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      // Set metric on thumbs down.
      updateCrFeedbackButtons(CrFeedbackOption.THUMBS_DOWN);
      assertEquals(
          2, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_THUMBS_DOWN_SELECTED));

      // Set metric on thumbs up.
      updateCrFeedbackButtons(CrFeedbackOption.THUMBS_UP);
      assertEquals(
          3, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_THUMBS_UP_SELECTED));
    });

    test('changing subject descriptor sets metric', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxA')!.value = 'bar';
      await flushTasks();

      assertEquals(
          1, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction
                  .WALLPAPER_SEARCH_SUBJECT_DESCRIPTOR_UPDATED));
    });

    test('changing style descriptor sets metric', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxB')!.value = 'foo';
      await flushTasks();

      assertEquals(
          1, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_STYLE_DESCRIPTOR_UPDATED));
    });

    test('changing mood descriptor sets metric', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      $$<CustomizeChromeCombobox>(
          wallpaperSearchElement, '#descriptorComboboxC')!.value = 'foo';
      await flushTasks();

      assertEquals(
          1, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_MOOD_DESCRIPTOR_UPDATED));
    });

    test('changing color descriptor sets metric', async () => {
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      // Set a default color.
      $$<HTMLElement>(
          wallpaperSearchElement, '#descriptorMenuD button')!.click();

      // Set a custom color.
      wallpaperSearchElement.$.hueSlider.selectedHue = 10;
      wallpaperSearchElement.$.hueSlider.dispatchEvent(
          new Event('selected-hue-changed'));

      // Should have 2 calls to color being changed.
      assertEquals(
          2, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          2,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction.WALLPAPER_SEARCH_COLOR_DESCRIPTOR_UPDATED));
    });

    [WallpaperSearchStatus.kError,
     WallpaperSearchStatus.kRequestThrottled,
     WallpaperSearchStatus.kOffline,
     WallpaperSearchStatus.kOk,
    ].forEach((status) => {
      test(`status ${status} sets metric`, async () => {
        handler.setResultFor(
            'getWallpaperSearchResults',
            Promise.resolve({status: status, results: []}));
        createWallpaperSearchElementWithDescriptors();
        await flushTasks();

        wallpaperSearchElement.$.submitButton.click();
        await waitAfterNextRender(wallpaperSearchElement);

        assertEquals(2, metrics.count('NewTabPage.WallpaperSearch.Status'));
        assertEquals(
            status === WallpaperSearchStatus.kOk ? 2 : 1,
            metrics.count('NewTabPage.WallpaperSearch.Status', status));
      });
    });

    test('onLine/offLine status sets metric', async () => {
      windowProxy.setResultFor('onLine', false);
      createWallpaperSearchElementWithDescriptors();
      await flushTasks();

      wallpaperSearchElement.$.submitButton.click();
      await waitAfterNextRender(wallpaperSearchElement);

      assertEquals(2, metrics.count('NewTabPage.WallpaperSearch.Status'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.WallpaperSearch.Status',
              WallpaperSearchStatus.kOffline));

      windowProxy.setResultFor('onLine', true);

      $$<HTMLElement>(wallpaperSearchElement, '#errorCTA')!.click();
      await waitAfterNextRender(wallpaperSearchElement);

      assertEquals(3, metrics.count('NewTabPage.WallpaperSearch.Status'));
      assertEquals(
          2,
          metrics.count(
              'NewTabPage.WallpaperSearch.Status', WallpaperSearchStatus.kOk));
    });

    test('clicking inspiration tile sets metric', async () => {
      loadTimeData.overrideValues(
          {wallpaperSearchInspirationCardEnabled: true});

      createWallpaperSearchElement(
          /*descriptors=*/ null, /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'foobar',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();

      const result =
          $$(wallpaperSearchElement, '#inspirationCard .tile.result');
      assertTrue(!!result);
      (result as HTMLElement).click();
      assertEquals(
          1, metrics.count('NewTabPage.CustomizeChromeSidePanelAction'));
      assertEquals(
          1,
          metrics.count(
              'NewTabPage.CustomizeChromeSidePanelAction',
              CustomizeChromeAction
                  .WALLPAPER_SEARCH_INSPIRATION_THEME_SELECTED));
    });
  });

  suite('Inspiration', () => {
    suiteSetup(() => {
      loadTimeData.overrideValues(
          {wallpaperSearchInspirationCardEnabled: true});
    });

    test('inspiration card shows if inspiration is enabled', async () => {
      createWallpaperSearchElement();
      await flushTasks();

      assertTrue(!!wallpaperSearchElement.shadowRoot!.querySelector(
          '#inspirationCard'));
    });

    test('inspirations are fetched from the backend', () => {
      createWallpaperSearchElement();

      assertEquals(1, handler.getCallCount('getInspirations'));
    });

    test('inspirations populate correctly', async () => {
      createWallpaperSearchElement(
          /*descriptors=*/ null, /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'foobar',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description foo',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
                {
                  id: {high: BigInt(8), low: BigInt(2)},
                  description: 'Description bar',
                  backgroundUrl: {url: 'https://example.com/bar_1.png'},
                  thumbnailUrl: {url: 'https://example.com/bar_2.png'},
                },
              ],
            },
            {
              descriptors: {
                subject: 'baz',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(7), low: BigInt(2)},
                  description: 'Description baz',
                  backgroundUrl: {url: 'https://example.com/baz_1.png'},
                  thumbnailUrl: {url: 'https://example.com/baz_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();

      // Ensure inspiration titles are correct.
      const inspirationTitles =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '#inspirationCard .inspiration-title');
      assertTrue(!!inspirationTitles);
      assertEquals(2, inspirationTitles.length);
      assertEquals('foobar', inspirationTitles[0]!.textContent!.trim());
      assertEquals('baz', inspirationTitles[1]!.textContent!.trim());
      // Ensure the correct amount of groups show.
      const inspirationsGroups =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '#inspirationCard cr-grid');
      assertTrue(!!inspirationsGroups);
      assertEquals(2, inspirationsGroups.length);
      // Ensure the correct amount of inspirations show.
      const inspirations = wallpaperSearchElement.shadowRoot!.querySelectorAll(
          '#inspirationCard .tile.result');
      assertTrue(!!inspirations);
      assertEquals(3, inspirations.length);
      // Ensure that inspirations are populated in the correct group with the
      // right image.
      const inspirationGridResults1 =
          inspirationsGroups[0]!.querySelectorAll('.tile.result');
      assertEquals(inspirations[0], inspirationGridResults1[0]);
      assertEquals(
          'https://example.com/foo_2.png',
          (inspirations[0]!.querySelector('img')! as CrAutoImgElement).autoSrc);
      assertEquals('Description foo', inspirations[0]!.ariaLabel);
      assertEquals(inspirations[1], inspirationGridResults1[1]);
      assertEquals(
          'https://example.com/bar_2.png',
          (inspirations[1]!.querySelector('img')! as CrAutoImgElement).autoSrc);
      assertEquals('Description bar', inspirations[1]!.ariaLabel);
      const inspirationGridResults2 =
          inspirationsGroups[1]!.querySelectorAll('.tile.result');
      assertEquals(inspirations[2], inspirationGridResults2[0]);
      assertEquals(
          'https://example.com/baz_2.png',
          (inspirations[2]!.querySelector('img')! as CrAutoImgElement).autoSrc);
      assertEquals('Description baz', inspirations[2]!.ariaLabel);
    });

    test('descriptor titles format properly', async () => {
      createWallpaperSearchElement(
          /*descriptors=*/ null, /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'foo',
                style: 'bar',
                mood: 'baz',
                color: {name: DescriptorDName.kYellow},
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
            {
              descriptors: {
                subject: 'foo',
                style: null,
                mood: 'baz',
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();

      const inspirationTitles =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '#inspirationCard .inspiration-title');
      assertTrue(!!inspirationTitles);
      assertEquals(2, inspirationTitles.length);
      assertEquals(
          'foo, bar, baz, Yellow',
          inspirationTitles[0]!.textContent!.trim(),
      );
      assertEquals('foo, baz', inspirationTitles[1]!.textContent!.trim());
    });

    test('setting inspiration to background calls backend', async () => {
      createWallpaperSearchElement(
          /*descriptors=*/ null, /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'foobar',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();

      const result =
          $$(wallpaperSearchElement, '#inspirationCard .tile.result');
      assertTrue(!!result);
      (result as HTMLElement).click();
      assertEquals(1, handler.getCallCount('setBackgroundToInspirationImage'));
      assertEquals(
          BigInt(10),
          handler.getArgs('setBackgroundToInspirationImage')[0][0].high);
      assertEquals(
          BigInt(1),
          handler.getArgs('setBackgroundToInspirationImage')[0][0].low);
      assertEquals(
          'https://example.com/foo_1.png',
          handler.getArgs('setBackgroundToInspirationImage')[0][1].url);
    });

    test('inspration group titles update selected descriptors', async () => {
      loadTimeData.overrideValues({
        'wallpaperSearchDescriptorsChangedA11yMessage': 'Descriptors updated',
      });
      createWallpaperSearchElement(
          /*descriptors=*/ {
            descriptorA: [{category: 'foo', labels: ['bar', 'baz']}],
            descriptorB: [{label: 'foo', imagePath: 'bar.png'}],
            descriptorC: ['foo', 'bar', 'baz'],
          },
          /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'baz',
                style: 'foo',
                mood: 'bar',
                color: {name: DescriptorDName.kYellow},
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description foo',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
            {
              descriptors: {
                subject: 'bar',
                mood: 'baz',
                style: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description foo',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();

      const groupTitles = wallpaperSearchElement.shadowRoot!.querySelectorAll(
          '.inspiration-title');
      const firstGroupTitle = groupTitles[0];
      const secondGroupTitle = groupTitles[1];
      assertTrue(!!firstGroupTitle);
      assertTrue(!!secondGroupTitle);

      let loadingEventPromise =
          eventToPromise('cr-a11y-announcer-messages-sent', document.body);
      (firstGroupTitle as HTMLElement).click();
      await flushTasks();

      assertEquals(
          'baz',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxA')!.value);
      assertEquals(
          'foo',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxB')!.value);
      assertEquals(
          'bar',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxC')!.value);
      const checkedColor =
          $$(wallpaperSearchElement, '#descriptorMenuD button [checked]');
      assertTrue(!!checkedColor);
      assertEquals('Yellow', checkedColor!.parentElement!.title);
      assertEquals(firstGroupTitle.getAttribute('aria-current'), 'true');
      assertEquals(secondGroupTitle.getAttribute('aria-current'), 'false');
      let loadingEvent = await loadingEventPromise;
      assertTrue(loadingEvent.detail.messages.includes('Descriptors updated'));

      loadingEventPromise =
          eventToPromise('cr-a11y-announcer-messages-sent', document.body);
      (secondGroupTitle as HTMLElement)
          .dispatchEvent(new KeyboardEvent('keydown', {key: ' '}));
      await flushTasks();

      assertEquals(
          'bar',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxA')!.value);
      assertEquals(
          null,
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxB')!.value);
      assertEquals(
          'baz',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxC')!.value);
      assertFalse(
          !!$$(wallpaperSearchElement, '#descriptorMenuD button [checked]'));
      assertEquals(firstGroupTitle.getAttribute('aria-current'), 'false');
      assertEquals(secondGroupTitle.getAttribute('aria-current'), 'true');
      loadingEvent = await loadingEventPromise;
      assertTrue(loadingEvent.detail.messages.includes('Descriptors updated'));
    });

    test('inspiration tiles updates selected descriptors', async () => {
      loadTimeData.overrideValues({
        'wallpaperSearchDescriptorsChangedA11yMessage': 'Descriptors updated',
      });
      createWallpaperSearchElement(
          /*descriptors=*/ {
            descriptorA: [{category: 'foo', labels: ['bar', 'baz']}],
            descriptorB: [{label: 'foo', imagePath: 'bar.png'}],
            descriptorC: ['foo', 'bar', 'baz'],
          },
          /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'baz',
                style: 'foo',
                mood: 'bar',
                color: {name: DescriptorDName.kYellow},
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description foo',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
            {
              descriptors: {
                subject: 'bar',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description bar',
                  backgroundUrl: {url: 'https://example.com/bar_1.png'},
                  thumbnailUrl: {url: 'https://example.com/bar_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();
      assertEquals(
          undefined,
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxA')!.value);
      assertEquals(
          undefined,
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxB')!.value);
      assertEquals(
          undefined,
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxC')!.value);
      assertFalse(
          !!$$(wallpaperSearchElement, '#descriptorMenuD button [checked]'));

      let loadingEventPromise =
          eventToPromise('cr-a11y-announcer-messages-sent', document.body);
      const inspirationGroupGrids =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '#inspirationCard cr-grid');
      assertEquals(2, inspirationGroupGrids.length);
      let inspirationTile = inspirationGroupGrids[0]!.querySelector('.tile');
      assertTrue(!!inspirationTile);
      (inspirationTile as HTMLElement).click();
      await flushTasks();

      assertEquals(
          'baz',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxA')!.value);
      assertEquals(
          'foo',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxB')!.value);
      assertEquals(
          'bar',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxC')!.value);
      const checkedColor =
          $$(wallpaperSearchElement, '#descriptorMenuD button [checked]');
      assertTrue(!!checkedColor);
      assertEquals('Yellow', checkedColor!.parentElement!.title);
      let loadingEvent = await loadingEventPromise;
      assertTrue(loadingEvent.detail.messages.includes('Descriptors updated'));

      loadingEventPromise =
          eventToPromise('cr-a11y-announcer-messages-sent', document.body);
      inspirationTile = inspirationGroupGrids[1]!.querySelector('.tile');
      assertTrue(!!inspirationTile);
      (inspirationTile as HTMLElement).click();
      await flushTasks();

      assertEquals(
          'bar',
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxA')!.value);
      assertEquals(
          null,
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxB')!.value);
      assertEquals(
          null,
          $$<CustomizeChromeCombobox>(
              wallpaperSearchElement, '#descriptorComboboxC')!.value);
      assertFalse(
          !!$$(wallpaperSearchElement, '#descriptorMenuD button [checked]'));
      loadingEvent = await loadingEventPromise;
      assertTrue(loadingEvent.detail.messages.includes('Descriptors updated'));
    });

    test('inspiration card toggles on click', async () => {
      createWallpaperSearchElement();
      await flushTasks();

      const ironCollapse =
          $$<IronCollapseElement>(wallpaperSearchElement, 'iron-collapse')!;
      assertFalse(ironCollapse.opened);
      assertEquals(
          'cr-icon expand-carets',
          wallpaperSearchElement.shadowRoot!
              .querySelector('#inspirationToggle div')!.className);
      assertEquals(
          'false',
          $$<IronCollapseElement>(
              wallpaperSearchElement, '#inspirationToggle')!.ariaExpanded);

      $$<CrIconButtonElement>(
          wallpaperSearchElement, '#inspirationToggle')!.click();

      assertTrue(ironCollapse.opened);
      assertEquals(
          'cr-icon collapse-carets',
          wallpaperSearchElement.shadowRoot!
              .querySelector('#inspirationToggle div')!.className);
      assertEquals(
          'true',
          $$<IronCollapseElement>(
              wallpaperSearchElement, '#inspirationToggle')!.ariaExpanded);

      $$<CrIconButtonElement>(wallpaperSearchElement, '#inspirationToggle')!
          .dispatchEvent(new KeyboardEvent('keydown', {key: ' '}));

      assertFalse(ironCollapse.opened);
      assertEquals(
          'cr-icon expand-carets',
          wallpaperSearchElement.shadowRoot!
              .querySelector('#inspirationToggle div')!.className);
      assertEquals(
          'false',
          $$<IronCollapseElement>(
              wallpaperSearchElement, '#inspirationToggle')!.ariaExpanded);
    });

    test('inspiration card collapsible reacts to history updates', async () => {
      createWallpaperSearchElement();
      await flushTasks();

      // Card collapsed when the element is created.
      const ironCollapse =
          $$<IronCollapseElement>(wallpaperSearchElement, 'iron-collapse')!;
      assertFalse(ironCollapse.opened);

      // Card opens if there is no history.
      wallpaperSearchCallbackRouterRemote.setHistory([]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      assertTrue(ironCollapse.opened);

      // Card collapses if there is history.
      wallpaperSearchCallbackRouterRemote.setHistory([
        {
          image: '123',
          id: {high: BigInt(10), low: BigInt(1)},
          descriptors: null,
        },
        {
          image: '456',
          id: {high: BigInt(8), low: BigInt(2)},
          descriptors: null,
        },
      ]);
      await wallpaperSearchCallbackRouterRemote.$.flushForTesting();

      assertTrue(!!$$(wallpaperSearchElement, '#historyCard .tile.result'));
      assertFalse(ironCollapse.opened);
    });

    test('inspiration card hides if inspiration is empty', async () => {
      createWallpaperSearchElement();
      await flushTasks();

      const inspirationCard = $$(wallpaperSearchElement, '#inspirationCard');
      assertTrue(!!inspirationCard);
      assertTrue((inspirationCard as HTMLElement).hidden);
    });

    test('inspiration card shows if inspiration is not empty', async () => {
      createWallpaperSearchElement(
          /*descriptors=*/ null, /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'foobar',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();

      const inspirationCard = $$(wallpaperSearchElement, '#inspirationCard');
      assertTrue(!!inspirationCard);
      assertFalse((inspirationCard as HTMLElement).hidden);
    });

    test('current inspiration theme is checked', async () => {
      createWallpaperSearchElement(
          /*descriptors=*/ null, /*inspirationGroups=*/[
            {
              descriptors: {
                subject: 'foobar',
                style: null,
                mood: null,
                color: null,
              },
              inspirations: [
                {
                  id: {high: BigInt(10), low: BigInt(1)},
                  description: 'Description foo',
                  backgroundUrl: {url: 'https://example.com/foo_1.png'},
                  thumbnailUrl: {url: 'https://example.com/foo_2.png'},
                },
                {
                  id: {high: BigInt(8), low: BigInt(2)},
                  description: 'Description bar',
                  backgroundUrl: {url: 'https://example.com/bar_1.png'},
                  thumbnailUrl: {url: 'https://example.com/bar_2.png'},
                },
              ],
            },
          ]);
      await flushTasks();

      // Set a default theme.
      let theme = createTheme();
      callbackRouterRemote.setTheme(theme);
      await callbackRouterRemote.$.flushForTesting();
      await waitAfterNextRender(wallpaperSearchElement);
      // There should be no checked tiles.
      assertFalse(!!$$(wallpaperSearchElement, '.tile [checked]'));

      // Set theme to the inspiration.
      theme = createTheme();
      theme.backgroundImage = createBackgroundImage('');
      theme.backgroundImage.localBackgroundId = {
        high: BigInt(10),
        low: BigInt(1),
      };
      callbackRouterRemote.setTheme(theme);
      await callbackRouterRemote.$.flushForTesting();
      await waitAfterNextRender(wallpaperSearchElement);

      // The first inspiration should be the only tile checked.
      const firstResult = $$(
          wallpaperSearchElement, '#inspirationCard .tile .image-check-mark');
      const checkedResults =
          wallpaperSearchElement.shadowRoot!.querySelectorAll(
              '.tile [checked]');
      assertEquals(1, checkedResults.length);
      assertEquals(firstResult, checkedResults[0]);
      assertEquals(
          'true',
          checkedResults[0]!.parentElement!.getAttribute('aria-current'));
    });
  });
});
