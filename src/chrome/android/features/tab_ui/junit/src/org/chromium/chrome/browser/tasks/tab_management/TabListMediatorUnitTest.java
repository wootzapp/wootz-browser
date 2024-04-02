// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management;

import static androidx.core.view.accessibility.AccessibilityNodeInfoCompat.ACTION_CLICK;
import static androidx.test.espresso.matcher.ViewMatchers.assertThat;

import static org.hamcrest.CoreMatchers.equalTo;
import static org.hamcrest.CoreMatchers.instanceOf;
import static org.hamcrest.CoreMatchers.not;
import static org.hamcrest.Matchers.contains;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import static org.chromium.chrome.browser.flags.ChromeFeatureList.INSTANT_START;
import static org.chromium.chrome.browser.flags.ChromeFeatureList.START_SURFACE_ANDROID;
import static org.chromium.chrome.browser.tasks.tab_management.MessageCardViewProperties.MESSAGE_TYPE;
import static org.chromium.chrome.browser.tasks.tab_management.MessageService.MessageType.FOR_TESTING;
import static org.chromium.chrome.browser.tasks.tab_management.MessageService.MessageType.PRICE_MESSAGE;
import static org.chromium.chrome.browser.tasks.tab_management.MessageService.MessageType.TAB_SUGGESTION;
import static org.chromium.chrome.browser.tasks.tab_management.TabListModel.CardProperties.CARD_TYPE;
import static org.chromium.chrome.browser.tasks.tab_management.TabListModel.CardProperties.ModelType.MESSAGE;
import static org.chromium.chrome.browser.tasks.tab_management.TabListModel.CardProperties.ModelType.OTHERS;
import static org.chromium.chrome.browser.tasks.tab_management.TabListModel.CardProperties.ModelType.TAB;

import android.app.Activity;
import android.content.ComponentCallbacks;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Pair;
import android.view.View;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeInfo.AccessibilityAction;

import androidx.annotation.IntDef;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.RecyclerView;

import com.google.protobuf.ByteString;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestRule;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.InOrder;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;
import org.robolectric.annotation.LooperMode;

import org.chromium.base.Callback;
import org.chromium.base.ContextUtils;
import org.chromium.base.FeatureList;
import org.chromium.base.Token;
import org.chromium.base.metrics.RecordHistogram;
import org.chromium.base.shared_preferences.SharedPreferencesManager;
import org.chromium.base.supplier.ObservableSupplierImpl;
import org.chromium.base.supplier.Supplier;
import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.base.test.util.Features;
import org.chromium.base.test.util.Features.DisableFeatures;
import org.chromium.base.test.util.Features.EnableFeatures;
import org.chromium.base.test.util.JniMocker;
import org.chromium.build.BuildConfig;
import org.chromium.chrome.browser.compositor.layouts.content.TabContentManager;
import org.chromium.chrome.browser.feature_engagement.TrackerFactory;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.optimization_guide.OptimizationGuideBridge;
import org.chromium.chrome.browser.optimization_guide.OptimizationGuideBridge.OptimizationGuideCallback;
import org.chromium.chrome.browser.optimization_guide.OptimizationGuideBridgeJni;
import org.chromium.chrome.browser.preferences.ChromePreferenceKeys;
import org.chromium.chrome.browser.preferences.ChromeSharedPreferences;
import org.chromium.chrome.browser.price_tracking.PriceTrackingFeatures;
import org.chromium.chrome.browser.price_tracking.PriceTrackingUtilities;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.chrome.browser.search_engines.TemplateUrlServiceFactory;
import org.chromium.chrome.browser.tab.MockTab;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabCreationState;
import org.chromium.chrome.browser.tab.TabLaunchType;
import org.chromium.chrome.browser.tab.TabObserver;
import org.chromium.chrome.browser.tab.TabSelectionType;
import org.chromium.chrome.browser.tab.state.PersistedTabDataConfiguration;
import org.chromium.chrome.browser.tab.state.ShoppingPersistedTabData;
import org.chromium.chrome.browser.tab.state.ShoppingPersistedTabData.PriceDrop;
import org.chromium.chrome.browser.tabmodel.TabModel;
import org.chromium.chrome.browser.tabmodel.TabModelFilter;
import org.chromium.chrome.browser.tabmodel.TabModelObserver;
import org.chromium.chrome.browser.tasks.pseudotab.PseudoTab;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupColorUtils;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupModelFilter;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupModelFilterObserver;
import org.chromium.chrome.browser.tasks.tab_management.PriceMessageService.PriceTabData;
import org.chromium.chrome.browser.tasks.tab_management.TabListCoordinator.TabListMode;
import org.chromium.chrome.browser.tasks.tab_management.TabListFaviconProvider.TabFavicon;
import org.chromium.chrome.browser.tasks.tab_management.TabListMediator.ShoppingPersistedTabDataFetcher;
import org.chromium.chrome.browser.tasks.tab_management.TabListMediator.ThumbnailFetcher;
import org.chromium.chrome.browser.tasks.tab_management.TabProperties.UiType;
import org.chromium.chrome.tab_ui.R;
import org.chromium.components.browser_ui.widget.selectable_list.SelectionDelegate;
import org.chromium.components.commerce.PriceTracking.BuyableProduct;
import org.chromium.components.commerce.PriceTracking.PriceTrackingData;
import org.chromium.components.commerce.PriceTracking.ProductPrice;
import org.chromium.components.embedder_support.util.UrlUtilities;
import org.chromium.components.embedder_support.util.UrlUtilitiesJni;
import org.chromium.components.feature_engagement.EventConstants;
import org.chromium.components.feature_engagement.Tracker;
import org.chromium.components.optimization_guide.OptimizationGuideDecision;
import org.chromium.components.optimization_guide.proto.CommonTypesProto.Any;
import org.chromium.components.search_engines.TemplateUrlService;
import org.chromium.content_public.browser.NavigationHandle;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.ui.modelutil.SimpleRecyclerViewAdapter;
import org.chromium.url.GURL;
import org.chromium.url.JUnitTestGURLs;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/** Tests for {@link TabListMediator}. */
@SuppressWarnings({
    "ArraysAsListWithZeroOrOneArgument",
    "ResultOfMethodCallIgnored",
    "ConstantConditions"
})
@RunWith(BaseRobolectricTestRunner.class)
@Config(
        manifest = Config.NONE,
        instrumentedPackages = {
            "androidx.recyclerview.widget.RecyclerView" // required to mock final
        })
@LooperMode(LooperMode.Mode.LEGACY)
@EnableFeatures({
    ChromeFeatureList.TAB_TO_GTS_ANIMATION,
    ChromeFeatureList.ANDROID_TAB_GROUP_STABLE_IDS
})
@DisableFeatures({START_SURFACE_ANDROID})
public class TabListMediatorUnitTest {
    @Rule public TestRule mProcessor = new Features.JUnitProcessor();

    @Rule public JniMocker mMocker = new JniMocker();

    private static final String TAB1_TITLE = "Tab1";
    private static final String TAB2_TITLE = "Tab2";
    private static final String TAB3_TITLE = "Tab3";
    private static final String TAB4_TITLE = "Tab4";
    private static final String TAB5_TITLE = "Tab5";
    private static final String TAB6_TITLE = "Tab6";
    private static final String TAB7_TITLE = "Tab7";
    private static final String NEW_TITLE = "New title";
    private static final String CUSTOMIZED_DIALOG_TITLE1 = "Cool Tabs";
    private static final String TAB_GROUP_COLORS_FILE_NAME = "tab_group_colors";
    private static final String TAB_GROUP_TITLES_FILE_NAME = "tab_group_titles";
    private static final GURL TAB1_URL = JUnitTestGURLs.URL_1;
    private static final GURL TAB2_URL = JUnitTestGURLs.URL_2;
    private static final GURL TAB3_URL = JUnitTestGURLs.URL_3;
    private static final GURL TAB4_URL = JUnitTestGURLs.RED_1;
    private static final GURL TAB5_URL = JUnitTestGURLs.RED_2;
    private static final GURL TAB6_URL = JUnitTestGURLs.RED_3;
    private static final GURL TAB7_URL = JUnitTestGURLs.URL_1;
    private static final String NEW_URL = JUnitTestGURLs.EXAMPLE_URL.getSpec();
    private static final int COLOR_2 = 1;
    private static final int TAB1_ID = 456;
    private static final int TAB2_ID = 789;
    private static final int TAB3_ID = 123;
    private static final int TAB4_ID = 290;
    private static final int TAB5_ID = 147;
    private static final int TAB6_ID = 258;
    private static final int TAB7_ID = 369;
    private static final int POSITION1 = 0;
    private static final int POSITION2 = 1;
    private static final Token TAB_GROUP_ID = new Token(829L, 283L);

    private static final BuyableProduct BUYABLE_PRODUCT_PROTO_INITIAL =
            BuyableProduct.newBuilder()
                    .setCurrentPrice(createProductPrice(123456789012345L, "USD"))
                    .build();

    private static ProductPrice createProductPrice(long amountMicros, String currencyCode) {
        return ProductPrice.newBuilder()
                .setCurrencyCode(currencyCode)
                .setAmountMicros(amountMicros)
                .build();
    }

    private static final PriceTrackingData PRICE_TRACKING_BUYABLE_PRODUCT_INITIAL =
            PriceTrackingData.newBuilder().setBuyableProduct(BUYABLE_PRODUCT_PROTO_INITIAL).build();
    private static final Any ANY_BUYABLE_PRODUCT_INITIAL =
            Any.newBuilder()
                    .setValue(
                            ByteString.copyFrom(
                                    PRICE_TRACKING_BUYABLE_PRODUCT_INITIAL.toByteArray()))
                    .build();
    private static final Any ANY_EMPTY = Any.newBuilder().build();

    @IntDef({
        TabListMediatorType.TAB_SWITCHER,
        TabListMediatorType.TAB_STRIP,
        TabListMediatorType.TAB_GRID_DIALOG
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface TabListMediatorType {
        int TAB_SWITCHER = 0;
        int TAB_STRIP = 1;
        int TAB_GRID_DIALOG = 2;
        int NUM_ENTRIES = 3;
    }

    @Mock TabContentManager mTabContentManager;
    @Mock TabModel mTabModel;
    @Mock TabModel mIncognitoTabModel;
    @Mock TabListFaviconProvider mTabListFaviconProvider;
    @Mock TabListFaviconProvider.TabFaviconFetcher mTabFaviconFetcher;
    @Mock RecyclerView mRecyclerView;
    @Mock TabListRecyclerView mTabListRecyclerView;
    @Mock RecyclerView.Adapter mAdapter;
    @Mock TabGroupModelFilter mTabGroupModelFilter;
    @Mock TabGroupModelFilter mIncognitoTabGroupModelFilter;
    @Mock TabListMediator.TabGridDialogHandler mTabGridDialogHandler;
    @Mock TabListMediator.GridCardOnClickListenerProvider mGridCardOnClickListenerProvider;
    @Mock TabFavicon mFavicon;
    @Mock Bitmap mFaviconBitmap;
    @Mock Activity mActivity;
    @Mock TabListMediator.TabActionListener mOpenGroupActionListener;
    @Mock GridLayoutManager mGridLayoutManager;
    @Mock GridLayoutManager.SpanSizeLookup mSpanSizeLookup;
    @Mock Profile mProfile;
    @Mock Tracker mTracker;
    @Mock PseudoTab.TitleProvider mTitleProvider;
    @Mock UrlUtilities.Natives mUrlUtilitiesJniMock;
    @Mock OptimizationGuideBridge.Natives mOptimizationGuideBridgeJniMock;
    @Mock TabListMediator.TabGridAccessibilityHelper mTabGridAccessibilityHelper;
    @Mock TemplateUrlService mTemplateUrlService;
    @Mock PriceWelcomeMessageController mPriceWelcomeMessageController;
    @Mock ShoppingPersistedTabData mShoppingPersistedTabData;
    @Mock SelectionDelegate<Integer> mSelectionDelegate;

    @Captor ArgumentCaptor<TabModelObserver> mTabModelObserverCaptor;
    @Captor ArgumentCaptor<TabObserver> mTabObserverCaptor;
    @Captor ArgumentCaptor<Callback<TabFavicon>> mCallbackCaptor;
    @Captor ArgumentCaptor<TabGroupModelFilterObserver> mTabGroupModelFilterObserverCaptor;
    @Captor ArgumentCaptor<ComponentCallbacks> mComponentCallbacksCaptor;

    @Captor
    ArgumentCaptor<TemplateUrlService.TemplateUrlServiceObserver> mTemplateUrlServiceObserver;

    @Captor ArgumentCaptor<RecyclerView.OnScrollListener> mOnScrollListenerCaptor;

    private final ObservableSupplierImpl<TabModelFilter> mCurrentTabModelFilterSupplier =
            new ObservableSupplierImpl<>();

    private TabObserver mTabObserver;
    private Tab mTab1;
    private Tab mTab2;
    private TabListMediator mMediator;
    private TabListModel mModel;
    private SimpleRecyclerViewAdapter.ViewHolder mViewHolder1;
    private SimpleRecyclerViewAdapter.ViewHolder mViewHolder2;
    private RecyclerView.ViewHolder mFakeViewHolder1;
    private RecyclerView.ViewHolder mFakeViewHolder2;
    private View mItemView1 = mock(View.class);
    private View mItemView2 = mock(View.class);
    private TabModelObserver mMediatorTabModelObserver;
    private TabGroupModelFilterObserver mMediatorTabGroupModelFilterObserver;
    private PriceDrop mPriceDrop;
    private PriceTabData mPriceTabData;
    private String mTab1Domain;
    private String mTab2Domain;
    private String mTab3Domain;
    private String mNewDomain;
    private GURL mFaviconUrl;
    private Resources mResources;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mMocker.mock(UrlUtilitiesJni.TEST_HOOKS, mUrlUtilitiesJniMock);
        mMocker.mock(OptimizationGuideBridgeJni.TEST_HOOKS, mOptimizationGuideBridgeJniMock);
        // Ensure native pointer is initialized
        doReturn(1L).when(mOptimizationGuideBridgeJniMock).init();

        mResources = spy(RuntimeEnvironment.application.getResources());
        when(mActivity.getResources()).thenReturn(mResources);
        when(mResources.getInteger(org.chromium.ui.R.integer.min_screen_width_bucket))
                .thenReturn(1);

        mTab1Domain = TAB1_URL.getHost().replace("www.", "");
        mTab2Domain = TAB2_URL.getHost().replace("www.", "");
        mTab3Domain = TAB3_URL.getHost().replace("www.", "");
        mNewDomain = new GURL(NEW_URL).getHost().replace("www.", "");
        mFaviconUrl = JUnitTestGURLs.RED_1;

        mTab1 = prepareTab(TAB1_ID, TAB1_TITLE, TAB1_URL);
        mTab2 = prepareTab(TAB2_ID, TAB2_TITLE, TAB2_URL);
        mViewHolder1 = prepareViewHolder(TAB1_ID, POSITION1);
        mViewHolder2 = prepareViewHolder(TAB2_ID, POSITION2);
        mFakeViewHolder1 = prepareFakeViewHolder(mItemView1, POSITION1);
        mFakeViewHolder2 = prepareFakeViewHolder(mItemView2, POSITION2);
        List<Tab> tabs1 = new ArrayList<>(Arrays.asList(mTab1));
        List<Tab> tabs2 = new ArrayList<>(Arrays.asList(mTab2));

        doNothing()
                .when(mTabContentManager)
                .getTabThumbnailWithCallback(anyInt(), any(), any(), anyBoolean(), anyBoolean());
        // Mock that tab restoring stage is over.
        doReturn(true).when(mTabGroupModelFilter).isTabModelRestored();
        doReturn(true).when(mIncognitoTabGroupModelFilter).isTabModelRestored();
        doReturn(mProfile).when(mTabModel).getProfile();

        doReturn(mTabModel).when(mTabGroupModelFilter).getTabModel();
        doReturn(mIncognitoTabModel).when(mIncognitoTabGroupModelFilter).getTabModel();
        mCurrentTabModelFilterSupplier.set(mTabGroupModelFilter);
        doNothing().when(mTabGroupModelFilter).addObserver(mTabModelObserverCaptor.capture());
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION2);
        doReturn(POSITION1).when(mTabModel).indexOf(mTab1);
        doReturn(POSITION2).when(mTabModel).indexOf(mTab2);
        doReturn(POSITION1).when(mTabModel).index();
        doNothing().when(mTab1).addObserver(mTabObserverCaptor.capture());
        doReturn(0).when(mTabModel).index();
        doReturn(2).when(mTabModel).getCount();
        doNothing()
                .when(mTabListFaviconProvider)
                .getFaviconForUrlAsync(any(GURL.class), anyBoolean(), mCallbackCaptor.capture());
        doReturn(mFavicon)
                .when(mTabListFaviconProvider)
                .getFaviconFromBitmap(any(Bitmap.class), any(GURL.class));
        doNothing().when(mTabFaviconFetcher).fetch(mCallbackCaptor.capture());
        doReturn(mTabFaviconFetcher)
                .when(mTabListFaviconProvider)
                .getDefaultFaviconFetcher(anyBoolean());
        doReturn(mTabFaviconFetcher)
                .when(mTabListFaviconProvider)
                .getFaviconForUrlFetcher(any(GURL.class), anyBoolean());
        doReturn(mTabFaviconFetcher)
                .when(mTabListFaviconProvider)
                .getFaviconFromBitmapFetcher(any(Bitmap.class), any(GURL.class));
        doReturn(mTabFaviconFetcher)
                .when(mTabListFaviconProvider)
                .getComposedFaviconImageFetcher(any(), anyBoolean());
        doReturn(mTabFaviconFetcher)
                .when(mTabListFaviconProvider)
                .getFaviconFromTabGroupColorFetcher(anyInt(), anyBoolean());
        doReturn(2).when(mTabGroupModelFilter).getCount();
        doReturn(tabs1).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(tabs2).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(mOpenGroupActionListener)
                .when(mGridCardOnClickListenerProvider)
                .openTabGridDialog(any(Tab.class));
        doNothing().when(mActivity).registerComponentCallbacks(mComponentCallbacksCaptor.capture());
        doReturn(mGridLayoutManager).when(mRecyclerView).getLayoutManager();
        doReturn(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_COMPACT)
                .when(mGridLayoutManager)
                .getSpanCount();
        doReturn(mSpanSizeLookup).when(mGridLayoutManager).getSpanSizeLookup();
        doReturn(mTab1Domain)
                .when(mUrlUtilitiesJniMock)
                .getDomainAndRegistry(eq(TAB1_URL.getSpec()), anyBoolean());
        doReturn(mTab2Domain)
                .when(mUrlUtilitiesJniMock)
                .getDomainAndRegistry(eq(TAB2_URL.getSpec()), anyBoolean());
        doReturn(mTab3Domain)
                .when(mUrlUtilitiesJniMock)
                .getDomainAndRegistry(eq(TAB3_URL.getSpec()), anyBoolean());
        doNothing().when(mTemplateUrlService).addObserver(mTemplateUrlServiceObserver.capture());
        doReturn(true).when(mTabListFaviconProvider).isInitialized();

        mModel = new TabListModel();
        TemplateUrlServiceFactory.setInstanceForTesting(mTemplateUrlService);
        PriceTrackingFeatures.setPriceTrackingEnabledForTesting(false);

        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.GRID);

