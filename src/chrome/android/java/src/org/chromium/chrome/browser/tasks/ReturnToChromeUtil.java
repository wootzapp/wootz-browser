// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks;

import static org.chromium.chrome.browser.ui.fold_transitions.FoldTransitionController.RESUME_HOME_SURFACE_ON_MODE_CHANGE;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Point;
import android.os.Bundle;
import android.text.TextUtils;
import android.text.format.DateUtils;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import org.chromium.base.ApplicationStatus;
import org.chromium.base.Callback;
import org.chromium.base.IntentUtils;
import org.chromium.base.ResettersForTesting;
import org.chromium.base.TraceEvent;
import org.chromium.base.cached_flags.IntCachedFieldTrialParameter;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.lifetime.Destroyable;
import org.chromium.base.metrics.RecordHistogram;
import org.chromium.base.metrics.RecordUserAction;
import org.chromium.base.supplier.ObservableSupplier;
import org.chromium.base.supplier.ObservableSupplierImpl;
import org.chromium.base.supplier.OneshotSupplier;
import org.chromium.base.supplier.Supplier;
import org.chromium.chrome.browser.ActivityTabProvider;
import org.chromium.chrome.browser.ChromeInactivityTracker;
import org.chromium.chrome.browser.IntentHandler;
import org.chromium.chrome.browser.app.ChromeActivity;
import org.chromium.chrome.browser.back_press.BackPressManager;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.homepage.HomepageManager;
import org.chromium.chrome.browser.homepage.HomepagePolicyManager;
import org.chromium.chrome.browser.layouts.LayoutManager;
import org.chromium.chrome.browser.layouts.LayoutStateProvider;
import org.chromium.chrome.browser.layouts.LayoutStateProvider.LayoutStateObserver;
import org.chromium.chrome.browser.layouts.LayoutType;
import org.chromium.chrome.browser.locale.LocaleManager;
import org.chromium.chrome.browser.new_tab_url.DseNewTabUrlManager;
import org.chromium.chrome.browser.ntp.NewTabPage;
import org.chromium.chrome.browser.preferences.ChromePreferenceKeys;
import org.chromium.chrome.browser.preferences.ChromeSharedPreferences;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.chrome.browser.segmentation_platform.SegmentationPlatformServiceFactory;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabLaunchType;
import org.chromium.chrome.browser.tabmodel.TabCreator;
import org.chromium.chrome.browser.tabmodel.TabModel;
import org.chromium.chrome.browser.tabmodel.TabModelObserver;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.chrome.browser.tabmodel.TabModelUtils;
import org.chromium.chrome.browser.tabmodel.TabPersistentStore.ActiveTabState;
import org.chromium.chrome.browser.ui.fold_transitions.FoldTransitionController;
import org.chromium.chrome.browser.ui.native_page.NativePage;
import org.chromium.chrome.browser.util.BrowserUiUtils;
import org.chromium.chrome.browser.util.BrowserUiUtils.HostSurface;
import org.chromium.chrome.browser.util.BrowserUiUtils.ModuleTypeOnStartAndNtp;
import org.chromium.chrome.features.start_surface.StartSurfaceConfiguration;
import org.chromium.chrome.features.start_surface.StartSurfaceState;
import org.chromium.chrome.features.start_surface.StartSurfaceUserData;
import org.chromium.components.browser_ui.widget.gesture.BackPressHandler;
import org.chromium.components.embedder_support.util.UrlConstants;
import org.chromium.components.embedder_support.util.UrlUtilities;
import org.chromium.components.segmentation_platform.ClassificationResult;
import org.chromium.components.segmentation_platform.PredictionOptions;
import org.chromium.components.segmentation_platform.SegmentationPlatformService;
import org.chromium.components.segmentation_platform.prediction_status.PredictionStatus;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content_public.common.ResourceRequestBody;
import org.chromium.ui.base.DeviceFormFactor;
import org.chromium.ui.base.PageTransition;
import org.chromium.url.GURL;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Locale;

/**
 * This is a utility class for managing features related to returning to Chrome after haven't used
 * Chrome for a while.
 */
public final class ReturnToChromeUtil {
    private static ChromeActivity sActivityPresentingOverivewWithOmniboxForTesting;

    /**
     * The reasons of failing to show the home surface UI on a NTP.
     *
     * These values are persisted to logs. Entries should not be renumbered and numeric values
     * should never be reused. See tools/metrics/histograms/enums.xml.
     */
    @IntDef({
        FailToShowHomeSurfaceReason.FAIL_TO_CREATE_NTP_TAB,
        FailToShowHomeSurfaceReason.FAIL_TO_FIND_NTP_TAB,
        FailToShowHomeSurfaceReason.NOT_A_NATIVE_PAGE,
        FailToShowHomeSurfaceReason.NOT_A_NTP_NATIVE_PAGE,
        FailToShowHomeSurfaceReason.NATIVE_PAGE_IS_FROZEN,
        FailToShowHomeSurfaceReason.NUM_ENTRIES
    })
    @Retention(RetentionPolicy.SOURCE)
    @interface FailToShowHomeSurfaceReason {
        int FAIL_TO_CREATE_NTP_TAB = 0;
        int FAIL_TO_FIND_NTP_TAB = 1;
        int NOT_A_NATIVE_PAGE = 2;

        int NOT_A_NTP_NATIVE_PAGE = 3;
        int NATIVE_PAGE_IS_FROZEN = 4;
        int NUM_ENTRIES = 5;
    }

    @VisibleForTesting
    public static final String LAST_VISITED_TAB_IS_SRP_WHEN_OVERVIEW_IS_SHOWN_AT_LAUNCH_UMA =
            "Startup.Android.LastVisitedTabIsSRPWhenOverviewShownAtLaunch";

