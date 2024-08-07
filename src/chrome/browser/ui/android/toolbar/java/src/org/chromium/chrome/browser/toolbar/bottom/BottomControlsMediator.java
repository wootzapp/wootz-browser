// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.toolbar.bottom;

import androidx.annotation.ColorInt;
import androidx.annotation.Nullable;

import org.chromium.base.CallbackController;
import org.chromium.base.supplier.ObservableSupplier;
import org.chromium.base.supplier.Supplier;
import org.chromium.chrome.browser.browser_controls.BrowserControlsSizer;
import org.chromium.chrome.browser.browser_controls.BrowserControlsStateProvider;
import org.chromium.chrome.browser.fullscreen.FullscreenManager;
import org.chromium.chrome.browser.layouts.LayoutStateProvider;
import org.chromium.chrome.browser.layouts.LayoutStateProvider.LayoutStateObserver;
import org.chromium.chrome.browser.layouts.LayoutType;
import org.chromium.chrome.browser.tab.TabObscuringHandler;
import org.chromium.chrome.browser.ui.edge_to_edge.EdgeToEdgeController;
import org.chromium.chrome.browser.ui.edge_to_edge.EdgeToEdgeSupplier.ChangeObserver;
import org.chromium.ui.KeyboardVisibilityDelegate;
import org.chromium.ui.base.WindowAndroid;
import org.chromium.ui.modelutil.PropertyModel;

/**
 * This class is responsible for reacting to events from the outside world, interacting with other
 * coordinators, running most of the business logic associated with the bottom controls component,
 * and updating the model accordingly.
 */
