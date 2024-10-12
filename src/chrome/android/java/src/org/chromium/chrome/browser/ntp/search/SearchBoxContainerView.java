// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ntp.search;

import android.content.Context;
import android.graphics.Typeface;
import android.graphics.drawable.RippleDrawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.content.res.AppCompatResources;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.flags.ChromeFeatureList;

/** Provides the additional capabilities needed for the SearchBox container layout. */
public class SearchBoxContainerView extends LinearLayout {
    private final boolean mIsSurfacePolishEnabled;
    private final int mEndPadding;
    private final int mStartPadding;
    private final int mLateralMargin;

    /** Constructor for inflating from XML. */
    public SearchBoxContainerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mIsSurfacePolishEnabled = ChromeFeatureList.sSurfacePolish.isEnabled();
        mEndPadding = getResources().getDimensionPixelSize(R.dimen.fake_search_box_end_padding);
        mStartPadding = getResources().getDimensionPixelSize(R.dimen.fake_search_box_start_padding);
        mLateralMargin =
                getResources().getDimensionPixelSize(R.dimen.mvt_container_lateral_margin_polish);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        if (mIsSurfacePolishEnabled) {
            setPaddingRelative(mStartPadding, 0, mEndPadding, 0);

            MarginLayoutParams params = (MarginLayoutParams) getLayoutParams();
            params.leftMargin = mLateralMargin;
            params.rightMargin = mLateralMargin;

            setBackground(
                    AppCompatResources.getDrawable(
                            getContext(), R.drawable.home_surface_search_box_background));

            TextView searchBoxTextView = findViewById(R.id.search_box_text);
            searchBoxTextView.setTextAppearance(
                    getContext(), R.style.TextAppearance_SearchBoxText_NewTabPage_SurfacePolish);
            Typeface typeface = Typeface.create("google-sans-medium", Typeface.NORMAL);
            searchBoxTextView.setTypeface(typeface);
        }
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        if (ev.getActionMasked() == MotionEvent.ACTION_DOWN) {
            if (getBackground() instanceof RippleDrawable) {
                ((RippleDrawable) getBackground()).setHotspot(ev.getX(), ev.getY());
            }
        }
        return super.onInterceptTouchEvent(ev);
    }
}
