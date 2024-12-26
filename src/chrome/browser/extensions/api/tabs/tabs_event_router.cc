// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/tabs/tabs_event_router.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/extensions/api/tabs/tabs_constants.h"
#include "chrome/browser/extensions/api/tabs/tabs_windows_api.h"
#include "chrome/browser/extensions/api/tabs/windows_event_router.h"
#include "chrome/browser/extensions/browser_extension_window_controller.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/recently_audible_helper.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/extensions/extension_constants.h"
#include "components/favicon/content/content_favicon_driver.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/features/feature.h"
#include "extensions/common/mojom/context_type.mojom.h"
#include "extensions/common/mojom/event_dispatcher.mojom-forward.h"
#include "third_party/blink/public/common/page/page_zoom.h"
#include "chrome/browser/android/tab_android.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"

using base::Value;
using content::WebContents;
using zoom::ZoomController;

namespace extensions {

namespace {

bool WillDispatchTabUpdatedEvent(
    WebContents* contents,
    const std::set<std::string> changed_property_names,
    content::BrowserContext* browser_context,
    mojom::ContextType target_context,
    const Extension* extension,
    const base::Value::Dict* listener_filter,
    std::optional<base::Value::List>& event_args_out,
    mojom::EventFilteringInfoPtr& event_filtering_info_out) {
  ExtensionTabUtil::ScrubTabBehavior scrub_tab_behavior =
      ExtensionTabUtil::GetScrubTabBehavior(extension, target_context,
                                            contents);
  api::tabs::Tab tab_object = ExtensionTabUtil::CreateTabObject(
      contents, scrub_tab_behavior, extension);

  base::Value::Dict tab_value = tab_object.ToValue();

  base::Value::Dict changed_properties;
  for (const auto& property : changed_property_names) {
    if (const base::Value* value = tab_value.Find(property))
      changed_properties.Set(property, value->Clone());
  }

  event_args_out.emplace();
  event_args_out->Append(ExtensionTabUtil::GetTabId(contents));
  event_args_out->Append(std::move(changed_properties));
  event_args_out->Append(std::move(tab_value));
  return true;
}

bool WillDispatchTabCreatedEvent(
    WebContents* contents,
    bool active,
    content::BrowserContext* browser_context,
    mojom::ContextType target_context,
    const Extension* extension,
    const base::Value::Dict* listener_filter,
    std::optional<base::Value::List>& event_args_out,
    mojom::EventFilteringInfoPtr& event_filtering_info_out) {
  ExtensionTabUtil::ScrubTabBehavior scrub_tab_behavior =
      ExtensionTabUtil::GetScrubTabBehavior(extension, target_context,
                                            contents);
  base::Value::Dict tab_value =
      ExtensionTabUtil::CreateTabObject(contents, scrub_tab_behavior, extension)
          .ToValue();
  tab_value.Set(tabs_constants::kSelectedKey, active);
  tab_value.Set(tabs_constants::kActiveKey, active);

  event_args_out.emplace();
  event_args_out->Append(std::move(tab_value));
  return true;
}

}  // namespace

TabsEventRouter::TabEntry::TabEntry(TabsEventRouter* router,
                                   TabAndroid* tab)
    : WebContentsObserver(tab->web_contents()),
      tab_(tab),
      complete_waiting_on_load_(false),
      was_audible_(false),
      was_muted_(tab->web_contents()->IsAudioMuted()),
      router_(router) {
  auto* audible_helper = RecentlyAudibleHelper::FromWebContents(tab->web_contents());
  was_audible_ = audible_helper->WasRecentlyAudible();
}

std::set<std::string> TabsEventRouter::TabEntry::UpdateLoadState() {
  // The tab may go in & out of loading (for instance if iframes navigate).
  // We only want to respond to the first change from loading to !loading after
  // the NavigationEntryCommitted() was fired.
  if (!complete_waiting_on_load_ || web_contents()->IsLoading()) {
    return std::set<std::string>();
  }

  // Send 'status' of tab change. Expecting 'complete' is fired.
  complete_waiting_on_load_ = false;
  std::set<std::string> changed_property_names;
  changed_property_names.insert(tabs_constants::kStatusKey);
  return changed_property_names;
}

bool TabsEventRouter::TabEntry::SetAudible(bool new_val) {
  if (was_audible_ == new_val)
    return false;
  was_audible_ = new_val;
  return true;
}

bool TabsEventRouter::TabEntry::SetMuted(bool new_val) {
  if (was_muted_ == new_val)
    return false;
  was_muted_ = new_val;
  return true;
}

void TabsEventRouter::TabEntry::NavigationEntryCommitted(
    const content::LoadCommittedDetails& load_details) {
  // Send 'status' of tab change. Expecting 'loading' is fired.
  complete_waiting_on_load_ = true;
  std::set<std::string> changed_property_names;
  changed_property_names.insert(tabs_constants::kStatusKey);

  if (web_contents()->GetURL() != url_) {
    url_ = web_contents()->GetURL();
    changed_property_names.insert(tabs_constants::kUrlKey);
  }

  router_->TabUpdated(this, std::move(changed_property_names));
}

void TabsEventRouter::TabEntry::TitleWasSet(content::NavigationEntry* entry) {
  std::set<std::string> changed_property_names;
  changed_property_names.insert(tabs_constants::kTitleKey);
  router_->TabUpdated(this, std::move(changed_property_names));
}

void TabsEventRouter::TabEntry::WebContentsDestroyed() {
  // This is necessary because it's possible for tabs to be created, detached
  // and then destroyed without ever having been re-attached and closed. This
  // happens in the case of a devtools WebContents that is opened in window,
  // docked, then closed.
  // Warning: |this| will be deleted after this call.
  router_->UnregisterForTabNotifications(tab_);
}

TabsEventRouter::TabsEventRouter(Profile* profile)
    : profile_(profile), browser_tab_strip_tracker_(this, this) {
  DCHECK(!profile->IsOffTheRecord());

  // BrowserList::AddObserver(this);
  browser_tab_strip_tracker_.Init();

  // tab_manager_scoped_observation_.Observe(g_browser_process->GetTabManager());
}

TabsEventRouter::~TabsEventRouter() {
  // BrowserList::RemoveObserver(this);
}

bool TabsEventRouter::ShouldTrackBrowser(Browser* browser) {
  return profile_->IsSameOrParent(browser->profile()) &&
         ExtensionTabUtil::BrowserSupportsTabs(browser);
}

void TabsEventRouter::OnBrowserSetLastActive(Browser* browser) { // wip
  // TabsWindowsAPI* tabs_window_api = TabsWindowsAPI::Get(profile_);
  // if (tabs_window_api) {
  //   tabs_window_api->windows_event_router()->OnActiveWindowChanged(
  //       browser ? browser->extension_window_controller() : nullptr);
  // }
}
/*
void TabsEventRouter::OnTabStripModelChanged(
    TabStripModel* tab_strip_model,
    const TabStripModelChange& change,
    const TabStripSelectionChange& selection) {
  switch (change.type()) {
    case TabStripModelChange::kInserted: {
      for (const auto& contents : change.GetInsert()->contents) {
        DispatchTabInsertedAt(tab_strip_model, contents.contents,
                              contents.index,
                              selection.new_contents == contents.contents);
      }
      break;
    }
    case TabStripModelChange::kRemoved: {
      for (const auto& contents : change.GetRemove()->contents) {
        if (contents.remove_reason ==
            TabStripModelChange::RemoveReason::kDeleted) {
          DispatchTabClosingAt(tab_strip_model, contents.contents,
                               contents.index);
        }

        DispatchTabDetachedAt(contents.contents, contents.index,
                              selection.old_contents == contents.contents);
      }
      break;
    }
    case TabStripModelChange::kMoved: {
      auto* move = change.GetMove();
      DispatchTabMoved(move->contents, move->from_index, move->to_index);
      break;
    }
    case TabStripModelChange::kReplaced: {
      auto* replace = change.GetReplace();
      DispatchTabReplacedAt(replace->old_contents, replace->new_contents,
                            replace->index);
      break;
    }
    case TabStripModelChange::kSelectionOnly:
      break;
  }

  if (tab_strip_model->empty())
    return;

  if (selection.active_tab_changed())
    DispatchActiveTabChanged(selection.old_contents, selection.new_contents);

  if (selection.selection_changed()) {
    DispatchTabSelectionChanged(tab_strip_model, selection.old_model);
  }
}

void TabsEventRouter::TabChangedAt(WebContents* contents,
                                   int index,
                                   TabChangeType change_type) {
  TabEntry* entry = GetTabEntry(contents);
  // TabClosingAt() may have already removed the entry for |contents| even
  // though the tab has not yet been detached.
  if (entry)
    TabUpdated(entry, entry->UpdateLoadState());
}

void TabsEventRouter::TabPinnedStateChanged(TabStripModel* tab_strip_model,
                                            WebContents* contents,
                                            int index) {
  std::set<std::string> changed_property_names;
  changed_property_names.insert(tabs_constants::kPinnedKey);
  DispatchTabUpdatedEvent(contents, std::move(changed_property_names));
}

void TabsEventRouter::TabGroupedStateChanged(
    std::optional<tab_groups::TabGroupId> group,
    content::WebContents* contents,
    int index) {
  std::set<std::string> changed_property_names;
  changed_property_names.insert(tabs_constants::kGroupIdKey);
  DispatchTabUpdatedEvent(contents, std::move(changed_property_names));
}
*/

void TabsEventRouter::RegisterTabObserver() {
    if (!TabModelList::models().empty()) {
        OnTabModelAdded();
    } else {
        TabModelList::AddObserver(this);
    }
}

void TabsEventRouter::OnTabModelAdded() {
    if (observed_tab_model_)
        return;
    // The assumption is that there can be at most one non-off-the-record tab
    // model. Observe it if it exists.
    for (TabModel* model : TabModelList::models()) {
        if (!model->IsOffTheRecord()) {
            observed_tab_model_ = model;
            observed_tab_model_->AddObserver(this);
            break;
        }
    }
}

void TabsEventRouter::OnTabModelRemoved() {
    if (!observed_tab_model_)
        return;
      
    for (TabModel* model : TabModelList::models()) {
        if (observed_tab_model_ == model)
            return;
    }
    observed_tab_model_ = nullptr;
}

void TabsEventRouter::DidSelectTab(TabAndroid* tab, TabModel::TabSelectionType type, int last_id) {
  LOG(INFO) << "tabs_event_router.cc: DidSelectTab: entry";
  WebContents* contents = tab->web_contents();
  DCHECK(contents);
  if (!contents) {
    LOG(INFO) << "tabs_event_router.cc: DidSelectTab: web_contents: nullptr";
    return;
  }
  
  last_tab_id_ = tab->GetAndroidId();

  if (!GetTabEntry(tab)) {
    TabModel* tab_model = TabModelList::GetTabModelForTabAndroid(tab);
    DispatchTabInsertedAt(tab_model,
                          tab,
                          tab->GetAndroidId(),
                          tab_model->GetActiveWebContents() == contents);
  } else {
    // If the tab is already registered, do nothing
  }
}

void TabsEventRouter::DidAddTab(TabAndroid* tab, TabModel::TabLaunchType type) {
  LOG(INFO) << "tabs_event_router.cc: DidAddTab: entry";
  DCHECK(tab->web_contents());
  TabModel* tab_model = TabModelList::GetTabModelForTabAndroid(tab);
  DispatchTabInsertedAt(tab_model,
                        tab,
                        tab->GetAndroidId(),
                        tab_model->GetActiveWebContents() == tab->web_contents());
}

void TabsEventRouter::WillCloseTab(TabAndroid* tab) {
  LOG(INFO) << "tabs_event_router.cc: WillCloseTab: entry";
  TabModel* tab_model = TabModelList::GetTabModelForTabAndroid(tab);
  DispatchTabClosingAt(tab_model,
                       tab,
                       tab->GetAndroidId());
}

void TabsEventRouter::TabRemoved(TabAndroid* tab) {
  LOG(INFO) << "tabs_event_router.cc: TabRemoved: entry";
  TabModel* tab_model = TabModelList::GetTabModelForTabAndroid(tab);
  DispatchTabDetachedAt(tab,
                        tab->GetAndroidId(),
                        tab_model->GetActiveWebContents() == tab->web_contents());
}

void TabsEventRouter::DidMoveTab(TabAndroid* tab, int new_index, int last_index) {
//   LOG(INFO) << "tabs_event_router.cc: DidMoveTab: entry";
  DispatchTabMoved(tab, last_tab_id_, new_index);
}

void TabsEventRouter::RestoreCompleted() {
  LOG(INFO) << "tabs_event_router.cc: RestoreCompleted: entry";
  TabModel* tab_model = TabModelList::GetCurrentTabModel();
  for (int i = 0; i < tab_model->GetTabCount(); ++i) {
    DCHECK(tab_model->GetTabAt(i));
    DidAddTab(tab_model->GetTabAt(i), TabModel::TabLaunchType::FROM_RESTORE);
  }
}

void TabsEventRouter::OnZoomControllerDestroyed(
    zoom::ZoomController* zoom_controller) {
  if (zoom_scoped_observations_.IsObservingSource(zoom_controller)) {
    zoom_scoped_observations_.RemoveObservation(zoom_controller);
  }
}

void TabsEventRouter::OnZoomChanged(
    const ZoomController::ZoomChangedEventData& data) {
  DCHECK(data.web_contents);
  int tab_id = ExtensionTabUtil::GetTabId(data.web_contents);
  if (tab_id < 0)
    return;

  // Prepare the zoom change information.
  api::tabs::OnZoomChange::ZoomChangeInfo zoom_change_info;
  zoom_change_info.tab_id = tab_id;
  zoom_change_info.old_zoom_factor =
      blink::PageZoomLevelToZoomFactor(data.old_zoom_level);
  zoom_change_info.new_zoom_factor =
      blink::PageZoomLevelToZoomFactor(data.new_zoom_level);
  ZoomModeToZoomSettings(data.zoom_mode, &zoom_change_info.zoom_settings);

  // Dispatch the |onZoomChange| event.
  Profile* profile =
      Profile::FromBrowserContext(data.web_contents->GetBrowserContext());
  DispatchEvent(profile, events::TABS_ON_ZOOM_CHANGE,
                api::tabs::OnZoomChange::kEventName,
                api::tabs::OnZoomChange::Create(zoom_change_info),
                EventRouter::USER_GESTURE_UNKNOWN);
}

void TabsEventRouter::OnFaviconUpdated(
    favicon::FaviconDriver* favicon_driver,
    NotificationIconType notification_icon_type,
    const GURL& icon_url,
    bool icon_url_changed,
    const gfx::Image& image) {
  if (notification_icon_type == NON_TOUCH_16_DIP && icon_url_changed) {
    favicon::ContentFaviconDriver* content_favicon_driver =
        static_cast<favicon::ContentFaviconDriver*>(favicon_driver);
    FaviconUrlUpdated(content_favicon_driver->web_contents());
  }
}

void TabsEventRouter::OnDiscardedStateChange(
    WebContents* contents,
    ::mojom::LifecycleUnitDiscardReason reason,
    bool is_discarded) {
  std::set<std::string> changed_property_names;
  // If the "discarded" property changes, so does the "status" property:
  // - a discarded tab has status "unloaded", and will transition to "loading"
  //   on un-discarding; and,
  // - a tab can only be discarded if its status is "complete" or "loading", in
  //   which case it will transition to "unloaded".
  changed_property_names.insert(tabs_constants::kDiscardedKey);
  changed_property_names.insert(tabs_constants::kStatusKey);
  DispatchTabUpdatedEvent(contents, std::move(changed_property_names));
}

void TabsEventRouter::OnAutoDiscardableStateChange(WebContents* contents,
                                                   bool is_auto_discardable) {
  std::set<std::string> changed_property_names;
  changed_property_names.insert(tabs_constants::kAutoDiscardableKey);
  DispatchTabUpdatedEvent(contents, std::move(changed_property_names));
}

void TabsEventRouter::DispatchTabInsertedAt(TabModel* tab_model,
                                            TabAndroid* tab,
                                            int index,
                                            bool active) {
  WebContents* contents = tab->web_contents();
  if (!GetTabEntry(tab)) {
    // We've never seen this tab, send create event as long as we're not in the
    // constructor.
    if (browser_tab_strip_tracker_.is_processing_initial_browsers())
      RegisterForTabNotifications(tab);
    else
      TabCreatedAt(tab, index, active);
    return;
  }

  int tab_id = tab->GetAndroidId();
  base::Value::List args;
  args.Append(tab_id);

  base::Value::Dict object_args;
  object_args.Set(tabs_constants::kNewWindowIdKey,
                  Value(ExtensionTabUtil::GetWindowIdOfTab(contents)));
  object_args.Set(tabs_constants::kNewPositionKey, Value(index));
  args.Append(std::move(object_args));

  Profile* profile = GetProfileFromBrowserContext(contents);
  DispatchEvent(profile, events::TABS_ON_ATTACHED,
                api::tabs::OnAttached::kEventName, std::move(args),
                EventRouter::USER_GESTURE_UNKNOWN);
}

void TabsEventRouter::DispatchTabClosingAt(TabModel* tab_model,
                                           TabAndroid* tab,
                                           int index) {
  int tab_id = ExtensionTabUtil::GetTabId(contents);

  base::Value::List args;
  args.Append(tab_id);

  base::Value::Dict object_args;
  object_args.Set(tabs_constants::kWindowIdKey,
                  ExtensionTabUtil::GetWindowIdOfTab(contents));
  object_args.Set(tabs_constants::kWindowClosing,
                  tab_strip_model->closing_all());
  args.Append(std::move(object_args));

  Profile* profile = GetProfileFromBrowserContext(contents);
  DispatchEvent(profile, events::TABS_ON_REMOVED,
                api::tabs::OnRemoved::kEventName, std::move(args),
                EventRouter::USER_GESTURE_UNKNOWN);

  UnregisterForTabNotifications(tab);
}

void TabsEventRouter::DispatchTabDetachedAt(TabAndroid* tab,
                                            int index,
                                            bool was_active) {
 WebContents* contents = tab->web_contents();
  if (!GetTabEntry(tab)) {
    // The tab was removed. Don't send detach event.
    return;
  }

  base::Value::List args;
  args.Append(ExtensionTabUtil::GetTabId(contents));

  base::Value::Dict object_args;
  object_args.Set(tabs_constants::kOldWindowIdKey,
                  ExtensionTabUtil::GetWindowIdOfTab(contents));
  object_args.Set(tabs_constants::kOldPositionKey, index);
  args.Append(std::move(object_args));

  Profile* profile = GetProfileFromBrowserContext(contents);
  DispatchEvent(profile, events::TABS_ON_DETACHED,
                api::tabs::OnDetached::kEventName, std::move(args),
                EventRouter::USER_GESTURE_UNKNOWN);
}

void TabsEventRouter::DispatchActiveTabChanged(WebContents* old_contents,
                                               WebContents* new_contents) {
  NOTREACHED();
  base::Value::List args;
  int tab_id = ExtensionTabUtil::GetTabId(new_contents);
  args.Append(tab_id);

  base::Value::Dict object_args;
  object_args.Set(tabs_constants::kWindowIdKey,
                  ExtensionTabUtil::GetWindowIdOfTab(new_contents));
  args.Append(object_args.Clone());

  // The onActivated event replaced onActiveChanged and onSelectionChanged. The
  // deprecated events take two arguments: tabId, {windowId}.
  Profile* profile =
      Profile::FromBrowserContext(new_contents->GetBrowserContext());

  DispatchEvent(profile, events::TABS_ON_SELECTION_CHANGED,
                api::tabs::OnSelectionChanged::kEventName, args.Clone(),
                EventRouter::USER_GESTURE_UNKNOWN);
  DispatchEvent(profile, events::TABS_ON_ACTIVE_CHANGED,
                api::tabs::OnActiveChanged::kEventName, std::move(args),
                EventRouter::USER_GESTURE_UNKNOWN);

  // The onActivated event takes one argument: {windowId, tabId}.
  base::Value::List on_activated_args;
  object_args.Set(tabs_constants::kTabIdKey, tab_id);
  on_activated_args.Append(std::move(object_args));
  DispatchEvent(
      profile, events::TABS_ON_ACTIVATED, api::tabs::OnActivated::kEventName,
      std::move(on_activated_args), EventRouter::USER_GESTURE_UNKNOWN);
}

void TabsEventRouter::DispatchTabSelectionChanged(
    TabStripModel* tab_strip_model,
    const ui::ListSelectionModel& old_model) {
  ui::ListSelectionModel::SelectedIndices new_selection =
      tab_strip_model->selection_model().selected_indices();
  base::Value::List all_tabs;

  for (int index : new_selection) {
    WebContents* contents = tab_strip_model->GetWebContentsAt(index);
    if (!contents)
      break;
    int tab_id = ExtensionTabUtil::GetTabId(contents);
    all_tabs.Append(tab_id);
  }

  base::Value::List args;
  base::Value::Dict select_info;

  select_info.Set(
      tabs_constants::kWindowIdKey,
      ExtensionTabUtil::GetWindowIdOfTabStripModel(tab_strip_model));

  select_info.Set(tabs_constants::kTabIdsKey, std::move(all_tabs));
  args.Append(std::move(select_info));

  // The onHighlighted event replaced onHighlightChanged.
  Profile* profile = tab_strip_model->profile();
  DispatchEvent(profile, events::TABS_ON_HIGHLIGHT_CHANGED,
                api::tabs::OnHighlightChanged::kEventName, args.Clone(),
                EventRouter::USER_GESTURE_UNKNOWN);
  DispatchEvent(profile, events::TABS_ON_HIGHLIGHTED,
                api::tabs::OnHighlighted::kEventName, std::move(args),
                EventRouter::USER_GESTURE_UNKNOWN);
}

void TabsEventRouter::DispatchTabMoved(TabAndroid* tab,
                                       int from_index,
                                       int to_index) {
  WebContents* contents = tab->web_contents();
  base::Value::List args;
  args.Append(ExtensionTabUtil::GetTabId(contents));

  base::Value::Dict object_args;
  object_args.Set(tabs_constants::kWindowIdKey,
                  ExtensionTabUtil::GetWindowIdOfTab(contents));
  object_args.Set(tabs_constants::kFromIndexKey, from_index);
  object_args.Set(tabs_constants::kToIndexKey, to_index);
  args.Append(std::move(object_args));

  Profile* profile = GetProfileFromBrowserContext(contents);
  DispatchEvent(profile, events::TABS_ON_MOVED, api::tabs::OnMoved::kEventName,
                std::move(args), EventRouter::USER_GESTURE_UNKNOWN);
}

void TabsEventRouter::DispatchTabReplacedAt(WebContents* old_contents,
                                            WebContents* new_contents,
                                            int index) {
  // Notify listeners that the next tabs closing or being added are due to
  // WebContents being swapped.
  const int new_tab_id = ExtensionTabUtil::GetTabId(new_contents);
  const int old_tab_id = ExtensionTabUtil::GetTabId(old_contents);
  base::Value::List args;
  args.Append(new_tab_id);
  args.Append(old_tab_id);

  DispatchEvent(Profile::FromBrowserContext(new_contents->GetBrowserContext()),
                events::TABS_ON_REPLACED, api::tabs::OnReplaced::kEventName,
                std::move(args), EventRouter::USER_GESTURE_UNKNOWN);

  // UnregisterForTabNotifications(old_contents);

  // if (!GetTabEntry(new_contents))
    // RegisterForTabNotifications(new_contents);
}

void TabsEventRouter::TabCreatedAt(TabAndroid* tab,
                                   int index,
                                   bool active) {
  WebContents* contents = tab->web_contents();
  Profile* profile = GetProfileFromBrowserContext(contents);
  auto event = std::make_unique<Event>(events::TABS_ON_CREATED,
                                       api::tabs::OnCreated::kEventName,
                                       base::Value::List(), profile);
  event->user_gesture = EventRouter::USER_GESTURE_NOT_ENABLED;
  event->will_dispatch_callback =
      base::BindRepeating(&WillDispatchTabCreatedEvent, contents, active);
  EventRouter::Get(profile)->BroadcastEvent(std::move(event));

  RegisterForTabNotifications(tab);
}

void TabsEventRouter::TabUpdated(TabEntry* entry,
                                 std::set<std::string> changed_property_names) {
  auto* audible_helper =
      RecentlyAudibleHelper::FromWebContents(entry->web_contents());
  bool audible = audible_helper->WasRecentlyAudible();
  if (entry->SetAudible(audible)) {
    changed_property_names.insert(tabs_constants::kAudibleKey);
  }

  bool muted = entry->web_contents()->IsAudioMuted();
  if (entry->SetMuted(muted)) {
    changed_property_names.insert(tabs_constants::kMutedInfoKey);
  }

  if (!changed_property_names.empty()) {
    DispatchTabUpdatedEvent(entry->web_contents(),
                            std::move(changed_property_names));
  }
}

void TabsEventRouter::FaviconUrlUpdated(WebContents* contents) {
  content::NavigationEntry* entry = contents->GetController().GetVisibleEntry();
  if (!entry || !entry->GetFavicon().valid)
    return;
  std::set<std::string> changed_property_names;
  changed_property_names.insert(tabs_constants::kFaviconUrlKey);
  DispatchTabUpdatedEvent(contents, std::move(changed_property_names));
}

void TabsEventRouter::DispatchEvent(
    Profile* profile,
    events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List args,
    EventRouter::UserGestureState user_gesture) {
  EventRouter* event_router = EventRouter::Get(profile);
  if (!profile_->IsSameOrParent(profile) || !event_router)
    return;

  auto event = std::make_unique<Event>(histogram_value, event_name,
                                       std::move(args), profile);
  event->user_gesture = user_gesture;
  event_router->BroadcastEvent(std::move(event));
}

void TabsEventRouter::DispatchTabUpdatedEvent(
    WebContents* contents,
    const std::set<std::string> changed_property_names) {
  DCHECK(!changed_property_names.empty());
  DCHECK(contents);

  Profile* profile = Profile::FromBrowserContext(contents->GetBrowserContext());

  auto event = std::make_unique<Event>(
      events::TABS_ON_UPDATED, api::tabs::OnUpdated::kEventName,
      // The event arguments depend on the extension's permission. They are set
      // in WillDispatchTabUpdatedEvent().
      base::Value::List(), profile);
  event->user_gesture = EventRouter::USER_GESTURE_NOT_ENABLED;
  event->will_dispatch_callback =
      base::BindRepeating(&WillDispatchTabUpdatedEvent, contents,
                          std::move(changed_property_names));
  EventRouter::Get(profile)->BroadcastEvent(std::move(event));
}

void TabsEventRouter::RegisterForTabNotifications(TabAndroid* tab) {
  // favicon_scoped_observations_.AddObservation(
  //     favicon::ContentFaviconDriver::FromWebContents(contents));
  // zoom_scoped_observations_.AddObservation(
  //     ZoomController::FromWebContents(contents));

  // int tab_id = ExtensionTabUtil::GetTabId(contents);
  if (!tab->web_contents())
    return;

  int tab_id = tab->GetAndroidId();
  DCHECK(tab_entries_.find(tab_id) == tab_entries_.end());
  tab_entries_[tab_id] = std::make_unique<TabEntry>(this, tab);
}

void TabsEventRouter::UnregisterForTabNotifications(TabAndroid* tab) {
  // if (auto* zoom_controller = ZoomController::FromWebContents(contents);
  //     zoom_scoped_observations_.IsObservingSource(zoom_controller)) {
  //   zoom_scoped_observations_.RemoveObservation(zoom_controller);
  // }
  // favicon_scoped_observations_.RemoveObservation(
  //     favicon::ContentFaviconDriver::FromWebContents(contents));

  // int tab_id = ExtensionTabUtil::GetTabId(contents);
  int tab_id = tab->GetAndroidId();
  int removed_count = tab_entries_.erase(tab_id);
  DCHECK_GT(removed_count, 0);
}

TabsEventRouter::TabEntry* TabsEventRouter::GetTabEntry(TabAndroid* tab) {
  const auto it = tab_entries_.find(tab->GetAndroidId());

  return it == tab_entries_.end() ? nullptr : it->second.get();
}

Profile* TabsEventRouter::GetProfileFromBrowserContext(WebContents* contents) {
  if (contents)
    return Profile::FromBrowserContext(contents->GetBrowserContext());
  else
    return ProfileManager::GetActiveUserProfile();
}

}  // namespace extensions