    public static final String LAST_ACTIVE_TAB_IS_NTP_WHEN_OVERVIEW_IS_SHOWN_AT_LAUNCH_UMA =
            "StartSurface.ColdStartup.IsLastActiveTabNtp";
    public static final String SHOWN_FROM_BACK_NAVIGATION_UMA =
            "StartSurface.ShownFromBackNavigation.";
    public static final String START_SHOW_STATE_UMA = "StartSurface.Show.State";

    public static final String HOME_SURFACE_SHOWN_AT_STARTUP_UMA =
            "NewTabPage.AsHomeSurface.ShownAtStartup";
    public static final String HOME_SURFACE_SHOWN_UMA = "NewTabPage.AsHomeSurface";
    public static final String FAIL_TO_SHOW_HOME_SURFACE_UI_UMA =
            "NewTabPage.FailToShowHomeSurfaceUI";

    private static final String START_V2_SEGMENTATION_PLATFORM_KEY = "chrome_start_android_v2";

    private static boolean sIsHomepagePolicyManagerInitializedRecorded;
    // Whether to skip the check of the initialization of HomepagePolicyManager.
    private static boolean sSkipInitializationCheckForTesting;

    public static void setActivityPresentingOverivewWithOmniboxForTesting(ChromeActivity value) {
        sActivityPresentingOverivewWithOmniboxForTesting = value;
        ResettersForTesting.register(() -> sActivityPresentingOverivewWithOmniboxForTesting = null);
    }

    private ReturnToChromeUtil() {}

    /**
     * A helper class to handle the back press related to ReturnToChrome feature. If a tab is opened
     * from start surface and this tab is unable to be navigated back further, then we trigger
     * the callback to show overview mode.
     */
    public static class ReturnToChromeBackPressHandler implements BackPressHandler, Destroyable {
        private final ObservableSupplierImpl<Boolean> mBackPressChangedSupplier =
                new ObservableSupplierImpl<>();
        private final Callback<Boolean> mOnBackPressedCallback;
        private final ActivityTabProvider.ActivityTabTabObserver mActivityTabObserver;
        private final ActivityTabProvider mActivityTabProvider;
        private final Supplier<Tab> mTabSupplier; // for debugging only
        private LayoutStateProvider mLayoutStateProvider;
        private LayoutStateObserver mLayoutStateObserver;
        private boolean mIsHandleTabSwitcherShownEnabled;

        public ReturnToChromeBackPressHandler(
                ActivityTabProvider activityTabProvider,
                Callback<Boolean> onBackPressedCallback,
                Supplier<Tab> tabSupplier,
                OneshotSupplier<LayoutStateProvider> layoutStateProviderSupplier,
                boolean isHandleTabSwitcherShownEnabled) {
            mActivityTabProvider = activityTabProvider;
            mActivityTabObserver =
                    new ActivityTabProvider.ActivityTabTabObserver(activityTabProvider, true) {
                        @Override
                        protected void onObservingDifferentTab(Tab tab, boolean hint) {
                            onBackPressStateChanged();
                        }
                    };
            mOnBackPressedCallback = onBackPressedCallback;
            mTabSupplier = tabSupplier;
            mIsHandleTabSwitcherShownEnabled = isHandleTabSwitcherShownEnabled;
            if (mIsHandleTabSwitcherShownEnabled) {
                layoutStateProviderSupplier.onAvailable(this::onLayoutStateProviderAvailable);
            }
            onBackPressStateChanged();
        }

        private void onLayoutStateProviderAvailable(LayoutStateProvider layoutStateProvider) {
            mLayoutStateProvider = layoutStateProvider;
            if (mLayoutStateObserver == null) {
                mLayoutStateObserver =
                        new LayoutStateObserver() {
                            @Override
                            public void onFinishedShowing(int layoutType) {
                                onBackPressStateChanged();
                            }
                        };
            }
            mLayoutStateProvider.addObserver(mLayoutStateObserver);
        }

        private void onBackPressStateChanged() {
            Tab tab = mActivityTabProvider.get();
            mBackPressChangedSupplier.set(
                    tab != null && isTabFromStartSurface(tab)
                            || shouldHandleTabSwitcherShown(
                                    mIsHandleTabSwitcherShownEnabled, mLayoutStateProvider));
        }

        @Override
        public @BackPressResult int handleBackPress() {
            Tab tab = mActivityTabProvider.get();
            boolean handleTabSwitcherShown =
                    shouldHandleTabSwitcherShown(
                            mIsHandleTabSwitcherShownEnabled, mLayoutStateProvider);
            boolean res =
                    tab != null && !tab.canGoBack() && isTabFromStartSurface(tab)
                            || handleTabSwitcherShown;
            if (!res) {
                var controlTab = mTabSupplier.get();
                int layoutType =
                        mLayoutStateProvider != null
                                ? mLayoutStateProvider.getActiveLayoutType()
                                : LayoutType.NONE;
                String msg =
                        "tab %s; control tab %s; back press state %s; layout %s; isFromSS: %s;";
                boolean isFromSS = tab != null && isTabFromStartSurface(tab);
                assert false
                        : String.format(
                                msg,
                                tab,
                                controlTab,
                                tab != null && tab.canGoBack(),
                                layoutType,
                                isFromSS);
                if (BackPressManager.correctTabNavigationOnFallback()) {
                    return BackPressResult.FAILURE;
                }
            }
            mOnBackPressedCallback.onResult(handleTabSwitcherShown);
            return res ? BackPressResult.SUCCESS : BackPressResult.FAILURE;
        }

        @Override
        public ObservableSupplier<Boolean> getHandleBackPressChangedSupplier() {
            return mBackPressChangedSupplier;
        }

