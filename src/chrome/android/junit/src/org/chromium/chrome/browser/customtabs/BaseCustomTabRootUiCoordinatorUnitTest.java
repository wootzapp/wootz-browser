// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.customtabs;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import androidx.test.filters.MediumTest;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.robolectric.annotation.Config;

import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.base.test.util.Batch;
import org.chromium.base.test.util.Features;
import org.chromium.base.test.util.Features.DisableFeatures;
import org.chromium.base.test.util.Features.EnableFeatures;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.page_insights.proto.Config.PageInsightsConfig;

/** JUnit tests for BaseCustomTabRootUiCoordinator. */
@RunWith(BaseRobolectricTestRunner.class)
@Batch(Batch.UNIT_TESTS)
@Config(manifest = Config.NONE)
public final class BaseCustomTabRootUiCoordinatorUnitTest {
    @Rule public Features.JUnitProcessor mFeaturesProcessor = new Features.JUnitProcessor();

    @Test
    @MediumTest
    @EnableFeatures(ChromeFeatureList.CCT_PAGE_INSIGHTS_HUB)
    public void testPageInsightsEnabledSync_cctPageInsightsHubTrue() throws Exception {
        CustomTabsConnection connection = Mockito.mock(CustomTabsConnection.class);
        CustomTabsConnection.setInstanceForTesting(connection);

        when(connection.shouldEnablePageInsightsForIntent(any())).thenReturn(true);
        assertTrue(
                "PageInsightsHub should be enabled",
                BaseCustomTabRootUiCoordinator.isPageInsightsHubEnabled(null));

        // The method should return false if any one of the conditions is not met .

        when(connection.shouldEnablePageInsightsForIntent(any())).thenReturn(false);
        assertFalse(
                "PageInsightsHub should be disabled",
                BaseCustomTabRootUiCoordinator.isPageInsightsHubEnabled(null));
    }

    @Test
    @MediumTest
    @DisableFeatures(ChromeFeatureList.CCT_PAGE_INSIGHTS_HUB)
    public void testPageInsightsEnabledSync_cctPageInsightsHubFalse() throws Exception {
        CustomTabsConnection connection = Mockito.mock(CustomTabsConnection.class);
        CustomTabsConnection.setInstanceForTesting(connection);
        when(connection.shouldEnablePageInsightsForIntent(any())).thenReturn(true);

        // The method returns false if the flag is set to false

        assertFalse(
                "PageInsightsHub should be disabled",
                BaseCustomTabRootUiCoordinator.isPageInsightsHubEnabled(null));
    }

    @Test
    @MediumTest
    public void testGetPageInsightsConfig() throws Exception {
        CustomTabsConnection connection = Mockito.mock(CustomTabsConnection.class);
        CustomTabsConnection.setInstanceForTesting(connection);
        when(connection.shouldEnableGoogleBottomBarForIntent(any())).thenReturn(true);
        when(connection.shouldEnablePageInsightsForIntent(any())).thenReturn(true);
        PageInsightsConfig pageInsightsConfig =
                PageInsightsConfig.newBuilder().setShouldAutoTrigger(true).build();
        when(connection.getPageInsightsConfig(any(), any(), any())).thenReturn(pageInsightsConfig);

        PageInsightsConfig updateConfig =
                BaseCustomTabRootUiCoordinator.getPageInsightsConfig(null, null, null);
        assertTrue(updateConfig.getShouldAutoTrigger());
    }

    @Test
    @MediumTest
    @EnableFeatures(ChromeFeatureList.CCT_GOOGLE_BOTTOM_BAR)
    public void testGoogleBottomBarEnabled_cctGoogleBottomBarTrue() throws Exception {
        CustomTabsConnection connection = Mockito.mock(CustomTabsConnection.class);
        CustomTabsConnection.setInstanceForTesting(connection);

        when(connection.shouldEnableGoogleBottomBarForIntent(any())).thenReturn(true);
        assertTrue(
                "Google Bottom Bar should be enabled",
                BaseCustomTabRootUiCoordinator.isGoogleBottomBarEnabled(null));

        // The method should return false if any one of the conditions is not met .

        when(connection.shouldEnableGoogleBottomBarForIntent(any())).thenReturn(false);
        assertFalse(
                "Google Bottom Bar should be disabled",
                BaseCustomTabRootUiCoordinator.isGoogleBottomBarEnabled(null));
    }

    @Test
    @MediumTest
    @DisableFeatures(ChromeFeatureList.CCT_GOOGLE_BOTTOM_BAR)
    public void testGoogleBottomBarEnabled_cctGoogleBottomBarFalse() throws Exception {
        CustomTabsConnection connection = Mockito.mock(CustomTabsConnection.class);
        CustomTabsConnection.setInstanceForTesting(connection);
        when(connection.shouldEnableGoogleBottomBarForIntent(any())).thenReturn(true);

        // The method returns false if the flag is set to false

        assertFalse(
                "Google Bottom Bar should be disabled",
                BaseCustomTabRootUiCoordinator.isGoogleBottomBarEnabled(null));
    }
}
