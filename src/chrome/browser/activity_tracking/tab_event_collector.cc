// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/tab_event_collector.h"

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/activity_tracking/navigation_event_collector.h"
#include "chrome/browser/activity_tracking/privacy_filter.h"
#include "content/public/browser/web_contents.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#else
#include "chrome/browser/android/tab_android.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#endif

namespace activity_tracking {

TabEventCollector::TabEventCollector(PrivacyFilter* privacy_filter,
                                     NavigationEventCollector* navigation_collector)
    : privacy_filter_(privacy_filter),
      navigation_collector_(navigation_collector) {
  LOG(ERROR) << "[TabCollector] Initialized";
}

TabEventCollector::~TabEventCollector() {
  Shutdown();
}

#if !BUILDFLAG(IS_ANDROID)
// Desktop implementation
void TabEventCollector::ObserveTabStripModel(TabStripModel* tab_strip_model) {
  if (!tab_strip_model) {
    return;
  }

  // Check if already observing
  if (observers_.find(tab_strip_model) != observers_.end()) {
    return;
  }

  auto observer =
      std::make_unique<TabStripObserver>(tab_strip_model, privacy_filter_);
  observers_[tab_strip_model] = std::move(observer);

  LOG(ERROR) << "[TabCollector] Now observing TabStripModel with "
            << tab_strip_model->count() << " tabs";
}

void TabEventCollector::StopObservingTabStripModel(
    TabStripModel* tab_strip_model) {
  auto it = observers_.find(tab_strip_model);
  if (it != observers_.end()) {
    LOG(ERROR) << "[TabCollector] Stopped observing TabStripModel";
    observers_.erase(it);
  }
}

void TabEventCollector::Shutdown() {
  LOG(ERROR) << "[TabCollector] Shutting down";
  observers_.clear();
}

// Desktop TabStripObserver implementation

TabEventCollector::TabStripObserver::TabStripObserver(
    TabStripModel* tab_strip_model,
    PrivacyFilter* privacy_filter)
    : tab_strip_model_(tab_strip_model), privacy_filter_(privacy_filter) {
  tab_strip_model_->AddObserver(this);
}

TabEventCollector::TabStripObserver::~TabStripObserver() {
  if (tab_strip_model_) {
    tab_strip_model_->RemoveObserver(this);
  }
}

void TabEventCollector::TabStripObserver::OnTabStripModelChanged(
    TabStripModel* tab_strip_model,
    const TabStripModelChange& change,
    const TabStripSelectionChange& selection) {
  // Log tab structure changes
  switch (change.type()) {
    case TabStripModelChange::kInserted:
      LogTabInserted(change.GetInsert());
      break;
    case TabStripModelChange::kRemoved:
      LogTabRemoved(change.GetRemove());
      break;
    case TabStripModelChange::kMoved:
      LogTabMoved(change.GetMove());
      break;
    case TabStripModelChange::kReplaced:
      LogTabReplaced(change.GetReplace());
      break;
    case TabStripModelChange::kSelectionOnly:
      // Handled by selection change below
      break;
  }

  // Log selection/activation changes
  if (selection.active_tab_changed()) {
    LogTabActivation(selection);
  }
}

void TabEventCollector::TabStripObserver::TabChangedAt(
    content::WebContents* contents,
    int index,
    TabChangeType change_type) {
  LogTabChanged(contents, index, change_type);
}

void TabEventCollector::TabStripObserver::TabPinnedStateChanged(
    TabStripModel* tab_strip_model,
    content::WebContents* contents,
    int index) {
  bool is_pinned = tab_strip_model->IsTabPinned(index);
  LogTabPinnedStateChanged(contents, index, is_pinned);
}

void TabEventCollector::TabStripObserver::TabGroupedStateChanged(
    std::optional<tab_groups::TabGroupId> group,
    content::WebContents* contents,
    int index) {
  std::string url =
      privacy_filter_->FilterUrl(contents->GetVisibleURL().spec());

  if (group.has_value()) {
    LOG(INFO) << "[Tab] TAB_GROUPED"
              << " | Index: " << index << " | URL: " << url
              << " | Group ID: " << group.value().ToString();
  } else {
    LOG(INFO) << "[Tab] TAB_UNGROUPED"
              << " | Index: " << index << " | URL: " << url;
  }
}

void TabEventCollector::TabStripObserver::OnTabGroupChanged(
    const TabGroupChange& change) {
  LogTabGroupChange(change);
}

void TabEventCollector::TabStripObserver::LogTabInserted(
    const TabStripModelChange::Insert* insert) {
  for (const auto& content : insert->contents) {
    std::string url =
        privacy_filter_->FilterUrl(content.contents->GetVisibleURL().spec());

    LOG(INFO) << "[Tab] TAB_INSERTED"
              << " | Index: " << content.index << " | URL: " << url
              << " | Total Tabs: " << tab_strip_model_->count();
  }
}

void TabEventCollector::TabStripObserver::LogTabRemoved(
    const TabStripModelChange::Remove* remove) {
  for (const auto& removed_tab : remove->contents) {
    std::string url = privacy_filter_->FilterUrl(
        removed_tab.contents->GetVisibleURL().spec());

    std::string reason_str;
    switch (removed_tab.remove_reason) {
      case TabStripModelChange::RemoveReason::kDeleted:
        reason_str = "Deleted";
        break;
      case TabStripModelChange::RemoveReason::kInsertedIntoOtherTabStrip:
        reason_str = "Moved to other TabStrip";
        break;
    }

    LOG(INFO) << "[Tab] TAB_REMOVED"
              << " | Index: " << removed_tab.index << " | URL: " << url
              << " | Reason: " << reason_str
              << " | Remaining Tabs: " << tab_strip_model_->count();
  }
}

void TabEventCollector::TabStripObserver::LogTabMoved(
    const TabStripModelChange::Move* move) {
  std::string url =
      privacy_filter_->FilterUrl(move->contents->GetVisibleURL().spec());

  LOG(INFO) << "[Tab] TAB_MOVED"
            << " | From Index: " << move->from_index
            << " | To Index: " << move->to_index << " | URL: " << url;
}

void TabEventCollector::TabStripObserver::LogTabReplaced(
    const TabStripModelChange::Replace* replace) {
  std::string old_url = privacy_filter_->FilterUrl(
      replace->old_contents->GetVisibleURL().spec());
  std::string new_url = privacy_filter_->FilterUrl(
      replace->new_contents->GetVisibleURL().spec());

  LOG(INFO) << "[Tab] TAB_REPLACED"
            << " | Index: " << replace->index << " | Old URL: " << old_url
            << " | New URL: " << new_url
            << " | Reason: Prerender Activation";
}

void TabEventCollector::TabStripObserver::LogTabActivation(
    const TabStripSelectionChange& selection) {
  std::string old_url;
  if (selection.old_contents) {
    old_url = privacy_filter_->FilterUrl(
        selection.old_contents->GetVisibleURL().spec());
  } else {
    old_url = "(none)";
  }

  std::string new_url;
  std::string new_title;
  if (selection.new_contents) {
    new_url = privacy_filter_->FilterUrl(
        selection.new_contents->GetVisibleURL().spec());
    new_title = privacy_filter_->FilterTitle(
        base::UTF16ToUTF8(selection.new_contents->GetTitle()));
  } else {
    new_url = "(none)";
    new_title = "(none)";
  }

  std::string reason_str = "Unknown";
  if (selection.reason & TabStripModelObserver::CHANGE_REASON_USER_GESTURE) {
    reason_str = "User Gesture";
  } else if (selection.reason &
             TabStripModelObserver::CHANGE_REASON_REPLACED) {
    reason_str = "Replaced";
  } else if (selection.reason == TabStripModelObserver::CHANGE_REASON_NONE) {
    reason_str = "Programmatic";
  }

  LOG(INFO) << "[Tab] TAB_ACTIVATED"
            << " | Previous URL: " << old_url << " | New URL: " << new_url
            << " | New Title: " << new_title << " | Reason: " << reason_str;
}

void TabEventCollector::TabStripObserver::LogTabChanged(
    content::WebContents* contents,
    int index,
    TabChangeType change_type) {
  std::string url =
      privacy_filter_->FilterUrl(contents->GetVisibleURL().spec());
  std::string title =
      privacy_filter_->FilterTitle(base::UTF16ToUTF8(contents->GetTitle()));

  std::string change_type_str;
  switch (change_type) {
    case TabChangeType::kAll:
      change_type_str = "All";
      break;
    case TabChangeType::kLoadingOnly:
      change_type_str = "Loading";
      break;
  }

  LOG(INFO) << "[Tab] TAB_CHANGED"
            << " | Index: " << index << " | URL: " << url
            << " | Title: " << title << " | Change Type: " << change_type_str;
}

void TabEventCollector::TabStripObserver::LogTabPinnedStateChanged(
    content::WebContents* contents,
    int index,
    bool pinned) {
  std::string url =
      privacy_filter_->FilterUrl(contents->GetVisibleURL().spec());

  LOG(INFO) << "[Tab] TAB_PINNED_STATE_CHANGED"
            << " | Index: " << index << " | URL: " << url
            << " | Pinned: " << (pinned ? "Yes" : "No");
}

void TabEventCollector::TabStripObserver::LogTabGroupChange(
    const TabGroupChange& change) {
  std::string change_type_str;
  switch (change.type) {
    case TabGroupChange::kCreated:
      change_type_str = "Created";
      break;
    case TabGroupChange::kEditorOpened:
      change_type_str = "Editor Opened";
      break;
    case TabGroupChange::kContentsChanged:
      change_type_str = "Contents Changed";
      break;
    case TabGroupChange::kVisualsChanged:
      change_type_str = "Visuals Changed";
      break;
    case TabGroupChange::kMoved:
      change_type_str = "Moved";
      break;
    case TabGroupChange::kClosed:
      change_type_str = "Closed";
      break;
  }

  LOG(INFO) << "[Tab] TAB_GROUP_CHANGED"
            << " | Group ID: " << change.group.ToString()
            << " | Change Type: " << change_type_str;
}

std::string TabEventCollector::TabStripObserver::GetTabInfo(
    content::WebContents* contents,
    int index) {
  std::string url =
      privacy_filter_->FilterUrl(contents->GetVisibleURL().spec());
  std::string title =
      privacy_filter_->FilterTitle(base::UTF16ToUTF8(contents->GetTitle()));

  return "Index: " + base::NumberToString(index) + ", URL: " + url +
         ", Title: " + title;
}
#endif  // !BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(IS_ANDROID)
// Android implementation
void TabEventCollector::ObserveTabModel(TabModel* tab_model) {
  if (!tab_model) {
    return;
  }

  // Check if already observing
  if (android_observers_.find(tab_model) != android_observers_.end()) {
    return;
  }

  auto observer = std::make_unique<TabModelObserverImpl>(
      tab_model, privacy_filter_, navigation_collector_);
  android_observers_[tab_model] = std::move(observer);

  LOG(ERROR) << "[TabCollector] Now observing TabModel with "
            << tab_model->GetTabCount() << " tabs";
}

void TabEventCollector::StopObservingTabModel(TabModel* tab_model) {
  auto it = android_observers_.find(tab_model);
  if (it != android_observers_.end()) {
    LOG(ERROR) << "[TabCollector] Stopped observing TabModel";
    android_observers_.erase(it);
  }
}

void TabEventCollector::Shutdown() {
  LOG(ERROR) << "[TabCollector] Shutting down (Android)";
  android_observers_.clear();
}

// Android TabModelObserverImpl implementation

TabEventCollector::TabModelObserverImpl::TabModelObserverImpl(
    TabModel* tab_model,
    PrivacyFilter* privacy_filter,
    NavigationEventCollector* navigation_collector)
    : tab_model_(tab_model),
      privacy_filter_(privacy_filter),
      navigation_collector_(navigation_collector) {
  tab_model_->AddObserver(this);
}

TabEventCollector::TabModelObserverImpl::~TabModelObserverImpl() {
  if (tab_model_) {
    tab_model_->RemoveObserver(this);
  }
}

void TabEventCollector::TabModelObserverImpl::DidSelectTab(
    TabAndroid* tab,
    TabModel::TabSelectionType type,
    int last_id) {
  content::WebContents* web_contents = tab->web_contents();
  if (!web_contents) {
    return;
  }

  // ⭐ CRITICAL: Attach navigation observer to this WebContents
  navigation_collector_->ObserveWebContents(web_contents);

  std::string url =
      privacy_filter_->FilterUrl(web_contents->GetVisibleURL().spec());
  std::string title = privacy_filter_->FilterTitle(
      base::UTF16ToUTF8(web_contents->GetTitle()));

  std::string selection_type_str;
  switch (type) {
    case TabModel::TabSelectionType::FROM_USER:
      selection_type_str = "User Gesture";
      break;
    case TabModel::TabSelectionType::FROM_NEW:
      selection_type_str = "New Tab";
      break;
    case TabModel::TabSelectionType::FROM_CLOSE:
      selection_type_str = "From Close";
      break;
    case TabModel::TabSelectionType::FROM_EXIT:
      selection_type_str = "From Exit";
      break;
    case TabModel::TabSelectionType::FROM_UNDO:
      selection_type_str = "From Undo";
      break;
    case TabModel::TabSelectionType::FROM_OMNIBOX:
      selection_type_str = "From Omnibox";
      break;
    case TabModel::TabSelectionType::SIZE:
      selection_type_str = "Unknown";
      break;
  }

  LOG(ERROR) << "[Tab] 🔵 TAB_SELECTED (Android)"
            << " | Tab ID: " << tab->GetAndroidId() << " | URL: " << url
            << " | Title: " << title << " | Type: " << selection_type_str
            << " | Last Tab ID: " << last_id;
}

void TabEventCollector::TabModelObserverImpl::WillCloseTab(TabAndroid* tab) {
  content::WebContents* web_contents = tab->web_contents();
  if (!web_contents) {
    return;
  }

  std::string url =
      privacy_filter_->FilterUrl(web_contents->GetVisibleURL().spec());

  LOG(ERROR) << "[Tab] ❌ TAB_CLOSING (Android)"
            << " | Tab ID: " << tab->GetAndroidId() << " | URL: " << url;
}

void TabEventCollector::TabModelObserverImpl::DidAddTab(TabAndroid* tab,
                                                         TabModel::TabLaunchType type) {
  content::WebContents* web_contents = tab->web_contents();
  if (!web_contents) {
    return;
  }

  // ⭐ CRITICAL: Attach navigation observer to this new WebContents
  navigation_collector_->ObserveWebContents(web_contents);

  std::string url =
      privacy_filter_->FilterUrl(web_contents->GetVisibleURL().spec());

  LOG(ERROR) << "[Tab] ➕ TAB_ADDED (Android)"
            << " | Tab ID: " << tab->GetAndroidId() << " | URL: " << url
            << " | Launch Type: " << static_cast<int>(type)
            << " | Total Tabs: " << tab_model_->GetTabCount();
}

void TabEventCollector::TabModelObserverImpl::DidMoveTab(TabAndroid* tab,
                                                           int new_index,
                                                           int old_index) {
  content::WebContents* web_contents = tab->web_contents();
  if (!web_contents) {
    return;
  }

  std::string url =
      privacy_filter_->FilterUrl(web_contents->GetVisibleURL().spec());

  LOG(ERROR) << "[Tab] ↔️ TAB_MOVED (Android)"
            << " | Tab ID: " << tab->GetAndroidId() << " | From: " << old_index
            << " | To: " << new_index << " | URL: " << url;
}
#endif  // BUILDFLAG(IS_ANDROID)

}  // namespace activity_tracking