class BottomControlsMediator
        implements BrowserControlsStateProvider.Observer,
                KeyboardVisibilityDelegate.KeyboardVisibilityListener,
                LayoutStateObserver,
                TabObscuringHandler.Observer {
    private static final String TAG = "BotControlsMediator";

    /** The model for the bottom controls component that holds all of its view state. */
    private final PropertyModel mModel;

    /** The fullscreen manager to observe fullscreen events. */
    private final FullscreenManager mFullscreenManager;

    /** The browser controls sizer/manager to observe browser controls events. */
    private final BrowserControlsSizer mBrowserControlsSizer;

    private final TabObscuringHandler mTabObscuringHandler;

    private final CallbackController mCallbackController;

    private final ObservableSupplier<EdgeToEdgeController> mEdgeToEdgeControllerSupplier;

    private final Supplier<Boolean> mReadAloudRestoringSupplier;

    /** The height of the bottom bar in pixels, not including the top shadow. */
    private int mBottomControlsHeight;

    /** A {@link WindowAndroid} for watching keyboard visibility events. */
    private final WindowAndroid mWindowAndroid;

    /** The bottom controls visibility. */
    private boolean mIsBottomControlsVisible;

    /** The background color for the bottom controls. */
    private @ColorInt int mBottomControlsColor;

    /** Whether any overlay panel is showing. */
    private boolean mIsOverlayPanelShowing;

    /** Whether the swipe layout is currently active. */
    private boolean mIsInSwipeLayout;

    /** Whether the soft keyboard is visible. */
    private boolean mIsKeyboardVisible;

    private LayoutStateProvider mLayoutStateProvider;

    @Nullable private ChangeObserver mEdgeToEdgeChangeObserver;
    private int mEdgeToEdgePaddingPx;

    /**
     * Build a new mediator that handles events from outside the bottom controls component.
     *
     * @param windowAndroid A {@link WindowAndroid} for watching keyboard visibility events.
     * @param model The {@link BottomControlsProperties} that holds all the view state for the
     *     bottom controls component.
     * @param controlsSizer The {@link BrowserControlsSizer} to manipulate browser controls.
     * @param fullscreenManager A {@link FullscreenManager} for events related to the browser
     *     controls.
     * @param tabObscuringHandler Delegate object handling obscuring views.
     * @param bottomControlsHeight The height of the bottom bar in pixels.
     * @param overlayPanelVisibilitySupplier Notifies overlay panel visibility event.
     * @param edgeToEdgeControllerSupplier Supplies an {@link EdgeToEdgeController} to adjust the
     *     height of the bottom controls when drawing all the way to the edge of the screen.
     * @param readAloudRestoringSupplier Supplier that returns true if Read Aloud is currently
     *     restoring its player, e.g. after theme change.
     */
    BottomControlsMediator(
            WindowAndroid windowAndroid,
            PropertyModel model,
            BrowserControlsSizer controlsSizer,
            FullscreenManager fullscreenManager,
            TabObscuringHandler tabObscuringHandler,
            int bottomControlsHeight,
            ObservableSupplier<Boolean> overlayPanelVisibilitySupplier,
            ObservableSupplier<EdgeToEdgeController> edgeToEdgeControllerSupplier,
            Supplier<Boolean> readAloudRestoringSupplier) {
        mModel = model;

        mFullscreenManager = fullscreenManager;
        mBrowserControlsSizer = controlsSizer;
        mBrowserControlsSizer.addObserver(this);
        mTabObscuringHandler = tabObscuringHandler;
        tabObscuringHandler.addObserver(this);

        mBottomControlsHeight = bottomControlsHeight;
        mCallbackController = new CallbackController();
        overlayPanelVisibilitySupplier.addObserver(
                mCallbackController.makeCancelable(
                        (showing) -> {
                            mIsOverlayPanelShowing = showing;
                            updateAndroidViewVisibility();
                        }));

        // Watch for keyboard events so we can hide the bottom toolbar when the keyboard is showing.
        mWindowAndroid = windowAndroid;
        mWindowAndroid.getKeyboardDelegate().addKeyboardVisibilityListener(this);

        mEdgeToEdgeControllerSupplier = edgeToEdgeControllerSupplier;
        if (mEdgeToEdgeControllerSupplier.get() != null) {
            mEdgeToEdgeChangeObserver =
                    (bottomInset) -> {
                        onEdgeToEdgeChanged(bottomInset);
                    };
            mEdgeToEdgeControllerSupplier.get().registerObserver(mEdgeToEdgeChangeObserver);
        }
        mReadAloudRestoringSupplier = readAloudRestoringSupplier;
    }

    void setLayoutStateProvider(LayoutStateProvider layoutStateProvider) {
        mLayoutStateProvider = layoutStateProvider;
        layoutStateProvider.addObserver(this);
    }

    void setBottomControlsVisible(boolean visible) {
        mIsBottomControlsVisible = visible;
        updateCompositedViewVisibility();
        updateAndroidViewVisibility();
    }

    void setBottomControlsColor(@ColorInt int color) {
        mBottomControlsColor = color;
    }

    /** Clean up anything that needs to be when the bottom controls component is destroyed. */
    void destroy() {
        mCallbackController.destroy();
        mBrowserControlsSizer.removeObserver(this);
        mWindowAndroid.getKeyboardDelegate().removeKeyboardVisibilityListener(this);
        if (mLayoutStateProvider != null) {
            mLayoutStateProvider.removeObserver(this);
            mLayoutStateProvider = null;
        }
        if (mEdgeToEdgeControllerSupplier.get() != null && mEdgeToEdgeChangeObserver != null) {
            mEdgeToEdgeControllerSupplier.get().unregisterObserver(mEdgeToEdgeChangeObserver);
            mEdgeToEdgeChangeObserver = null;
        }
        mTabObscuringHandler.removeObserver(this);
    }

    @Override
    public void onControlsOffsetChanged(
            int topOffset,
            int topControlsMinHeightOffset,
            int bottomOffset,
            int bottomControlsMinHeightOffset,
            boolean needsAnimate) {
        int minHeight = mBrowserControlsSizer.getBottomControlsMinHeight();
        mModel.set(BottomControlsProperties.Y_OFFSET, bottomOffset - minHeight);

        // This call also updates the view's position if the animation has just finished.
        updateAndroidViewVisibility();
    }

    @Override
    public void onBottomControlsHeightChanged(
            int bottomControlsHeight, int bottomControlsMinHeight) {
        // TODO(331829509): Set position in a way that doesn't rely on browser controls size system.
        // Normally our Android view is translated at the end of bottom controls min height
        // animations to place its bottom edge at the min height. This doesn't work during theme
        // change because onControlsOffsetChanged() is never called in that case. Instead we have
        // this special case to make sure the bottom controls aren't covered by the Read Aloud
        // player when it is shown again following browser UI being recreated.
        if (mReadAloudRestoringSupplier.get()) {
            mModel.set(BottomControlsProperties.ANDROID_VIEW_TRANSLATE_Y, -bottomControlsMinHeight);
        }
        // A min height greater than 0 suggests the presence of some other UI component underneath
        // the bottom controls.
        if (bottomControlsMinHeight == 0) {
            mBrowserControlsSizer.notifyBackgroundColor(mBottomControlsColor);
        }
    }

    @Override
    public void keyboardVisibilityChanged(boolean isShowing) {
        mIsKeyboardVisible = isShowing;
        updateCompositedViewVisibility();
        updateAndroidViewVisibility();
    }

    // LayoutStateObserver

    @Override
    public void onStartedShowing(@LayoutType int layoutType) {
        mIsInSwipeLayout = layoutType == LayoutType.TOOLBAR_SWIPE;
        updateAndroidViewVisibility();
    }

    private void onEdgeToEdgeChanged(int bottomInset) {
        mEdgeToEdgePaddingPx = bottomInset;

        updateBrowserControlsHeight();

        int androidViewHeight = getAndroidViewHeight();
        if (androidViewHeight != mModel.get(BottomControlsProperties.ANDROID_VIEW_HEIGHT)) {
            mModel.set(BottomControlsProperties.ANDROID_VIEW_HEIGHT, androidViewHeight);
        }
    }

    /**
     * @return Whether the browser is currently in fullscreen mode.
     */
    private boolean isInFullscreenMode() {
        return mFullscreenManager != null && mFullscreenManager.getPersistentFullscreenMode();
    }

    /**
     * The composited view is the composited version of the Android View. It is used to be able to
     * scroll the bottom controls off-screen synchronously. Since the bottom controls live below
     * the webcontents we re-size the webcontents through
     * {@link BrowserControlsSizer#setBottomControlsHeight(int,int)} whenever the composited view
     * visibility changes.
     */
    private void updateCompositedViewVisibility() {
        final boolean isCompositedViewVisible = isCompositedViewVisible();
        mModel.set(BottomControlsProperties.COMPOSITED_VIEW_VISIBLE, isCompositedViewVisible);
        updateBrowserControlsHeight();
    }

    private int getBrowserControlsHeight() {
        int minHeight = mBrowserControlsSizer.getBottomControlsMinHeight();
        int androidViewHeight = getAndroidViewHeight();

        return isCompositedViewVisible() ? androidViewHeight + minHeight : minHeight;
    }

    private int getAndroidViewHeight() {
        int edgeToEdgePadding = 0;

        if (mEdgeToEdgeControllerSupplier.get() != null) {
            // TODO(https://crbug.com/327274751): Account for presence of Read Aloud when
            // determining bottom controls height.
            edgeToEdgePadding = mEdgeToEdgePaddingPx;
        }

        return mBottomControlsHeight + edgeToEdgePadding;
    }

    private void updateBrowserControlsHeight() {
        mBrowserControlsSizer.setBottomControlsHeight(
                getBrowserControlsHeight(), mBrowserControlsSizer.getBottomControlsMinHeight());
    }

    boolean isCompositedViewVisible() {
        return mIsBottomControlsVisible && !mIsKeyboardVisible && !isInFullscreenMode();
    }

    /**
     * The Android View is the interactive view. The composited view should always be behind the
     * Android view which means we hide the Android view whenever the composited view is hidden. We
     * also hide the Android view as we are scrolling the bottom controls off screen this is done by
     * checking if {@link BrowserControlsSizer#getBottomControlOffset()} is non-zero.
     */
    private void updateAndroidViewVisibility() {
        final boolean visible =
                isCompositedViewVisible()
                        && !mIsOverlayPanelShowing
                        && !mIsInSwipeLayout
                        && mBrowserControlsSizer.getBottomControlOffset() == 0;
        if (visible) {
            // Translate view so that its bottom is aligned with browser controls min height.
            mModel.set(
                    BottomControlsProperties.ANDROID_VIEW_TRANSLATE_Y,
                    -mBrowserControlsSizer.getBottomControlsMinHeight());
        }
        mModel.set(BottomControlsProperties.ANDROID_VIEW_VISIBLE, visible);
    }

    @Override
    public void updateObscured(boolean obscureTabContent, boolean obscureToolbar) {
        mModel.set(BottomControlsProperties.IS_OBSCURED, obscureToolbar);
    }

    ChangeObserver getEdgeToEdgeChangeObserverForTesting() {
        return mEdgeToEdgeChangeObserver;
    }

    void simulateEdgeToEdgeChangeForTesting(int bottomInset) {
        onEdgeToEdgeChanged(bottomInset);
    }
}
