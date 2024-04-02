// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tab.tab_restore;

import androidx.annotation.VisibleForTesting;

import org.chromium.base.Token;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tabmodel.TabList;
import org.chromium.chrome.browser.tabmodel.TabModelFilter;
import org.chromium.chrome.browser.tabmodel.TabModelObserver;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupColorUtils;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupModelFilter;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupTitleUtils;
import org.chromium.components.tab_groups.TabGroupColorId;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/** A tab model observer for managing bulk closures. */
public class HistoricalTabModelObserver implements TabModelObserver {
    private final TabGroupModelFilter mTabGroupModelFilter;
    private final HistoricalTabSaver mHistoricalTabSaver;

    /**
     * @param tabModelFilter The tab model filter to observe tab closures in.
     */
    public HistoricalTabModelObserver(TabModelFilter tabModelFilter) {
        this(tabModelFilter, new HistoricalTabSaverImpl(tabModelFilter.getTabModel()));
    }

    @VisibleForTesting
    public HistoricalTabModelObserver(
            TabModelFilter tabModelFilter, HistoricalTabSaver historicalTabSaver) {
        mTabGroupModelFilter = (TabGroupModelFilter) tabModelFilter;
        mHistoricalTabSaver = historicalTabSaver;

        tabModelFilter.getTabModel().addObserver(this);
    }

    /** Removes observers. */
    public void destroy() {
        mTabGroupModelFilter.getTabModel().removeObserver(this);
    }

    @Override
    public void onFinishingMultipleTabClosure(List<Tab> tabs) {
        if (tabs.isEmpty()) return;

        if (tabs.size() == 1) {
            Tab tab = tabs.get(0);
            if (!isTabGroupWithOneTab(tab)) {
                mHistoricalTabSaver.createHistoricalTab(tab);
                return;
            }
        }

        buildGroupsAndCreateClosure(tabs);
    }

    private void buildGroupsAndCreateClosure(List<Tab> tabs) {
        HashMap<Integer, HistoricalEntry> idToGroup = new HashMap<>();
        List<HistoricalEntry> entries = new ArrayList<>();

        for (Tab tab : tabs) {
            // {@link TabGroupModelFilter} removes tabs from its data model as soon as they are
            // pending closure so it cannot be directly relied upon for group structure. Instead
            // rely on the underlying root ID in the tab's persisted data which is used to restore
            // groups across an pending closure cancellation (undo). The root ID is the group ID
            // unless the tab is ungrouped in which case the root ID is the tab's ID.
            int rootId = tab.getRootId();
            if (idToGroup.containsKey(rootId)) {
                idToGroup.get(rootId).getTabs().add(tab);
                continue;
            }
            // null title for default title is handled in HistoricalTabSaver.
            String title = TabGroupTitleUtils.getTabGroupTitle(rootId);
            // Give a tab group the first color in the color list as a placeholder.
            @TabGroupColorId int color = TabGroupColorId.GREY;
            if (ChromeFeatureList.isEnabled(ChromeFeatureList.TAB_GROUP_PARITY_ANDROID)) {
                color = TabGroupColorUtils.getOrCreateTabGroupColor(rootId, mTabGroupModelFilter);
            }
            List<Tab> groupTabs = new ArrayList<Tab>();
            groupTabs.add(tab);
            HistoricalEntry historicalGroup =
                    new HistoricalEntry(rootId, tab.getTabGroupId(), title, color, groupTabs);
            entries.add(historicalGroup);
            idToGroup.put(rootId, historicalGroup);
        }

        // If only a subset of tabs in the tab group are closing tabs should be saved individually
        // so that a duplicate of the tab group isn't created on restore.
        // TODO(crbug/327166316): Wire up tab group IDs when saving in native so that the tabs
        // can be restored into their prior group if it still exists.
        List<HistoricalEntry> groupAdjustedEntries = new ArrayList<>();
        for (HistoricalEntry entry : entries) {
            if (shouldSaveSeparateTabs(entry)) {
                for (Tab tab : entry.getTabs()) {
                    groupAdjustedEntries.add(new HistoricalEntry(tab));
                }
            } else {
                groupAdjustedEntries.add(entry);
            }
        }

        mHistoricalTabSaver.createHistoricalBulkClosure(groupAdjustedEntries);
    }

    private boolean shouldSaveSeparateTabs(HistoricalEntry entry) {
        if (entry.getRootId() == Tab.INVALID_TAB_ID) return false;

        int rootId = entry.getRootId();
        boolean groupExists = mTabGroupModelFilter.tabGroupExistsForRootId(rootId);
        if (groupExists
                && entry.getTabs().size()
                        != mTabGroupModelFilter.getRelatedTabCountForRootId(rootId)) {
            // Case: Group information not lost yet (non-undoable closure). Rely on whether all the
            // tabs in the group are closing.
            return true;
        } else if (!groupExists) {
            // Case: Group information already lost (undoable closure). Rely on whether any unclosed
            // tabs share a root ID with the closing group.
            TabList comprehensiveModel = mTabGroupModelFilter.getTabModel().getComprehensiveModel();
            for (int i = 0; i < comprehensiveModel.getCount(); i++) {
                if (rootId == comprehensiveModel.getTabAt(i).getRootId()) return true;
            }
        }
        return false;
    }

    private boolean isTabGroupWithOneTab(Tab tab) {
        int rootId = tab.getRootId();
        if (!mTabGroupModelFilter.tabGroupExistsForRootId(rootId)) {
            Token tabGroupId = tab.getTabGroupId();
            if (tabGroupId == null) return false;
            // Case: Group information already lost (undoable closure). Rely on whether the tab
            // still has a tab group ID.
            TabList comprehensiveModel = mTabGroupModelFilter.getTabModel().getComprehensiveModel();
            for (int i = 0; i < comprehensiveModel.getCount(); i++) {
                if (tabGroupId.equals(comprehensiveModel.getTabAt(i).getTabGroupId())) return false;
            }
            return true;
        } else {
            // Case: Group information not lost yet (non-undoable closure). Rely on whether the tab
            // is the only tab in its tab group.
            return mTabGroupModelFilter.isTabInTabGroup(tab)
                    && mTabGroupModelFilter.getRelatedTabCountForRootId(rootId) == 1;
        }
    }
}
