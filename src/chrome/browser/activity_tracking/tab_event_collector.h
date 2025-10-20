// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_TAB_EVENT_COLLECTOR_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_TAB_EVENT_COLLECTOR_H_

#include <map>
#include <memory>

#include "base/memory/raw_ptr.h"
#include "build/build_config.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
class TabStripModel;
#else
#include "chrome/browser/ui/android/tab_model/tab_model_observer.h"
class TabModel;
#endif

namespace content {
class WebContents;
}

namespace activity_tracking {

class NavigationEventCollector;
class PrivacyFilter;

// TabEventCollector observes tab operations across all TabStripModels
// and logs detailed information about each tab event.
//
// This class tracks:
// - Tab creation/insertion
// - Tab removal/closure
// - Tab activation/switching
// - Tab moves/reordering
// - Tab replacement (prerendering)
// - Tab grouping operations
class TabEventCollector {
 public:
  explicit TabEventCollector(PrivacyFilter* privacy_filter, 
                             NavigationEventCollector* navigation_collector);
  ~TabEventCollector();

  TabEventCollector(const TabEventCollector&) = delete;
  TabEventCollector& operator=(const TabEventCollector&) = delete;

#if !BUILDFLAG(IS_ANDROID)
  // Start observing a TabStripModel (Desktop)
  void ObserveTabStripModel(TabStripModel* tab_strip_model);

  // Stop observing a specific TabStripModel (Desktop)
  void StopObservingTabStripModel(TabStripModel* tab_strip_model);
#else
  // Start observing a TabModel (Android)
  void ObserveTabModel(TabModel* tab_model);

  // Stop observing a specific TabModel (Android)
  void StopObservingTabModel(TabModel* tab_model);
#endif

  // Stop all observations and clean up
  void Shutdown();

 private:
#if !BUILDFLAG(IS_ANDROID)
  // Per-TabStripModel observer that tracks tab events (Desktop)
  class TabStripObserver : public TabStripModelObserver {
   public:
    TabStripObserver(TabStripModel* tab_strip_model,
                     PrivacyFilter* privacy_filter);
    ~TabStripObserver() override;

    // TabStripModelObserver implementation:
    void OnTabStripModelChanged(
        TabStripModel* tab_strip_model,
        const TabStripModelChange& change,
        const TabStripSelectionChange& selection) override;

    void TabChangedAt(content::WebContents* contents,
                      int index,
                      TabChangeType change_type) override;

    void TabPinnedStateChanged(TabStripModel* tab_strip_model,
                               content::WebContents* contents,
                               int index) override;

    void TabGroupedStateChanged(std::optional<tab_groups::TabGroupId> group,
                                content::WebContents* contents,
                                int index) override;

    void OnTabGroupChanged(const TabGroupChange& change) override;

   private:
    void LogTabInserted(const TabStripModelChange::Insert* insert);
    void LogTabRemoved(const TabStripModelChange::Remove* remove);
    void LogTabMoved(const TabStripModelChange::Move* move);
    void LogTabReplaced(const TabStripModelChange::Replace* replace);
    void LogTabActivation(const TabStripSelectionChange& selection);
    void LogTabChanged(content::WebContents* contents,
                       int index,
                       TabChangeType change_type);
    void LogTabPinnedStateChanged(content::WebContents* contents,
                                  int index,
                                  bool pinned);
    void LogTabGroupChange(const TabGroupChange& change);

    std::string GetTabInfo(content::WebContents* contents, int index);

    raw_ptr<TabStripModel> tab_strip_model_;
    raw_ptr<PrivacyFilter> privacy_filter_;
  };

  // Map of TabStripModel to their observers
  std::map<raw_ptr<TabStripModel>, std::unique_ptr<TabStripObserver>>
      observers_;
#else
  // Per-TabModel observer that tracks tab events (Android)
  class TabModelObserverImpl : public TabModelObserver {
   public:
    TabModelObserverImpl(TabModel* tab_model, 
                        PrivacyFilter* privacy_filter,
                        NavigationEventCollector* navigation_collector);
    ~TabModelObserverImpl() override;

    // TabModelObserver implementation:
    void DidSelectTab(TabAndroid* tab, TabModel::TabSelectionType type, int last_id) override;
    void WillCloseTab(TabAndroid* tab) override;
    void DidAddTab(TabAndroid* tab, TabModel::TabLaunchType type) override;
    void DidMoveTab(TabAndroid* tab, int new_index, int old_index) override;

   private:
    raw_ptr<TabModel> tab_model_;
    raw_ptr<PrivacyFilter> privacy_filter_;
    raw_ptr<NavigationEventCollector> navigation_collector_;
  };

  // Map of TabModel to their observers
  std::map<raw_ptr<TabModel>, std::unique_ptr<TabModelObserverImpl>>
      android_observers_;
#endif

  raw_ptr<PrivacyFilter> privacy_filter_;
  raw_ptr<NavigationEventCollector> navigation_collector_;
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_TAB_EVENT_COLLECTOR_H_

