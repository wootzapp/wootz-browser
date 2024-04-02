// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import static org.chromium.chrome.browser.tasks.tab_management.TabGroupRowProperties.COLOR_INDEX;
import static org.chromium.chrome.browser.tasks.tab_management.TabGroupRowProperties.TITLE_DATA;

import android.graphics.Color;

import androidx.core.util.Pair;
import androidx.test.filters.SmallTest;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.shadows.ShadowLooper;

import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.tab.MockTab;
import org.chromium.chrome.browser.tabmodel.TabModel;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupColorUtils;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupModelFilter;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupModelFilterObserver;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupTitleUtils;
import org.chromium.components.tab_groups.TabGroupColorId;
import org.chromium.ui.modelutil.MVCListAdapter.ModelList;
import org.chromium.ui.modelutil.PropertyModel;

import java.util.Arrays;
import java.util.Collections;

/** Tests for {@link TabGroupListMediator}. */
@RunWith(BaseRobolectricTestRunner.class)
public class TabGroupListMediatorUnitTest {
    @Rule public MockitoRule mMockitoRule = MockitoJUnit.rule();

    @Mock private TabGroupModelFilter mTabGroupModelFilter;
    @Mock private TabModel mTabModel;
    @Mock private Profile mProfile;

    @Captor private ArgumentCaptor<TabGroupModelFilterObserver> mTabGroupModelFilterObserverCaptor;

    private ModelList mModelList;

    @Before
    public void setUp() {
        mModelList = new ModelList();
    }

    @Test
    @SmallTest
    public void testNoTabs() {
        when(mTabGroupModelFilter.getCount()).thenReturn(0);
        new TabGroupListMediator(mModelList, mTabGroupModelFilter);
        assertEquals(0, mModelList.size());
    }

    @Test
    @SmallTest
    public void testNoTabGroups() {
        MockTab tab = new MockTab(0, mProfile);
        when(mTabGroupModelFilter.getCount()).thenReturn(1);
        when(mTabGroupModelFilter.getTabAt(0)).thenReturn(tab);
        when(mTabGroupModelFilter.isTabInTabGroup(tab)).thenReturn(false);

        new TabGroupListMediator(mModelList, mTabGroupModelFilter);
        assertEquals(0, mModelList.size());
    }

    @Test
    @SmallTest
    public void testOneGroup() {
        MockTab tab = new MockTab(0, mProfile);
        TabGroupTitleUtils.storeTabGroupTitle(0, "Title");
        TabGroupColorUtils.storeTabGroupColor(0, TabGroupColorId.GREY);
        when(mTabGroupModelFilter.getCount()).thenReturn(1);
        when(mTabGroupModelFilter.getTabAt(0)).thenReturn(tab);
        when(mTabGroupModelFilter.isTabInTabGroup(tab)).thenReturn(true);
        when(mTabGroupModelFilter.getRelatedTabList(0)).thenReturn(Collections.singletonList(tab));

        new TabGroupListMediator(mModelList, mTabGroupModelFilter);
        assertEquals(1, mModelList.size());

        PropertyModel model = mModelList.get(0).model;
        assertEquals(new Pair<>("Title", 1), model.get(TITLE_DATA));
        assertEquals(TabGroupColorId.GREY, model.get(COLOR_INDEX));
    }

    @Test
    @SmallTest
    public void testDifferentRoot() {
        // Tabs look like [1, 2], where 1 was last shown but 2 is root.
        MockTab tab1 = new MockTab(1, mProfile);
        tab1.setRootId(2);

        MockTab tab2 = new MockTab(2, mProfile);
        TabGroupTitleUtils.storeTabGroupTitle(2, "Title");
        TabGroupColorUtils.storeTabGroupColor(2, TabGroupColorId.GREY);

        when(mTabGroupModelFilter.getTabModel()).thenReturn(mTabModel);
        when(mTabGroupModelFilter.getCount()).thenReturn(1);
        when(mTabGroupModelFilter.getTabAt(0)).thenReturn(tab1);
        when(mTabGroupModelFilter.getTabAt(1)).thenReturn(tab2);
        when(mTabGroupModelFilter.isTabInTabGroup(tab1)).thenReturn(true);
        when(mTabGroupModelFilter.getRelatedTabList(2)).thenReturn(Arrays.asList(tab1, tab2));
        when(mTabModel.getCount()).thenReturn(2);
        when(mTabModel.getTabAt(0)).thenReturn(tab1);
        when(mTabModel.getTabAt(1)).thenReturn(tab2);

        new TabGroupListMediator(mModelList, mTabGroupModelFilter);
        assertEquals(1, mModelList.size());

        PropertyModel model = mModelList.get(0).model;
        assertEquals(new Pair<>("Title", 2), model.get(TITLE_DATA));
        assertEquals(TabGroupColorId.GREY, model.get(COLOR_INDEX));
    }

