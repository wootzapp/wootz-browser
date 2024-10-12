// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.toolbar;

import androidx.annotation.OptIn;
import androidx.core.os.BuildCompat;

import org.chromium.base.ResettersForTesting;
import org.chromium.base.cached_flags.BooleanCachedFieldTrialParameter;
import org.chromium.base.cached_flags.IntCachedFieldTrialParameter;
import org.chromium.chrome.browser.flags.ChromeFeatureList;

/** Utility class for toolbar code interacting with features and params. */
public final class ToolbarFeatures {
    private static final int DEFAULT_DTC_THRESHOLD_DP = 412;
    private static final String DTC_TRANSITION_THRESHOLD_DP_PARAM_NAME = "transition_threshold_dp";

    /** Width threshold for the tab strip to start transition. */
    public static final IntCachedFieldTrialParameter DTC_TRANSITION_THRESHOLD_DP =
            ChromeFeatureList.newIntCachedFieldTrialParameter(
                    ChromeFeatureList.DYNAMIC_TOP_CHROME,
                    DTC_TRANSITION_THRESHOLD_DP_PARAM_NAME,
                    DEFAULT_DTC_THRESHOLD_DP);

    private static final String USE_TOOLBAR_BG_COLOR_FOR_STRIP_TRANSITION_SCRIM_PARAM =
            "use_toolbar_bg_color_for_strip_transition_scrim";

    /**
     * Whether the toolbar bg color will be used as the scrim overlay on the tab strip / status bar
     * during a tab strip transition.
     */
    public static final BooleanCachedFieldTrialParameter
            USE_TOOLBAR_BG_COLOR_FOR_STRIP_TRANSITION_SCRIM =
                    ChromeFeatureList.newBooleanCachedFieldTrialParameter(
                            ChromeFeatureList.DYNAMIC_TOP_CHROME,
                            USE_TOOLBAR_BG_COLOR_FOR_STRIP_TRANSITION_SCRIM_PARAM,
                            true);

    private static Boolean sTabStripLayoutOptimizationEnabledForTesting;

    /** Private constructor to avoid instantiation. */
    private ToolbarFeatures() {}

    public static boolean shouldSuppressCaptures() {
        return ChromeFeatureList.sSuppressionToolbarCaptures.isEnabled();
    }

    /**
     * Returns whether to record metrics from suppression experiment. This allows an arm of
     * suppression to run without the overhead from reporting any extra metrics in Java. Using a
     * feature instead of a param to utilize Java side caching.
     */
    public static boolean shouldRecordSuppressionMetrics() {
        return ChromeFeatureList.sRecordSuppressionMetrics.isEnabled();
    }

    /**
     * @return Whether the toolbar bg color will be used as the scrim overlay on the tab strip and
     *     status bar during a tab strip transition.
     */
    public static boolean shouldUseToolbarBgColorForStripTransitionScrim() {
        return isDynamicTopChromeEnabled()
                && USE_TOOLBAR_BG_COLOR_FOR_STRIP_TRANSITION_SCRIM.getValue();
    }

    /**
     * @return Whether the tab strip will be hidden/shown on a tablet when the window width changes.
     *     This feature will not be supported when the tab strip window layout optimization feature
     *     is enabled.
     */
    public static boolean isDynamicTopChromeEnabled() {
        return ChromeFeatureList.sDynamicTopChrome.isEnabled();
    }

    /** Returns if we are using optimized window layout for tab strip. */
    @OptIn(markerClass = androidx.core.os.BuildCompat.PrereleaseSdkCheck.class)
    public static boolean isTabStripWindowLayoutOptimizationEnabled(boolean isTablet) {
        if (sTabStripLayoutOptimizationEnabledForTesting != null) {
            return sTabStripLayoutOptimizationEnabledForTesting;
        }
        return ChromeFeatureList.sTabStripLayoutOptimization.isEnabled()
                && isTablet
                && BuildCompat.isAtLeastV();
    }

    /** Set the return value for {@link #isTabStripWindowLayoutOptimizationEnabled(boolean)}. */
    public static void setIsTabStripLayoutOptimizationEnabledForTesting(boolean enabled) {
        sTabStripLayoutOptimizationEnabledForTesting = enabled;
        ResettersForTesting.register(() -> sTabStripLayoutOptimizationEnabledForTesting = null);
    }

    /** Returns whether the tab strip height can change during run time. */
    public static boolean canTabStripHeightChange(boolean isTablet) {
        return isTabStripWindowLayoutOptimizationEnabled(isTablet) || isDynamicTopChromeEnabled();
    }
}
