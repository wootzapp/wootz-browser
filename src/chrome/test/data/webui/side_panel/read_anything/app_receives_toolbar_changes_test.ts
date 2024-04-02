// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome-untrusted://read-anything-side-panel.top-chrome/read_anything_toolbar.js';

import {BrowserProxy} from '//resources/cr_components/color_change_listener/browser_proxy.js';
import type {ReadAnythingElement} from 'chrome-untrusted://read-anything-side-panel.top-chrome/app.js';
import {defaultFontName} from 'chrome-untrusted://read-anything-side-panel.top-chrome/common.js';
import {FONT_EVENT, FONT_SIZE_EVENT, HIGHLIGHT_TOGGLE_EVENT, NEXT_GRANULARITY_EVENT, PLAY_PAUSE_EVENT, PREVIOUS_GRANULARITY_EVENT, RATE_EVENT} from 'chrome-untrusted://read-anything-side-panel.top-chrome/read_anything_toolbar.js';
import {assertEquals, assertFalse, assertNotEquals, assertTrue} from 'chrome-untrusted://webui-test/chai_assert.js';

import {emitEvent, suppressInnocuousErrors} from './common.js';
import {FakeReadingMode} from './fake_reading_mode.js';
import {TestColorUpdaterBrowserProxy} from './test_color_updater_browser_proxy.js';