        @Override
        public void destroy() {
            mActivityTabObserver.destroy();
            if (mLayoutStateProvider != null) {
                mLayoutStateProvider.removeObserver(mLayoutStateObserver);
                mLayoutStateProvider = null;
            }
        }
    }

    /**
     * Returns whether to handle the back operation if the Tab switcher is showing.
     * @param shouldHandleTabSwitcherShown Whether the back operation should be handled when the
     *     Tab switcher is showing. It is only true when both Start surface and Start surface
     * refactor feature flags are enabled.
     * @param layoutStateProvider The provider of the current layout state.
     */
    public static boolean shouldHandleTabSwitcherShown(
            boolean shouldHandleTabSwitcherShown, LayoutStateProvider layoutStateProvider) {
        return shouldHandleTabSwitcherShown
                && layoutStateProvider != null
                && layoutStateProvider.isLayoutVisible(LayoutType.TAB_SWITCHER);
    }

    /**
     * Shows the Start surface if the given {@link handleTabSwitcherShown} is true.
     * @param handleTabSwitcherShown Whether to handle the back operation from the current showing
     *                               Tab switcher.
     * @param layoutManager The {@link LayoutManager} object.
     */
    public static boolean mayReturnToStartSurface(
            boolean handleTabSwitcherShown, LayoutManager layoutManager) {
        if (!handleTabSwitcherShown) return false;

        recordStartSurfaceState(StartSurfaceState.SHOWING_HOMEPAGE);
        recordBackNavigationToStart("FromTabSwitcher");
        layoutManager.showLayout(LayoutType.START_SURFACE, false);
        return true;
    }

    /**
     * Determine if we should show the tab switcher on returning to Chrome. Returns true if enough
     * time has elapsed since the app was last backgrounded or foreground, depending on which time
     * is the max. The threshold time in milliseconds is set by experiment
     * "enable-start-surface-return-time" or from segmentation platform result if {@link
     * ChromeFeatureList.START_SURFACE_RETURN_TIME} is enabled.
     *
     * @param lastTimeMillis The last time the application was backgrounded or foreground, depends
     *     on which time is the max. Set in ChromeTabbedActivity::onStopWithNative
     * @param useNewReturnTime Whether to use a new return time feature flag. The new flag is
     *     equivalent to the existing one, but allows a different default value other than 8 hours.
     * @return true if past threshold, false if not past threshold or experiment cannot be loaded.
     */
    public static boolean shouldShowTabSwitcher(
            final long lastTimeMillis, boolean useNewReturnTime) {
        long tabSwitcherAfterMillis =
                getReturnTime(
                        useNewReturnTime
                                ? StartSurfaceConfiguration
                                        .START_SURFACE_RETURN_TIME_ON_TABLET_SECONDS
                                : StartSurfaceConfiguration.START_SURFACE_RETURN_TIME_SECONDS);

        if (lastTimeMillis == -1) {
            // No last background timestamp set, use control behavior unless "immediate" was set.
            return tabSwitcherAfterMillis == 0;
        }

        if (tabSwitcherAfterMillis < 0) {
            // If no value for experiment, use control behavior.
            return false;
        }

        return System.currentTimeMillis() - lastTimeMillis >= tabSwitcherAfterMillis;
    }

    /**
     * Gets the return time interval. The return time is in the unit of milliseconds.
     * @param returnTime The return time parameter based on form factor, either phones or tablets.
     */
    private static long getReturnTime(IntCachedFieldTrialParameter returnTime) {
        if (returnTime.getValue() != 0
                && StartSurfaceConfiguration.START_SURFACE_RETURN_TIME_USE_MODEL.getValue()) {
            return getReturnTimeFromSegmentation(returnTime);
        }

        return returnTime.getValue() * DateUtils.SECOND_IN_MILLIS;
    }

    /**
     * Gets the cached return time obtained from the segmentation platform service.
     * Note: this function should NOT been called on tablets! The default value for tablets is -1
     * which means not showing.
     * @return How long to show the Start surface again on startup. A negative value means not show,
     *         0 means showing immediately. The return time is in the unit of milliseconds.
     */
    @VisibleForTesting
    public static long getReturnTimeFromSegmentation(IntCachedFieldTrialParameter returnTime) {
        // Sets the default value as 8 hours; 0 means showing immediately.
        return ChromeSharedPreferences.getInstance()
                .readLong(
                        ChromePreferenceKeys.START_RETURN_TIME_SEGMENTATION_RESULT_MS,
                        returnTime.getDefaultValue());
    }

    /**
     * Check if we should handle the navigation. If so, create a new tab and load the URL.
     *
     * @param params The LoadUrlParams to load.
     * @param incognito Whether to load URL in an incognito Tab.
     * @param parentTab  The parent tab used to create a new tab if needed.
     * @return Current tab created if we have handled the navigation, null otherwise.
     */
    public static Tab handleLoadUrlFromStartSurface(
            LoadUrlParams params, @Nullable Boolean incognito, @Nullable Tab parentTab) {
        return handleLoadUrlFromStartSurface(params, false, incognito, parentTab);
    }

    /**
     * Check if we should handle the navigation. If so, create a new tab and load the URL.
     *
     * @param params The LoadUrlParams to load.
     * @param isBackground Whether to load the URL in a new tab in the background.
     * @param incognito Whether to load URL in an incognito Tab.
     * @param parentTab  The parent tab used to create a new tab if needed.
     * @return Current tab created if we have handled the navigation, null otherwise.
     */
    public static Tab handleLoadUrlFromStartSurface(
            LoadUrlParams params,
            boolean isBackground,
            @Nullable Boolean incognito,
            @Nullable Tab parentTab) {
        try (TraceEvent e = TraceEvent.scoped("StartSurface.LoadUrl")) {
            return handleLoadUrlWithPostDataFromStartSurface(
                    params, null, null, isBackground, incognito, parentTab);
        }
    }

