// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.hub;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.content.Context;
import android.content.res.ColorStateList;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.FrameLayout;

import androidx.annotation.ColorInt;
import androidx.annotation.Nullable;
import androidx.annotation.StyleRes;
import androidx.core.widget.TextViewCompat;

import org.chromium.ui.widget.ButtonCompat;

import java.util.Objects;

/** Holds the current pane's {@link View}. */
public class HubPaneHostView extends FrameLayout {
    // Chosen to exactly match the default add/remove animation duration of RecyclerView.
    private static final int FADE_ANIMATION_DURATION_MILLIS = 120;

    private FrameLayout mPaneFrame;
    private ButtonCompat mActionButton;
    private @Nullable View mCurrentViewRoot;
    private @Nullable Animator mCurrentAnimator;

    /** Default {@link FrameLayout} constructor called by inflation. */
    public HubPaneHostView(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mPaneFrame = findViewById(R.id.pane_frame);
        mActionButton = findViewById(R.id.host_action_button);
    }

    void setRootView(@Nullable View newRootView) {
        final View oldRootView = mCurrentViewRoot;
        mCurrentViewRoot = newRootView;
        if (mCurrentAnimator != null) {
            mCurrentAnimator.end();
            assert mCurrentAnimator == null;
        }

        if (oldRootView != null && newRootView != null) {
            newRootView.setAlpha(0);
            tryAddViewToFrame(newRootView);

            Animator fadeOut = ObjectAnimator.ofFloat(oldRootView, View.ALPHA, 1, 0);
            fadeOut.setDuration(FADE_ANIMATION_DURATION_MILLIS);

            Animator fadeIn = ObjectAnimator.ofFloat(newRootView, View.ALPHA, 0, 1);
            fadeIn.setDuration(FADE_ANIMATION_DURATION_MILLIS);

            AnimatorSet animatorSet = new AnimatorSet();
            animatorSet.playSequentially(fadeOut, fadeIn);
            animatorSet.addListener(
                    new AnimatorListenerAdapter() {
                        @Override
                        public void onAnimationEnd(Animator animation) {
                            mPaneFrame.removeView(oldRootView);
                            oldRootView.setAlpha(1);
                            mCurrentAnimator = null;
                        }
                    });
            mCurrentAnimator = animatorSet;
            animatorSet.start();
        } else if (newRootView == null) {
            mPaneFrame.removeAllViews();
        } else { // oldRootView == null
            tryAddViewToFrame(newRootView);
        }
    }

    void setActionButtonData(@Nullable FullButtonData buttonData) {
        ApplyButtonData.apply(buttonData, mActionButton);
    }

    void setColorScheme(@HubColorScheme int colorScheme) {
        Context context = getContext();

        @ColorInt int backgroundColor = HubColors.getBackgroundColor(context, colorScheme);
        mPaneFrame.setBackgroundColor(backgroundColor);

        ColorStateList iconColor = HubColors.getIconColor(context, colorScheme);
        TextViewCompat.setCompoundDrawableTintList(mActionButton, iconColor);

        ColorStateList buttonColor =
                HubColors.getSecondaryContainerColorStateList(context, colorScheme);
        mActionButton.setButtonColor(buttonColor);

        @StyleRes int textAppearance = HubColors.getTextAppearanceMedium(colorScheme);
        mActionButton.setTextAppearance(textAppearance);
    }

    private void tryAddViewToFrame(View rootView) {
        ViewParent parent = rootView.getParent();
        if (!Objects.equals(parent, mPaneFrame)) {
            if (parent instanceof ViewGroup viewGroup) {
                viewGroup.removeView(rootView);
            }
            mPaneFrame.addView(rootView);
        }
    }
}