        doAnswer(
                        invocation -> {
                            int position = invocation.getArgument(0);
                            int itemType = mModel.get(position).type;
                            if (itemType == UiType.MESSAGE || itemType == UiType.LARGE_MESSAGE) {
                                return mGridLayoutManager.getSpanCount();
                            }
                            return 1;
                        })
                .when(mSpanSizeLookup)
                .getSpanSize(anyInt());
    }

    @After
    public void tearDown() {
        PseudoTab.clearForTesting();
        ProfileManager.resetForTesting();
    }

    private static SharedPreferences getGroupColorSharedPreferences() {
        return ContextUtils.getApplicationContext()
                .getSharedPreferences(TAB_GROUP_COLORS_FILE_NAME, Context.MODE_PRIVATE);
    }

    private static SharedPreferences getGroupTitleSharedPreferences() {
        return ContextUtils.getApplicationContext()
                .getSharedPreferences(TAB_GROUP_TITLES_FILE_NAME, Context.MODE_PRIVATE);
    }

    @Test
    public void initializesWithCurrentTabs() {
        initAndAssertAllProperties();
    }

    @Test
    public void updatesTitle_WithoutStoredTitle() {
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));

        when(mTitleProvider.getTitle(mActivity, PseudoTab.fromTab(mTab1))).thenReturn(NEW_TITLE);
        mTabObserver.onTitleUpdated(mTab1);

        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(NEW_TITLE));
    }

    @Test
    public void updatesTitle_WithStoredTitle_TabGroup() {
        // Mock that tab1 and new tab are in the same group with root ID as TAB1_ID.
        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, newTab));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        // Mock that we have a stored title stored with reference to root ID of tab1.
        getGroupTitleSharedPreferences()
                .edit()
                .putString(String.valueOf(mTab1.getRootId()), CUSTOMIZED_DIALOG_TITLE1)
                .apply();
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));

        mTabObserver.onTitleUpdated(mTab1);

        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(CUSTOMIZED_DIALOG_TITLE1));
    }

    @Test
    public void updatesFaviconFetcher_SingleTab_GTS() {
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));

        mTabObserver.onFaviconUpdated(mTab1, mFaviconBitmap, mFaviconUrl);

        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
        TabListFaviconProvider.TabFavicon[] favicon = new TabListFaviconProvider.TabFavicon[1];
        mModel.get(0)
                .model
                .get(TabProperties.FAVICON_FETCHER)
                .fetch(
                        tabFavicon -> {
                            favicon[0] = tabFavicon;
                        });
        mCallbackCaptor.getValue().onResult(mFavicon);
        assertEquals(favicon[0], mFavicon);
    }

    @Test
    public void updatesFaviconFetcher_SingleTabGroup_GTS() {
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));

        createTabGroup(Arrays.asList(mTab1), TAB1_ID, TAB_GROUP_ID);

        mTabObserver.onFaviconUpdated(mTab1, mFaviconBitmap, mFaviconUrl);

        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void updatesFaviconFetcher_SingleTab_NonGTS() {
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));

        mTabObserver.onFaviconUpdated(mTab1, mFaviconBitmap, mFaviconUrl);

        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
        TabListFaviconProvider.TabFavicon[] favicon = new TabListFaviconProvider.TabFavicon[1];
        mModel.get(0)
                .model
                .get(TabProperties.FAVICON_FETCHER)
                .fetch(
                        tabFavicon -> {
                            favicon[0] = tabFavicon;
                        });
        mCallbackCaptor.getValue().onResult(mFavicon);
        assertEquals(favicon[0], mFavicon);
    }

    @Test
    public void updatesFaviconFetcher_TabGroup_ListGTS() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);

        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        // Assert that tab1 is in a group.
        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        createTabGroup(Arrays.asList(mTab1, newTab), TAB1_ID, TAB_GROUP_ID);
        List<GURL> urls = Arrays.asList(mTab1.getUrl(), newTab.getUrl());

        mTabObserver.onFaviconUpdated(mTab1, mFaviconBitmap, mFaviconUrl);

        verify(mTabListFaviconProvider).getComposedFaviconImageFetcher(eq(urls), anyBoolean());
        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void updatesFaviconFetcher_TabGroup_ListGTS_SingleTab() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);

        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        // Assert that tab1 is in a group.
        createTabGroup(Arrays.asList(mTab1), TAB1_ID, TAB_GROUP_ID);
        List<GURL> urls = Arrays.asList(mTab1.getUrl());

        mTabObserver.onFaviconUpdated(mTab1, mFaviconBitmap, mFaviconUrl);

        verify(mTabListFaviconProvider).getComposedFaviconImageFetcher(eq(urls), anyBoolean());
        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void updatesFaviconFetcher_Navigation_NoOpSameDocument() {
        doReturn(mFavicon).when(mTabListFaviconProvider).getDefaultFavicon(anyBoolean());

        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));

        NavigationHandle navigationHandle = mock(NavigationHandle.class);
        when(navigationHandle.getUrl()).thenReturn(TAB2_URL);
        when(navigationHandle.isSameDocument()).thenReturn(true);

        mTabObserver.onDidStartNavigationInPrimaryMainFrame(mTab1, navigationHandle);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void updatesFaviconFetcher_Navigation_NoOpSameUrl() {
        doReturn(mFavicon).when(mTabListFaviconProvider).getDefaultFavicon(anyBoolean());

        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));

        NavigationHandle navigationHandle = mock(NavigationHandle.class);
        when(navigationHandle.getUrl()).thenReturn(TAB1_URL);
        when(navigationHandle.isSameDocument()).thenReturn(false);

        mTabObserver.onDidStartNavigationInPrimaryMainFrame(mTab1, navigationHandle);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void updatesFaviconFetcher_Navigation_NoOpNtpUrl() {
        doReturn(mFavicon).when(mTabListFaviconProvider).getDefaultFavicon(anyBoolean());

        GURL ntpUrl = JUnitTestGURLs.NTP_URL;
        doReturn("")
                .when(mUrlUtilitiesJniMock)
                .getDomainAndRegistry(eq(ntpUrl.getSpec()), anyBoolean());

        NavigationHandle navigationHandle = mock(NavigationHandle.class);
        when(navigationHandle.getUrl()).thenReturn(TAB2_URL);
        when(navigationHandle.isSameDocument()).thenReturn(false);

        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, ntpUrl);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(newTab).when(mTabGroupModelFilter).getTabAt(2);
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(Arrays.asList(newTab)).when(mTabGroupModelFilter).getRelatedTabList(eq(TAB3_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab, TabLaunchType.FROM_CHROME_UI, TabCreationState.LIVE_IN_FOREGROUND, false);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(2).model.get(TabProperties.TAB_ID), equalTo(TAB3_ID));
        assertThat(mModel.get(2).model.get(TabProperties.TITLE), equalTo(TAB3_TITLE));

        mModel.get(2).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(2).model.get(TabProperties.FAVICON_FETCHER));

        mTabObserver.onDidStartNavigationInPrimaryMainFrame(newTab, navigationHandle);
        assertNull(mModel.get(2).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void updatesFaviconFetcher_Navigation() {
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));

        NavigationHandle navigationHandle = mock(NavigationHandle.class);

        when(navigationHandle.isSameDocument()).thenReturn(false);
        when(navigationHandle.getUrl()).thenReturn(TAB2_URL);
        mTabObserver.onDidStartNavigationInPrimaryMainFrame(mTab1, navigationHandle);

        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void updatesFaviconFetcher_Navigation_NoOpTabGroup() {
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
        when(mTabGroupModelFilter.isTabInTabGroup(mTab1)).thenReturn(true);

        NavigationHandle navigationHandle = mock(NavigationHandle.class);

        when(navigationHandle.isSameDocument()).thenReturn(false);
        when(navigationHandle.getUrl()).thenReturn(TAB2_URL);
        mTabObserver.onDidStartNavigationInPrimaryMainFrame(mTab1, navigationHandle);

        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void sendsSelectSignalCorrectly() {
        mModel.get(1)
                .model
                .get(TabProperties.TAB_SELECTED_LISTENER)
                .run(mModel.get(1).model.get(TabProperties.TAB_ID));

        verify(mGridCardOnClickListenerProvider)
                .onTabSelecting(mModel.get(1).model.get(TabProperties.TAB_ID), true);
    }

    @Test
    public void sendsOpenGroupSignalCorrectly_SingleTabGroup() {
        List<Tab> tabs = Arrays.asList(mTab1);
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        mMediator.resetWithListOfTabs(
                PseudoTab.getListOfPseudoTab(Arrays.asList(mTab1, mTab2)), false);
        mModel.get(0)
                .model
                .get(TabProperties.TAB_SELECTED_LISTENER)
                .run(mModel.get(0).model.get(TabProperties.TAB_ID));

        verify(mOpenGroupActionListener).run(TAB1_ID);
    }

    @Test
    public void sendsOpenGroupSignalCorrectly_TabGroup() {
        List<Tab> tabs = Arrays.asList(mTab1, mTab2);
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        mMediator.resetWithListOfTabs(
                PseudoTab.getListOfPseudoTab(Arrays.asList(mTab1, mTab2)), false);
        mModel.get(0)
                .model
                .get(TabProperties.TAB_SELECTED_LISTENER)
                .run(mModel.get(0).model.get(TabProperties.TAB_ID));

        verify(mOpenGroupActionListener).run(TAB1_ID);
    }

    @Test
    public void sendsCloseSignalCorrectly() {
        mModel.get(1)
                .model
                .get(TabProperties.TAB_CLOSED_LISTENER)
                .run(mModel.get(1).model.get(TabProperties.TAB_ID));

        verify(mTabModel).closeTab(eq(mTab2), eq(null), eq(false), eq(false), eq(true));
    }

    @Test
    public void sendsMoveTabSignalCorrectlyWithGroup() {
        TabGridItemTouchHelperCallback itemTouchHelperCallback = getItemTouchHelperCallback();
        itemTouchHelperCallback.setActionsOnAllRelatedTabsForTesting(true);

        itemTouchHelperCallback.onMove(mRecyclerView, mViewHolder1, mViewHolder2);

        verify(mTabGroupModelFilter).moveRelatedTabs(eq(TAB1_ID), eq(2));
    }

    @Test
    public void sendsMoveTabSignalCorrectlyWithinGroup() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        getItemTouchHelperCallback().onMove(mRecyclerView, mViewHolder1, mViewHolder2);

        verify(mTabModel).moveTab(eq(TAB1_ID), eq(2));
    }

    @Test
    public void sendsMergeTabSignalCorrectly() {
        TabGridItemTouchHelperCallback itemTouchHelperCallback = getItemTouchHelperCallback();
        itemTouchHelperCallback.setActionsOnAllRelatedTabsForTesting(true);
        itemTouchHelperCallback.setHoveredTabIndexForTesting(POSITION1);
        itemTouchHelperCallback.setSelectedTabIndexForTesting(POSITION2);
        itemTouchHelperCallback.getMovementFlags(mRecyclerView, mFakeViewHolder1);

        doReturn(mAdapter).when(mRecyclerView).getAdapter();

        // Simulate the drop action.
        itemTouchHelperCallback.onSelectedChanged(
                mFakeViewHolder2, ItemTouchHelper.ACTION_STATE_IDLE);

        verify(mTabGroupModelFilter).mergeTabsToGroup(eq(TAB2_ID), eq(TAB1_ID));
        verify(mGridLayoutManager).removeView(mItemView2);
        verify(mTracker).notifyEvent(eq(EventConstants.TAB_DRAG_AND_DROP_TO_GROUP));
    }

    // Regression test for https://crbug.com/1372487
    @Test
    public void handlesGroupMergeCorrectly_InOrder() {
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        Tab tab4 = prepareTab(TAB4_ID, TAB4_TITLE, TAB4_URL);
        when(mTabModel.getTabAt(2)).thenReturn(tab3);
        when(mTabModel.getTabAt(3)).thenReturn(tab4);
        View itemView3 = mock(View.class);
        View itemView4 = mock(View.class);
        RecyclerView.ViewHolder fakeViewHolder3 = prepareFakeViewHolder(itemView3, 2);
        RecyclerView.ViewHolder fakeViewHolder4 = prepareFakeViewHolder(itemView4, 3);

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3, tab4));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(4));

        // Merge 2 to 1.
        TabGridItemTouchHelperCallback itemTouchHelperCallback = getItemTouchHelperCallback();
        itemTouchHelperCallback.setActionsOnAllRelatedTabsForTesting(true);
        itemTouchHelperCallback.setHoveredTabIndexForTesting(POSITION1);
        itemTouchHelperCallback.setSelectedTabIndexForTesting(POSITION2);
        itemTouchHelperCallback.getMovementFlags(mRecyclerView, mFakeViewHolder1);

        doReturn(mAdapter).when(mRecyclerView).getAdapter();

        itemTouchHelperCallback.onSelectedChanged(
                mFakeViewHolder2, ItemTouchHelper.ACTION_STATE_IDLE);

        verify(mTabGroupModelFilter).mergeTabsToGroup(eq(TAB2_ID), eq(TAB1_ID));
        verify(mGridLayoutManager).removeView(mItemView2);
        verify(mTracker).notifyEvent(eq(EventConstants.TAB_DRAG_AND_DROP_TO_GROUP));

        when(mTabGroupModelFilter.getRelatedTabList(TAB2_ID))
                .thenReturn(Arrays.asList(mTab1, mTab2));
        when(mTabModel.indexOf(mTab1)).thenReturn(POSITION1);
        when(mTabModel.indexOf(mTab2)).thenReturn(POSITION2);
        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(mTab2, TAB1_ID);

        assertThat(mModel.size(), equalTo(3));
        mFakeViewHolder1 = prepareFakeViewHolder(mItemView1, 0);
        fakeViewHolder3 = prepareFakeViewHolder(itemView3, 1);
        fakeViewHolder4 = prepareFakeViewHolder(itemView4, 2);

        // Merge 4 to 3.
        when(mTabGroupModelFilter.getTabAt(1)).thenReturn(tab3);
        when(mTabGroupModelFilter.getTabAt(2)).thenReturn(tab4);
        itemTouchHelperCallback.setHoveredTabIndexForTesting(1);
        itemTouchHelperCallback.setSelectedTabIndexForTesting(2);
        itemTouchHelperCallback.getMovementFlags(mRecyclerView, fakeViewHolder3);

        itemTouchHelperCallback.onSelectedChanged(
                fakeViewHolder4, ItemTouchHelper.ACTION_STATE_IDLE);

        verify(mTabGroupModelFilter).mergeTabsToGroup(eq(TAB4_ID), eq(TAB3_ID));
        verify(mGridLayoutManager).removeView(itemView4);
        verify(mTracker, times(2)).notifyEvent(eq(EventConstants.TAB_DRAG_AND_DROP_TO_GROUP));

        when(mTabGroupModelFilter.getRelatedTabList(TAB4_ID)).thenReturn(Arrays.asList(tab3, tab4));
        when(mTabGroupModelFilter.getRelatedTabList(TAB3_ID)).thenReturn(Arrays.asList(tab3, tab4));
        when(mTabModel.indexOf(tab3)).thenReturn(2);
        when(mTabModel.indexOf(tab4)).thenReturn(3);
        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(tab4, TAB3_ID);

        assertThat(mModel.size(), equalTo(2));
        mFakeViewHolder1 = prepareFakeViewHolder(mItemView1, 0);
        fakeViewHolder3 = prepareFakeViewHolder(itemView3, 1);

        // Merge 3 to 1.
        when(mTabGroupModelFilter.getTabAt(0)).thenReturn(mTab1);
        when(mTabGroupModelFilter.getTabAt(1)).thenReturn(tab3);
        itemTouchHelperCallback.setHoveredTabIndexForTesting(0);
        itemTouchHelperCallback.setSelectedTabIndexForTesting(1);
        itemTouchHelperCallback.getMovementFlags(mRecyclerView, mFakeViewHolder1);

        itemTouchHelperCallback.onSelectedChanged(
                fakeViewHolder3, ItemTouchHelper.ACTION_STATE_IDLE);

        verify(mTabGroupModelFilter).mergeTabsToGroup(eq(TAB3_ID), eq(TAB1_ID));
        verify(mGridLayoutManager).removeView(itemView3);
        verify(mTracker, times(3)).notifyEvent(eq(EventConstants.TAB_DRAG_AND_DROP_TO_GROUP));

        when(mTabGroupModelFilter.getRelatedTabList(TAB3_ID))
                .thenReturn(Arrays.asList(mTab1, mTab2, tab3, tab4));
        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(tab3, TAB1_ID);

        assertThat(mModel.size(), equalTo(1));
    }

    @Test
    public void sendsUngroupSignalCorrectly() {
        TabGridItemTouchHelperCallback itemTouchHelperCallback = getItemTouchHelperCallback();
        itemTouchHelperCallback.setActionsOnAllRelatedTabsForTesting(false);
        itemTouchHelperCallback.setUnGroupTabIndexForTesting(POSITION1);
        itemTouchHelperCallback.getMovementFlags(mRecyclerView, mFakeViewHolder1);

        doReturn(mAdapter).when(mRecyclerView).getAdapter();
        doReturn(1).when(mAdapter).getItemCount();

        // Simulate the ungroup action.
        itemTouchHelperCallback.onSelectedChanged(
                mFakeViewHolder1, ItemTouchHelper.ACTION_STATE_IDLE);

        verify(mTabGroupModelFilter).moveTabOutOfGroup(eq(TAB1_ID));
        verify(mGridLayoutManager).removeView(mItemView1);
    }

    @Test
    public void tabClosure() {
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.willCloseTab(mTab2, false, true);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
    }

    @Test
    public void tabClosure_IgnoresUpdatesForTabsOutsideOfModel() {
        mMediatorTabModelObserver.willCloseTab(
                prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL), false, true);

        assertThat(mModel.size(), equalTo(2));
    }

    @Test
    @EnableFeatures(INSTANT_START)
    public void tabAddition_RestoreNotComplete() {
        mMediator.resetWithListOfTabs(null, false);

        // Mock that tab restoring stage is started.
        doReturn(false).when(mTabGroupModelFilter).isTabModelRestored();
        initAndAssertAllProperties();

        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(newTab).when(mTabGroupModelFilter).getTabAt(2);
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(Arrays.asList(newTab)).when(mTabGroupModelFilter).getRelatedTabList(eq(TAB3_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab, TabLaunchType.FROM_RESTORE, TabCreationState.LIVE_IN_FOREGROUND, false);

        // When tab restoring stage is not yet finished, this tab info should not be added to
        // property model.
        assertThat(mModel.size(), equalTo(2));
    }

    @Test
    @EnableFeatures(INSTANT_START)
    public void tabAddition_Restore() {
        mMediator.resetWithListOfTabs(null, false);

        // Mock that tab restoring stage is started.
        doReturn(false).when(mTabGroupModelFilter).isTabModelRestored();
        initAndAssertAllProperties();
        // Mock that tab restoring stage is over.
        doReturn(true).when(mTabGroupModelFilter).isTabModelRestored();
        TabListMediator.TabActionListener actionListenerBeforeUpdate =
                mModel.get(1).model.get(TabProperties.TAB_SELECTED_LISTENER);

        // Mock that newTab was in the same group with tab, and now it is restored.
        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = Arrays.asList(mTab2, newTab);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(2).when(mTabGroupModelFilter).getCount();
        doReturn(1).when(mTabGroupModelFilter).indexOf(newTab);
        doReturn(tabs).when(mTabGroupModelFilter).getRelatedTabList(eq(TAB3_ID));
        doReturn(tabs).when(mTabGroupModelFilter).getRelatedTabList(eq(TAB2_ID));
        doReturn(true).when(mTabGroupModelFilter).isTabInTabGroup(mTab2);
        doReturn(true).when(mTabGroupModelFilter).isTabInTabGroup(newTab);
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab, TabLaunchType.FROM_RESTORE, TabCreationState.LIVE_IN_FOREGROUND, false);

        TabListMediator.TabActionListener actionListenerAfterUpdate =
                mModel.get(1).model.get(TabProperties.TAB_SELECTED_LISTENER);
        // The selection listener should be updated which indicates that corresponding property
        // model is updated.
        assertThat(actionListenerBeforeUpdate, not(actionListenerAfterUpdate));
        assertThat(mModel.size(), equalTo(2));
    }

    @Test
    public void tabAddition_Restore_SyncingTabListModelWithTabModel() {
        // Mock that tab1 and tab2 are in the same group, and they are being restored. The
        // TabListModel has been cleaned out before the restoring happens. This case could happen
        // within a incognito tab group when user switches between light/dark mode.
        createTabGroup(new ArrayList<>(Arrays.asList(mTab1, mTab2)), TAB1_ID, TAB_GROUP_ID);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(1).when(mTabGroupModelFilter).getCount();
        mModel.clear();

        mMediatorTabModelObserver.didAddTab(
                mTab2, TabLaunchType.FROM_RESTORE, TabCreationState.LIVE_IN_FOREGROUND, false);
        assertThat(mModel.size(), equalTo(0));

        mMediatorTabModelObserver.didAddTab(
                mTab1, TabLaunchType.FROM_RESTORE, TabCreationState.LIVE_IN_FOREGROUND, false);
        assertThat(mModel.size(), equalTo(1));
    }

    @Test
    public void tabAddition_GTS() {
        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(newTab).when(mTabGroupModelFilter).getTabAt(2);
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(Arrays.asList(newTab)).when(mTabGroupModelFilter).getRelatedTabList(eq(TAB3_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab,
                TabLaunchType.FROM_TAB_SWITCHER_UI,
                TabCreationState.LIVE_IN_FOREGROUND,
                false);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(2).model.get(TabProperties.TAB_ID), equalTo(TAB3_ID));
        assertThat(mModel.get(2).model.get(TabProperties.TITLE), equalTo(TAB3_TITLE));
    }

    @Test
    public void tabAddition_GTS_delayAdd() {
        mMediator.setComponentNameForTesting(TabSwitcherCoordinator.COMPONENT_NAME);
        initAndAssertAllProperties();

        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(newTab).when(mTabGroupModelFilter).getTabAt(2);
        doReturn(Arrays.asList(mTab1, mTab2, newTab))
                .when(mTabGroupModelFilter)
                .getRelatedTabList(TAB1_ID);
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(Arrays.asList(newTab)).when(mTabGroupModelFilter).getRelatedTabList(eq(TAB3_ID));
        assertThat(mModel.size(), equalTo(2));

        // Add tab marked as delayed
        mMediatorTabModelObserver.didAddTab(
                newTab,
                TabLaunchType.FROM_TAB_SWITCHER_UI,
                TabCreationState.LIVE_IN_FOREGROUND,
                true);

        // Verify tab did not get added and delayed tab is captured.
        assertThat(mModel.size(), equalTo(2));
        assertThat(mMediator.getTabToAddDelayedForTesting(), equalTo(newTab));

        // Select delayed tab
        mMediatorTabModelObserver.didSelectTab(newTab, TabSelectionType.FROM_USER, mTab1.getId());
        // Assert old tab is still marked as selected
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));

        // Hide GTS to complete tab addition and selection
        mMediator.postHiding();
        // Assert tab added and selected. Assert old tab is de-selected.
        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(2).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertNull(mMediator.getTabToAddDelayedForTesting());
    }

    @Test
    public void tabAddition_GTS_Skip() {
        // Add a new tab to the group with mTab2.
        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(2).when(mTabGroupModelFilter).getCount();
        doReturn(Arrays.asList(mTab2, newTab))
                .when(mTabGroupModelFilter)
                .getRelatedTabList(eq(TAB3_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab,
                TabLaunchType.FROM_TAB_SWITCHER_UI,
                TabCreationState.LIVE_IN_FOREGROUND,
                false);

        assertThat(mModel.size(), equalTo(2));
    }

    @Test
    public void tabAddition_GTS_Middle() {
        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(newTab).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(2);
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(Arrays.asList(newTab)).when(mTabGroupModelFilter).getRelatedTabList(eq(TAB3_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab, TabLaunchType.FROM_CHROME_UI, TabCreationState.LIVE_IN_FOREGROUND, false);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB3_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB3_TITLE));
    }

    @Test
    public void tabAddition_Dialog_End() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        doReturn(true).when(mTabGroupModelFilter).isTabModelRestored();

        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        doReturn(3).when(mTabModel).getCount();
        doReturn(Arrays.asList(mTab1, mTab2, newTab))
                .when(mTabGroupModelFilter)
                .getRelatedTabList(eq(TAB1_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab, TabLaunchType.FROM_CHROME_UI, TabCreationState.LIVE_IN_FOREGROUND, false);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(2).model.get(TabProperties.TAB_ID), equalTo(TAB3_ID));
        assertThat(mModel.get(2).model.get(TabProperties.TITLE), equalTo(TAB3_TITLE));
    }

    @Test
    public void tabAddition_Dialog_Middle() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        doReturn(true).when(mTabGroupModelFilter).isTabModelRestored();

        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        doReturn(3).when(mTabModel).getCount();
        doReturn(Arrays.asList(mTab1, newTab, mTab2))
                .when(mTabGroupModelFilter)
                .getRelatedTabList(eq(TAB1_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab, TabLaunchType.FROM_CHROME_UI, TabCreationState.LIVE_IN_FOREGROUND, false);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB3_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB3_TITLE));
    }

    @Test
    public void tabAddition_Dialog_Skip() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        doReturn(true).when(mTabGroupModelFilter).isTabModelRestored();

        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        // newTab is of another group.
        doReturn(Arrays.asList(mTab1, mTab2))
                .when(mTabGroupModelFilter)
                .getRelatedTabList(eq(TAB1_ID));
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.didAddTab(
                newTab, TabLaunchType.FROM_CHROME_UI, TabCreationState.LIVE_IN_FOREGROUND, false);

        assertThat(mModel.size(), equalTo(2));
    }

    @Test
    public void tabSelection() {
        ThumbnailFetcher tab1Fetcher = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);
        ThumbnailFetcher tab2Fetcher = mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER);

        mMediatorTabModelObserver.didSelectTab(mTab2, TabLaunchType.FROM_CHROME_UI, TAB1_ID);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertNotEquals(mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER), tab1Fetcher);
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertNotEquals(mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER), tab2Fetcher);
    }

    @Test
    public void tabSelection_updatePreviousSelectedTabThumbnailFetcher() {
        mMediator =
                new TabListMediator(
                        mActivity,
                        mModel,
                        TabListMode.GRID,
                        mCurrentTabModelFilterSupplier,
                        () -> mTabModel,
                        getTabThumbnailCallback(),
                        mTitleProvider,
                        mTabListFaviconProvider,
                        true,
                        null,
                        mGridCardOnClickListenerProvider,
                        null,
                        null,
                        getClass().getSimpleName(),
                        UiType.CLOSABLE);
        mMediator.initWithNative(mProfile);

        // mTabModelObserverCaptor captures on every initWithNative call.
        verify(mTabGroupModelFilter, times(2)).addObserver(mTabModelObserverCaptor.capture());
        initAndAssertAllProperties();

        ThumbnailFetcher tab1Fetcher = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);
        ThumbnailFetcher tab2Fetcher = mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER);

        mMediatorTabModelObserver.didSelectTab(mTab2, TabLaunchType.FROM_CHROME_UI, TAB1_ID);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertNotEquals(tab1Fetcher, mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertNotEquals(tab2Fetcher, mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void tabClosureUndone() {
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabModelObserver.willCloseTab(mTab2, false, true);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));

        mMediatorTabModelObserver.tabClosureUndone(mTab2);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
    }

    @Test
    public void tabClosureUndone_SingleTabGroup() {
        assertThat(mModel.size(), equalTo(2));

        createTabGroup(Arrays.asList(mTab2), TAB2_ID, TAB_GROUP_ID);

        mMediator.getTabGroupTitleEditor().storeTabGroupTitle(TAB2_ID, CUSTOMIZED_DIALOG_TITLE1);
        mMediatorTabModelObserver.willCloseTab(mTab2, false, true);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));

        mMediatorTabModelObserver.tabClosureUndone(mTab2);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(CUSTOMIZED_DIALOG_TITLE1));
    }

    @Test
    public void tabMergeIntoGroup() {
        // Assume that moveTab in TabModel is finished. Selected tab in the group becomes mTab1.
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION2);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION1);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);

        // Assume that reset in TabGroupModelFilter is finished.
        createTabGroup(Arrays.asList(mTab1, mTab2), TAB1_ID, TAB_GROUP_ID);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
        assertThat(mModel.indexFromId(TAB1_ID), equalTo(POSITION1));
        assertThat(mModel.indexFromId(TAB2_ID), equalTo(POSITION2));
        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
        assertNotNull(mModel.get(1).model.get(TabProperties.FAVICON_FETCHER));

        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(mTab1, TAB2_ID);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void tabMoveOutOfGroup_GTS_Moved_Tab_Selected() {
        // Assume that two tabs are in the same group before ungroup.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab2));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        // Assume that TabGroupModelFilter is already updated.
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(false).when(mTabGroupModelFilter).isTabInTabGroup(mTab1);
        doReturn(2).when(mTabGroupModelFilter).getCount();

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab1, POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));
    }

    @Test
    public void tabMoveOutOfGroup_GTS_Origin_Tab_Selected() {
        // Assume that two tabs are in the same group before ungroup.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));

        // Assume that TabGroupModelFilter is already updated.
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(2).when(mTabGroupModelFilter).getCount();

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab2, POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(false));
    }

    @Test
    public void tabMoveOutOfGroup_GTS_LastTab() {
        // Assume that tab1 is a single tab group that became a single tab.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        doReturn(1).when(mTabGroupModelFilter).getCount();
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(tabs).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);

        // These properties should get reset.
        mModel.get(0).model.set(TabProperties.TITLE, CUSTOMIZED_DIALOG_TITLE1);
        ThumbnailFetcher fetcher = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);

        // Ungroup the single tab.
        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab1, POSITION1);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertNotEquals(fetcher, mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void tabMoveOutOfGroup_GTS_TabAdditionWithSameId() {
        // Assume that two tabs are in the same group before ungroup.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));

        // Assume that TabGroupModelFilter is already updated.
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(2).when(mTabGroupModelFilter).getCount();

        // The ungroup will add tab1 to the TabListModel at index 0. Note that before this addition,
        // there is the PropertyModel represents the group with the same id at the same index. The
        // addition should still take effect in this case.
        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab1, POSITION2);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(false));
    }

    @Test
    public void testShoppingFetcherActiveForForUngroupedTabs() {
        prepareForPriceDrop();
        resetWithRegularTabs(false);

        assertThat(mModel.size(), equalTo(2));
        assertThat(
                mModel.get(0).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER),
                instanceOf(TabListMediator.ShoppingPersistedTabDataFetcher.class));
        assertThat(
                mModel.get(1).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER),
                instanceOf(TabListMediator.ShoppingPersistedTabDataFetcher.class));
    }

    @Test
    public void testShoppingFetcherInactiveForForGroupedTabs() {
        prepareForPriceDrop();
        resetWithRegularTabs(true);

        assertThat(mModel.size(), equalTo(2));
        assertNull(mModel.get(0).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
        assertNull(mModel.get(1).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
    }

    @Test
    public void testShoppingFetcherGroupedThenUngrouped() {
        prepareForPriceDrop();
        resetWithRegularTabs(true);

        assertThat(mModel.size(), equalTo(2));
        assertNull(mModel.get(0).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
        assertNull(mModel.get(1).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
        resetWithRegularTabs(false);
        assertThat(mModel.size(), equalTo(2));
        assertThat(
                mModel.get(0).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER),
                instanceOf(TabListMediator.ShoppingPersistedTabDataFetcher.class));
        assertThat(
                mModel.get(1).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER),
                instanceOf(TabListMediator.ShoppingPersistedTabDataFetcher.class));
    }

    @Test
    public void testShoppingFetcherUngroupedThenGrouped() {
        prepareForPriceDrop();
        resetWithRegularTabs(false);

        assertThat(mModel.size(), equalTo(2));
        assertThat(
                mModel.get(0).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER),
                instanceOf(TabListMediator.ShoppingPersistedTabDataFetcher.class));
        assertThat(
                mModel.get(1).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER),
                instanceOf(TabListMediator.ShoppingPersistedTabDataFetcher.class));
        resetWithRegularTabs(true);
        assertThat(mModel.size(), equalTo(2));
        assertNull(mModel.get(0).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
        assertNull(mModel.get(1).model.get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
    }

    /** Set flags and initialize for verifying price drop behavior */
    private void prepareForPriceDrop() {
        setPriceTrackingEnabledForTesting(true);
        PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(true);
        PersistedTabDataConfiguration.setUseTestConfig(true);
        initAndAssertAllProperties();
    }

    /**
     * Reset mediator with non-incognito tabs which are optionally grouped
     *
     * @param isGrouped true if the tabs should be grouped
     */
    private void resetWithRegularTabs(boolean isGrouped) {
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(0);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(1);
        doReturn(2).when(mTabGroupModelFilter).getCount();
        if (isGrouped) {
            doReturn(Arrays.asList(mTab1, mTab2))
                    .when(mTabGroupModelFilter)
                    .getRelatedTabList(eq(TAB1_ID));
            doReturn(Arrays.asList(mTab1, mTab2))
                    .when(mTabGroupModelFilter)
                    .getRelatedTabList(eq(TAB2_ID));
            doReturn(true).when(mTabGroupModelFilter).isTabInTabGroup(mTab1);
            doReturn(true).when(mTabGroupModelFilter).isTabInTabGroup(mTab2);
        } else {
            doReturn(Arrays.asList(mTab1))
                    .when(mTabGroupModelFilter)
                    .getRelatedTabList(eq(TAB1_ID));
            doReturn(Arrays.asList(mTab2))
                    .when(mTabGroupModelFilter)
                    .getRelatedTabList(eq(TAB2_ID));
            doReturn(false).when(mTabGroupModelFilter).isTabInTabGroup(mTab1);
            doReturn(false).when(mTabGroupModelFilter).isTabInTabGroup(mTab2);
        }
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        doReturn(false).when(mTab1).isIncognito();
        doReturn(false).when(mTab2).isIncognito();
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
    }

    @Test
    public void tabMoveOutOfGroup_Dialog() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        // Assume that filter is already updated.
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab1, POSITION1);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
        verify(mTabGridDialogHandler).updateDialogContent(TAB2_ID);
    }

    @Test
    public void tabMoveOutOfGroup_Dialog_LastTab() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        // Assume that tab1 is a single tab.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        doReturn(1).when(mTabGroupModelFilter).getCount();
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(tabs).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);

        // Ungroup the single tab.
        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab1, POSITION1);

        verify(mTabGridDialogHandler).updateDialogContent(Tab.INVALID_TAB_ID);
    }

    @Test
    public void tabMoveOutOfGroup_Strip() {
        setUpTabListMediator(TabListMediatorType.TAB_STRIP, TabListMode.GRID);

        // Assume that filter is already updated.
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab1, POSITION1);

        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
        verify(mTabGridDialogHandler, never()).updateDialogContent(anyInt());
    }

    @Test
    public void tabMovementWithGroup_Forward() {
        // Assume that moveTab in TabModel is finished.
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION2);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        mMediatorTabGroupModelFilterObserver.didMoveTabGroup(mTab2, POSITION2, POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
    }

    @Test
    public void tabMovementWithGroup_Backward() {
        // Assume that moveTab in TabModel is finished.
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION2);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        mMediatorTabGroupModelFilterObserver.didMoveTabGroup(mTab1, POSITION1, POSITION2);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
    }

    @Test
    public void tabMovementWithinGroup_TabGridDialog_Forward() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        // Assume that moveTab in TabModel is finished.
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION2);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION1);
        doReturn(TAB1_ID).when(mTab1).getRootId();
        doReturn(TAB1_ID).when(mTab2).getRootId();

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        mMediatorTabGroupModelFilterObserver.didMoveWithinGroup(mTab2, POSITION2, POSITION1);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
    }

    @Test
    public void tabMovementWithinGroup_TabGridDialog_Backward() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        // Assume that moveTab in TabModel is finished.
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION2);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION1);
        doReturn(TAB1_ID).when(mTab1).getRootId();
        doReturn(TAB1_ID).when(mTab2).getRootId();

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        mMediatorTabGroupModelFilterObserver.didMoveWithinGroup(mTab1, POSITION1, POSITION2);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));
    }

    @Test
    public void tabMovementWithinGroup_TabSwitcher_Forward() {
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);

        // Setup three tabs with groups (mTab1, mTab2) and tab3.
        List<Tab> group = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        List<Integer> groupTabIds = new ArrayList<>(Arrays.asList(TAB1_ID, TAB2_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(tab3).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(tab3);
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION2);
        doReturn(tab3).when(mTabModel).getTabAt(2);
        doReturn(TAB1_ID).when(mTab1).getRootId();
        doReturn(TAB1_ID).when(mTab2).getRootId();
        doReturn(TAB3_ID).when(tab3).getRootId();

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, tab3));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(2));

        // Select tab3 so the group doesn't have the selected tab.
        doReturn(2).when(mTabModel).index();
        mMediatorTabModelObserver.didSelectTab(tab3, TabLaunchType.FROM_CHROME_UI, TAB1_ID);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB3_ID));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));

        // Assume that moveTab in TabModel is finished (swap mTab1 and mTab2).
        group = new ArrayList<>(Arrays.asList(mTab2, mTab1));
        groupTabIds = new ArrayList<>(Arrays.asList(TAB2_ID, TAB1_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION2);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION1);

        // mTab1 is first in group before the move.
        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        ThumbnailFetcher tab1Fetcher = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);

        mMediatorTabGroupModelFilterObserver.didMoveWithinGroup(mTab2, POSITION2, POSITION1);

        // mTab1 is still first in group after the move (last selected), but the thumbnail updated.
        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        // TODO(crbug/1374361): Make this an assertion and don't update.
        // Thumbnail order was: tab1, tab2, tab3. Now: tab1, tab2, tab3. Update is precautionary.
        assertNotEquals(tab1Fetcher, mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void tabMovementWithinGroup_TabSwitcher_Backward() {
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);

        // Setup three tabs with groups (mTab1, mTab2) and tab3.
        List<Tab> group = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        List<Integer> groupTabIds = new ArrayList<>(Arrays.asList(TAB1_ID, TAB2_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(tab3).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(tab3);
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION2);
        doReturn(tab3).when(mTabModel).getTabAt(2);
        doReturn(TAB1_ID).when(mTab1).getRootId();
        doReturn(TAB1_ID).when(mTab2).getRootId();
        doReturn(TAB3_ID).when(tab3).getRootId();

        // Select tab3 so the group doesn't have the selected tab.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, tab3));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(2));

        doReturn(2).when(mTabModel).index();
        mMediatorTabModelObserver.didSelectTab(tab3, TabLaunchType.FROM_CHROME_UI, TAB1_ID);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB3_ID));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));

        // Assume that moveTab in TabModel is finished (swap mTab1 and mTab2).
        group = new ArrayList<>(Arrays.asList(mTab2, mTab1));
        groupTabIds = new ArrayList<>(Arrays.asList(TAB2_ID, TAB1_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION2);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION1);

        // mTab1 is first in group before the move.
        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        ThumbnailFetcher tab1Fetcher = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);

        mMediatorTabGroupModelFilterObserver.didMoveWithinGroup(mTab1, POSITION1, POSITION2);

        // mTab1 is first in group after the move (last selected), but the thumbnail updated.
        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        // TODO(crbug/1374361): Make this an assertion and don't update.
        // Thumbnail order was: tab1, tab2, tab3. Now: tab1, tab2, tab3. Update is precautionary.
        assertNotEquals(tab1Fetcher, mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void tabMovementWithinGroup_TabSwitcher_SelectedNotMoved() {
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);

        // Setup three tabs grouped together.
        List<Tab> group = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3));
        List<Integer> groupTabIds = new ArrayList<>(Arrays.asList(TAB1_ID, TAB2_ID, TAB3_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB3_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB3_ID);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(tab3);
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION2);
        doReturn(tab3).when(mTabModel).getTabAt(2);
        doReturn(TAB1_ID).when(mTab1).getRootId();
        doReturn(TAB1_ID).when(mTab2).getRootId();
        doReturn(TAB1_ID).when(tab3).getRootId();

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(1));

        // Assume that moveTab in TabModel is finished.

        // mTab1 selected.
        doReturn(POSITION1).when(mTabModel).index();
        mMediatorTabModelObserver.didSelectTab(mTab1, TabLaunchType.FROM_CHROME_UI, TAB1_ID);

        // Swap mTab2 and tab3.
        doReturn(mTab2).when(mTabModel).getTabAt(2);
        doReturn(tab3).when(mTabModel).getTabAt(POSITION2);
        group = new ArrayList<>(Arrays.asList(mTab1, tab3, mTab2));
        groupTabIds = new ArrayList<>(Arrays.asList(TAB1_ID, TAB3_ID, TAB2_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB3_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB3_ID);

        // mTab1 selected before update.
        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        ThumbnailFetcher tab1Fetcher = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);

        mMediatorTabGroupModelFilterObserver.didMoveWithinGroup(mTab2, POSITION2, 2);

        // mTab1 still selected after the move (last selected), but the thumbnail updated.
        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        // TODO(crbug/1374361): Make this an assertion.
        // Thumbnail order was: tab1, tab2, tab3. Now: tab1, tab3, tab2.
        assertNotEquals(tab1Fetcher, mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void tabMovementWithinGroup_TabSwitcher_SelectedMoved() {
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);

        // Setup three tabs grouped together.
        List<Tab> group = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3));
        List<Integer> groupTabIds = new ArrayList<>(Arrays.asList(TAB1_ID, TAB2_ID, TAB3_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB3_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB3_ID);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(tab3);
        doReturn(mTab1).when(mTabModel).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabModel).getTabAt(POSITION2);
        doReturn(tab3).when(mTabModel).getTabAt(2);
        doReturn(TAB1_ID).when(mTab1).getRootId();
        doReturn(TAB1_ID).when(mTab2).getRootId();
        doReturn(TAB1_ID).when(tab3).getRootId();

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(1));

        // Assume that moveTab in TabModel is finished.

        // mTab1 selected (at new position).
        doReturn(2).when(mTabModel).index();
        mMediatorTabModelObserver.didSelectTab(mTab1, TabLaunchType.FROM_CHROME_UI, TAB1_ID);

        // Swap mTab1 and mTab3.
        doReturn(mTab1).when(mTabModel).getTabAt(2);
        doReturn(tab3).when(mTabModel).getTabAt(POSITION1);
        group = new ArrayList<>(Arrays.asList(tab3, mTab2, mTab1));
        groupTabIds = new ArrayList<>(Arrays.asList(TAB3_ID, TAB2_ID, TAB1_ID));
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB1_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB2_ID);
        doReturn(group).when(mTabGroupModelFilter).getRelatedTabList(TAB3_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB1_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB2_ID);
        doReturn(groupTabIds).when(mTabGroupModelFilter).getRelatedTabIds(TAB3_ID);

        // mTab1 selected before update.
        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        ThumbnailFetcher tab1Fetcher = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);

        mMediatorTabGroupModelFilterObserver.didMoveWithinGroup(mTab1, 2, POSITION1);

        // mTab1 still selected after the move (last selected), but the thumbnail updated.
        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        // TODO(crbug/1374361): Make this an assertion.
        // Thumbnail order was: tab1, tab2, tab3. Now: tab1, tab3, tab2.
        assertNotEquals(tab1Fetcher, mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void undoGrouped_One_Adjacent_Tab() {
        // Assume there are 3 tabs in TabModel, mTab2 just grouped with mTab1;
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, tab3));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(2));

        // Assume undo grouping mTab2 with mTab1.
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(tab3).when(mTabGroupModelFilter).getTabAt(2);

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab2, POSITION1);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.indexFromId(TAB1_ID), equalTo(0));
        assertThat(mModel.indexFromId(TAB2_ID), equalTo(1));
        assertThat(mModel.indexFromId(TAB3_ID), equalTo(2));
    }

    @Test
    public void undoForwardGrouped_One_Tab() {
        // Assume there are 3 tabs in TabModel, tab3 just grouped with mTab1;
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(2));

        // Assume undo grouping tab3 with mTab1.
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(2).when(mTabGroupModelFilter).indexOf(tab3);
        doReturn(tab3).when(mTabGroupModelFilter).getTabAt(2);
        doReturn(false).when(mTabGroupModelFilter).isTabInTabGroup(tab3);

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(tab3, POSITION1);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.indexFromId(TAB1_ID), equalTo(0));
        assertThat(mModel.indexFromId(TAB2_ID), equalTo(1));
        assertThat(mModel.indexFromId(TAB3_ID), equalTo(2));
    }

    @Test
    public void undoBackwardGrouped_One_Tab() {
        // Assume there are 3 tabs in TabModel, mTab1 just grouped with mTab2;
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab2, tab3));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(2));

        // Assume undo grouping mTab1 from mTab2.
        doReturn(3).when(mTabGroupModelFilter).getCount();
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(tab3).when(mTabGroupModelFilter).getTabAt(2);
        doReturn(2).when(mTabGroupModelFilter).indexOf(tab3);
        doReturn(false).when(mTabGroupModelFilter).isTabInTabGroup(mTab1);

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab1, POSITION2);

        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.indexFromId(TAB1_ID), equalTo(0));
        assertThat(mModel.indexFromId(TAB2_ID), equalTo(1));
        assertThat(mModel.indexFromId(TAB3_ID), equalTo(2));
    }

    @Test
    public void undoForwardGrouped_BetweenGroups() {
        // Assume there are 3 tabs in TabModel, tab3, tab4, just grouped with mTab1;
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        Tab tab4 = prepareTab(TAB4_ID, TAB4_TITLE, TAB4_URL);
        doReturn(4).when(mTabModel).getCount();
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(1));

        // Assume undo grouping tab3 with mTab1.
        doReturn(2).when(mTabGroupModelFilter).getCount();

        // Undo tab 3.
        List<Tab> relatedTabs = Arrays.asList(tab3);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(mTab1).when(mTabModel).getTabAt(0);
        doReturn(mTab2).when(mTabModel).getTabAt(1);
        doReturn(tab4).when(mTabModel).getTabAt(2);
        doReturn(tab3).when(mTabModel).getTabAt(3);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(tab4);
        doReturn(0).when(mTabModel).indexOf(mTab1);
        doReturn(1).when(mTabModel).indexOf(mTab2);
        doReturn(2).when(mTabModel).indexOf(tab4);
        doReturn(tab3).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(tab3);
        doReturn(3).when(mTabModel).indexOf(tab3);
        doReturn(false).when(mTabGroupModelFilter).isTabInTabGroup(tab3);
        doReturn(true).when(mTabGroupModelFilter).isTabInTabGroup(tab4);
        doReturn(relatedTabs).when(mTabGroupModelFilter).getRelatedTabList(TAB3_ID);
        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(tab3, POSITION1);
        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.indexFromId(TAB1_ID), equalTo(0));
        assertThat(mModel.indexFromId(TAB2_ID), equalTo(-1));
        assertThat(mModel.indexFromId(TAB3_ID), equalTo(1));
        assertThat(mModel.indexFromId(TAB4_ID), equalTo(-1));

        // Undo tab 4
        relatedTabs = Arrays.asList(tab3, tab4);
        doReturn(POSITION2).when(mTabGroupModelFilter).indexOf(tab4);
        doReturn(2).when(mTabModel).indexOf(tab3);
        doReturn(3).when(mTabModel).indexOf(tab4);
        doReturn(true).when(mTabGroupModelFilter).isTabInTabGroup(tab3);
        doReturn(true).when(mTabGroupModelFilter).isTabInTabGroup(tab4);
        doReturn(tab3).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(tab3).when(mTabModel).getTabAt(2);
        doReturn(tab4).when(mTabModel).getTabAt(3);
        doReturn(relatedTabs).when(mTabGroupModelFilter).getRelatedTabList(TAB3_ID);
        doReturn(relatedTabs).when(mTabGroupModelFilter).getRelatedTabList(TAB4_ID);
        doReturn(TAB3_ID).when(tab4).getRootId();
        doReturn(2).when(mTabGroupModelFilter).getRelatedTabCountForRootId(TAB3_ID);
        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(tab4, POSITION1);
        assertThat(mModel.size(), equalTo(2));

        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(tab4, TAB4_ID);

        assertThat(mModel.size(), equalTo(2));
        assertThat(mModel.indexFromId(TAB1_ID), equalTo(0));
        assertThat(mModel.indexFromId(TAB2_ID), equalTo(-1));
        assertThat(mModel.indexFromId(TAB3_ID), equalTo(1));
        assertThat(mModel.indexFromId(TAB4_ID), equalTo(-1));
    }

    @Test
    public void updateSpanCount_Portrait_SingleWindow() {
        initAndAssertAllProperties();
        // Mock that we are switching to portrait mode.
        Configuration configuration = new Configuration();
        configuration.orientation = Configuration.ORIENTATION_PORTRAIT;
        configuration.screenWidthDp = TabListCoordinator.MAX_SCREEN_WIDTH_COMPACT_DP - 1;

        mComponentCallbacksCaptor.getValue().onConfigurationChanged(configuration);

        verify(mGridLayoutManager).setSpanCount(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_COMPACT);
    }

    @Test
    public void updateSpanCount_Landscape_SingleWindow() {
        initAndAssertAllProperties();
        // Mock that we are switching to landscape mode.
        Configuration configuration = new Configuration();
        configuration.orientation = Configuration.ORIENTATION_LANDSCAPE;
        configuration.screenWidthDp = TabListCoordinator.MAX_SCREEN_WIDTH_MEDIUM_DP - 1;

        mComponentCallbacksCaptor.getValue().onConfigurationChanged(configuration);

        verify(mGridLayoutManager).setSpanCount(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_MEDIUM);
    }

    @Test
    @Config(qualifiers = "sw600dp")
    public void updateSpanCount_onTablet_multipleScreenWidths() {
        initAndAssertAllProperties(3);
        // Mock tablet
        when(mResources.getInteger(R.integer.min_screen_width_bucket))
                .thenReturn(TabListCoordinator.MAX_SCREEN_WIDTH_MEDIUM_DP + 1);
        Configuration portraitConfiguration = new Configuration();
        portraitConfiguration.orientation = Configuration.ORIENTATION_LANDSCAPE;

        // Compact width
        portraitConfiguration.screenWidthDp = TabListCoordinator.MAX_SCREEN_WIDTH_COMPACT_DP - 1;
        mComponentCallbacksCaptor.getValue().onConfigurationChanged(portraitConfiguration);
        verify(mGridLayoutManager).setSpanCount(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_COMPACT);

        // Medium width
        portraitConfiguration.screenWidthDp = TabListCoordinator.MAX_SCREEN_WIDTH_MEDIUM_DP - 1;
        mComponentCallbacksCaptor.getValue().onConfigurationChanged(portraitConfiguration);
        verify(mGridLayoutManager).setSpanCount(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_MEDIUM);

        // Large width
        portraitConfiguration.screenWidthDp = TabListCoordinator.MAX_SCREEN_WIDTH_MEDIUM_DP + 1;
        mComponentCallbacksCaptor.getValue().onConfigurationChanged(portraitConfiguration);
        verify(mGridLayoutManager).setSpanCount(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_LARGE);
    }

    @Test
    public void getLatestTitle_NotGTS() {
        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        // Mock that we have a stored title stored with reference to root ID of tab1.
        getGroupTitleSharedPreferences()
                .edit()
                .putString(String.valueOf(mTab1.getRootId()), CUSTOMIZED_DIALOG_TITLE1)
                .apply();
        assertThat(
                mMediator.getTabGroupTitleEditor().getTabGroupTitle(mTab1.getRootId()),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));

        // Mock that tab1 and tab2 are in the same group and group root id is TAB1_ID.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        // Even if we have a stored title, we only show it in tab switcher.
        assertThat(mMediator.getLatestTitleForTab(PseudoTab.fromTab(mTab1)), equalTo(TAB1_TITLE));
    }

    @Test
    public void getLatestTitle_SingleTabGroupSupported_GTS() {
        // Mock that we have a stored title stored with reference to root ID of tab1.
        getGroupTitleSharedPreferences()
                .edit()
                .putString(String.valueOf(mTab1.getRootId()), CUSTOMIZED_DIALOG_TITLE1)
                .apply();
        assertThat(
                mMediator.getTabGroupTitleEditor().getTabGroupTitle(mTab1.getRootId()),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));

        // Mock that tab1 is a single tab.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        // We never show stored title for single tab.
        assertThat(
                mMediator.getLatestTitleForTab(PseudoTab.fromTab(mTab1)),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));
    }

    @Test
    public void getLatestTitle_SingleTabGroupNotSupported_GTS() {
        // Mock that we have a stored title stored with reference to root ID of tab1.
        getGroupTitleSharedPreferences()
                .edit()
                .putString(String.valueOf(mTab1.getRootId()), CUSTOMIZED_DIALOG_TITLE1)
                .apply();
        assertThat(
                mMediator.getTabGroupTitleEditor().getTabGroupTitle(mTab1.getRootId()),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));

        // Mock that tab1 is a single tab.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1));
        createTabGroup(tabs, TAB1_ID, null);
        when(mTabGroupModelFilter.isTabInTabGroup(mTab1)).thenReturn(false);

        // We never show stored title for single tab.
        assertThat(mMediator.getLatestTitleForTab(PseudoTab.fromTab(mTab1)), equalTo(TAB1_TITLE));
    }

    @Test
    public void getLatestTitle_Stored_GTS() {
        // Mock that we have a stored title stored with reference to root ID of tab1.
        getGroupTitleSharedPreferences()
                .edit()
                .putString(String.valueOf(mTab1.getRootId()), CUSTOMIZED_DIALOG_TITLE1)
                .apply();
        assertThat(
                mMediator.getTabGroupTitleEditor().getTabGroupTitle(mTab1.getRootId()),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));

        // Mock that tab1 and tab2 are in the same group and group root id is TAB1_ID.
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        assertThat(
                mMediator.getLatestTitleForTab(PseudoTab.fromTab(mTab1)),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));
    }

    @Test
    public void updateTabGroupTitle_GTS() {
        setUpTabGroupCardDescriptionString();
        String targetString = "Expand tab group with 2 tabs.";
        assertThat(mModel.get(POSITION1).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));

        // Mock that tab1 and newTab are in the same group and group root id is TAB1_ID.
        Tab newTab = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, newTab));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);

        mMediator.getTabGroupTitleEditor().updateTabGroupTitle(mTab1, CUSTOMIZED_DIALOG_TITLE1);

        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.TITLE),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));
        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.CONTENT_DESCRIPTION_STRING),
                equalTo(targetString));
    }

    @Test
    public void updateTabGroupTitle_SingleTab_GTS() {
        setUpTabGroupCardDescriptionString();
        String targetString = "Expand tab group with 1 tab.";
        assertThat(mModel.get(POSITION1).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));

        createTabGroup(Arrays.asList(mTab1), TAB1_ID, TAB_GROUP_ID);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);

        mMediator.getTabGroupTitleEditor().updateTabGroupTitle(mTab1, CUSTOMIZED_DIALOG_TITLE1);

        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.TITLE),
                equalTo(CUSTOMIZED_DIALOG_TITLE1));
        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.CONTENT_DESCRIPTION_STRING),
                equalTo(targetString));
    }

    @Test
    public void tabGroupTitleEditor_storeTitle() {
        TabGroupTitleEditor tabGroupTitleEditor = mMediator.getTabGroupTitleEditor();

        assertNull(
                getGroupTitleSharedPreferences()
                        .getString(String.valueOf(mTab1.getRootId()), null));

        tabGroupTitleEditor.storeTabGroupTitle(mTab1.getRootId(), CUSTOMIZED_DIALOG_TITLE1);
        assertEquals(
                CUSTOMIZED_DIALOG_TITLE1,
                getGroupTitleSharedPreferences()
                        .getString(String.valueOf(mTab1.getRootId()), null));
    }

    @Test
    public void tabGroupTitleEditor_deleteTitle() {
        TabGroupTitleEditor tabGroupTitleEditor = mMediator.getTabGroupTitleEditor();

        getGroupTitleSharedPreferences()
                .edit()
                .putString(String.valueOf(mTab1.getRootId()), CUSTOMIZED_DIALOG_TITLE1)
                .apply();
        assertEquals(
                CUSTOMIZED_DIALOG_TITLE1,
                getGroupTitleSharedPreferences()
                        .getString(String.valueOf(mTab1.getRootId()), null));

        tabGroupTitleEditor.deleteTabGroupTitle(mTab1.getRootId());
        assertNull(
                getGroupTitleSharedPreferences()
                        .getString(String.valueOf(mTab1.getRootId()), null));
    }

    @Test
    public void addSpecialItem() {
        mMediator.resetWithListOfTabs(null, false);

        PropertyModel model = mock(PropertyModel.class);
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        mMediator.addSpecialItemToModel(0, TabProperties.UiType.DIVIDER, model);

        assertTrue(mModel.size() > 0);
        assertEquals(TabProperties.UiType.DIVIDER, mModel.get(0).type);
    }

    @Test
    public void addSpecialItem_notPersistOnReset() {
        mMediator.resetWithListOfTabs(null, false);

        PropertyModel model = mock(PropertyModel.class);
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        mMediator.addSpecialItemToModel(0, TabProperties.UiType.DIVIDER, model);
        assertEquals(TabProperties.UiType.DIVIDER, mModel.get(0).type);

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), /* quickMode= */ false);
        assertThat(mModel.size(), equalTo(2));
        assertNotEquals(TabProperties.UiType.DIVIDER, mModel.get(0).type);
        assertNotEquals(TabProperties.UiType.DIVIDER, mModel.get(1).type);

        mMediator.addSpecialItemToModel(1, TabProperties.UiType.DIVIDER, model);
        assertThat(mModel.size(), equalTo(3));
        assertEquals(TabProperties.UiType.DIVIDER, mModel.get(1).type);
    }

    @Test
    public void addSpecialItem_withoutTabListModelProperties() {
        if (!BuildConfig.ENABLE_ASSERTS) return;

        mMediator.resetWithListOfTabs(null, false);

        try {
            mMediator.addSpecialItemToModel(0, TabProperties.UiType.DIVIDER, new PropertyModel());
        } catch (AssertionError e) {
            return;
        }
        fail("PropertyModel#validateKey() assert should have failed.");
    }

    @Test
    public void removeSpecialItem_Message() {
        mMediator.resetWithListOfTabs(null, false);

        PropertyModel model = mock(PropertyModel.class);
        int expectedMessageType = FOR_TESTING;
        int wrongMessageType = TAB_SUGGESTION;
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        when(model.get(MESSAGE_TYPE)).thenReturn(expectedMessageType);
        mMediator.addSpecialItemToModel(0, TabProperties.UiType.MESSAGE, model);
        assertEquals(1, mModel.size());

        mMediator.removeSpecialItemFromModel(TabProperties.UiType.MESSAGE, wrongMessageType);
        assertEquals(1, mModel.size());

        mMediator.removeSpecialItemFromModel(TabProperties.UiType.MESSAGE, expectedMessageType);
        assertEquals(0, mModel.size());
    }

    @Test
    public void removeSpecialItem_Message_PriceMessage() {
        mMediator.resetWithListOfTabs(null, false);

        PropertyModel model = mock(PropertyModel.class);
        int expectedMessageType = PRICE_MESSAGE;
        int wrongMessageType = TAB_SUGGESTION;
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        when(model.get(MESSAGE_TYPE)).thenReturn(expectedMessageType);
        mMediator.addSpecialItemToModel(0, TabProperties.UiType.LARGE_MESSAGE, model);
        assertEquals(1, mModel.size());

        mMediator.removeSpecialItemFromModel(TabProperties.UiType.MESSAGE, wrongMessageType);
        assertEquals(1, mModel.size());

        mMediator.removeSpecialItemFromModel(
                TabProperties.UiType.LARGE_MESSAGE, expectedMessageType);
        assertEquals(0, mModel.size());
    }

    @Test
    public void testUrlUpdated_forSingleTab_GTS() {
        assertNotEquals(mNewDomain, mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));

        doReturn(mNewDomain)
                .when(mUrlUtilitiesJniMock)
                .getDomainAndRegistry(eq(NEW_URL), anyBoolean());

        doReturn(new GURL(NEW_URL)).when(mTab1).getUrl();
        mTabObserver.onUrlUpdated(mTab1);

        assertEquals(mNewDomain, mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));
        assertEquals(mTab2Domain, mModel.get(POSITION2).model.get(TabProperties.URL_DOMAIN));
    }

    @Test
    public void testUrlUpdated_forGroup_GTS() {
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);

        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(mTab2, TAB1_ID);
        assertEquals(
                mTab1Domain + ", " + mTab2Domain,
                mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));

        doReturn(mNewDomain)
                .when(mUrlUtilitiesJniMock)
                .getDomainAndRegistry(eq(NEW_URL), anyBoolean());

        // Update URL_DOMAIN for mTab1.
        doReturn(new GURL(NEW_URL)).when(mTab1).getUrl();
        mTabObserver.onUrlUpdated(mTab1);

        assertEquals(
                mNewDomain + ", " + mTab2Domain,
                mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));

        // Update URL_DOMAIN for mTab2.
        doReturn(new GURL(NEW_URL)).when(mTab2).getUrl();
        mTabObserver.onUrlUpdated(mTab2);

        assertEquals(
                mNewDomain + ", " + mNewDomain,
                mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));
    }

    @Test
    public void testUrlUpdated_forGroup_Dialog() {
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab1);
        doReturn(POSITION1).when(mTabGroupModelFilter).indexOf(mTab2);

        setUpTabListMediator(TabListMediatorType.TAB_GRID_DIALOG, TabListMode.GRID);

        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(mTab2, TAB1_ID);
        assertEquals(mTab1Domain, mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));
        assertEquals(mTab2Domain, mModel.get(POSITION2).model.get(TabProperties.URL_DOMAIN));

        doReturn(mNewDomain)
                .when(mUrlUtilitiesJniMock)
                .getDomainAndRegistry(eq(NEW_URL), anyBoolean());

        // Update URL_DOMAIN for mTab1.
        doReturn(new GURL(NEW_URL)).when(mTab1).getUrl();
        mTabObserver.onUrlUpdated(mTab1);

        assertEquals(mNewDomain, mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));
        assertEquals(mTab2Domain, mModel.get(POSITION2).model.get(TabProperties.URL_DOMAIN));

        // Update URL_DOMAIN for mTab2.
        doReturn(new GURL(NEW_URL)).when(mTab2).getUrl();
        mTabObserver.onUrlUpdated(mTab2);

        assertEquals(mNewDomain, mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));
        assertEquals(mNewDomain, mModel.get(POSITION2).model.get(TabProperties.URL_DOMAIN));
    }

    @Test
    public void testUrlUpdated_forUnGroup() {
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        mMediatorTabGroupModelFilterObserver.didMergeTabToGroup(mTab2, TAB1_ID);
        assertEquals(
                mTab1Domain + ", " + mTab2Domain,
                mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));

        // Assume that TabGroupModelFilter is already updated.
        when(mTabGroupModelFilter.getRelatedTabList(TAB1_ID)).thenReturn(Arrays.asList(mTab1));
        when(mTabGroupModelFilter.getRelatedTabList(TAB2_ID)).thenReturn(Arrays.asList(mTab2));
        when(mTabGroupModelFilter.isTabInTabGroup(mTab1)).thenReturn(true);
        when(mTabGroupModelFilter.isTabInTabGroup(mTab2)).thenReturn(false);
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        doReturn(TAB1_ID).when(mTab1).getRootId();
        doReturn(TAB2_ID).when(mTab2).getRootId();
        doReturn(1).when(mTabGroupModelFilter).getRelatedTabCountForRootId(TAB1_ID);
        doReturn(1).when(mTabGroupModelFilter).getRelatedTabCountForRootId(TAB2_ID);
        doReturn(2).when(mTabGroupModelFilter).getCount();

        mMediatorTabGroupModelFilterObserver.didMoveTabOutOfGroup(mTab2, POSITION1);
        assertEquals(mTab1Domain, mModel.get(POSITION1).model.get(TabProperties.URL_DOMAIN));
        assertEquals(mTab2Domain, mModel.get(POSITION2).model.get(TabProperties.URL_DOMAIN));
    }

    @Test
    @EnableFeatures(ChromeFeatureList.TAB_GROUP_PARITY_ANDROID)
    public void testTabGroupCreation_listMode() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);

        TabListMediator mMediatorSpy = spy(mMediator);
        doReturn(true).when(mMediatorSpy).isPseudoTabInTabGroup(any());

        mMediatorSpy.setComponentNameForTesting(TabSwitcherCoordinator.COMPONENT_NAME);
        initAndAssertAllProperties(mMediatorSpy);

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        // Get the next suggested color id to mock the setting of a color on tab group creation.
        int nextSuggestedColorId = TabGroupColorUtils.getNextSuggestedColorId(mTabGroupModelFilter);

        // Assert that the next suggested color was assigned to that group.
        assertEquals(
                nextSuggestedColorId,
                mModel.get(POSITION1).model.get(TabProperties.TAB_GROUP_COLOR_ID));

        // Fake a different color on the group to check that the color was set properly on group
        // creation.
        mModel.get(POSITION1).model.set(TabProperties.TAB_GROUP_COLOR_ID, COLOR_2);

        mMediatorTabGroupModelFilterObserver.didCreateNewGroup(mTab1, mTabGroupModelFilter);
        assertEquals(
                nextSuggestedColorId,
                mModel.get(POSITION1).model.get(TabProperties.TAB_GROUP_COLOR_ID));
    }

    @Test
    @EnableFeatures(ChromeFeatureList.TAB_GROUP_PARITY_ANDROID)
    public void testRefreshTabList_colorIcon() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);

        TabListMediator mMediatorSpy = spy(mMediator);
        doReturn(true).when(mMediatorSpy).isPseudoTabInTabGroup(any());

        mMediatorSpy.setComponentNameForTesting(TabSwitcherCoordinator.COMPONENT_NAME);
        initAndAssertAllProperties(mMediatorSpy);

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        // Get the next suggested color id to mock the setting of a color on tab group creation.
        int nextSuggestedColorId = TabGroupColorUtils.getNextSuggestedColorId(mTabGroupModelFilter);

        // Assert that the next suggested color was assigned to that group.
        assertEquals(
                nextSuggestedColorId,
                mModel.get(POSITION1).model.get(TabProperties.TAB_GROUP_COLOR_ID));

        // Fake a different color on the group to check that the color was set properly on refresh.
        mModel.get(POSITION1).model.set(TabProperties.TAB_GROUP_COLOR_ID, COLOR_2);

        // Pass in a mocked newly created tab group.
        mMediatorSpy.resetWithListOfTabs(
                PseudoTab.getListOfPseudoTab(tabs), /* quickMode= */ false);
        assertEquals(
                nextSuggestedColorId,
                mModel.get(POSITION1).model.get(TabProperties.TAB_GROUP_COLOR_ID));
    }

    @Test
    public void testOnInitializeAccessibilityNodeInfo() {
        // Setup related mocks and initialize needed components.
        AccessibilityNodeInfo accessibilityNodeInfo = mock(AccessibilityNodeInfo.class);
        AccessibilityAction action1 = new AccessibilityAction(R.id.move_tab_left, "left");
        AccessibilityAction action2 = new AccessibilityAction(R.id.move_tab_right, "right");
        AccessibilityAction action3 = new AccessibilityAction(R.id.move_tab_up, "up");
        doReturn(new ArrayList<>(Arrays.asList(action1, action2, action3)))
                .when(mTabGridAccessibilityHelper)
                .getPotentialActionsForView(mItemView1);
        InOrder accessibilityNodeInfoInOrder = Mockito.inOrder(accessibilityNodeInfo);
        assertNull(mMediator.getAccessibilityDelegateForTesting());
        mMediator.setupAccessibilityDelegate(mTabGridAccessibilityHelper);
        View.AccessibilityDelegate delegate = mMediator.getAccessibilityDelegateForTesting();
        assertNotNull(delegate);

        delegate.onInitializeAccessibilityNodeInfo(mItemView1, accessibilityNodeInfo);

        accessibilityNodeInfoInOrder.verify(accessibilityNodeInfo).addAction(eq(action1));
        accessibilityNodeInfoInOrder.verify(accessibilityNodeInfo).addAction(eq(action2));
        accessibilityNodeInfoInOrder.verify(accessibilityNodeInfo).addAction(eq(action3));
    }

    @Test
    public void testPerformAccessibilityAction() {
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));

        // Setup related mocks and initialize needed components.
        Bundle args = mock(Bundle.class);
        int action = R.id.move_tab_left;
        // Mock that the action indicates that tab2 will move left and thus tab2 and tab1 should
        // switch positions.
        doReturn(new Pair<>(1, 0))
                .when(mTabGridAccessibilityHelper)
                .getPositionsOfReorderAction(mItemView1, action);
        doReturn(true).when(mTabGridAccessibilityHelper).isReorderAction(action);
        assertNull(mMediator.getAccessibilityDelegateForTesting());
        mMediator.setupAccessibilityDelegate(mTabGridAccessibilityHelper);
        View.AccessibilityDelegate delegate = mMediator.getAccessibilityDelegateForTesting();
        assertNotNull(delegate);

        delegate.performAccessibilityAction(mItemView1, action, args);

        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
    }

    @Test
    public void testPerformAccessibilityAction_defaultAccessibilityAction() {
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));

        // Setup related mocks and initialize needed components.
        Bundle args = mock(Bundle.class);
        int action = ACTION_CLICK;
        // Mock that the action indicates that tab2 will move to position 2 which is invalid.
        doReturn(false).when(mTabGridAccessibilityHelper).isReorderAction(action);
        assertNull(mMediator.getAccessibilityDelegateForTesting());
        mMediator.setupAccessibilityDelegate(mTabGridAccessibilityHelper);
        View.AccessibilityDelegate delegate = mMediator.getAccessibilityDelegateForTesting();
        assertNotNull(delegate);

        delegate.performAccessibilityAction(mItemView1, action, args);
        verify(mTabGridAccessibilityHelper, never())
                .getPositionsOfReorderAction(mItemView1, action);
    }

    @Test
    public void testPerformAccessibilityAction_InvalidIndex() {
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));

        // Setup related mocks and initialize needed components.
        Bundle args = mock(Bundle.class);
        int action = R.id.move_tab_left;
        // Mock that the action indicates that tab2 will move to position 2 which is invalid.
        doReturn(new Pair<>(1, 2))
                .when(mTabGridAccessibilityHelper)
                .getPositionsOfReorderAction(mItemView1, action);
        assertNull(mMediator.getAccessibilityDelegateForTesting());
        mMediator.setupAccessibilityDelegate(mTabGridAccessibilityHelper);
        View.AccessibilityDelegate delegate = mMediator.getAccessibilityDelegateForTesting();
        assertNotNull(delegate);

        delegate.performAccessibilityAction(mItemView1, action, args);

        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));
    }

    @Test
    public void testTabObserverRemovedFromClosedTab() {
        initAndAssertAllProperties();

        assertThat(mModel.size(), equalTo(2));
        mMediatorTabModelObserver.willCloseTab(mTab2, false, true);
        verify(mTab2).removeObserver(mTabObserver);
        assertThat(mModel.size(), equalTo(1));
        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
    }

    @Test
    public void testTabObserverReattachToUndoClosedTab() {
        initAndAssertAllProperties();

        assertThat(mModel.size(), equalTo(2));
        mMediatorTabModelObserver.willCloseTab(mTab2, false, true);
        assertThat(mModel.size(), equalTo(1));

        // Assume that TabModelFilter is already updated to reflect closed tab is undone.
        doReturn(2).when(mTabGroupModelFilter).getCount();
        doReturn(mTab1).when(mTabGroupModelFilter).getTabAt(POSITION1);
        doReturn(mTab2).when(mTabGroupModelFilter).getTabAt(POSITION2);
        when(mTabGroupModelFilter.getRelatedTabList(TAB1_ID)).thenReturn(Arrays.asList(mTab1));
        when(mTabGroupModelFilter.getRelatedTabList(TAB2_ID)).thenReturn(Arrays.asList(mTab2));

        mMediatorTabModelObserver.tabClosureUndone(mTab2);
        assertThat(mModel.size(), equalTo(2));
        // First time is when mTab2 initially added to mModel; second time is when mTab2 added back
        // to mModel because of undo action.
        verify(mTab2, times(2)).addObserver(mTabObserver);
    }

    @Test
    public void testUnchangeCheckIgnoreNonTabs() {
        initAndAssertAllProperties();
        List<Tab> tabs = new ArrayList<>();
        for (int i = 0; i < mTabModel.getCount(); i++) {
            tabs.add(mTabModel.getTabAt(i));
        }

        boolean showQuickly =
                mMediator.resetWithListOfTabs(
                        PseudoTab.getListOfPseudoTab(tabs), /* quickMode= */ false);
        assertThat(showQuickly, equalTo(true));

        // Create a PropertyModel that is not a tab and add it to the existing TabListModel.
        PropertyModel propertyModel = mock(PropertyModel.class);
        when(propertyModel.get(CARD_TYPE)).thenReturn(OTHERS);
        mMediator.addSpecialItemToModel(mModel.size(), TabProperties.UiType.MESSAGE, propertyModel);
        assertThat(mModel.size(), equalTo(tabs.size() + 1));

        // TabListModel unchange check should ignore the non-Tab item.
        showQuickly =
                mMediator.resetWithListOfTabs(
                        PseudoTab.getListOfPseudoTab(tabs), /* quickMode= */ false);
        assertThat(showQuickly, equalTo(true));
    }

    // TODO(crbug.com/1177036): the assertThat in fetch callback is never reached.
    @Test
    public void testPriceTrackingProperty() {
        setPriceTrackingEnabledForTesting(true);
        for (boolean signedInAndSyncEnabled : new boolean[] {false, true}) {
            for (boolean priceTrackingEnabled : new boolean[] {false, true}) {
                for (boolean incognito : new boolean[] {false, true}) {
                    TabListMediator mMediatorSpy = spy(mMediator);
                    doReturn(false).when(mMediatorSpy).isPseudoTabInTabGroup(any());
                    PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(
                            signedInAndSyncEnabled);
                    PriceTrackingUtilities.SHARED_PREFERENCES_MANAGER.writeBoolean(
                            PriceTrackingUtilities.TRACK_PRICES_ON_TABS, priceTrackingEnabled);
                    Map<GURL, Any> responses = new HashMap<>();
                    responses.put(TAB1_URL, ANY_BUYABLE_PRODUCT_INITIAL);
                    responses.put(TAB2_URL, ANY_EMPTY);
                    mockOptimizationGuideResponse(OptimizationGuideDecision.TRUE, responses);
                    PersistedTabDataConfiguration.setUseTestConfig(true);
                    initAndAssertAllProperties(mMediatorSpy);
                    List<Tab> tabs = new ArrayList<>();
                    doReturn(incognito).when(mTab1).isIncognito();
                    doReturn(incognito).when(mTab2).isIncognito();

                    for (int i = 0; i < 2; i++) {
                        long timestamp = System.currentTimeMillis();
                        Tab tab = mTabModel.getTabAt(i);
                        doReturn(timestamp).when(tab).getTimestampMillis();
                    }

                    tabs.add(mTabModel.getTabAt(0));
                    tabs.add(mTabModel.getTabAt(1));

                    mMediatorSpy.resetWithListOfTabs(
                            PseudoTab.getListOfPseudoTab(tabs), /* quickMode= */ false);
                    if (signedInAndSyncEnabled && priceTrackingEnabled && !incognito) {
                        mModel.get(0)
                                .model
                                .get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER)
                                .fetch(
                                        (shoppingPersistedTabData) -> {
                                            assertThat(
                                                    shoppingPersistedTabData.getPriceMicros(),
                                                    equalTo(123456789012345L));
                                        });
                        mModel.get(1)
                                .model
                                .get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER)
                                .fetch(
                                        (shoppingPersistedTabData) -> {
                                            assertThat(
                                                    shoppingPersistedTabData.getPriceMicros(),
                                                    equalTo(
                                                            ShoppingPersistedTabData
                                                                    .NO_PRICE_KNOWN));
                                        });
                    } else {
                        assertNull(
                                mModel.get(0)
                                        .model
                                        .get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
                        assertNull(
                                mModel.get(1)
                                        .model
                                        .get(TabProperties.SHOPPING_PERSISTED_TAB_DATA_FETCHER));
                    }
                }
            }
        }
        // Set incognito status back to how it was
        doReturn(true).when(mTab1).isIncognito();
        doReturn(true).when(mTab2).isIncognito();
    }

    @Test
    public void testGetPriceWelcomeMessageInsertionIndex() {
        initWithThreeTabs();

        doReturn(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_COMPACT)
                .when(mGridLayoutManager)
                .getSpanCount();
        assertThat(mMediator.getPriceWelcomeMessageInsertionIndex(), equalTo(2));

        doReturn(TabListCoordinator.GRID_LAYOUT_SPAN_COUNT_MEDIUM)
                .when(mGridLayoutManager)
                .getSpanCount();
        assertThat(mMediator.getPriceWelcomeMessageInsertionIndex(), equalTo(3));
    }

    @Test
    public void testUpdateLayout_PriceMessage() {
        initAndAssertAllProperties();
        addSpecialItem(1, TabProperties.UiType.LARGE_MESSAGE, PRICE_MESSAGE);
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(1));

        doAnswer(
                        invocation -> {
                            int position = invocation.getArgument(0);
                            int itemType = mModel.get(position).type;
                            if (itemType == TabProperties.UiType.LARGE_MESSAGE) {
                                return mGridLayoutManager.getSpanCount();
                            }
                            return 1;
                        })
                .when(mSpanSizeLookup)
                .getSpanSize(anyInt());
        mMediator.updateLayout();
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(1));
        setPriceTrackingEnabledForTesting(true);
        PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(true);
        PriceTrackingUtilities.SHARED_PREFERENCES_MANAGER.writeBoolean(
                PriceTrackingUtilities.PRICE_WELCOME_MESSAGE_CARD, true);
        mMediator.updateLayout();
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(2));
    }

    @Test
    public void testUpdateLayout_Divider() {
        initAndAssertAllProperties();
        addSpecialItem(1, TabProperties.UiType.DIVIDER, 0);
        assertThat(mModel.get(1).type, equalTo(TabProperties.UiType.DIVIDER));

        doAnswer(
                        invocation -> {
                            int position = invocation.getArgument(0);
                            int itemType = mModel.get(position).type;
                            if (itemType == TabProperties.UiType.DIVIDER) {
                                return mGridLayoutManager.getSpanCount();
                            }
                            return 1;
                        })
                .when(mSpanSizeLookup)
                .getSpanSize(anyInt());
        PriceTrackingUtilities.SHARED_PREFERENCES_MANAGER.writeBoolean(
                PriceTrackingUtilities.PRICE_WELCOME_MESSAGE_CARD, true);
        mMediator.updateLayout();
        assertThat(mModel.get(1).type, equalTo(TabProperties.UiType.DIVIDER));
    }

    @Test
    public void testIndexOfNthTabCard() {
        initAndAssertAllProperties();
        addSpecialItem(1, TabProperties.UiType.LARGE_MESSAGE, PRICE_MESSAGE);

        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(1));
        assertThat(mModel.indexOfNthTabCard(-1), equalTo(TabModel.INVALID_TAB_INDEX));
        assertThat(mModel.indexOfNthTabCard(0), equalTo(0));
        assertThat(mModel.indexOfNthTabCard(1), equalTo(2));
        assertThat(mModel.indexOfNthTabCard(2), equalTo(3));
    }

    @Test
    public void testGetTabCardCountsBefore() {
        initAndAssertAllProperties();
        addSpecialItem(1, TabProperties.UiType.LARGE_MESSAGE, PRICE_MESSAGE);

        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(1));
        assertThat(mModel.getTabCardCountsBefore(-1), equalTo(TabModel.INVALID_TAB_INDEX));
        assertThat(mModel.getTabCardCountsBefore(0), equalTo(0));
        assertThat(mModel.getTabCardCountsBefore(1), equalTo(1));
        assertThat(mModel.getTabCardCountsBefore(2), equalTo(1));
        assertThat(mModel.getTabCardCountsBefore(3), equalTo(2));
    }

    @Test
    public void testGetTabIndexBefore() {
        initAndAssertAllProperties();
        addSpecialItem(1, TabProperties.UiType.LARGE_MESSAGE, PRICE_MESSAGE);
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(1));
        assertThat(mModel.getTabIndexBefore(2), equalTo(0));
        assertThat(mModel.getTabIndexBefore(0), equalTo(TabModel.INVALID_TAB_INDEX));
    }

    @Test
    public void testGetTabIndexAfter() {
        initAndAssertAllProperties();
        addSpecialItem(1, TabProperties.UiType.LARGE_MESSAGE, PRICE_MESSAGE);
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(1));
        assertThat(mModel.getTabIndexAfter(0), equalTo(2));
        assertThat(mModel.getTabIndexAfter(2), equalTo(TabModel.INVALID_TAB_INDEX));
    }

    @Test
    public void testListObserver_OnItemRangeInserted() {
        PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(true);
        setPriceTrackingEnabledForTesting(true);
        mMediator =
                new TabListMediator(
                        mActivity,
                        mModel,
                        TabListMode.GRID,
                        mCurrentTabModelFilterSupplier,
                        () -> mTabModel,
                        getTabThumbnailCallback(),
                        mTitleProvider,
                        mTabListFaviconProvider,
                        true,
                        null,
                        null,
                        null,
                        null,
                        getClass().getSimpleName(),
                        TabProperties.UiType.CLOSABLE);
        mMediator.registerOrientationListener(mGridLayoutManager);
        mMediator.initWithNative(mProfile);
        initAndAssertAllProperties();

        PropertyModel model = mock(PropertyModel.class);
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        when(model.get(MESSAGE_TYPE)).thenReturn(PRICE_MESSAGE);
        mMediator.addSpecialItemToModel(1, TabProperties.UiType.LARGE_MESSAGE, model);
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(2));
    }

    @Test
    public void testListObserver_OnItemRangeRemoved() {
        PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(true);
        setPriceTrackingEnabledForTesting(true);
        mMediator =
                new TabListMediator(
                        mActivity,
                        mModel,
                        TabListMode.GRID,
                        mCurrentTabModelFilterSupplier,
                        () -> mTabModel,
                        getTabThumbnailCallback(),
                        mTitleProvider,
                        mTabListFaviconProvider,
                        true,
                        null,
                        null,
                        null,
                        null,
                        getClass().getSimpleName(),
                        TabProperties.UiType.CLOSABLE);
        mMediator.registerOrientationListener(mGridLayoutManager);
        mMediator.initWithNative(mProfile);
        initWithThreeTabs();

        PropertyModel model = mock(PropertyModel.class);
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        when(model.get(MESSAGE_TYPE)).thenReturn(PRICE_MESSAGE);
        mMediator.addSpecialItemToModel(2, TabProperties.UiType.LARGE_MESSAGE, model);
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(2));
        mModel.removeAt(0);
        assertThat(mModel.lastIndexForMessageItemFromType(PRICE_MESSAGE), equalTo(2));
    }

    @Test
    public void testMaybeShowPriceWelcomeMessage() {
        prepareTestMaybeShowPriceWelcomeMessage();
        ShoppingPersistedTabDataFetcher fetcher =
                new ShoppingPersistedTabDataFetcher(mTab1, () -> mPriceWelcomeMessageController);
        fetcher.maybeShowPriceWelcomeMessage(mShoppingPersistedTabData);
        verify(mPriceWelcomeMessageController, times(1)).showPriceWelcomeMessage(mPriceTabData);
    }

    @Test
    public void testMaybeShowPriceWelcomeMessage_MessageDisabled() {
        prepareTestMaybeShowPriceWelcomeMessage();
        ShoppingPersistedTabDataFetcher fetcher =
                new ShoppingPersistedTabDataFetcher(mTab1, () -> mPriceWelcomeMessageController);

        PriceTrackingUtilities.SHARED_PREFERENCES_MANAGER.writeBoolean(
                PriceTrackingUtilities.PRICE_WELCOME_MESSAGE_CARD, false);
        assertThat(
                PriceTrackingUtilities.isPriceWelcomeMessageCardEnabled(mProfile), equalTo(false));
        fetcher.maybeShowPriceWelcomeMessage(mShoppingPersistedTabData);
        verify(mPriceWelcomeMessageController, times(0)).showPriceWelcomeMessage(mPriceTabData);
    }

    @Test
    public void testMaybeShowPriceWelcomeMessage_SupplierIsNull() {
        prepareTestMaybeShowPriceWelcomeMessage();

        new ShoppingPersistedTabDataFetcher(mTab1, null)
                .maybeShowPriceWelcomeMessage(mShoppingPersistedTabData);
        verify(mPriceWelcomeMessageController, times(0)).showPriceWelcomeMessage(mPriceTabData);
    }

    @Test
    public void testMaybeShowPriceWelcomeMessage_SupplierContainsNull() {
        prepareTestMaybeShowPriceWelcomeMessage();

        Supplier<PriceWelcomeMessageController> supplier = () -> null;
        new ShoppingPersistedTabDataFetcher(mTab1, supplier)
                .maybeShowPriceWelcomeMessage(mShoppingPersistedTabData);
        verify(mPriceWelcomeMessageController, times(0)).showPriceWelcomeMessage(mPriceTabData);
    }

    @Test
    public void testMaybeShowPriceWelcomeMessage_NoPriceDrop() {
        prepareTestMaybeShowPriceWelcomeMessage();
        ShoppingPersistedTabDataFetcher fetcher =
                new ShoppingPersistedTabDataFetcher(mTab1, () -> mPriceWelcomeMessageController);

        fetcher.maybeShowPriceWelcomeMessage(null);
        verify(mPriceWelcomeMessageController, times(0)).showPriceWelcomeMessage(mPriceTabData);

        doReturn(null).when(mShoppingPersistedTabData).getPriceDrop();
        fetcher.maybeShowPriceWelcomeMessage(mShoppingPersistedTabData);
        verify(mPriceWelcomeMessageController, times(0)).showPriceWelcomeMessage(mPriceTabData);
    }

    @Test
    @EnableFeatures(ChromeFeatureList.TAB_GROUP_PARITY_ANDROID)
    public void testUpdateFaviconFetcherForGroup_gridModeWithColors() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.GRID);
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);

        when(mTabModel.isIncognito()).thenReturn(false);
        // Mock that we have a stored color stored with reference to root ID of tab1.
        getGroupColorSharedPreferences()
                .edit()
                .putInt(String.valueOf(mTab1.getRootId()), COLOR_2)
                .apply();

        // Test a group of three.
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        mTabObserver.onFaviconUpdated(mTab1, mFaviconBitmap, mFaviconUrl);
        verify(mTabListFaviconProvider).getFaviconFromTabGroupColorFetcher(COLOR_2, false);
        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void testUpdateFaviconFetcherForGroup() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);

        // Test a group of three.
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        mTabObserver.onFaviconUpdated(mTab1, mFaviconBitmap, mFaviconUrl);
        List<GURL> urls = new ArrayList<>(Arrays.asList(TAB1_URL, TAB2_URL, TAB3_URL));
        verify(mTabListFaviconProvider).getComposedFaviconImageFetcher(eq(urls), anyBoolean());
        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));

        // Test a group of five.
        mModel.get(1).model.set(TabProperties.FAVICON_FETCHER, null);
        Tab tab4 = prepareTab(0, "tab 4", TAB2_URL);
        Tab tab5 = prepareTab(1, "tab 5", JUnitTestGURLs.EXAMPLE_URL);
        tabs.addAll(Arrays.asList(tab4, tab5));
        createTabGroup(tabs, TAB2_ID, TAB_GROUP_ID);
        mTabObserver.onFaviconUpdated(mTab2, mFaviconBitmap, mFaviconUrl);
        urls = new ArrayList<>(Arrays.asList(TAB2_URL, TAB1_URL, TAB3_URL, TAB2_URL));
        verify(mTabListFaviconProvider).getComposedFaviconImageFetcher(eq(urls), anyBoolean());
        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void testUpdateFaviconFetcherForGroup_StaleIndex_SelectAnotherTabWithinGroup() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        mModel.get(1).model.set(TabProperties.FAVICON_FETCHER, null);

        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> group1 = new ArrayList<>(Arrays.asList(mTab2, tab3));
        List<GURL> group1Urls = new ArrayList<>(Arrays.asList(mTab2.getUrl(), tab3.getUrl()));
        createTabGroup(group1, TAB2_ID, TAB_GROUP_ID);
        assertEquals(1, mModel.indexFromId(TAB2_ID));

        mTabObserver.onFaviconUpdated(mTab2, mFaviconBitmap, mFaviconUrl);
        verify(mTabListFaviconProvider)
                .getComposedFaviconImageFetcher(eq(group1Urls), anyBoolean());

        // Simulate selecting another Tab within TabGroup.
        mModel.get(1).model.set(TabProperties.TAB_ID, TAB3_ID);

        assertNotEquals(1, mModel.indexFromId(TAB2_ID));
        assertNotNull(mModel.get(1).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void testUpdateFaviconFetcherForGroup_StaleIndex_CloseTab() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        mModel.get(1).model.set(TabProperties.FAVICON_FETCHER, null);

        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> group1 = new ArrayList<>(Arrays.asList(mTab2, tab3));
        List<GURL> group1Urls = new ArrayList<>(Arrays.asList(mTab2.getUrl(), tab3.getUrl()));
        createTabGroup(group1, TAB2_ID, TAB_GROUP_ID);
        assertEquals(1, mModel.indexFromId(TAB2_ID));

        mTabObserver.onFaviconUpdated(mTab2, mFaviconBitmap, mFaviconUrl);
        verify(mTabListFaviconProvider)
                .getComposedFaviconImageFetcher(eq(group1Urls), anyBoolean());

        // Simulate closing mTab1 at index 0.
        mModel.removeAt(0);

        assertEquals(0, mModel.indexFromId(TAB2_ID));
        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
    }

    @Test
    public void testUpdateFaviconFetcherForGroup_StaleIndex_Reset() {
        setUpTabListMediator(TabListMediatorType.TAB_SWITCHER, TabListMode.LIST);
        mModel.get(0).model.set(TabProperties.FAVICON_FETCHER, null);
        mModel.get(1).model.set(TabProperties.FAVICON_FETCHER, null);

        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> group1 = new ArrayList<>(Arrays.asList(mTab2, tab3));
        List<GURL> group1Urls = new ArrayList<>(Arrays.asList(mTab2.getUrl(), tab3.getUrl()));
        createTabGroup(group1, TAB2_ID, TAB_GROUP_ID);
        assertEquals(1, mModel.indexFromId(TAB2_ID));

        mTabObserver.onFaviconUpdated(mTab2, mFaviconBitmap, mFaviconUrl);
        verify(mTabListFaviconProvider)
                .getComposedFaviconImageFetcher(eq(group1Urls), anyBoolean());

        // Simulate TabListMediator reset with null.
        mModel.set(new ArrayList<>());
    }

    @Test(expected = AssertionError.class)
    public void testGetDomainOnDestroyedTab() {
        Tab tab = new MockTab(TAB1_ID, mProfile);
        tab.destroy();
        TabListMediator.getDomain(tab);
    }

    @Test
    public void testTabDescriptionStringSetup() {
        // Setup the string template.
        setUpTabGroupCardDescriptionString();
        String targetString = "Expand tab group with 2 tabs.";

        // Setup a tab group with {tab2, tab3}.
        List<Tab> tabs = new ArrayList<>();
        for (int i = 0; i < mTabModel.getCount(); i++) {
            tabs.add(mTabModel.getTabAt(i));
        }
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> group1 = new ArrayList<>(Arrays.asList(mTab2, tab3));
        createTabGroup(group1, TAB2_ID, TAB_GROUP_ID);

        // Reset with show quickly.
        assertThat(
                mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false),
                equalTo(true));
        assertThat(
                mModel.get(POSITION2).model.get(TabProperties.CONTENT_DESCRIPTION_STRING),
                equalTo(targetString));

        // Reset without show quickly.
        mModel.clear();
        assertThat(
                mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false),
                equalTo(false));
        assertThat(
                mModel.get(POSITION2).model.get(TabProperties.CONTENT_DESCRIPTION_STRING),
                equalTo(targetString));

        // Set group name.
        targetString = String.format("Expand %s tab group with 2 tabs.", CUSTOMIZED_DIALOG_TITLE1);
        mMediator.getTabGroupTitleEditor().storeTabGroupTitle(TAB2_ID, CUSTOMIZED_DIALOG_TITLE1);
        mMediator.getTabGroupTitleEditor().updateTabGroupTitle(mTab2, CUSTOMIZED_DIALOG_TITLE1);
        assertThat(
                mModel.get(POSITION2).model.get(TabProperties.CONTENT_DESCRIPTION_STRING),
                equalTo(targetString));
    }

    @Test
    public void testCloseButtonDescriptionStringSetup_TabSwitcher() {
        setUpCloseButtonDescriptionString(false);
        String targetString = "Close Tab1 tab";

        List<Tab> tabs = new ArrayList<>();
        for (int i = 0; i < mTabModel.getCount(); i++) {
            tabs.add(mTabModel.getTabAt(i));
        }

        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.CLOSE_BUTTON_DESCRIPTION_STRING),
                equalTo(targetString));

        // Create tab group.
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> group1 = new ArrayList<>(Arrays.asList(mTab1, tab3));
        createTabGroup(group1, TAB1_ID, TAB_GROUP_ID);
        setUpCloseButtonDescriptionString(true);
        targetString = "Close tab group with 2 tabs.";

        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.CLOSE_BUTTON_DESCRIPTION_STRING),
                equalTo(targetString));

        // Set group name.
        targetString = String.format("Close %s group with 2 tabs.", CUSTOMIZED_DIALOG_TITLE1);
        mMediator.getTabGroupTitleEditor().storeTabGroupTitle(TAB1_ID, CUSTOMIZED_DIALOG_TITLE1);
        mMediator.getTabGroupTitleEditor().updateTabGroupTitle(mTab1, CUSTOMIZED_DIALOG_TITLE1);
        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.CLOSE_BUTTON_DESCRIPTION_STRING),
                equalTo(targetString));
    }

    @Test
    public void testCloseButtonDescriptionStringSetup_SingleTabGroup_TabSwitcher() {
        // Create tab group.
        List<Tab> tabs = Arrays.asList(mTab1);
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);
        setUpCloseButtonDescriptionString(true);
        String targetString = "Close tab group with 1 tab.";

        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.CLOSE_BUTTON_DESCRIPTION_STRING),
                equalTo(targetString));

        // Set group name.
        targetString = String.format("Close %s group with 1 tab.", CUSTOMIZED_DIALOG_TITLE1);
        mMediator.getTabGroupTitleEditor().storeTabGroupTitle(TAB1_ID, CUSTOMIZED_DIALOG_TITLE1);
        mMediator.getTabGroupTitleEditor().updateTabGroupTitle(mTab1, CUSTOMIZED_DIALOG_TITLE1);
        assertThat(
                mModel.get(POSITION1).model.get(TabProperties.CLOSE_BUTTON_DESCRIPTION_STRING),
                equalTo(targetString));
    }

    @Test
    public void testRecordPriceAnnotationsEnabledMetrics() {
        setPriceTrackingEnabledForTesting(true);
        PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(true);
        String histogramName = "Commerce.PriceDrop.AnnotationsEnabled";

        SharedPreferencesManager preferencesManager = ChromeSharedPreferences.getInstance();
        long presetTime = System.currentTimeMillis() - TimeUnit.DAYS.toMillis(1);
        preferencesManager.writeLong(
                ChromePreferenceKeys.PRICE_TRACKING_ANNOTATIONS_ENABLED_METRICS_TIMESTAMP,
                presetTime);
        mMediator.recordPriceAnnotationsEnabledMetrics();
        assertThat(RecordHistogram.getHistogramTotalCountForTesting(histogramName), equalTo(1));
        long updatedTime =
                preferencesManager.readLong(
                        ChromePreferenceKeys.PRICE_TRACKING_ANNOTATIONS_ENABLED_METRICS_TIMESTAMP,
                        presetTime);
        assertNotEquals(presetTime, updatedTime);

        // This metrics should only be recorded once within one day.
        mMediator.recordPriceAnnotationsEnabledMetrics();
        assertThat(RecordHistogram.getHistogramTotalCountForTesting(histogramName), equalTo(1));
        assertEquals(
                updatedTime,
                preferencesManager.readLong(
                        ChromePreferenceKeys.PRICE_TRACKING_ANNOTATIONS_ENABLED_METRICS_TIMESTAMP,
                        -1));
    }

    @Test
    public void testPriceDropSeen() throws TimeoutException {
        setPriceTrackingEnabledForTesting(true);
        PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(true);
        PriceTrackingUtilities.SHARED_PREFERENCES_MANAGER.writeBoolean(
                PriceTrackingUtilities.TRACK_PRICES_ON_TABS, true);

        doReturn(false).when(mTab1).isIncognito();
        doReturn(false).when(mTab2).isIncognito();

        List<Tab> tabs = new ArrayList<>();
        tabs.add(mTabModel.getTabAt(0));
        tabs.add(mTabModel.getTabAt(1));

        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), /* quickMode= */ false);

        prepareRecyclerViewForScroll();
        mMediator.registerOnScrolledListener(mRecyclerView);
        verify(mRecyclerView).addOnScrollListener(mOnScrollListenerCaptor.capture());
        mOnScrollListenerCaptor
                .getValue()
                .onScrolled(mRecyclerView, /* dx= */ mTabModel.getCount(), /* dy= */ 0);
        assertEquals(2, mMediator.getViewedTabIdsForTesting().size());
    }

    @Test
    public void testSelectableUpdates_withoutRelated() {
        when(mSelectionDelegate.isItemSelected(TAB1_ID)).thenReturn(true);
        when(mSelectionDelegate.isItemSelected(TAB2_ID)).thenReturn(false);
        when(mSelectionDelegate.isItemSelected(TAB3_ID)).thenReturn(false);
        mMediator =
                new TabListMediator(
                        mActivity,
                        mModel,
                        TabListMode.GRID,
                        mCurrentTabModelFilterSupplier,
                        () -> mTabModel,
                        getTabThumbnailCallback(),
                        mTitleProvider,
                        mTabListFaviconProvider,
                        true,
                        () -> {
                            return mSelectionDelegate;
                        },
                        null,
                        null,
                        null,
                        getClass().getSimpleName(),
                        TabProperties.UiType.SELECTABLE);
        mMediator.registerOrientationListener(mGridLayoutManager);
        mMediator.initWithNative(mProfile);
        initAndAssertAllProperties();
        when(mSelectionDelegate.isItemSelected(TAB1_ID)).thenReturn(false);
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(2).model.get(TabProperties.IS_SELECTED), equalTo(false));

        when(mTabGroupModelFilter.isTabInTabGroup(mTab2)).thenReturn(false);
        ThumbnailFetcher fetcher2 = mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER);
        mModel.get(1).model.get(TabProperties.SELECTABLE_TAB_CLICKED_LISTENER).run(TAB2_ID);
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertEquals(fetcher2, mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void testSelectableUpdates_withRelated() {
        when(mSelectionDelegate.isItemSelected(TAB1_ID)).thenReturn(true);
        when(mSelectionDelegate.isItemSelected(TAB2_ID)).thenReturn(false);
        when(mSelectionDelegate.isItemSelected(TAB3_ID)).thenReturn(false);
        mMediator =
                new TabListMediator(
                        mActivity,
                        mModel,
                        TabListMode.GRID,
                        mCurrentTabModelFilterSupplier,
                        () -> mTabModel,
                        getTabThumbnailCallback(),
                        mTitleProvider,
                        mTabListFaviconProvider,
                        true,
                        () -> {
                            return mSelectionDelegate;
                        },
                        null,
                        null,
                        null,
                        getClass().getSimpleName(),
                        TabProperties.UiType.SELECTABLE);
        mMediator.registerOrientationListener(mGridLayoutManager);
        mMediator.initWithNative(mProfile);
        initAndAssertAllProperties();
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        when(mSelectionDelegate.isItemSelected(TAB1_ID)).thenReturn(false);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(2).model.get(TabProperties.IS_SELECTED), equalTo(false));

        when(mTabGroupModelFilter.isTabInTabGroup(mTab2)).thenReturn(true);
        ThumbnailFetcher fetcher2 = mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER);
        mModel.get(1).model.get(TabProperties.SELECTABLE_TAB_CLICKED_LISTENER).run(TAB2_ID);
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertNotEquals(fetcher2, mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void testSelectableUpdates_onReset() {
        when(mSelectionDelegate.isItemSelected(TAB1_ID)).thenReturn(true);
        when(mSelectionDelegate.isItemSelected(TAB2_ID)).thenReturn(false);
        when(mSelectionDelegate.isItemSelected(TAB3_ID)).thenReturn(false);
        mMediator =
                new TabListMediator(
                        mActivity,
                        mModel,
                        TabListMode.GRID,
                        mCurrentTabModelFilterSupplier,
                        () -> mTabModel,
                        getTabThumbnailCallback(),
                        mTitleProvider,
                        mTabListFaviconProvider,
                        true,
                        () -> {
                            return mSelectionDelegate;
                        },
                        null,
                        null,
                        null,
                        getClass().getSimpleName(),
                        TabProperties.UiType.SELECTABLE);
        mMediator.registerOrientationListener(mGridLayoutManager);
        mMediator.initWithNative(mProfile);
        initAndAssertAllProperties();
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        Tab tab4 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        when(mTabGroupModelFilter.getRelatedTabList(TAB1_ID)).thenReturn(Arrays.asList(mTab1));
        when(mTabGroupModelFilter.getRelatedTabList(TAB2_ID))
                .thenReturn(Arrays.asList(mTab2, tab4));
        when(mTabGroupModelFilter.getRelatedTabList(TAB3_ID)).thenReturn(Arrays.asList(tab3));
        when(mTabGroupModelFilter.isTabInTabGroup(mTab1)).thenReturn(false);
        when(mTabGroupModelFilter.isTabInTabGroup(mTab2)).thenReturn(true);
        when(mTabGroupModelFilter.isTabInTabGroup(tab4)).thenReturn(true);
        when(mTabGroupModelFilter.isTabInTabGroup(tab3)).thenReturn(false);
        List<Tab> tabs = Arrays.asList(mTab1, mTab2, tab3);
        when(mSelectionDelegate.isItemSelected(TAB1_ID)).thenReturn(false);
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(2).model.get(TabProperties.IS_SELECTED), equalTo(false));

        when(mSelectionDelegate.isItemSelected(TAB1_ID)).thenReturn(true);
        when(mSelectionDelegate.isItemSelected(TAB2_ID)).thenReturn(true);
        when(mSelectionDelegate.isItemSelected(TAB3_ID)).thenReturn(false);
        ThumbnailFetcher fetcher1 = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);
        ThumbnailFetcher fetcher2 = mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER);
        ThumbnailFetcher fetcher3 = mModel.get(2).model.get(TabProperties.THUMBNAIL_FETCHER);
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), true);

        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertThat(mModel.get(2).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertEquals(fetcher1, mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER));
        assertNotEquals(fetcher2, mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER));
        assertEquals(fetcher3, mModel.get(2).model.get(TabProperties.THUMBNAIL_FETCHER));
    }

    @Test
    public void testChangingTabModelFilters() {
        mCurrentTabModelFilterSupplier.set(mIncognitoTabGroupModelFilter);
        // Once for the Mediator.
        verify(mTabGroupModelFilter).removeObserver(any());
        verify(mTabGroupModelFilter).removeTabGroupObserver(any());
        verify(mIncognitoTabGroupModelFilter).addObserver(any());
        verify(mIncognitoTabGroupModelFilter).addTabGroupObserver(any());
    }

    @Test
    public void testSpecialItemExist() {
        mMediator.resetWithListOfTabs(null, false);

        PropertyModel model = mock(PropertyModel.class);
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        when(model.get(MESSAGE_TYPE)).thenReturn(FOR_TESTING);
        mMediator.addSpecialItemToModel(0, TabProperties.UiType.LARGE_MESSAGE, model);

        assertTrue(mModel.size() > 0);
        assertTrue(mMediator.specialItemExistsInModel(FOR_TESTING));
        assertFalse(mMediator.specialItemExistsInModel(PRICE_MESSAGE));
        assertTrue(mMediator.specialItemExistsInModel(MessageService.MessageType.ALL));
    }

    @Test
    public void tabClosure_updatesTabGroup_inTabSwitcher() {
        initAndAssertAllProperties();

        // Mock that tab1 and tab3 are in the same group and group root id is TAB1_ID.
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, tab3));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(List.of(mTab1, mTab2)), true);
        ThumbnailFetcher fetcherBefore = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);
        assertEquals(2, mModel.size());

        mMediator.setActionOnAllRelatedTabsForTesting(true);
        doReturn(true).when(mTabGroupModelFilter).tabGroupExistsForRootId(TAB1_ID);
        doReturn(false).when(mTab1).isClosing();

        mMediatorTabModelObserver.willCloseTab(tab3, false, true);

        assertEquals(2, mModel.size());

        ThumbnailFetcher fetcherAfter = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);
        assertThat(fetcherBefore, not(fetcherAfter));
    }

    @Test
    public void tabClosure_doesNotUpdateTabGroup_inTabSwitcher_WhenClosing() {
        initAndAssertAllProperties();

        // Mock that tab1 and tab3 are in the same group and group root id is TAB1_ID.
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, tab3));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(List.of(mTab1, mTab2)), true);
        ThumbnailFetcher fetcherBefore = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);
        assertEquals(2, mModel.size());

        mMediator.setActionOnAllRelatedTabsForTesting(true);
        doReturn(true).when(mTabGroupModelFilter).tabGroupExistsForRootId(TAB1_ID);
        doReturn(true).when(mTab1).isClosing();

        mMediatorTabModelObserver.willCloseTab(tab3, false, true);

        assertEquals(2, mModel.size());

        ThumbnailFetcher fetcherAfter = mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER);
        assertThat(fetcherBefore, equalTo(fetcherAfter));
    }

    @Test
    public void tabClosure_ignoresUpdateForTabGroup_outsideTabSwitcher() {
        initAndAssertAllProperties();
        TabListMediator.TabActionListener actionListenerBeforeUpdate =
                mModel.get(0).model.get(TabProperties.TAB_SELECTED_LISTENER);

        // Mock that tab1 and tab3 are in the same group and group root id is TAB1_ID.
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, tab3));
        createTabGroup(tabs, TAB1_ID, TAB_GROUP_ID);

        assertEquals(2, mModel.size());

        mMediator.setActionOnAllRelatedTabsForTesting(false);
        doReturn(true).when(mTabGroupModelFilter).tabGroupExistsForRootId(TAB1_ID);

        mMediatorTabModelObserver.willCloseTab(mTab1, false, true);

        assertEquals(1, mModel.size());

        TabListMediator.TabActionListener actionListenerAfterUpdate =
                mModel.get(0).model.get(TabProperties.TAB_SELECTED_LISTENER);
        // The selection listener should remain unchanged, since the property model of the tab group
        // should not get updated when the closure is triggered from outside the tab switcher.
        assertThat(actionListenerBeforeUpdate, equalTo(actionListenerAfterUpdate));
    }

    @Test
    public void testQuickDeleteAnimationTabFiltering() {
        // Add five more tabs.
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        Tab tab4 = prepareTab(TAB4_ID, TAB4_TITLE, TAB4_URL);
        Tab tab5 = prepareTab(TAB5_ID, TAB5_TITLE, TAB5_URL);
        Tab tab6 = prepareTab(TAB6_ID, TAB6_TITLE, TAB6_URL);
        Tab tab7 = prepareTab(TAB7_ID, TAB7_TITLE, TAB7_URL);
        when(mTabModel.getTabAt(4)).thenReturn(tab7);

        // Mock that tab3 and tab4 are in the same group and group root id is TAB3_ID.
        List<Tab> groupTabs1 = new ArrayList<>(Arrays.asList(tab3, tab4));
        createTabGroup(groupTabs1, TAB3_ID, TAB_GROUP_ID, 2);
        when(mTabGroupModelFilter.getTabAt(2)).thenReturn(tab3);

        // Mock that tab5 and tab6 are in the same group and group root id is TAB5_ID.
        List<Tab> groupTabs2 = new ArrayList<>(Arrays.asList(tab5, tab6));
        createTabGroup(groupTabs2, TAB5_ID, TAB_GROUP_ID, 3);
        when(mTabGroupModelFilter.getTabAt(3)).thenReturn(tab5);

        Rect tab1Rect = new Rect();
        tab1Rect.bottom = 1;
        when(mTabListRecyclerView.getRectOfCurrentThumbnail(0, TAB1_ID)).thenReturn(tab1Rect);

        Rect tab2Rect = new Rect();
        tab2Rect.bottom = 1;
        when(mTabListRecyclerView.getRectOfCurrentThumbnail(1, TAB2_ID)).thenReturn(tab2Rect);

        Rect tab3Rect = new Rect();
        tab3Rect.bottom = 2;
        when(mTabListRecyclerView.getRectOfCurrentThumbnail(2, TAB3_ID)).thenReturn(tab3Rect);

        Rect tab5Rect = new Rect();
        tab5Rect.bottom = 2;
        when(mTabListRecyclerView.getRectOfCurrentThumbnail(3, TAB5_ID)).thenReturn(tab5Rect);

        // Mock tab7 is outside the screen view.
        when(mTabListRecyclerView.getRectOfCurrentThumbnail(4, TAB7_ID)).thenReturn(null);

        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3, tab5, tab7));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(5));

        TreeMap<Integer, List<Integer>> resultMap = new TreeMap<>();

        List<Tab> tabsToFade = new ArrayList<>(Arrays.asList(mTab1, tab4, tab5, tab6, tab7));

        mMediator.getOrderOfTabsForQuickDeleteAnimation(
                mTabListRecyclerView, tabsToFade, resultMap);

        assertThat(resultMap.keySet(), contains(1, 2));

        // Tab 1 and group tab 5 & 6 should be filtered for animation.
        assertThat(resultMap.get(1), contains(0));
        assertThat(resultMap.get(2), contains(3));
    }

    private void setUpCloseButtonDescriptionString(boolean isGroup) {
        if (isGroup) {
            doAnswer(
                            invocation -> {
                                String title = invocation.getArgument(1);
                                String num = invocation.getArgument(2);
                                return String.format("Close %s group with %s tabs", title, num);
                            })
                    .when(mActivity)
                    .getString(anyInt(), anyString(), anyString());

            doAnswer(
                            invocation -> {
                                String num = invocation.getArgument(1);
                                return String.format("Close tab group with %s tabs", num);
                            })
                    .when(mActivity)
                    .getString(anyInt(), anyString());
        } else {
            doAnswer(
                            invocation -> {
                                String title = invocation.getArgument(1);
                                return String.format("Close %s tab", title);
                            })
                    .when(mActivity)
                    .getString(anyInt(), anyString());
        }
    }

    private void setUpTabGroupCardDescriptionString() {
        doAnswer(
                        invocation -> {
                            String title = invocation.getArgument(1);
                            String num = invocation.getArgument(2);
                            return String.format("Expand %s tab group with %s tabs.", title, num);
                        })
                .when(mActivity)
                .getString(anyInt(), anyString(), anyString());

        doAnswer(
                        invocation -> {
                            String num = invocation.getArgument(1);
                            return String.format("Expand tab group with %s tabs.", num);
                        })
                .when(mActivity)
                .getString(anyInt(), anyString());
    }

    // initAndAssertAllProperties called with regular mMediator
    private void initAndAssertAllProperties() {
        initAndAssertAllProperties(mMediator);
    }

    // initAndAssertAllProperties called with regular mMediator
    private void initAndAssertAllProperties(int extraTabCount) {
        int index = mTabModel.getCount();
        int totalCount = mTabModel.getCount() + extraTabCount;
        while (index < totalCount) {
            Tab tab = prepareTab(index, TAB1_TITLE, TAB1_URL);
            doReturn(tab).when(mTabModel).getTabAt(index);
            doReturn(index).when(mTabModel).indexOf(tab);
            index++;
        }
        doReturn(totalCount).when(mTabModel).getCount();
        initAndAssertAllProperties(mMediator);
    }

    // initAndAssertAllProperties called with custom mMediator (e.g. if spy needs to be used)
    private void initAndAssertAllProperties(TabListMediator mediator) {
        List<Tab> tabs = new ArrayList<>();
        for (int i = 0; i < mTabModel.getCount(); i++) {
            tabs.add(mTabModel.getTabAt(i));
        }
        mediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        for (Callback<TabFavicon> callback : mCallbackCaptor.getAllValues()) {
            callback.onResult(mFavicon);
        }

        assertThat(mModel.size(), equalTo(mTabModel.getCount()));

        assertThat(mModel.get(0).model.get(TabProperties.TAB_ID), equalTo(TAB1_ID));
        assertThat(mModel.get(1).model.get(TabProperties.TAB_ID), equalTo(TAB2_ID));

        assertThat(mModel.get(0).model.get(TabProperties.TITLE), equalTo(TAB1_TITLE));
        assertThat(mModel.get(1).model.get(TabProperties.TITLE), equalTo(TAB2_TITLE));

        assertNotNull(mModel.get(0).model.get(TabProperties.FAVICON_FETCHER));
        assertNotNull(mModel.get(1).model.get(TabProperties.FAVICON_FETCHER));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(false));

        assertThat(
                mModel.get(0).model.get(TabProperties.THUMBNAIL_FETCHER),
                instanceOf(TabListMediator.ThumbnailFetcher.class));
        assertThat(
                mModel.get(1).model.get(TabProperties.THUMBNAIL_FETCHER),
                instanceOf(TabListMediator.ThumbnailFetcher.class));

        if (mModel.get(0).model.get(TabProperties.SELECTABLE_TAB_CLICKED_LISTENER) != null) return;

        assertThat(
                mModel.get(0).model.get(TabProperties.TAB_SELECTED_LISTENER),
                instanceOf(TabListMediator.TabActionListener.class));
        assertThat(
                mModel.get(1).model.get(TabProperties.TAB_SELECTED_LISTENER),
                instanceOf(TabListMediator.TabActionListener.class));

        assertThat(
                mModel.get(0).model.get(TabProperties.TAB_CLOSED_LISTENER),
                instanceOf(TabListMediator.TabActionListener.class));
        assertThat(
                mModel.get(1).model.get(TabProperties.TAB_CLOSED_LISTENER),
                instanceOf(TabListMediator.TabActionListener.class));
    }

    private Tab prepareTab(int id, String title, GURL url) {
        Tab tab = TabUiUnitTestUtils.prepareTab(id, title, url);
        when(tab.getView()).thenReturn(mock(View.class));
        doReturn(true).when(tab).isIncognito();
        when(mTitleProvider.getTitle(mActivity, PseudoTab.fromTab(tab))).thenReturn(title);
        int count = mTabModel.getCount();
        doReturn(tab).when(mTabModel).getTabAt(count);
        doReturn(count).when(mTabModel).getCount();
        when(mTabGroupModelFilter.getRelatedTabCountForRootId(id)).thenReturn(1);
        return tab;
    }

    private SimpleRecyclerViewAdapter.ViewHolder prepareViewHolder(int id, int position) {
        SimpleRecyclerViewAdapter.ViewHolder viewHolder =
                mock(SimpleRecyclerViewAdapter.ViewHolder.class);
        viewHolder.model =
                new PropertyModel.Builder(TabProperties.ALL_KEYS_TAB_GRID)
                        .with(TabProperties.TAB_ID, id)
                        .with(CARD_TYPE, TAB)
                        .build();
        doReturn(position).when(viewHolder).getAdapterPosition();
        return viewHolder;
    }

    private RecyclerView.ViewHolder prepareFakeViewHolder(View itemView, int index) {
        RecyclerView.ViewHolder viewHolder = new RecyclerView.ViewHolder(itemView) {};
        when(mRecyclerView.findViewHolderForAdapterPosition(index)).thenReturn(viewHolder);
        return viewHolder;
    }

    private TabGridItemTouchHelperCallback getItemTouchHelperCallback() {
        return (TabGridItemTouchHelperCallback) mMediator.getItemTouchHelperCallback(0f, 0f, 0f);
    }

    private void setUpTabListMediator(@TabListMediatorType int type, @TabListMode int mode) {
        if (mMediator != null) {
            mMediator.resetWithListOfTabs(null, false);
            mMediator.destroy();
            mMediator = null;
        }
        doNothing()
                .when(mTabGroupModelFilter)
                .addTabGroupObserver(mTabGroupModelFilterObserverCaptor.capture());
        doNothing().when(mTabGroupModelFilter).addObserver(mTabModelObserverCaptor.capture());

        TabListMediator.TabGridDialogHandler handler =
                type == TabListMediatorType.TAB_GRID_DIALOG ? mTabGridDialogHandler : null;
        boolean actionOnRelatedTabs = type == TabListMediatorType.TAB_SWITCHER;
        int uiType = 0;
        if (type == TabListMediatorType.TAB_SWITCHER
                || type == TabListMediatorType.TAB_GRID_DIALOG) {
            uiType = TabProperties.UiType.CLOSABLE;
        } else if (type == TabListMediatorType.TAB_STRIP) {
            uiType = TabProperties.UiType.STRIP;
        }

        mMediator =
                new TabListMediator(
                        mActivity,
                        mModel,
                        mode,
                        mCurrentTabModelFilterSupplier,
                        () -> mTabModel,
                        getTabThumbnailCallback(),
                        mTitleProvider,
                        mTabListFaviconProvider,
                        actionOnRelatedTabs,
                        null,
                        mGridCardOnClickListenerProvider,
                        handler,
                        null,
                        getClass().getSimpleName(),
                        uiType);
        TrackerFactory.setTrackerForTests(mTracker);
        mMediator.registerOrientationListener(mGridLayoutManager);

        // TabGroupModelFilterObserver is registered when native is ready.
        int tabGroupModelFilterObserverCount =
                mTabGroupModelFilterObserverCaptor.getAllValues().size();
        int tabModelObserverCount = mTabModelObserverCaptor.getAllValues().size();
        mMediator.initWithNative(mProfile);

        assertThat(
                mTabModelObserverCaptor.getAllValues().size(), equalTo(tabModelObserverCount + 1));
        assertThat(
                mTabGroupModelFilterObserverCaptor.getAllValues().size(),
                equalTo(tabGroupModelFilterObserverCount + 1));

        mMediatorTabModelObserver =
                mTabModelObserverCaptor.getAllValues().get(tabModelObserverCount);
        mMediatorTabGroupModelFilterObserver =
                mTabGroupModelFilterObserverCaptor
                        .getAllValues()
                        .get(tabGroupModelFilterObserverCount);

        initAndAssertAllProperties();

        mTabObserver = mTabObserverCaptor.getValue();
    }

    private void createTabGroup(List<Tab> tabs, int rootId, @Nullable Token tabGroupId) {
        when(mTabGroupModelFilter.getRelatedTabCountForRootId(rootId)).thenReturn(tabs.size());
        for (Tab tab : tabs) {
            when(mTabGroupModelFilter.getRelatedTabList(tab.getId())).thenReturn(tabs);
            when(mTabGroupModelFilter.isTabInTabGroup(tab)).thenReturn(true);
            when(tab.getRootId()).thenReturn(rootId);
            when(tab.getTabGroupId()).thenReturn(tabGroupId);
        }
    }

    private void createTabGroup(List<Tab> tabs, int rootId, @Nullable Token tabGroupId, int index) {
        when(mTabGroupModelFilter.getRelatedTabCountForRootId(rootId)).thenReturn(tabs.size());
        for (Tab tab : tabs) {
            when(mTabGroupModelFilter.getRelatedTabList(tab.getId())).thenReturn(tabs);
            when(mTabGroupModelFilter.isTabInTabGroup(tab)).thenReturn(true);
            when(tab.getRootId()).thenReturn(rootId);
            when(tab.getTabGroupId()).thenReturn(tabGroupId);
            when(mTabGroupModelFilter.indexOf(tab)).thenReturn(index);
        }
    }

    private void mockOptimizationGuideResponse(
            @OptimizationGuideDecision int decision, Map<GURL, Any> responses) {
        for (Map.Entry<GURL, Any> responseEntry : responses.entrySet()) {
            doAnswer(
                            new Answer<Void>() {
                                @Override
                                public Void answer(InvocationOnMock invocation) {
                                    OptimizationGuideCallback callback =
                                            (OptimizationGuideCallback)
                                                    invocation.getArguments()[3];
                                    callback.onOptimizationGuideDecision(
                                            decision, responseEntry.getValue());
                                    return null;
                                }
                            })
                    .when(mOptimizationGuideBridgeJniMock)
                    .canApplyOptimization(
                            anyLong(),
                            eq(responseEntry.getKey()),
                            anyInt(),
                            any(OptimizationGuideCallback.class));
        }
    }

    private void initWithThreeTabs() {
        Tab tab3 = prepareTab(TAB3_ID, TAB3_TITLE, TAB3_URL);
        List<Tab> tabs = new ArrayList<>(Arrays.asList(mTab1, mTab2, tab3));
        mMediator.resetWithListOfTabs(PseudoTab.getListOfPseudoTab(tabs), false);
        assertThat(mModel.size(), equalTo(3));
        assertThat(mModel.get(0).model.get(TabProperties.IS_SELECTED), equalTo(true));
        assertThat(mModel.get(1).model.get(TabProperties.IS_SELECTED), equalTo(false));
        assertThat(mModel.get(2).model.get(TabProperties.IS_SELECTED), equalTo(false));
    }

    private void addSpecialItem(int index, @UiType int uiType, int itemIdentifier) {
        PropertyModel model = mock(PropertyModel.class);
        when(model.get(CARD_TYPE)).thenReturn(MESSAGE);
        if (uiType == TabProperties.UiType.MESSAGE
                || uiType == TabProperties.UiType.LARGE_MESSAGE) {
            when(model.get(MESSAGE_TYPE)).thenReturn(itemIdentifier);
        }
        // Avoid auto-updating the layout when inserting the special card.
        doReturn(1).when(mSpanSizeLookup).getSpanSize(anyInt());
        mMediator.addSpecialItemToModel(index, uiType, model);
    }

    private void prepareTestMaybeShowPriceWelcomeMessage() {
        initAndAssertAllProperties();
        setPriceTrackingEnabledForTesting(true);
        PriceTrackingFeatures.setIsSignedInAndSyncEnabledForTesting(true);
        PriceTrackingUtilities.SHARED_PREFERENCES_MANAGER.writeBoolean(
                PriceTrackingUtilities.PRICE_WELCOME_MESSAGE_CARD, true);
        mPriceDrop = new PriceDrop("1", "2");
        mPriceTabData = new PriceTabData(TAB1_ID, mPriceDrop);
        doReturn(mPriceDrop).when(mShoppingPersistedTabData).getPriceDrop();
    }

    private void prepareRecyclerViewForScroll() {
        View seenView = mock(View.class);
        for (int i = 0; i < mTabModel.getCount(); i++) {
            when(mRecyclerView.getChildAt(i)).thenReturn(seenView);
        }

        doReturn(true).when(mGridLayoutManager).isViewPartiallyVisible(seenView, false, true);
        doReturn(mTabModel.getCount()).when(mRecyclerView).getChildCount();
    }

    private ThumbnailProvider getTabThumbnailCallback() {
        return (tabId, thumbnailSize, callback, forceUpdate, writeToCache, isSelected) -> {
            mTabContentManager.getTabThumbnailWithCallback(
                    tabId, thumbnailSize, callback, forceUpdate, writeToCache);
        };
    }

    private static void setPriceTrackingEnabledForTesting(boolean value) {
        FeatureList.TestValues testValues = new FeatureList.TestValues();
        testValues.addFeatureFlagOverride(ChromeFeatureList.COMMERCE_PRICE_TRACKING, true);
        testValues.addFieldTrialParamOverride(
                ChromeFeatureList.COMMERCE_PRICE_TRACKING,
                PriceTrackingFeatures.PRICE_DROP_IPH_ENABLED_PARAM,
                String.valueOf(value));
        FeatureList.setTestValues(testValues);

        PriceTrackingFeatures.setPriceTrackingEnabledForTesting(value);
    }
}
