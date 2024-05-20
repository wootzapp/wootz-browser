// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.compositor.overlays.strip;

import org.chromium.base.MathUtils;
import org.chromium.chrome.browser.compositor.layouts.Layout;
import org.chromium.ui.base.LocalizationUtils;

/**
 * An interface that defines how to stack views and how they should look visually. This lets certain
 * components customize how the {@link StripLayoutHelper} functions and how other {@link Layout}s
 * visually order tabs.
 */
public abstract class StripStacker {
    /**
     * This gives the implementing class a chance to determine how the tabs should be ordered
     * visually. The positioning logic is the same regardless, this just has to do with visual
     * stacking.
     *
     * @param selectedIndex The selected index of the tabs.
     * @param indexOrderedTabs A list of tabs ordered by index.
     * @param outVisualOrderedTabs The new list of tabs, ordered from back (low z-index) to front
     *     (high z-index) visually.
     */
    public void createVisualOrdering(
            int selectedIndex,
            StripLayoutTab[] indexOrderedTabs,
            StripLayoutTab[] outVisualOrderedTabs) {
        // TODO(crbug.com/40268645): Stacking order can be ignored for TSR.
        assert indexOrderedTabs.length == outVisualOrderedTabs.length;

        selectedIndex = MathUtils.clamp(selectedIndex, 0, indexOrderedTabs.length);

        int outIndex = 0;
        for (int i = 0; i < selectedIndex; i++) {
            outVisualOrderedTabs[outIndex++] = indexOrderedTabs[i];
        }

        for (int i = indexOrderedTabs.length - 1; i >= selectedIndex; --i) {
            outVisualOrderedTabs[outIndex++] = indexOrderedTabs[i];
        }
    }

    /**
     * Computes and sets the draw X, draw Y, visibility and content offset for each view.
     *
     * @param indexOrderedViews A list of tabs ordered by index.
     * @param tabClosing Whether a tab is being closed.
     * @param tabCreating Whether a tab is being created.
     * @param cachedTabWidth Whether The ideal tab width.
     */
    public abstract void setViewOffsets(
            StripLayoutView[] indexOrderedViews,
            boolean tabClosing,
            boolean tabCreating,
            boolean groupTitleSlidingAnimRunning,
            boolean groupCollapsingOrExpanding,
            float cachedTabWidth);

    /**
     * Computes the X offset for the new tab button.
     *
     * @param indexOrderedTabs A list of tabs ordered by index.
     * @param tabOverlapWidth The amount tabs overlap.
     * @param stripLeftMargin The left margin of the tab strip.
     * @param stripRightMargin The right margin of the tab strip.
     * @param stripWidth The width of the tab strip.
     * @param buttonWidth The width of the new tab button.
     * @return The x offset for the new tab button.
     */
    public float computeNewTabButtonOffset(
            StripLayoutTab[] indexOrderedTabs,
            float tabOverlapWidth,
            float stripLeftMargin,
            float stripRightMargin,
            float stripWidth,
            float buttonWidth,
            float cachedTabWidth,
            boolean animate) {
        return LocalizationUtils.isLayoutRtl()
                ? computeNewTabButtonOffsetRtl(
                        indexOrderedTabs,
                        stripLeftMargin,
                        stripRightMargin,
                        stripWidth,
                        buttonWidth,
                        animate)
                : computeNewTabButtonOffsetLtr(
                        indexOrderedTabs,
                        tabOverlapWidth,
                        stripLeftMargin,
                        stripRightMargin,
                        stripWidth,
                        cachedTabWidth,
                        animate);
    }

    private float computeNewTabButtonOffsetLtr(
            StripLayoutTab[] indexOrderedTabs,
            float tabOverlapWidth,
            float stripLeftMargin,
            float stripRightMargin,
            float stripWidth,
            float cachedTabWidth,
            boolean animate) {
        float rightEdge = stripLeftMargin;
        for (StripLayoutTab tab : indexOrderedTabs) {
            if (tab.isDying() || tab.isDraggedOffStrip()) continue;

            float tabWidth;
            float tabDrawX;
            float tabWidthWeight;
            if (animate) {
                // This value is set to 1.f to avoid the new tab button jitter for the improved tab
                // strip design. The tab.width and tab.drawX may not reflect the final values before
                // the tab closing animations are completed.
                tabWidthWeight = 1.f;
                tabWidth = cachedTabWidth;
                tabDrawX = tab.getIdealX();
            } else {
                tabWidthWeight = tab.getWidthWeight();
                tabWidth = tab.getWidth();
                tabDrawX = tab.getDrawX();
            }
            float layoutWidth = (tabWidth - tabOverlapWidth) * tabWidthWeight;
            rightEdge = Math.max(tabDrawX + layoutWidth, rightEdge); // use idealX here
        }

        rightEdge = Math.min(rightEdge + tabOverlapWidth, stripWidth - stripRightMargin);


        // The draw X position for the new tab button is the rightEdge of the tab strip.
        return rightEdge;
    }

    private float computeNewTabButtonOffsetRtl(
            StripLayoutTab[] indexOrderedTabs,
            float stripLeftMargin,
            float stripRightMargin,
            float stripWidth,
            float newTabButtonWidth,
            boolean animate) {
        float leftEdge = stripWidth - stripRightMargin;

        for (StripLayoutTab tab : indexOrderedTabs) {
            if (tab.isDying() || tab.isDraggedOffStrip()) continue;

            float drawX = animate ? tab.getIdealX() : tab.getDrawX();
            leftEdge = Math.min(drawX, leftEdge);
        }

        leftEdge = Math.max(leftEdge, stripLeftMargin);

        // The draw X position for the new tab button is the left edge of the tab strip minus
        // the new tab button width.
        return leftEdge - newTabButtonWidth;
    }

    /**
     * Performs an occlusion pass, setting the visibility on tabs. This is relegated to this
     * interface because the implementing class knows the proper visual order to optimize this pass.
     *
     * @param indexOrderedViews A list of views ordered by index.
     * @param xOffset The xOffset for the start of the strip.
     * @param visibleWidth The width of the visible space on the tab strip.
     */
    public abstract void performOcclusionPass(
            StripLayoutView[] indexOrderedViews, float xOffset, float visibleWidth);
}
