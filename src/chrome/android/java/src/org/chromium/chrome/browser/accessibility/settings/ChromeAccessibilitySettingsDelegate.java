// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.accessibility.settings;

import org.chromium.chrome.browser.preferences.Pref;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.components.browser_ui.accessibility.AccessibilitySettingsDelegate;
import org.chromium.components.user_prefs.UserPrefs;
import org.chromium.content_public.browser.BrowserContextHandle;
import org.chromium.chrome.R;
import android.app.Activity;
import org.chromium.chrome.browser.ui.messages.snackbar.SnackbarManager;
import org.chromium.chrome.browser.ui.messages.snackbar.Snackbar;
import org.chromium.chrome.browser.ApplicationLifetime;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.flags.CromiteNativeUtils;
import org.chromium.chrome.browser.preferences.ChromePreferenceKeys;
import org.chromium.chrome.browser.preferences.ChromeSharedPreferences;


/** The Chrome implementation of AccessibilitySettingsDelegate. */
public class ChromeAccessibilitySettingsDelegate implements AccessibilitySettingsDelegate {
    private static class TextSizeContrastAccessibilityDelegate
            implements IntegerPreferenceDelegate {
        private final BrowserContextHandle mBrowserContextHandle;

        public TextSizeContrastAccessibilityDelegate(BrowserContextHandle mBrowserContextHandle) {
            this.mBrowserContextHandle = mBrowserContextHandle;
        }

        @Override
        public int getValue() {
            return UserPrefs.get(mBrowserContextHandle)
                    .getInteger(Pref.ACCESSIBILITY_TEXT_SIZE_CONTRAST_FACTOR);
        }

        @Override
        public void setValue(int value) {
            UserPrefs.get(mBrowserContextHandle)
                    .setInteger(Pref.ACCESSIBILITY_TEXT_SIZE_CONTRAST_FACTOR, value);
        }
    }
    private SnackbarManager mSnackbarManager;

    public void setSnackbarManager(SnackbarManager snackbarManager) {
        mSnackbarManager = snackbarManager;
    }
    private final Profile mProfile;

    /**
     * Constructs a delegate for the given profile.
     * @param profile The profile associated with the delegate.
     */
    public ChromeAccessibilitySettingsDelegate(Profile profile) {
        mProfile = profile;
    }
    private static class MoveTopToolbarToBottomDelegate implements BooleanPreferenceDelegate {
        @Override
        public boolean isEnabled() {
            return ChromeFeatureList.sMoveTopToolbarToBottom.isEnabled();
        }

        @Override
        public void setEnabled(boolean value) {
            CromiteNativeUtils.setFlagEnabled(ChromeFeatureList.MOVE_TOP_TOOLBAR_TO_BOTTOM,
                    "move-top-toolbar-to-bottom", value);
        }
    }

    private static class DisableToolbarSwipeUpDelegate implements BooleanPreferenceDelegate {
        @Override
        public boolean isEnabled() {
            return ChromeFeatureList.sDisableToolbarSwipeUp.isEnabled();
        }

        @Override
        public void setEnabled(boolean value) {
            CromiteNativeUtils.setFlagEnabled(ChromeFeatureList.DISABLE_TOOLBAR_SWIPE_UP,
                    "disable-toolbar-swipe-up", value);
        }
    }

    @Override
    public BooleanPreferenceDelegate getMoveTopToolbarToBottomDelegate() {
        return new MoveTopToolbarToBottomDelegate();
    }

    @Override
    public BooleanPreferenceDelegate getDisableToolbarSwipeUpDelegate() {
        return new DisableToolbarSwipeUpDelegate();
    }

    @Override
    public void requestRestart(Activity activity) {
        Snackbar mSnackbar = Snackbar.make(activity.getString(R.string.ui_relaunch_notice),
                new SnackbarManager.SnackbarController() {
                        @Override
                        public void onDismissNoAction(Object actionData) { }

                        @Override
                        public void onAction(Object actionData) {
                                ApplicationLifetime.terminate(true);
                        }
                }, Snackbar.TYPE_NOTIFICATION, Snackbar.UMA_UNKNOWN)
                .setSingleLine(false)
                .setAction(activity.getString(R.string.relaunch),
                        /*actionData*/null)
                .setDuration(/*durationMs*/70000);
        if (!mSnackbarManager.isShowing())
            mSnackbarManager.showSnackbar(mSnackbar);
    }
    @Override
    public BrowserContextHandle getBrowserContextHandle() {
        return mProfile;
    }

    @Override
    public IntegerPreferenceDelegate getTextSizeContrastAccessibilityDelegate() {
        return new TextSizeContrastAccessibilityDelegate(getBrowserContextHandle());
    }
}