    /**
     * Check if we should handle the navigation. If so, create a new tab and load the URL with POST
     * data.
     *
     * @param params The LoadUrlParams to load.
     * @param postDataType postData type.
     * @param postData POST data to include in the tab URL's request body, ex. bitmap when image
     *                 search.
     * @param incognito Whether to load URL in an incognito Tab. If null, the current tab model will
     *                  be used.
     * @param parentTab The parent tab used to create a new tab if needed.
     * @return true if we have handled the navigation, false otherwise.
     */
    public static boolean handleLoadUrlWithPostDataFromStartSurface(
            LoadUrlParams params,
            @Nullable String postDataType,
            @Nullable byte[] postData,
            @Nullable Boolean incognito,
            @Nullable Tab parentTab) {
        return handleLoadUrlWithPostDataFromStartSurface(
                        params, postDataType, postData, false, incognito, parentTab)
                != null;
    }

    /**
     * Check if we should handle the navigation. If so, create a new tab and load the URL with POST
     * data.
     *
     * @param params The LoadUrlParams to load.
     * @param postDataType   postData type.
     * @param postData       POST data to include in the tab URL's request body, ex. bitmap when
     *         image search.
     * @param isBackground Whether to load the URL in a new tab in the background.
     * @param incognito Whether to load URL in an incognito Tab. If null, the current tab model will
     *         be used.
     * @param parentTab  The parent tab used to create a new tab if needed.
     * @return Current tab created if we have handled the navigation, null otherwise.
     */
    private static Tab handleLoadUrlWithPostDataFromStartSurface(
            LoadUrlParams params,
            @Nullable String postDataType,
            @Nullable byte[] postData,
            boolean isBackground,
            @Nullable Boolean incognito,
            @Nullable Tab parentTab) {
        String url = params.getUrl();
        ChromeActivity chromeActivity = getActivityPresentingOverviewWithOmnibox(url);
        if (chromeActivity == null) return null;

        // Create a new unparented tab.
        boolean incognitoParam;
        if (incognito == null) {
            incognitoParam = chromeActivity.getCurrentTabModel().isIncognito();
        } else {
            incognitoParam = incognito;
        }

        if (!TextUtils.isEmpty(postDataType) && postData != null && postData.length != 0) {
            params.setVerbatimHeaders("Content-Type: " + postDataType);
            params.setPostData(ResourceRequestBody.createFromBytes(postData));
        }

        Tab newTab =
                chromeActivity
                        .getTabCreator(incognitoParam)
                        .createNewTab(
                                params,
                                isBackground
                                        ? TabLaunchType.FROM_LONGPRESS_BACKGROUND
                                        : TabLaunchType.FROM_START_SURFACE,
                                parentTab);
        if (isBackground) {
            StartSurfaceUserData.setOpenedFromStart(newTab);
        }

        int transitionAfterMask = params.getTransitionType() & PageTransition.CORE_MASK;
        if (transitionAfterMask == PageTransition.TYPED
                || transitionAfterMask == PageTransition.GENERATED) {
            RecordUserAction.record("MobileOmniboxUse.StartSurface");
            BrowserUiUtils.recordModuleClickHistogram(
                    BrowserUiUtils.HostSurface.START_SURFACE, ModuleTypeOnStartAndNtp.OMNIBOX);

            // These are not duplicated here with the recording in LocationBarLayout#loadUrl.
            RecordUserAction.record("MobileOmniboxUse");
            LocaleManager.getInstance()
                    .recordLocaleBasedSearchMetrics(false, url, params.getTransitionType());
        }

        return newTab;
    }

    /**
     * @param url The URL to load.
     * @return The ChromeActivity if it is presenting the omnibox on the tab switcher, else null.
     */
    private static ChromeActivity getActivityPresentingOverviewWithOmnibox(String url) {
        if (sActivityPresentingOverivewWithOmniboxForTesting != null) {
            return sActivityPresentingOverivewWithOmniboxForTesting;
        }
        Activity activity = ApplicationStatus.getLastTrackedFocusedActivity();
        if (activity == null
                || !isStartSurfaceEnabled(activity)
                || !(activity instanceof ChromeActivity)) {
            return null;
        }

        ChromeActivity chromeActivity = (ChromeActivity) activity;

        assert LibraryLoader.getInstance().isInitialized();
        if (!chromeActivity.isInOverviewMode() && !UrlUtilities.isNtpUrl(url)) return null;

        return chromeActivity;
    }

    /**
     * Check whether we should show Start Surface as the home page. This is used for all cases
     * except initial tab creation, which uses {@link
     * ReturnToChromeUtil#isStartSurfaceEnabled(Context)}.
     *
     * @return Whether Start Surface should be shown as the home page.
     * @param context The activity context
     */
    public static boolean shouldShowStartSurfaceAsTheHomePage(Context context) {
        return isStartSurfaceEnabled(context)
                && StartSurfaceConfiguration.START_SURFACE_OPEN_START_AS_HOMEPAGE.getValue()
                && useChromeHomepage();
    }

    /**
     * Returns whether to use Chrome's homepage. This function doesn't distinguish whether to show
     * NTP or Start though. If checking whether to show Start as homepage, use {@link
     * ReturnToChromeUtil#shouldShowStartSurfaceAsTheHomePage(Context)} instead.
     */
    @VisibleForTesting
    public static boolean useChromeHomepage() {
        HomepageManager homepageManager = HomepageManager.getInstance();
        GURL homePageGurl = homepageManager.getHomepageGurl();
        return homepageManager.isHomepageEnabled()
                && ((HomepagePolicyManager.isInitializedWithNative()
                                || sSkipInitializationCheckForTesting)
                        && (homePageGurl.isEmpty() || UrlUtilities.isNtpUrl(homePageGurl)));
    }

