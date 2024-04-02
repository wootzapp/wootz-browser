// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tab_resumption;

import android.graphics.Bitmap;

import org.chromium.chrome.browser.recent_tabs.ForeignSessionHelper.ForeignSession;
import org.chromium.chrome.browser.recent_tabs.ForeignSessionHelper.ForeignSessionTab;
import org.chromium.chrome.browser.recent_tabs.ForeignSessionHelper.ForeignSessionWindow;
import org.chromium.components.sync_device_info.FormFactor;
import org.chromium.url.JUnitTestGURLs;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class TestSupport {
    static final long BASE_TIME_MS = 1705000000000L; // 2024-01-11, 14:06:40 EST.

    // Injected "current time" for each test, set at 1 day after `BASE_TIME_MS`.
    static final long CURRENT_TIME_MS = makeTimestamp(24, 0, 0);

    static final ForeignSessionTab TAB1 =
            new ForeignSessionTab(
                    JUnitTestGURLs.BLUE_1,
                    "Blue 1",
                    makeTimestamp(3, 0, 0),
                    makeTimestamp(3, 0, 0),
                    101);
    // This one is stale.
    static final ForeignSessionTab TAB2 =
            new ForeignSessionTab(
                    JUnitTestGURLs.GOOGLE_URL_DOG,
                    "Google Dog",
                    makeTimestamp(0, 0, -1),
                    makeTimestamp(0, 0, -1),
                    102);
    static final ForeignSessionTab TAB3 =
            new ForeignSessionTab(
                    JUnitTestGURLs.CHROME_ABOUT,
                    "About",
                    makeTimestamp(0, 0, -1), // timestamp != lastUpdatedTime.
                    makeTimestamp(7, 0, 0),
                    103);
    static final ForeignSessionTab TAB4 =
            new ForeignSessionTab(
                    JUnitTestGURLs.URL_1,
                    "One",
                    makeTimestamp(0, 30, 0),
                    makeTimestamp(0, 30, 0),
                    104);
    static final ForeignSessionTab TAB5 =
            new ForeignSessionTab(
                    JUnitTestGURLs.MAPS_URL,
                    "Maps",
                    makeTimestamp(4, 0, 0),
                    makeTimestamp(4, 0, 0),
                    105);
    // This one is the most recent.
    static final ForeignSessionTab TAB6 =
            new ForeignSessionTab(
                    JUnitTestGURLs.INITIAL_URL,
                    "Initial",
                    makeTimestamp(2, 0, 0), // timestamp != lastUpdatedTime.
                    makeTimestamp(8, 0, 0),
                    106);
    static final ForeignSessionTab TAB7 =
            new ForeignSessionTab(
                    JUnitTestGURLs.HTTP_URL,
                    "Old HTTP",
                    makeTimestamp(3, 0, 0),
                    makeTimestamp(3, 0, 0),
                    107);

    /** Makes a test bitmap with specified dimensions. */
    static Bitmap makeBitmap(int width, int height) {
        return Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
    }

    /**
     * Makes a test timestamp in ms since the epoch relative to `BASE_TIME_MS`.
     *
     * @param hours Relative hours, may be negative.
     * @param minutes Relative minutes, may be negative.
     * @param seconds Relative seconds, may be negative.
     */
    static long makeTimestamp(int hours, int minutes, int seconds) {
        return BASE_TIME_MS + ((hours * 60L + minutes) * 60L + seconds) * 1000L;
    }

    /** Makes a list of fixed ForeignSession test data. */
    static List<ForeignSession> makeForeignSessionsA() {
        ForeignSessionWindow desktopWindow1 =
                new ForeignSessionWindow(
                        /* timestamp= */ makeTimestamp(3, 30, 0),
                        /* sessionId= */ 201,
                        /* tabs= */ new ArrayList<>(Arrays.asList(TAB1, TAB2)));
        ForeignSessionWindow desktopWindow2 =
                new ForeignSessionWindow(
                        /* timestamp= */ makeTimestamp(9, 15, 20),
                        /* sessionId= */ 202,
                        /* tabs= */ new ArrayList<>(Arrays.asList(TAB3, TAB4)));
        ForeignSession desktopForeignSession =
                new ForeignSession(
                        /* tag= */ "TagForDesktop",
                        /* name= */ "My Desktop",
                        /* modifiedTime= */ makeTimestamp(10, 0, 0),
                        /* windows= */ new ArrayList<>(
                                Arrays.asList(desktopWindow1, desktopWindow2)),
                        /* formFactor= */ FormFactor.DESKTOP);

        ForeignSessionWindow tabletWindow1 =
                new ForeignSessionWindow(
                        /* timestamp= */ makeTimestamp(8, 1, 15),
                        /* sessionId= */ 301,
                        /* tabs= */ new ArrayList<>(Arrays.asList(TAB5, TAB6, TAB7)));
        ForeignSession tabletForeignSession =
                new ForeignSession(
                        /* tag= */ "TagForTablet",
                        /* name= */ "My Tablet",
                        /* modifiedTime= */ makeTimestamp(8, 5, 20),
                        /* windows= */ new ArrayList<>(Arrays.asList(tabletWindow1)),
                        /* formFactor= */ FormFactor.TABLET);

        return new ArrayList<>(Arrays.asList(desktopForeignSession, tabletForeignSession));
    }

    /** Makes a shorter, altered list of fixed ForeignSession test data. */
    static List<ForeignSession> makeForeignSessionsB() {
        ForeignSessionWindow tabletWindow1 =
                new ForeignSessionWindow(
                        /* timestamp= */ makeTimestamp(9, 1, 15),
                        /* sessionId= */ 301,
                        /* tabs= */ new ArrayList<>(Arrays.asList(TAB5, TAB7)));
        ForeignSession tabletForeignSession =
                new ForeignSession(
                        /* tag= */ "TagForTablet",
                        /* name= */ "My Tablet",
                        /* modifiedTime= */ makeTimestamp(9, 5, 20),
                        /* windows= */ new ArrayList<>(Arrays.asList(tabletWindow1)),
                        /* formFactor= */ FormFactor.TABLET);

        return new ArrayList<>(Arrays.asList(tabletForeignSession));
    }
}
