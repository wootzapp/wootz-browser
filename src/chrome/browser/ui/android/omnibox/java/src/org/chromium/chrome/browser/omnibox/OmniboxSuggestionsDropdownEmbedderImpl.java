// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox;

import static org.chromium.build.NullUtil.assertNonNull;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.ComponentCallbacks;
import android.content.Context;
import android.content.res.Configuration;
import android.view.View;
import android.view.View.OnLayoutChangeListener;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.WindowInsets;

import androidx.core.graphics.Insets;
import androidx.core.view.WindowInsetsCompat;

import java.util.function.Supplier;
import javax.swing.ViewportLayout;

import org.chromium.base.BuildInfo;
import org.chromium.base.Callback;
import org.chromium.base.supplier.ObservableSupplierImpl;
import org.chromium.chrome.browser.omnibox.suggestions.OmniboxSuggestionsDropdown;
import org.chromium.base.supplier.Supplier;
import org.chromium.build.annotations.NullMarked;
import org.chromium.chrome.browser.omnibox.suggestions.OmniboxSuggestionsDropdownEmbedder.OmniboxAlignment;
import org.chromium.build.annotations.Nullable;
import org.chromium.chrome.browser.omnibox.styles.OmniboxResourceProvider;
import org.chromium.chrome.browser.omnibox.suggestions.OmniboxSuggestionsDropdownEmbedder;
import org.chromium.ui.base.DeviceFormFactor;
import org.chromium.ui.base.ViewUtils;
import org.chromium.ui.base.WindowAndroid;
import org.chromium.ui.display.DisplayUtil;

import android.util.Log;
import org.chromium.ui.KeyboardVisibilityDelegate;
import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ValueAnimator;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.DecelerateInterpolator;
import org.chromium.base.ContextUtils;
import android.graphics.Rect;

/**
 * Implementation of {@link OmniboxSuggestionsDropdownEmbedder} that positions
 * it using an "anchor"
 * and "horizontal alignment" view.
 */
