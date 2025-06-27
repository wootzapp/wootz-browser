// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ui.appmenu;

import android.content.Context;
import android.graphics.Rect;
import android.view.View;

import org.chromium.base.Callback;
import org.chromium.base.supplier.Supplier;
import org.chromium.build.annotations.NullMarked;
import org.chromium.chrome.browser.browser_controls.BrowserControlsStateProvider;
import org.chromium.chrome.browser.lifecycle.ActivityLifecycleDispatcher;
import org.chromium.ui.base.WindowAndroid;

import androidx.fragment.app.FragmentManager;
import android.util.Log;

/** A factory for creating an {@link AppMenuCoordinator}. */
@NullMarked
public class AppMenuCoordinatorFactory {
    private static final String TAG = "AppMenuCoordinatorFactory";

    private AppMenuCoordinatorFactory() {
    }

    /**
     * Create a new AppMenuCoordinator.
     *
     * @param context                      The activity context.
     * @param activityLifecycleDispatcher  The {@link ActivityLifecycleDispatcher}
     *                                     for the containing
     *                                     activity.
     * @param buttonDelegate               The {@link MenuButtonDelegate} for the
     *                                     containing activity.
     * @param appMenuDelegate              The {@link AppMenuDelegate} for the
     *                                     containing activity.
     * @param decorView                    The decor {@link View}, e.g. from
     *                                     Window#getDecorView(), for the containing
     *                                     activity.
     * @param hardwareButtonAnchorView     The {@link View} used as an anchor for
     *                                     the menu when it is
     *                                     displayed using a hardware button.
     * @param appRect                      Supplier of the app area in Window that
     *                                     the menu should fit in.
     * @param windowAndroid                The window that will be used to fetch
     *                                     KeyboardVisibilityDelegate
     * @param browserControlsStateProvider a provider that can provide the state of
     *                                     the toolbar
     * @param fragmentManager              The {@link FragmentManager} for the
     *                                     containing activity.
     * @param itemRowHeight                The height of each item row in the menu.
     */
    public static AppMenuCoordinator createAppMenuCoordinator(
            Context context,
            ActivityLifecycleDispatcher activityLifecycleDispatcher,
            MenuButtonDelegate buttonDelegate,
            AppMenuDelegate appMenuDelegate,
            View decorView,
            View hardwareButtonAnchorView,
            Supplier<Rect> appRect,
            WindowAndroid windowAndroid,
            BrowserControlsStateProvider browserControlsStateProvider,
            FragmentManager fragmentManager,
            int itemRowHeight) {
        try{
        AppMenuCoordinator coordinator =  new AppMenuCoordinatorImpl(
                context,
                activityLifecycleDispatcher,
                buttonDelegate,
                appMenuDelegate,
                decorView,
                hardwareButtonAnchorView,
                appRect,
                windowAndroid,
                browserControlsStateProvider,
                fragmentManager,
                itemRowHeight);
        return coordinator;
        } catch (Exception e) {
            Log.e(TAG, "Error creating AppMenuCoordinator", e);
            throw e; // Rethrow the exception to ensure it is handled properly.
        }
    }

    /**
     * @param reporter A means of reporting an exception without crashing.
     */
    public static void setExceptionReporter(Callback<Throwable> reporter) {
        AppMenuCoordinatorImpl.setExceptionReporter(reporter);
    }
}