    /**
     * Returns whether we should show Start Surface as the home page on phone. Start surface hasn't
     * been enabled on tablet yet.
     */
    public static boolean shouldShowStartSurfaceAsTheHomePageOnPhone(
            Context context, boolean isTablet) {
        return !isTablet && shouldShowStartSurfaceAsTheHomePage(context);
    }

    /** Returns whether Start Surface should be shown as a new Tab. */
    public static boolean shouldShowStartSurfaceHomeAsNewTab(
            Context context, boolean incognito, boolean isTablet) {
        return !incognito
                && !isTablet
                && isStartSurfaceEnabled(context)
                && !StartSurfaceConfiguration.START_SURFACE_OPEN_NTP_INSTEAD_OF_START.getValue();
    }

    /**
     * Returns whether Start Surface is enabled in the given context. This includes checks of: 1)
     * whether home page is enabled; 2) whether it is on phone; 3) whether show NTP at start up is
     * not enabled.
     *
     * @param context The activity context.
     */
    public static boolean isStartSurfaceEnabled(Context context) {
        return (!ChromeFeatureList.sShowNtpAtStartupAndroid.isEnabled())
                && (!DseNewTabUrlManager.isNewTabSearchEngineUrlAndroidEnabled()
                        || DseNewTabUrlManager.isDefaultSearchEngineGoogle())
                && StartSurfaceConfiguration.isStartSurfaceFlagEnabled()
                && !DeviceFormFactor.isNonMultiDisplayContextOnTablet(context);
    }

    /**
     * @param tabModelSelector The tab model selector.
     * @return the total tab count, and works before native initialization.
     */
    public static int getTotalTabCount(TabModelSelector tabModelSelector) {
        if (!tabModelSelector.isTabStateInitialized()) {
            return ChromeSharedPreferences.getInstance()
                            .readInt(ChromePreferenceKeys.REGULAR_TAB_COUNT)
                    + ChromeSharedPreferences.getInstance()
                            .readInt(ChromePreferenceKeys.INCOGNITO_TAB_COUNT);
        }

        return tabModelSelector.getTotalTabCount();
    }

    /** Returns whether grid Tab switcher or the Start surface should be shown at startup. */
    public static boolean shouldShowOverviewPageOnStart(
            Context context,
            Intent intent,
            TabModelSelector tabModelSelector,
            ChromeInactivityTracker inactivityTracker,
            boolean isTablet) {
        // Neither Start surface or GTS should be shown on Tablet at startup.
        if (isTablet) return false;

        String intentUrl = IntentHandler.getUrlFromIntent(intent);

        // If user launches Chrome by tapping the app icon, the intentUrl is NULL;
        // If user taps the "New Tab" item from the app icon, the intentUrl will be chrome://newtab,
        // and UrlUtilities.isCanonicalizedNtpUrl(intentUrl) returns true.
        // If user taps the "New Incognito Tab" item from the app icon, skip here and continue the
        // following checks.
        if (UrlUtilities.isCanonicalizedNtpUrl(intentUrl)
                && ReturnToChromeUtil.shouldShowStartSurfaceHomeAsNewTab(
                        context, tabModelSelector.isIncognitoSelected(), isTablet)
                && !intent.getBooleanExtra(IntentHandler.EXTRA_OPEN_NEW_INCOGNITO_TAB, false)) {
            return true;
        }

        // If Start surface isn't enabled, return false.
        if (!ReturnToChromeUtil.isStartSurfaceEnabled(context)) return false;

        return shouldShowHomeSurfaceAtStartupImpl(
                /* useNewReturnTime= */ false, intent, tabModelSelector, inactivityTracker);
    }

    private static boolean shouldShowHomeSurfaceAtStartupImpl(
            boolean useNewReturnTime,
            Intent intent,
            TabModelSelector tabModelSelector,
            ChromeInactivityTracker inactivityTracker) {
        // All of the following checks are based on Start surface is enabled.
        // If there's no tab existing, handle the initial tab creation.
        // Note: if user has a customized homepage, we don't show Start even there isn't any tab.
        // However, if NTP is used as homepage, we show Start when there isn't any tab. See
        // https://crbug.com/1368224.
        if (IntentUtils.isMainIntentFromLauncher(intent)
                && ReturnToChromeUtil.getTotalTabCount(tabModelSelector) <= 0) {
            boolean initialized = HomepagePolicyManager.isInitializedWithNative();
            if (!sIsHomepagePolicyManagerInitializedRecorded) {
                sIsHomepagePolicyManagerInitializedRecorded = true;
                RecordHistogram.recordBooleanHistogram(
                        "Startup.Android.IsHomepagePolicyManagerInitialized", initialized);
            }
            if (!initialized && !sSkipInitializationCheckForTesting) {
                return false;
            } else {
                return useChromeHomepage();
            }
        }

        // Checks whether to show the Start surface due to feature flag TAB_SWITCHER_ON_RETURN_MS.
        long lastVisibleTimeMs = inactivityTracker.getLastVisibleTimeMs();
        long lastBackgroundTimeMs = inactivityTracker.getLastBackgroundedTimeMs();
        return IntentUtils.isMainIntentFromLauncher(intent)
                && ReturnToChromeUtil.shouldShowTabSwitcher(
                        Math.max(lastBackgroundTimeMs, lastVisibleTimeMs), useNewReturnTime);
    }