@NullMarked
public class OmniboxSuggestionsDropdownEmbedderImpl
        implements OmniboxSuggestionsDropdownEmbedder,
        OnLayoutChangeListener,
        OnGlobalLayoutListener,
        ComponentCallbacks {
    private final ObservableSupplierImpl<OmniboxAlignment> mOmniboxAlignmentSupplier = new ObservableSupplierImpl<>(
            OmniboxAlignment.UNSPECIFIED);
    private final WindowAndroid mWindowAndroid;
    private final View mAnchorView;
    private final View mAlignmentView;
    private final boolean mForcePhoneStyleOmnibox;
    private final Supplier<Integer> mKeyboardHeightSupplier;
    private final Supplier<Integer> mBottomWindowPaddingSupplier;
    private final Context mContext;
    // Reusable int array to pass to positioning methods that operate on a two
    // element int array.
    // Keeping it as a member lets us avoid allocating a temp array every time.
    private final int[] mPositionArray = new int[2];
    private int mVerticalOffsetInWindow;
    private int mWindowWidthDp;
    private int mWindowHeightDp;
    private @Nullable WindowInsetsCompat mWindowInsetsCompat;
    private @Nullable View mBaseChromeLayout;
    private KeyboardVisibilityDelegate mKeyboardVisibilityDelegate;
    private ValueAnimator mPaddingAnimator;
    private int mCurrentBottomPadding;
    private View mContentView;
    private boolean isKeyboardShowing;
    private int mKeyboardHeight;
    private boolean isWhitePatchVisible;
    private KeyboardVisibilityDelegate.KeyboardVisibilityListener mKeyboardVisibilityListener;
    private @Nullable View mCompositorViewHolder;

    /**
     * @param windowAndroid               Window object in which the dropdown will
     *                                    be displayed.
     * @param anchorView                  View to which the dropdown should be
     *                                    "anchored" i.e. vertically positioned
     *                                    next to and matching the width of. This
     *                                    must be a descendant of the top-level
     *                                    content
     *                                    (android.R.id.content) view.
     * @param alignmentView               View to which: 1. The dropdown should be
     *                                    horizontally aligned to when
     *                                    its width is smaller than the anchor view.
     *                                    2. The dropdown should vertically align to
     *                                    during animations. This must be a
     *                                    descendant of the anchor view.
     * @param baseChromeLayout            The base view hosting Chrome that certain
     *                                    views (e.g. the omnibox
     *                                    suggestion list) will position themselves
     *                                    relative to. If null, the content view
     *                                    will be
     *                                    used.
     * @param keyboardHeightSupplier      Supplies the current height of the
     *                                    keyboard.
     * @param bottomWindowPaddingSupplier Supplier of the height of the bottom-most
     *                                    region of the
     *                                    window that should be considered part of
     *                                    the window's height. This region is
     *                                    suitable for
     *                                    rendering content, particularly to achieve
     *                                    a full-bleed visual effect, though it
     *                                    should
     *                                    also be incorporated as bottom padding to
     *                                    ensure that such content can be fully
     *                                    scrolled
     *                                    out of this region to be fully visible and
     *                                    interactable. This is used to ensure the
     *                                    suggestions list draws edge to edge when
     *                                    appropriate. This should only be used when
     *                                    the
     *                                    soft keyboard is not visible.
     */
    OmniboxSuggestionsDropdownEmbedderImpl(
            WindowAndroid windowAndroid,
            View anchorView,
            View alignmentView,
            boolean forcePhoneStyleOmnibox,
            @Nullable View baseChromeLayout,
            @Nullable View compositorViewHolder,
            Supplier<Integer> keyboardHeightSupplier,
            Supplier<Integer> bottomWindowPaddingSupplier) {
        mWindowAndroid = windowAndroid;
        mAnchorView = anchorView;
        mAlignmentView = alignmentView;
        mForcePhoneStyleOmnibox = forcePhoneStyleOmnibox;
        mKeyboardHeightSupplier = keyboardHeightSupplier;
        mBottomWindowPaddingSupplier = bottomWindowPaddingSupplier;
        mContext = mAnchorView.getContext();
        mContext.registerComponentCallbacks(this);
        Configuration configuration = mContext.getResources().getConfiguration();
        mWindowWidthDp = configuration.smallestScreenWidthDp;
        mWindowHeightDp = configuration.screenHeightDp;
        mBaseChromeLayout = baseChromeLayout;
        mCompositorViewHolder = compositorViewHolder;
        mKeyboardVisibilityDelegate = KeyboardVisibilityDelegate.getInstance();
        mCurrentBottomPadding = 0;
        mContentView = mAnchorView.getRootView().findViewById(android.R.id.content);
        isKeyboardShowing = false;
        mKeyboardHeight = 0;
        isWhitePatchVisible = true;
        mKeyboardVisibilityListener = (isShowing) -> {
            Log.d("Omni", "keyboard " + isShowing);
            if (isShowing) {
                mKeyboardHeight = mKeyboardVisibilityDelegate
                        .calculateKeyboardHeight(mCompositorViewHolder.getRootView());
                Log.d("Omni", "keyboardHeight " + mKeyboardHeight);
                recalculateOmniboxAlignment();
            } else {
                mKeyboardHeight = 0;
                Log.d("Omni", "keyboardHeight " + mKeyboardHeight);
                recalculateOmniboxAlignment();
            }
        };
        setupWindowAndroidKeyboardDelegate();
        recalculateOmniboxAlignment();
    }

    private void setupWindowAndroidKeyboardDelegate() {
        if (mWindowAndroid != null) {
            mWindowAndroid.getKeyboardDelegate().addKeyboardVisibilityListener(mKeyboardVisibilityListener);
        }
    }

    private void onKeyboardVisibilityChanged(boolean opened) {
        // Log.d("Omni", "keyboard " + opened);
        recalculateOmniboxAlignment();
    }

    @Override
    public OmniboxAlignment addAlignmentObserver(Callback<OmniboxAlignment> obs) {
        return assertNonNull(mOmniboxAlignmentSupplier.addObserver(obs));
    }

    @Override
    public void removeAlignmentObserver(Callback<OmniboxAlignment> obs) {
        mOmniboxAlignmentSupplier.removeObserver(obs);
    }

    @Override
    public View getAnchorView() {

        return mAnchorView;
    }

    @Override
    public OmniboxAlignment getCurrentAlignment() {
        OmniboxAlignment currentAlignment = mOmniboxAlignmentSupplier.get();
        if (currentAlignment == null) {
            // Provide default alignment if none exists
            return new OmniboxAlignment(
                    0, // left
                    0, // top
                    mAnchorView.getMeasuredWidth(), // width
                    mAnchorView.getMeasuredHeight(), // height
                    0, // paddingLeft
                    0 // paddingRight
            );
        }
        return currentAlignment;
    }

    public void setWhitePatchVisible(boolean visible) {

        isWhitePatchVisible = visible;
        recalculateOmniboxAlignment();
    }

    public boolean isWhitePatchVisible() {

        return isWhitePatchVisible;
    }

    @Override
    public boolean isTablet() {
        if (mForcePhoneStyleOmnibox)
            return false;
        return mWindowWidthDp >= DeviceFormFactor.MINIMUM_TABLET_WIDTH_DP
                && DeviceFormFactor.isWindowOnTablet(mWindowAndroid);
    }
    public int calculateKeyboardHeightFromWindowInsets(View rootView) {

            if (rootView == null || rootView.getRootWindowInsets() == null) return 0;
            WindowInsetsCompat windowInsetsCompat =
                    WindowInsetsCompat.toWindowInsetsCompat(
                            rootView.getRootWindowInsets(), rootView);
            int imeHeightIncludingSystemBars =
                    windowInsetsCompat.getInsets(WindowInsetsCompat.Type.ime()).bottom;
            if (imeHeightIncludingSystemBars == 0) return 0;
            int bottomSystemBarsHeight =
                    windowInsetsCompat.getInsets(WindowInsetsCompat.Type.systemBars()).bottom;
            return imeHeightIncludingSystemBars - bottomSystemBarsHeight;
    }

    @Override
    public void onAttachedToWindow() {
        Activity activity = (Activity) mContext;
        if(activity != null){
            activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        }
        mAnchorView.addOnLayoutChangeListener(this);
        mAlignmentView.addOnLayoutChangeListener(this);
        mAnchorView.getViewTreeObserver().addOnGlobalLayoutListener(this);
        onConfigurationChanged(mContext.getResources().getConfiguration());
        recalculateOmniboxAlignment();
    }

    @Override
    public void onDetachedFromWindow() {
        Activity activity = (Activity) mContext;
        if(activity != null){

            activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        }
        View contentView = mAnchorView.getRootView().findViewById(android.R.id.content);
        if (contentView != null) {
            ViewCompat.setPaddingRelative(contentView, 0, 0, 0, 0);
        }
        mKeyboardHeight = 0;
        recalculateOmniboxAlignment();
        mAnchorView.removeOnLayoutChangeListener(this);
        mAlignmentView.removeOnLayoutChangeListener(this);
        mAnchorView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
    }

    // View.OnLayoutChangeListener
    @Override
    public void onLayoutChange(
            View v,
            int left,
            int top,
            int right,
            int bottom,
            int oldLeft,
            int oldTop,
            int oldRight,
            int oldBottom) {
        recalculateOmniboxAlignment();
    }

    // OnGlobalLayoutListener
    @Override
    public void onGlobalLayout() {
        if (offsetInWindowChanged(mAnchorView) || insetsHaveChanged(mAnchorView)) {
            recalculateOmniboxAlignment();
        }
    }

    // ComponentCallbacks
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        int windowWidth = newConfig.screenWidthDp;
        int windowHeight = newConfig.screenHeightDp;
        if (windowWidth == mWindowWidthDp && mWindowHeightDp == windowHeight)
            return;
        mWindowWidthDp = windowWidth;
        mWindowHeightDp = windowHeight;

        recalculateOmniboxAlignment();
    }

    @Override
    public void onLowMemory() {
    }

    @Override
    public float getVerticalTranslationForAnimation() {
        return mAlignmentView.getTranslationY();
    }

    /**
     * Recalculates the desired alignment of the omnibox and sends the updated
     * alignment data to any
     * observers. Currently will send an update message unconditionally. This method
     * is called
     * during layout and should avoid memory allocations other than the necessary
     * new
     * OmniboxAlignment(). The method aligns the omnibox dropdown as follows:
     *
     * <p>
     * Case 1: Omnibox revamp enabled on tablet window.
     *
     * <pre>
     *  | anchor  [  alignment  ]       |
     *            |  dropdown   |
     * </pre>
     *
     * <p>
     * Case 2: Omnibox revamp disabled on tablet window.
     *
     * <pre>
     *  | anchor    [alignment]         |
     *  |{pad_left} dropdown {pad_right}|
     * </pre>
     *
     * <p>
     * Case 3: Phone window. Full width and no padding.
     *
     * <pre>
     *  | anchor     [alignment]        |
     *  |           dropdown            |
     * </pre>
     */
    public void recalculateOmniboxAlignment() {
        View contentView = mBaseChromeLayout;
        // TODO(b:397495817): delete the section below once no longer needed.\
        assert contentView != null
                : "b:397495817: please share backtrace and repro steps on the bug!";
        if (contentView == null) {
            contentView = mAnchorView.getRootView().findViewById(android.R.id.content);
        }
        if(!mKeyboardVisibilityDelegate.isKeyboardShowing(mContext,contentView)) {
            // Immediately reset everything if keyboard is hidden
            mKeyboardHeight = 0;
            ViewCompat.setPaddingRelative(contentView, 0, 0, 0, 0);
            contentView.requestLayout();
            return; // Exit early to prevent any delayed adjustments
        }


        ViewUtils.getRelativeLayoutPosition(contentView, mAnchorView, mPositionArray);
        int top = mPositionArray[1] + mAnchorView.getMeasuredHeight() - contentView.getPaddingTop();
        top -= mPositionArray[1];
        int left;
        int width;
        int paddingLeft;
        int paddingRight;
        if (isTablet()) {
            ViewUtils.getRelativeLayoutPosition(mAnchorView, mAlignmentView, mPositionArray);
            // Width equal to alignment view and left equivalent to left of alignment view.
            // Top
            // minus a small overlap.
            top -= mContext.getResources()
                    .getDimensionPixelSize(R.dimen.omnibox_suggestion_list_toolbar_overlap);
            int sideSpacing = OmniboxResourceProvider.getDropdownSideSpacing(mContext);
            width = mAlignmentView.getMeasuredWidth() + 2 * sideSpacing;

            if (mAnchorView.getLayoutDirection() == View.LAYOUT_DIRECTION_RTL) {
                // The view will be shifted to the left, so the adjustment needs to be negative.
                left = -(mAnchorView.getMeasuredWidth() - width - mPositionArray[0] + sideSpacing);
            } else {
                left = mPositionArray[0] - sideSpacing;
            }
            paddingLeft = 0;
            paddingRight = 0;
        } else {
            // Case 3: phones or phone-sized windows on tablets. Full bleed width with no
            // padding or
            // positioning adjustments.
            left = 0;
            width = mAnchorView.getMeasuredWidth();
            paddingLeft = 0;
            paddingRight = 0;
        }

        // int keyboardHeight = mKeyboardHeightSupplier.get();

        int windowHeight;
        if (BuildInfo.getInstance().isAutomotive
                && contentView != null
                && contentView.getRootWindowInsets() != null) {
            // Some automotive devices dismiss bottom system bars when bringing up the
            // keyboard,
            // preventing the height of those bottom bars from being subtracted from the
            // keyboard.
            // To avoid a bottom-bar-sized gap above the keyboard, Chrome needs to calculate
            // a new
            // window height from the display with the new system bar insets, rather than
            // rely on
            // the cached mWindowHeightDp (that implicitly assumes persistence of the
            // now-dismissed
            // bottom system bars).
            WindowInsetsCompat windowInsets = WindowInsetsCompat
                    .toWindowInsetsCompat(contentView.getRootWindowInsets());
            Insets systemBars = windowInsets.getInsets(WindowInsetsCompat.Type.systemBars());
            windowHeight = mWindowAndroid.getDisplay().getDisplayHeight()
                    - systemBars.top
                    - systemBars.bottom;
        } else {
            windowHeight = DisplayUtil.dpToPx(mWindowAndroid.getDisplay(), mWindowHeightDp);
        }

        int paddingBottom = 0;
        // Apply extra bottom padding if the keyboard isn't showing.
        if (keyboardHeight <= 0) {
            paddingBottom = mBottomWindowPaddingSupplier.get();
            windowHeight += paddingBottom;
        }

        int minSpaceAboveWindowBottom = mContext.getResources()
                .getDimensionPixelSize(R.dimen.omnibox_min_space_above_window_bottom);
        int windowSpace = Math.min(windowHeight - keyboardHeight, windowHeight - minSpaceAboveWindowBottom);
        // If content view is null, then omnibox might not be in the activity content.
        int contentSpace = contentView == null
                ? Integer.MAX_VALUE
                : contentView.getMeasuredHeight() - mKeyboardHeight;
        int height = Math.min(windowSpace,  Integer.MAX_VALUE) - top;
         int offset = -25;
        Log.d("Omnibox", "windowSpace: " + windowSpace + 
        " contentSpace: " + contentSpace + 
        " height: " + height + 
        " KeyboardHeight: " + mKeyboardHeight +
        " windowHeight: " + windowHeight +
        " contentViewHeight: " + (contentView != null ? contentView.getMeasuredHeight() : "null"));

        if(mKeyboardVisibilityDelegate.isKeyboardShowing(mContext,contentView) || (mKeyboardHeight > 0)){
            // Only add padding if keyboard is showing and there's a gap
            if(windowSpace - mKeyboardHeight - contentSpace < offset){
                ViewCompat.setPaddingRelative(contentView, 0, 0, 0, mKeyboardHeight);
            }
        } else {
            // Reset padding when keyboard is hidden
            ViewCompat.setPaddingRelative(contentView, 0, 0, 0, 0);
            mKeyboardHeight = 0;
        }

        // Force layout refresh
        if (contentView != null) {
            contentView.requestLayout();
        }

        top = 0;
        // TODO(pnoland@, https://crbug.com/1416985): avoid pushing changes that are
        // identical to
        // the previous alignment value.
        OmniboxAlignment omniboxAlignment = new OmniboxAlignment(
                left, top, width, height, paddingLeft, paddingRight, paddingBottom);
        mOmniboxAlignmentSupplier.set(omniboxAlignment);
    }

    /**
     * Returns whether the given view's position in the window has changed since the
     * last call to
     * offsetInWindowChanged().
     */
    private boolean offsetInWindowChanged(View view) {
        view.getLocationInWindow(mPositionArray);
        boolean result = mVerticalOffsetInWindow != mPositionArray[1];
        mVerticalOffsetInWindow = mPositionArray[1];
        return result;
    }

    /**
     * Returns whether the window insets corresponding to the given view have
     * changed since the last
     * call to insetsHaveChanged().
     */
    private boolean insetsHaveChanged(View view) {
        WindowInsets rootWindowInsets = view.getRootWindowInsets();
        if (rootWindowInsets == null)
            return false;
        WindowInsetsCompat windowInsetsCompat = WindowInsetsCompat.toWindowInsetsCompat(view.getRootWindowInsets(),
                view);
        boolean result = !windowInsetsCompat.equals(mWindowInsetsCompat);
        mWindowInsetsCompat = windowInsetsCompat;
        return result;
    }
}