    @Test
    @SmallTest
    public void testTwoGroups() {
        // Tabs look like: [1, 2], [3, 4, 5].
        MockTab tab1 = new MockTab(1, mProfile);
        TabGroupTitleUtils.storeTabGroupTitle(1, "Foo");
        TabGroupColorUtils.storeTabGroupColor(1, TabGroupColorId.BLUE);

        MockTab tab2 = new MockTab(2, mProfile);
        MockTab tab3 = new MockTab(3, mProfile);
        TabGroupTitleUtils.storeTabGroupTitle(3, "Bar");
        TabGroupColorUtils.storeTabGroupColor(3, TabGroupColorId.RED);
        MockTab tab4 = new MockTab(4, mProfile);
        MockTab tab5 = new MockTab(5, mProfile);

        when(mTabGroupModelFilter.getCount()).thenReturn(2);
        when(mTabGroupModelFilter.getTabAt(0)).thenReturn(tab1);
        when(mTabGroupModelFilter.getTabAt(1)).thenReturn(tab3);
        when(mTabGroupModelFilter.isTabInTabGroup(tab1)).thenReturn(true);
        when(mTabGroupModelFilter.isTabInTabGroup(tab3)).thenReturn(true);
        when(mTabGroupModelFilter.getRelatedTabList(1)).thenReturn(Arrays.asList(tab1, tab2));
        when(mTabGroupModelFilter.getRelatedTabList(3)).thenReturn(Arrays.asList(tab3, tab4, tab5));

        new TabGroupListMediator(mModelList, mTabGroupModelFilter);
        assertEquals(2, mModelList.size());

        PropertyModel model1 = mModelList.get(0).model;
        assertEquals(new Pair<>("Foo", 2), model1.get(TITLE_DATA));
        assertEquals(TabGroupColorId.BLUE, model1.get(COLOR_INDEX));

        PropertyModel model2 = mModelList.get(1).model;
        assertEquals(new Pair<>("Bar", 3), model2.get(TITLE_DATA));
        assertEquals(TabGroupColorId.RED, model2.get(COLOR_INDEX));
    }

    @Test
    @SmallTest
    public void testObservation() {
        TabGroupListMediator mediator = new TabGroupListMediator(mModelList, mTabGroupModelFilter);
        assertEquals(0, mModelList.size());

        MockTab tab = new MockTab(0, mProfile);
        TabGroupTitleUtils.storeTabGroupTitle(0, "Title");
        TabGroupColorUtils.storeTabGroupColor(0, Color.GREEN);
        when(mTabGroupModelFilter.getCount()).thenReturn(1);
        when(mTabGroupModelFilter.getTabAt(0)).thenReturn(tab);
        when(mTabGroupModelFilter.isTabInTabGroup(tab)).thenReturn(true);
        when(mTabGroupModelFilter.getRelatedTabList(0)).thenReturn(Collections.singletonList(tab));
        assertEquals(0, mModelList.size());

        verify(mTabGroupModelFilter)
                .addTabGroupObserver(mTabGroupModelFilterObserverCaptor.capture());
        mTabGroupModelFilterObserverCaptor.getValue().didMergeTabToGroup(null, 0);
        assertEquals(0, mModelList.size());

        ShadowLooper.idleMainLooper();
        assertEquals(1, mModelList.size());

        when(mTabGroupModelFilter.getCount()).thenReturn(2);
        when(mTabGroupModelFilter.getTabAt(1)).thenReturn(tab);
        mTabGroupModelFilterObserverCaptor.getValue().didMergeTabToGroup(null, 0);
        ShadowLooper.idleMainLooper();
        assertEquals(2, mModelList.size());

        when(mTabGroupModelFilter.getCount()).thenReturn(0);
        mediator.destroy();
        mTabGroupModelFilterObserverCaptor.getValue().didMergeTabToGroup(null, 0);
        ShadowLooper.idleMainLooper();
        assertEquals(2, mModelList.size());
    }
}