    /**
     * Returns whether should show a NTP as the home surface at startup. This feature is only
     * enabled on Tablet.
     */
    public static boolean shouldShowNtpAsHomeSurfaceAtStartup(
            boolean isTablet,
            Intent intent,
            Bundle bundle,
            TabModelSelector tabModelSelector,
            ChromeInactivityTracker inactivityTracker) {
        // If "Start surface on tablet" isn't enabled, or
        // ChromeFeatureList.SHOW_NTP_AT_STARTUP_ANDROID isn't enabled, return false.
        if (!StartSurfaceConfiguration.isNtpAsHomeSurfaceEnabled(isTablet)) return false;

        // If the current session is recreated due to a transition from the phone mode to the tablet
        // mode on foldable, checks if the Start surface was shown on the phone mode before the
        // transition.
        if (shouldResumeHomeSurfaceOnFoldConfigurationChange(bundle)) return true;

        return shouldShowHomeSurfaceAtStartupImpl(
                /* useNewReturnTime= */ true, intent, tabModelSelector, inactivityTracker);
    }

    /**
     * Returns whether to show a Home surface on foldable when transiting from the phone mode to the
     * tablet mode. Returns true if Start surface was showing on phone mode before the transition.
     */
    @VisibleForTesting
    public static boolean shouldResumeHomeSurfaceOnFoldConfigurationChange(Bundle bundle) {
        if (bundle == null) return false;

        return bundle.getBoolean(FoldTransitionController.DID_CHANGE_TABLET_MODE, false)
                && bundle.getBoolean(RESUME_HOME_SURFACE_ON_MODE_CHANGE, false);
    }

    /**
     * @param currentTab  The current {@link Tab}.
     * @return Whether the Tab is launched with launchType TabLaunchType.FROM_START_SURFACE or it
     *         has "OpenedFromStart" property.
     */
    public static boolean isTabFromStartSurface(Tab currentTab) {
        final @TabLaunchType int type = currentTab.getLaunchType();
        return type == TabLaunchType.FROM_START_SURFACE
                || StartSurfaceUserData.isOpenedFromStart(currentTab);
    }

    /**
     * Creates a new Tab and show Home surface UI. This is called when the last active Tab isn't a
     * NTP, and we need to create one and show Home surface UI (a module showing the last active
     * Tab).
     * @param tabCreator The {@link TabCreator} object.
     * @param tabModelSelector The {@link TabModelSelector} object.
     * @param homeSurfaceTracker The {@link HomeSurfaceTracker} object.
     * @param lastActiveTabUrl The URL of the last active Tab. It is non-null in cold startup before
     *                         the Tab is restored.
     * @param lastActiveTab The object of the last active Tab. It is non-null after TabModel is
     *                      initialized, e.g., in warm startup.
     */
    public static Tab createNewTabAndShowHomeSurfaceUi(
            @NonNull TabCreator tabCreator,
            @NonNull HomeSurfaceTracker homeSurfaceTracker,
            @Nullable TabModelSelector tabModelSelector,
            @Nullable String lastActiveTabUrl,
            @Nullable Tab lastActiveTab) {
        assert lastActiveTab != null || lastActiveTabUrl != null;

        // Creates a new Tab if doesn't find an existing to reuse.
        Tab ntpTab =
                tabCreator.createNewTab(
                        new LoadUrlParams(UrlConstants.NTP_URL), TabLaunchType.FROM_STARTUP, null);
        boolean isNtpUrl = UrlUtilities.isNtpUrl(ntpTab.getUrl());
        assert isNtpUrl : "The URL of the newly created NTP doesn't match NTP URL!";
        if (!isNtpUrl) {
            recordFailToShowHomeSurfaceReasonUma(
                    FailToShowHomeSurfaceReason.FAIL_TO_CREATE_NTP_TAB);
            return null;
        }

        // In cold startup, we only have the URL of the last active Tab.
        if (lastActiveTab == null) {
            // If the last active Tab isn't ready yet, we will listen to the willAddTab() event and
            // find the Tab instance with the given last active Tab's URL. The last active Tab is
            // always the first one to be restored.
            assert lastActiveTabUrl != null;
            TabModelObserver observer =
                    new TabModelObserver() {
                        @Override
                        public void willAddTab(Tab tab, int type) {
                            boolean isTabExpected =
                                    TextUtils.equals(lastActiveTabUrl, tab.getUrl().getSpec());
                            assert isTabExpected
                                    : String.format(
                                            Locale.ENGLISH,
                                            "The URL of first Tab restored doesn't match the URL of"
                                                + " the last active Tab read from the Tab state"
                                                + " metadata file! Existing Tab count = %d. Last"
                                                + " active tab = %s. First tab = %s.",
                                            tabModelSelector.getModel(false).getCount(),
                                            lastActiveTabUrl,
                                            tab.getUrl().getSpec());
                            if (!isTabExpected) {
                                return;
                            }
                            showHomeSurfaceUiOnNtp(ntpTab, tab, homeSurfaceTracker);
                            tabModelSelector.getModel(false).removeObserver(this);
                        }

                        @Override
                        public void restoreCompleted() {
                            // This would be no-op if the observer has been removed in willAddTab().
                            tabModelSelector.getModel(false).removeObserver(this);
                        }
                    };
            tabModelSelector.getModel(false).addObserver(observer);
        } else {
            // In warm startup, the last active Tab is ready.
            showHomeSurfaceUiOnNtp(ntpTab, lastActiveTab, homeSurfaceTracker);
        }

        return ntpTab;
    }

