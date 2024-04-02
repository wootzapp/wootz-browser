// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.compositor.overlays.strip;

import org.chromium.ui.base.LocalizationUtils;

/**
 * A stacker that tells the {@link StripLayoutHelper} how to layer the tabs for the
 * {@link StaticLayout} when the available window width is < 600dp. Tabs will be stacked side by
 * side and the entire strip will scroll. Tabs will never completely overlap each other.
 */
public class ScrollingStripStacker extends StripStacker {
    @Override
    public void setTabOffsets(
            StripLayoutTab[] indexOrderedTabs,
            boolean tabClosing,
            boolean tabCreating,
            float cachedTabWidth) {
        boolean rtl = LocalizationUtils.isLayoutRtl();
        for (int i = 0; i < indexOrderedTabs.length; i++) {
            StripLayoutTab tab = indexOrderedTabs[i];
            // When a tab is closed, drawX and width update will be animated so skip this.
            if (!tabClosing) {
                tab.setDrawX(tab.getIdealX() + tab.getOffsetX());

                // Properly animate container slide-out in RTL.
                if (tabCreating && rtl) {
                    tab.setDrawX(tab.getDrawX() + cachedTabWidth - tab.getWidth());
                }

                // When a tab is being created, all tabs are animating to their desired width.
                if (!tabCreating) {
                    tab.setWidth(cachedTabWidth);
                }
            }
            tab.setDrawY(tab.getOffsetY());
        }
    }

    @Override
    public void performOcclusionPass(
            StripLayoutView[] indexOrderedViews, float xOffset, float visibleWidth) {
        for (int i = 0; i < indexOrderedViews.length; i++) {
            StripLayoutView view = indexOrderedViews[i];
            view.setVisible(
                    (view.getDrawX() + view.getWidth()) >= xOffset
                            && view.getDrawX() <= xOffset + visibleWidth);
        }
    }
}
