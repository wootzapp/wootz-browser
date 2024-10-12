// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.readaloud.player.expanded;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.verify;

import android.app.Activity;

import androidx.appcompat.app.AppCompatActivity;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.Robolectric;
import org.robolectric.annotation.Config;

import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.chrome.browser.readaloud.player.InteractionHandler;
import org.chromium.chrome.browser.readaloud.player.PlayerProperties;
import org.chromium.chrome.browser.readaloud.player.R;
import org.chromium.chrome.browser.readaloud.player.expanded.OptionsMenuSheetContent.Item;
import org.chromium.chrome.modules.readaloud.PlaybackArgs.PlaybackVoice;
import org.chromium.components.browser_ui.bottomsheet.BottomSheetController;
import org.chromium.ui.modelutil.PropertyModel;

import java.util.List;

/** Unit tests for {@link OptionsMenuSheetContent}. */
@RunWith(BaseRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class OptionsMenuSheetContentUnitTest {
    @Mock private BottomSheetController mBottomSheetController;
    @Mock private ExpandedPlayerSheetContent mBottomSheetContent;
    @Mock private PropertyModel mModel;
    @Mock private InteractionHandler mHandler;

    private Activity mActivity;
    private Menu mMenu;
    private OptionsMenuSheetContent mContent;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mActivity = Robolectric.buildActivity(AppCompatActivity.class).setup().get();
        // Need to set theme before inflating layout.
        mActivity.setTheme(R.style.Theme_BrowserUI_DayNight);
        mMenu = (Menu) mActivity.getLayoutInflater().inflate(R.layout.readaloud_menu, null);
        mContent =
                new OptionsMenuSheetContent(
                        mActivity, mBottomSheetContent, mBottomSheetController, mMenu, mModel);
    }

    @Test
    public void testSetup() {
        assertTrue(mMenu.getItem(Item.VOICE) != null);
        assertTrue(mMenu.getItem(Item.HIGHLIGHT) != null);
    }

    @Test
    public void testClickVoiceMenu() {
        doReturn(
                        List.of(
                                new PlaybackVoice(
                                        "en",
                                        "US",
                                        "a",
                                        "Sand",
                                        PlaybackVoice.Pitch.NONE,
                                        PlaybackVoice.Tone.NONE)))
                .when(mModel)
                .get(eq(PlayerProperties.VOICES_LIST));
        doReturn("a").when(mModel).get(eq(PlayerProperties.SELECTED_VOICE_ID));

        assertNull(mContent.getVoiceMenu());

        ((Menu) mContent.getContentView())
                .getItem(OptionsMenuSheetContent.Item.VOICE)
                .getChildAt(0)
                .performClick();
        assertNotNull(mContent.getVoiceMenu());
        verify(mBottomSheetController).hideContent(eq(mContent), eq(false));
        verify(mBottomSheetController).requestShowContent(eq(mContent.getVoiceMenu()), eq(true));
    }

    @Test
    public void testToggleChangeUpdatesHighlighting() {
        mContent.setInteractionHandler(mHandler);

        mMenu.getItem(Item.HIGHLIGHT).setValue(true);
        verify(mHandler).onHighlightingChange(true);

        mMenu.getItem(Item.HIGHLIGHT).setValue(false);
        verify(mHandler).onHighlightingChange(false);
    }

    @Test
    public void testCloseVoiceMenu() {
        // Show the voice menu.
        doReturn(
                        List.of(
                                new PlaybackVoice(
                                        "en",
                                        "US",
                                        "a",
                                        "Sand",
                                        PlaybackVoice.Pitch.NONE,
                                        PlaybackVoice.Tone.NONE)))
                .when(mModel)
                .get(eq(PlayerProperties.VOICES_LIST));
        doReturn("a").when(mModel).get(eq(PlayerProperties.SELECTED_VOICE_ID));

        assertNull(mContent.getVoiceMenu());

        ((Menu) mContent.getContentView())
                .getItem(OptionsMenuSheetContent.Item.VOICE)
                .getChildAt(0)
                .performClick();
        assertNotNull(mContent.getVoiceMenu());

        // Close.
        mContent.setInteractionHandler(mHandler);
        mContent.notifySheetClosed(mContent.getVoiceMenu());

        verify(mHandler).onVoiceMenuClosed();
        // Options menu should show again.
        verify(mBottomSheetController).requestShowContent(eq(mContent), eq(true));
    }
}