    /**
     * Shows a NTP on warm startup on tablets if return time arrives. Only create a new NTP if there
     * isn't any existing NTP to reuse.
     *
     * @param isIncognito Whether the incognito mode is selected.
     * @param shouldShowNtpHomeSurfaceOnStartup Whether to show a NTP as home surface on startup.
     * @param currentTabModel The object of the current {@link TabModel}.
     * @param tabCreator The {@link TabCreator} object.
     * @param homeSurfaceTracker The {@link HomeSurfaceTracker} object.
     * @return whether an NTP was shown.
     */
    public static boolean setInitialOverviewStateOnResumeWithNtp(
            boolean isIncognito,
            boolean shouldShowNtpHomeSurfaceOnStartup,
            TabModel currentTabModel,
            TabCreator tabCreator,
            HomeSurfaceTracker homeSurfaceTracker) {
        if (isIncognito || !shouldShowNtpHomeSurfaceOnStartup) {
            return false;
        }

        int index = currentTabModel.index();
        Tab lastActiveTab = TabModelUtils.getCurrentTab(currentTabModel);
        // Early exits if there isn't any Tab, i.e., don't create a home surface.
        if (lastActiveTab == null) return false;

        // If the last active Tab is a NTP, we continue to show this NTP as it is now.
        if (UrlUtilities.isNtpUrl(lastActiveTab.getUrl())) {
            if (!homeSurfaceTracker.isHomeSurfaceTab(lastActiveTab)) {
                homeSurfaceTracker.updateHomeSurfaceAndTrackingTabs(lastActiveTab, null);
            }
        } else {
            int indexOfFirstNtp =
                    TabModelUtils.getTabIndexByUrl(currentTabModel, UrlConstants.NTP_URL);
            if (indexOfFirstNtp != TabModel.INVALID_TAB_INDEX) {
                Tab ntpTab = currentTabModel.getTabAt(indexOfFirstNtp);
                assert indexOfFirstNtp != index;
                boolean isNtpUrl = UrlUtilities.isNtpUrl(ntpTab.getUrl());
                assert isNtpUrl
                        : "The URL of the first NTP found onResume doesn't match a NTP URL!";
                if (!isNtpUrl) {
                    recordFailToShowHomeSurfaceReasonUma(
                            FailToShowHomeSurfaceReason.FAIL_TO_FIND_NTP_TAB);
                    return false;
                }

                // Sets the found NTP as home surface.
                TabModelUtils.setIndex(currentTabModel, indexOfFirstNtp, false);
                showHomeSurfaceUiOnNtp(ntpTab, lastActiveTab, homeSurfaceTracker);
            } else {
                // There isn't any existing NTP, create one.
                createNewTabAndShowHomeSurfaceUi(
                        tabCreator, homeSurfaceTracker, null, null, lastActiveTab);
            }
        }

        recordHomeSurfaceShownAtStartup();
        recordHomeSurfaceShown();
        return true;
    }

    /*
     * Computes a return time from the result of the segmentation platform and stores to prefs.
     */
    public static void cacheReturnTimeFromSegmentation() {
        SegmentationPlatformService segmentationPlatformService =
                SegmentationPlatformServiceFactory.getForProfile(
                        ProfileManager.getLastUsedRegularProfile());
        PredictionOptions predictionOptions = new PredictionOptions(false);
        segmentationPlatformService.getClassificationResult(
                START_V2_SEGMENTATION_PLATFORM_KEY,
                predictionOptions,
                null,
                result -> {
                    cacheReturnTimeFromSegmentationImpl(result);
                });
    }

    @VisibleForTesting
    public static void cacheReturnTimeFromSegmentationImpl(ClassificationResult result) {
        long returnTimeMs;
        if (result.status != PredictionStatus.SUCCEEDED || result.orderedLabels.isEmpty()) {
            // Model execution failed or no label selected.
            returnTimeMs = -1;
        } else {
            String label = result.orderedLabels.get(0);
            // When label is non-integer return -1, else convert label to microseconds.
            returnTimeMs =
                    isValidLong(label) ? (Long.parseLong(label) * DateUtils.SECOND_IN_MILLIS) : -1;
        }
        ChromeSharedPreferences.getInstance()
                .writeLong(
                        ChromePreferenceKeys.START_RETURN_TIME_SEGMENTATION_RESULT_MS,
                        returnTimeMs);
    }

    /** Called when Start surface is shown at startup. */
    public static void recordHistogramsWhenOverviewIsShownAtLaunch() {
        // Records whether the last visited tab shown in the single tab switcher or carousel tab
        // switcher is a search result page or not.
        RecordHistogram.recordBooleanHistogram(
                LAST_VISITED_TAB_IS_SRP_WHEN_OVERVIEW_IS_SHOWN_AT_LAUNCH_UMA,
                ChromeSharedPreferences.getInstance()
                        .readBoolean(ChromePreferenceKeys.IS_LAST_VISITED_TAB_SRP, false));

        // Records whether the last active tab from tab restore is a NTP.
        RecordHistogram.recordBooleanHistogram(
                LAST_ACTIVE_TAB_IS_NTP_WHEN_OVERVIEW_IS_SHOWN_AT_LAUNCH_UMA,
                ChromeSharedPreferences.getInstance()
                                .readInt(
                                        ChromePreferenceKeys.APP_LAUNCH_LAST_KNOWN_ACTIVE_TAB_STATE)
                        == ActiveTabState.NTP);
    }

    /**
     * Records a user action that Start surface is showing due to tapping the back button.
     *
     * @param from: Where the back navigation is initiated, either "FromTab" or "FromTabSwitcher".
     */
    public static void recordBackNavigationToStart(String from) {
        RecordUserAction.record(SHOWN_FROM_BACK_NAVIGATION_UMA + from);
    }

    /**
     * Records the StartSurfaceState when overview page is shown.
     * @param state: the current StartSurfaceState.
     */
    public static void recordStartSurfaceState(@StartSurfaceState int state) {
        RecordHistogram.recordEnumeratedHistogram(
                START_SHOW_STATE_UMA, state, StartSurfaceState.NUM_ENTRIES);
    }