suite('AppReceivesToolbarChanges', () => {
  let testBrowserProxy: TestColorUpdaterBrowserProxy;
  let app: ReadAnythingElement;

  setup(() => {
    suppressInnocuousErrors();
    testBrowserProxy = new TestColorUpdaterBrowserProxy();
    BrowserProxy.setInstance(testBrowserProxy);
    document.body.innerHTML = window.trustedTypes!.emptyHTML;
    const readingMode = new FakeReadingMode();
    chrome.readingMode = readingMode as unknown as typeof chrome.readingMode;
    app = document.createElement('read-anything-app');
    document.body.appendChild(app);
  });

  suite('on font size change', () => {
    function containerFontSize(): number {
      return +window.getComputedStyle(app.$.container)
                  .getPropertyValue('--font-size')
                  .replace('em', '');
    }

    function emitFontSize(): void {
      emitEvent(app, FONT_SIZE_EVENT);
    }

    test('container font size updated', () => {
      const fontSize1 = 12;
      chrome.readingMode.fontSize = fontSize1;
      emitFontSize();
      assertEquals(containerFontSize(), fontSize1);

      const fontSize2 = 16;
      chrome.readingMode.fontSize = fontSize2;
      emitFontSize();
      assertEquals(containerFontSize(), fontSize2);

      const fontSize3 = 9;
      chrome.readingMode.fontSize = fontSize3;
      emitFontSize();
      assertEquals(containerFontSize(), fontSize3);
    });
  });

  suite('on font change', () => {
    function containerFont(): string {
      return window.getComputedStyle(app.$.container)
          .getPropertyValue('font-family');
    }

    function emitFont(fontName: string): void {
      emitEvent(app, FONT_EVENT, {detail: {fontName}});
    }

    function assertFontsEqual(actual: string, expected: string): void {
      assertEquals(
          actual.trim().toLowerCase().replaceAll('"', ''),
          expected.trim().toLowerCase().replaceAll('"', ''));
    }

    test('valid font updates container font', () => {
      const font1 = 'Andika';
      emitFont(font1);
      assertFontsEqual(containerFont(), font1);

      const font2 = 'Comic Neue';
      emitFont(font2);
      assertFontsEqual(containerFont(), font2);
    });

    test('invalid font uses default', () => {
      const font1 = 'not a real font';
      emitFont(font1);
      assertFontsEqual(containerFont(), defaultFontName);

      const font2 = 'FakeFont';
      emitFont(font2);
      assertFontsEqual(containerFont(), defaultFontName);
    });

    test('unsupported font uses default', () => {
      const font1 = 'Comic Sans';
      emitFont(font1);
      assertFontsEqual(containerFont(), defaultFontName);

      const font2 = 'Times New Roman';
      emitFont(font2);
      assertFontsEqual(containerFont(), defaultFontName);
    });
  });

  suite('on speech rate change', () => {
    function emitRate(rate: number): void {
      emitEvent(app, RATE_EVENT, {detail: {rate}});
    }

    test('speech rate updated', () => {
      const speechRate1 = 2;
      emitRate(speechRate1);
      assertEquals(app.rate, speechRate1);

      const speechRate2 = 0.5;
      emitRate(speechRate2);
      assertEquals(app.rate, speechRate2);

      const speechRate3 = 4;
      emitRate(speechRate3);
      assertEquals(app.rate, speechRate3);
    });
  });

  suite('play/pause', () => {
    setup(() => {
      app.updateContent();
    });

    function emitPlayPause(): void {
      emitEvent(app, PLAY_PAUSE_EVENT);
    }

    suite('by default', () => {
      test('is paused', () => {
        assertTrue(app.speechPlayingState.paused);
        assertFalse(app.speechPlayingState.speechStarted);
      });
    });

    suite('on first click', () => {
      setup(() => {
        emitPlayPause();
      });

      test('starts speech', () => {
        assertFalse(app.speechPlayingState.paused);
        // TODO: b/323960128 - Since this test browser doesn't have any
        // voices, speechStarted doesn't get set to true. Find a way to add a
        // mock voice to this browser, and test that app.speechStarted is true.
      });
    });

    suite('on second click', () => {
      setup(() => {
        emitPlayPause();
        emitPlayPause();
      });

      test('stops speech', () => {
        assertTrue(app.speechPlayingState.paused);
      });
    });

    suite('on keyboard k pressed', () => {
      let kPress: KeyboardEvent;

      setup(() => {
        kPress = new KeyboardEvent('keydown', {key: 'k'});
      });

      test('first press plays', () => {
        app.$.flexParent!.dispatchEvent(kPress);
        assertFalse(app.speechPlayingState.paused);
      });

      test('second press pauses', () => {
        app.$.flexParent!.dispatchEvent(kPress);
        app.$.flexParent!.dispatchEvent(kPress);
        assertTrue(app.speechPlayingState.paused);
      });
    });
  });

  suite('on highlight toggle', () => {
    function highlightColor(): string {
      return window.getComputedStyle(app.$.container)
          .getPropertyValue('--current-highlight-bg-color');
    }

    function emitHighlight(highlightOn: boolean): void {
      emitEvent(app, HIGHLIGHT_TOGGLE_EVENT, {detail: {highlightOn}});
    }

    setup(() => {
      app.updateThemeFromWebUi('');
      app.updateContent();
      app.playSpeech();
    });

    test('on hide, uses transparent highlight', () => {
      emitHighlight(false);
      assertEquals(highlightColor(), 'transparent');
    });

    test('on show, uses colored highlight', () => {
      emitHighlight(true);
      assertNotEquals(highlightColor(), 'transparent');
    });
  });

  suite('on granularity change', () => {
    setup(() => {
      app.updateContent();
    });

    function emitNextGranularity(): void {
      emitEvent(app, NEXT_GRANULARITY_EVENT);
    }

    function emitPreviousGranularity(): void {
      emitEvent(app, PREVIOUS_GRANULARITY_EVENT);
    }

    suite('next', () => {
      test('propagates change', () => {
        let movedToNext = false;
        chrome.readingMode.movePositionToNextGranularity = () => {
          movedToNext = true;
        };

        emitNextGranularity();

        assertTrue(movedToNext);
      });

      test('highlights text', () => {
        emitNextGranularity();
        const currentHighlight =
            app.$.container.querySelector('.current-read-highlight');
        assertTrue(!!currentHighlight!.textContent);
      });
    });

    suite('previous', () => {
      test('propagates change', () => {
        let movedToPrevious: boolean = false;
        chrome.readingMode.movePositionToPreviousGranularity = () => {
          movedToPrevious = true;
        };

        emitPreviousGranularity();

        assertTrue(movedToPrevious);
      });

      test('highlights text', () => {
        emitPreviousGranularity();
        const currentHighlight =
            app.$.container.querySelector('.current-read-highlight');
        assertTrue(!!currentHighlight!.textContent);
      });
    });
  });
});