    public static void setSkipInitializationCheckForTesting(boolean skipInitializationCheck) {
        sSkipInitializationCheckForTesting = skipInitializationCheck;
        ResettersForTesting.register(() -> sSkipInitializationCheckForTesting = false);
    }

    /**
     * Records user clicks on the tab switcher button in New tab page or Start surface.
     * @param isInOverview Whether the current tab is in overview mode.
     * @param currentTab Current tab or null if none exists.
     */
    public static void recordClickTabSwitcher(boolean isInOverview, @Nullable Tab currentTab) {
        if (isInOverview) {
            BrowserUiUtils.recordModuleClickHistogram(
                    HostSurface.START_SURFACE, ModuleTypeOnStartAndNtp.TAB_SWITCHER_BUTTON);
        } else if (currentTab != null
                && !currentTab.isIncognito()
                && UrlUtilities.isNtpUrl(currentTab.getUrl())) {
            BrowserUiUtils.recordModuleClickHistogram(
                    HostSurface.NEW_TAB_PAGE, ModuleTypeOnStartAndNtp.TAB_SWITCHER_BUTTON);
        }
    }

    /** Recorded when the home surface NTP is shown at startup. */
    public static void recordHomeSurfaceShownAtStartup() {
        RecordHistogram.recordBooleanHistogram(HOME_SURFACE_SHOWN_AT_STARTUP_UMA, true);
    }

    /** Records the home surface shown impressions. */
    public static void recordHomeSurfaceShown() {
        RecordHistogram.recordBooleanHistogram(HOME_SURFACE_SHOWN_UMA, true);
    }

    public static boolean isScrollableMvtEnabled(Context context) {
        // TODO(b/331667743): Clean up the flag for scrollable mvt while cleaning up surface polish
        // code.
        if (ChromeFeatureList.sSurfacePolish.isEnabled()) {
            return true;
        }

        if (!DeviceFormFactor.isNonMultiDisplayContextOnTablet(context)) {
            // On phones, parameter SHOW_SCROLLABLE_MVT_ON_NTP_PHONE_ANDROID is checked when feature
            // flag surface polish is disabled.
            return ChromeFeatureList.isEnabled(
                    ChromeFeatureList.SHOW_SCROLLABLE_MVT_ON_NTP_PHONE_ANDROID);
        }

        return ChromeFeatureList.isEnabled(ChromeFeatureList.SHOW_SCROLLABLE_MVT_ON_NTP_ANDROID);
    }

    /**
     * Returns the start position of the context menu of a home module.
     *
     * @param resources The {@link Resources} instance to load Android resources from.
     */
    public static Point calculateContextMenuStartPosition(Resources resources) {
        // On the single tab module, the x starts from the right of the tab thumbnail.
        int contextMenuStartX =
                resources.getDimensionPixelSize(
                                org.chromium.chrome.start_surface.R.dimen
                                        .single_tab_module_lateral_margin)
                        + resources.getDimensionPixelSize(
                                org.chromium.chrome.start_surface.R.dimen
                                        .single_tab_module_padding_bottom)
                        + resources.getDimensionPixelSize(
                                org.chromium.chrome.start_surface.R.dimen
                                        .single_tab_module_tab_thumbnail_size);
        // The y starts from the same height of the tab thumbnail.
        int contextMenuStartY =
                resources.getDimensionPixelSize(
                                org.chromium.chrome.start_surface.R.dimen
                                        .single_tab_module_padding_top)
                        * 3;
        return new Point(contextMenuStartX, contextMenuStartY);
    }

    /** Shows the home surface UI on the given NTP on tablets. */
    static void showHomeSurfaceUiOnNtp(
            Tab ntpTab, Tab lastActiveTab, HomeSurfaceTracker homeSurfaceTracker) {
        NativePage nativePage = ntpTab.getNativePage();
        if (nativePage == null) {
            recordFailToShowHomeSurfaceReasonUma(FailToShowHomeSurfaceReason.NOT_A_NATIVE_PAGE);
            return;
        }

        // It is possible to get null after casting ntpTab.getNativePage() to NewTabPage, early
        // exit here. See https://crbug.com/1449900.
        if (!(nativePage instanceof NewTabPage)) {
            recordFailToShowHomeSurfaceReasonUma(FailToShowHomeSurfaceReason.NOT_A_NTP_NATIVE_PAGE);
            if (nativePage.isFrozen()) {
                recordFailToShowHomeSurfaceReasonUma(
                        FailToShowHomeSurfaceReason.NATIVE_PAGE_IS_FROZEN);
            }
            return;
        }

        // This cast is now guaranteed to succeed to a non-null value.
        NewTabPage newTabPage = (NewTabPage) nativePage;
        homeSurfaceTracker.updateHomeSurfaceAndTrackingTabs(ntpTab, lastActiveTab);
        if (StartSurfaceConfiguration.useMagicStack()) {
            newTabPage.showMagicStack(lastActiveTab);
        } else {
            newTabPage.showHomeSurfaceUi(lastActiveTab);
        }
    }

    // TODO(crbug.com/40270227): Removes this histogram once we understand the root cause of
    // the crash.
    private static void recordFailToShowHomeSurfaceReasonUma(
            @FailToShowHomeSurfaceReason int reason) {
        RecordHistogram.recordEnumeratedHistogram(
                FAIL_TO_SHOW_HOME_SURFACE_UI_UMA, reason, FailToShowHomeSurfaceReason.NUM_ENTRIES);
    }

    private static boolean isValidLong(String str) {
        try {
            Long.parseLong(str);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }
}
