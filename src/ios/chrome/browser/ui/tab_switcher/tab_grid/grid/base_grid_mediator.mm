// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/base_grid_mediator.h"

#import <UIKit/UIKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <memory>

#import "base/debug/dump_without_crashing.h"
#import "base/functional/bind.h"
#import "base/metrics/histogram_functions.h"
#import "base/metrics/histogram_macros.h"
#import "base/metrics/user_metrics.h"
#import "base/metrics/user_metrics_action.h"
#import "base/scoped_multi_source_observation.h"
#import "base/strings/stringprintf.h"
#import "base/strings/sys_string_conversions.h"
#import "components/bookmarks/common/bookmark_pref_names.h"
#import "components/prefs/pref_service.h"
#import "components/tab_groups/tab_group_visual_data.h"
#import "ios/chrome/browser/bookmarks/model/legacy_bookmark_model.h"
#import "ios/chrome/browser/bookmarks/model/local_or_syncable_bookmark_model_factory.h"
#import "ios/chrome/browser/commerce/model/shopping_persisted_data_tab_helper.h"
#import "ios/chrome/browser/default_browser/model/utils.h"
#import "ios/chrome/browser/drag_and_drop/model/drag_item_util.h"
#import "ios/chrome/browser/main/model/browser_util.h"
#import "ios/chrome/browser/policy/model/policy_util.h"
#import "ios/chrome/browser/reading_list/model/reading_list_browser_agent.h"
#import "ios/chrome/browser/shared/coordinator/scene/scene_state.h"
#import "ios/chrome/browser/shared/model/browser/browser.h"
#import "ios/chrome/browser/shared/model/browser/browser_list.h"
#import "ios/chrome/browser/shared/model/browser/browser_list_factory.h"
#import "ios/chrome/browser/shared/model/browser_state/chrome_browser_state.h"
#import "ios/chrome/browser/shared/model/url/chrome_url_constants.h"
#import "ios/chrome/browser/shared/model/url/url_util.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list_observer_bridge.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_opener.h"
#import "ios/chrome/browser/shared/public/commands/bookmarks_commands.h"
#import "ios/chrome/browser/shared/public/commands/command_dispatcher.h"
#import "ios/chrome/browser/shared/public/commands/reading_list_add_command.h"
#import "ios/chrome/browser/shared/public/features/features.h"
#import "ios/chrome/browser/shared/public/features/system_flags.h"
#import "ios/chrome/browser/shared/ui/util/url_with_title.h"
#import "ios/chrome/browser/snapshots/model/snapshot_browser_agent.h"
#import "ios/chrome/browser/snapshots/model/snapshot_storage.h"
#import "ios/chrome/browser/snapshots/model/snapshot_storage_observer.h"
#import "ios/chrome/browser/snapshots/model/snapshot_tab_helper.h"
#import "ios/chrome/browser/tabs/model/features.h"
#import "ios/chrome/browser/tabs_search/model/tabs_search_service.h"
#import "ios/chrome/browser/tabs_search/model/tabs_search_service_factory.h"
#import "ios/chrome/browser/ui/menu/action_factory.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_collection_consumer.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_collection_drag_drop_metrics.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/grid_consumer.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/grid_item_identifier.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/grid_mediator_delegate.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/grid_toolbars_mutator.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/grid_utils.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/selected_grid_items.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/tab_groups/tab_groups_commands.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/tab_context_menu/tab_item.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/tab_grid_metrics.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/toolbars/tab_grid_toolbars_configuration.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/toolbars/tab_grid_toolbars_main_tab_grid_delegate.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_group_item.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_utils.h"
#import "ios/chrome/browser/ui/tab_switcher/web_state_tab_switcher_item.h"
#import "ios/web/public/navigation/navigation_manager.h"
#import "ios/web/public/web_state.h"
#import "ios/web/public/web_state_observer_bridge.h"
#import "net/base/apple/url_conversions.h"
#import "ui/gfx/image/image.h"

using PinnedState = WebStateSearchCriteria::PinnedState;

namespace {

void LogPriceDropMetrics(web::WebState* web_state) {
  ShoppingPersistedDataTabHelper* shopping_helper =
      ShoppingPersistedDataTabHelper::FromWebState(web_state);
  if (!shopping_helper) {
    return;
  }
  const ShoppingPersistedDataTabHelper::PriceDrop* price_drop =
      shopping_helper->GetPriceDrop();
  BOOL has_price_drop =
      price_drop && price_drop->current_price && price_drop->previous_price;
  base::RecordAction(base::UserMetricsAction(
      base::StringPrintf("Commerce.TabGridSwitched.%s",
                         has_price_drop ? "HasPriceDrop" : "NoPriceDrop")
          .c_str()));
}

// Returns the Browser with `identifier` in its WebStateList. Returns `nullptr`
// if not found.
Browser* GetBrowserForTabWithId(BrowserList* browser_list,
                                web::WebStateID identifier,
                                bool is_otr_tab) {
  std::set<Browser*> browsers = is_otr_tab
                                    ? browser_list->AllIncognitoBrowsers()
                                    : browser_list->AllRegularBrowsers();
  for (Browser* browser : browsers) {
    WebStateList* webStateList = browser->GetWebStateList();
    int index = GetWebStateIndex(webStateList,
                                 WebStateSearchCriteria{
                                     .identifier = identifier,
                                     .pinned_state = PinnedState::kNonPinned,
                                 });
    if (index != WebStateList::kInvalidIndex) {
      return browser;
    }
  }
  return nullptr;
}

// Returns the identifier of the currently active unpinned tab.
web::WebStateID GetActiveNonPinnedTabID(WebStateList* web_state_list) {
  return GetActiveWebStateIdentifier(web_state_list, PinnedState::kNonPinned);
}

}  // namespace

@interface BaseGridMediator () <CRWWebStateObserver,
                                SnapshotStorageObserver,
                                WebStateListObserving>
// The browser state from the browser.
@property(nonatomic, readonly) ChromeBrowserState* browserState;

@end

@implementation BaseGridMediator {
  // Observers for WebStateList.
  std::unique_ptr<WebStateListObserverBridge> _webStateListObserverBridge;
  std::unique_ptr<
      base::ScopedMultiSourceObservation<WebStateList, WebStateListObserver>>
      _scopedWebStateListObservation;
  // Observer for WebStates.
  std::unique_ptr<web::WebStateObserverBridge> _webStateObserverBridge;
  std::unique_ptr<
      base::ScopedMultiSourceObservation<web::WebState, web::WebStateObserver>>
      _scopedWebStateObservation;

  // ItemID of the dragged tab. Used to check if the dropped tab is from the
  // same Chrome window.
  web::WebStateID _dragItemID;
  base::WeakPtr<Browser> _browser;

  // Current mode.
  TabGridMode _currentMode;

  // Items selected for editing.
  SelectedGridItems* _selectedEditingItems;
}

- (instancetype)init {
  if (self = [super init]) {
    _webStateListObserverBridge =
        std::make_unique<WebStateListObserverBridge>(self);
    _scopedWebStateListObservation = std::make_unique<
        base::ScopedMultiSourceObservation<WebStateList, WebStateListObserver>>(
        _webStateListObserverBridge.get());
    _webStateObserverBridge =
        std::make_unique<web::WebStateObserverBridge>(self);
    _scopedWebStateObservation =
        std::make_unique<base::ScopedMultiSourceObservation<
            web::WebState, web::WebStateObserver>>(
            _webStateObserverBridge.get());
    _currentMode = TabGridModeNormal;
  }
  return self;
}

#pragma mark - Public properties

- (Browser*)browser {
  return _browser.get();
}

- (TabGridMode)currentMode {
  return _currentMode;
}

- (void)setBrowser:(Browser*)browser {
  [self.snapshotStorage removeObserver:self];
  _scopedWebStateListObservation->RemoveAllObservations();
  _scopedWebStateObservation->RemoveAllObservations();

  _browser.reset();
  if (browser) {
    _browser = browser->AsWeakPtr();
  }

  _webStateList = browser ? browser->GetWebStateList() : nullptr;
  _browserState = browser ? browser->GetBrowserState() : nullptr;

  [self.snapshotStorage addObserver:self];

  if (_webStateList) {
    _scopedWebStateListObservation->AddObservation(_webStateList);
    [self addWebStateObservations];
    _selectedEditingItems =
        [[SelectedGridItems alloc] initWithWebStateList:_webStateList];

    if (self.webStateList->count() > 0) {
      [self populateConsumerItems];
    }
  }
}

- (void)setConsumer:(id<TabCollectionConsumer>)consumer {
  _consumer = consumer;
  [self resetToAllItems];
}

- (void)setCurrentMode:(TabGridMode)mode {
  if (_currentMode != mode && (_currentMode == TabGridModeSelection ||
                               _currentMode == TabGridModeSearch)) {
    // Clear selections.
    [_selectedEditingItems removeAllItems];
  }
  _currentMode = mode;
  [self configureToolbarsButtons];
  [self.gridConsumer setPageMode:_currentMode];
}

#pragma mark - Subclassing

- (void)disconnect {
  _browser.reset();
  _browserState = nil;
  _consumer = nil;
  _delegate = nil;
  _toolbarsMutator = nil;
  _containedGridToolbarsProvider = nil;
  _toolbarTabGridDelegate = nil;
  _gridConsumer = nil;
  _tabPresentationDelegate = nil;

  _scopedWebStateListObservation->RemoveAllObservations();
  _scopedWebStateObservation->RemoveAllObservations();
  _scopedWebStateObservation.reset();
  _scopedWebStateListObservation.reset();

  _webStateObserverBridge.reset();
  _webStateList->RemoveObserver(_webStateListObserverBridge.get());
  _webStateListObserverBridge.reset();
  _webStateList = nil;
}

- (void)configureToolbarsButtons {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (void)configureButtonsInSelectionMode:
    (TabGridToolbarsConfiguration*)configuration {
  NSUInteger selectedItemsCount = _selectedEditingItems.tabsCount;
  NSUInteger selectedShareableItemsCount =
      _selectedEditingItems.sharableItemsCount;

  BOOL allItemsSelected =
      static_cast<int>(selectedItemsCount) ==
      (self.webStateList->count() - self.webStateList->pinned_tabs_count());

  configuration.selectAllButton = !allItemsSelected;
  configuration.deselectAllButton = allItemsSelected;
  configuration.doneButton = YES;
  configuration.closeSelectedTabsButton = selectedItemsCount > 0;
  configuration.shareButton = selectedShareableItemsCount > 0;
  configuration.addToButton = selectedShareableItemsCount > 0;
  configuration.selectedItemsCount = selectedItemsCount;

  configuration.addToButtonMenu = [UIMenu
      menuWithChildren:[self addToButtonMenuElementsForItems:
                                 [_selectedEditingItems sharableItems]]];
}

#pragma mark - WebStateListObserving

- (void)willChangeWebStateList:(WebStateList*)webStateList
                        change:(const WebStateListChangeDetach&)detachChange
                        status:(const WebStateListStatus&)status {
  DCHECK_EQ(_webStateList, webStateList);
  if (webStateList->IsBatchInProgress()) {
    return;
  }

  web::WebState* detachedWebState = detachChange.detached_web_state();
  // If the WebState is pinned and it is not in the consumer's items list,
  // consumer will filter it out in the method's implementation.
  [self.consumer removeItemWithID:detachedWebState->GetUniqueIdentifier()
                   selectedItemID:GetActiveNonPinnedTabID(webStateList)];
  TabSwitcherItem* itemToRemove = [[TabSwitcherItem alloc]
      initWithIdentifier:detachedWebState->GetUniqueIdentifier()];
  GridItemIdentifier* identifierToRemove =
      [GridItemIdentifier tabIdentifier:itemToRemove];
  [self removeFromSelectionItemID:identifierToRemove];

  // The pinned WebState could be detached only in case it was displayed in
  // the Tab Search and was closed from the context menu. In such a case
  // there were no observation added for it. Therefore, there is no need to
  // remove one.
  if (![self isPinnedWebState:detachChange.detached_from_index()]) {
    _scopedWebStateObservation->RemoveObservation(detachedWebState);
  }
}

- (void)didChangeWebStateList:(WebStateList*)webStateList
                       change:(const WebStateListChange&)change
                       status:(const WebStateListStatus&)status {
  DCHECK_EQ(_webStateList, webStateList);
  if (webStateList->IsBatchInProgress()) {
    return;
  }

  switch (change.type()) {
    case WebStateListChange::Type::kStatusOnly: {
      const WebStateListChangeStatusOnly& selectionOnlyChange =
          change.As<WebStateListChangeStatusOnly>();
      if (selectionOnlyChange.pinned_state_changed()) {
        [self changePinnedStateForWebState:selectionOnlyChange.web_state()
                                   atIndex:selectionOnlyChange.index()];
        break;
      }
      // The activation is handled after this switch statement.
      break;
    }
    case WebStateListChange::Type::kDetach:
      // Do nothing when a WebState is detached.
      break;
    case WebStateListChange::Type::kMove: {
      const WebStateListChangeMove& moveChange =
          change.As<WebStateListChangeMove>();
      if (![self isPinnedWebState:moveChange.moved_to_index()]) {
        // BaseGridMediator handles only non pinned tabs because pinned tabs are
        // handled in PinnedTabsMediator.
        NSUInteger itemIndex =
            [self itemIndexFromWebStateListIndex:moveChange.moved_to_index()];
        [self.consumer
            moveItemWithID:moveChange.moved_web_state()->GetUniqueIdentifier()
                   toIndex:itemIndex];
      }

      // The pinned state can be updated when a tab is moved.
      if (moveChange.pinned_state_changed()) {
        [self changePinnedStateForWebState:moveChange.moved_web_state()
                                   atIndex:moveChange.moved_to_index()];
      }
      break;
    }
    case WebStateListChange::Type::kReplace: {
      const WebStateListChangeReplace& replaceChange =
          change.As<WebStateListChangeReplace>();
      if ([self isPinnedWebState:replaceChange.index()]) {
        break;
      }
      web::WebState* replacedWebState = replaceChange.replaced_web_state();
      web::WebState* insertedWebState = replaceChange.inserted_web_state();
      TabSwitcherItem* oldItem =
          [[WebStateTabSwitcherItem alloc] initWithWebState:replacedWebState];
      TabSwitcherItem* newItem =
          [[WebStateTabSwitcherItem alloc] initWithWebState:insertedWebState];
      [self.consumer replaceItem:[GridItemIdentifier tabIdentifier:oldItem]
             withReplacementItem:[GridItemIdentifier tabIdentifier:newItem]];

      _scopedWebStateObservation->RemoveObservation(replacedWebState);
      _scopedWebStateObservation->AddObservation(insertedWebState);
      break;
    }
    case WebStateListChange::Type::kInsert: {
      const WebStateListChangeInsert& insertChange =
          change.As<WebStateListChangeInsert>();
      if ([self isPinnedWebState:insertChange.index()]) {
        [self.consumer selectItemWithID:GetActiveNonPinnedTabID(webStateList)];
        break;
      }
      web::WebState* insertedWebState = insertChange.inserted_web_state();
      TabSwitcherItem* item =
          [[WebStateTabSwitcherItem alloc] initWithWebState:insertedWebState];
      NSUInteger itemIndex =
          [self itemIndexFromWebStateListIndex:insertChange.index()];
      [self.consumer insertItem:[GridItemIdentifier tabIdentifier:item]
                        atIndex:itemIndex
                 selectedItemID:GetActiveNonPinnedTabID(webStateList)];

      _scopedWebStateObservation->AddObservation(insertedWebState);
      break;
    }
  }
  [self updateToolbarAfterNumberOfItemsChanged];
  if (status.active_web_state_change()) {
    // If the selected index changes as a result of the last webstate being
    // detached, the active index will be kInvalidIndex.
    if (webStateList->active_index() == WebStateList::kInvalidIndex) {
      [self.consumer selectItemWithID:web::WebStateID()];
      return;
    }

    [self.consumer
        selectItemWithID:status.new_active_web_state->GetUniqueIdentifier()];
  }
}

- (void)webStateListWillBeginBatchOperation:(WebStateList*)webStateList {
  DCHECK_EQ(_webStateList, webStateList);
  _scopedWebStateObservation->RemoveAllObservations();
}

- (void)webStateListBatchOperationEnded:(WebStateList*)webStateList {
  DCHECK_EQ(_webStateList, webStateList);

  [self addWebStateObservations];
  [self populateConsumerItems];
  [self updateToolbarAfterNumberOfItemsChanged];
}

#pragma mark - CRWWebStateObserver

- (void)webStateDidStartLoading:(web::WebState*)webState {
  [self updateConsumerItemForWebState:webState];
}

- (void)webStateDidStopLoading:(web::WebState*)webState {
  [self updateConsumerItemForWebState:webState];
}

- (void)webStateDidChangeTitle:(web::WebState*)webState {
  [self updateConsumerItemForWebState:webState];
}

- (void)updateConsumerItemForWebState:(web::WebState*)webState {
  GridItemIdentifier* item =
      [GridItemIdentifier tabIdentifier:[[WebStateTabSwitcherItem alloc]
                                            initWithWebState:webState]];
  [self.consumer replaceItem:item withReplacementItem:item];
}

#pragma mark - SnapshotStorageObserver

- (void)snapshotStorage:(SnapshotStorage*)snapshotStorage
    didUpdateSnapshotForID:(SnapshotID)snapshotID {
  web::WebState* webState = nullptr;
  for (int i = self.webStateList->pinned_tabs_count();
       i < self.webStateList->count(); i++) {
    SnapshotTabHelper* snapshotTabHelper =
        SnapshotTabHelper::FromWebState(self.webStateList->GetWebStateAt(i));
    if (snapshotID == snapshotTabHelper->GetSnapshotID()) {
      webState = self.webStateList->GetWebStateAt(i);
      break;
    }
  }
  if (webState) {
    // It is possible to observe an updated snapshot for a WebState before
    // observing that the WebState has been added to the WebStateList. It is the
    // consumer's responsibility to ignore any updates before inserts.
    GridItemIdentifier* item =
        [GridItemIdentifier tabIdentifier:[[WebStateTabSwitcherItem alloc]
                                              initWithWebState:webState]];
    [self.consumer replaceItem:item withReplacementItem:item];
  }
}

#pragma mark - GridCommands

- (BOOL)addNewItem {
  if (self.browserState &&
      !IsAddNewTabAllowedByPolicy(self.browserState->GetPrefs(),
                                  self.browserState->IsOffTheRecord())) {
    return NO;
  }

  NSUInteger itemIndex =
      [self itemIndexFromWebStateListIndex:self.webStateList->count()];
  [self insertNewItemAtIndex:itemIndex];
  return YES;
}

- (void)insertNewItemAtIndex:(NSUInteger)index {
  [self insertNewItemAtIndex:index withURL:GURL(kChromeUINewTabURL)];
}

- (void)moveItemWithID:(web::WebStateID)itemID
               toIndex:(NSUInteger)destinationIndex {
  int sourceIndex = GetWebStateIndex(
      self.webStateList, WebStateSearchCriteria{
                             .identifier = itemID,
                         });
  if (sourceIndex != WebStateList::kInvalidIndex) {
    int destinationWebStateListIndex =
        [self webStateListIndexFromItemIndex:destinationIndex];
    self.webStateList->MoveWebStateAt(sourceIndex,
                                      destinationWebStateListIndex);
  }
}

- (void)selectItemWithID:(web::WebStateID)itemID pinned:(BOOL)pinned {
  WebStateSearchCriteria searchCriteria{
      .identifier = itemID,
      .pinned_state = pinned ? PinnedState::kPinned : PinnedState::kNonPinned,
  };

  int index = GetWebStateIndex(self.webStateList, searchCriteria);
  // TODO(crbug.com/1501837): Adapt the condition to open a tab group UI only
  // when `itemID` match a group.
  if (base::FeatureList::IsEnabled(kTabGroupsInGrid)) {
    // TODO(crbug.com/1501837): This should be move in the function (when
    // available) which handle when a user tab on a group cell. This should also
    // get the real group and not create one.
    tab_groups::TabGroupVisualData temporaryVisualData(
        u"To remove", tab_groups::TabGroupColorId::kCyan);
    if (index == WebStateList::kInvalidIndex) {
      [self.dispatcher showTabGroup:self.webStateList->CreateGroup(
                                        {index}, temporaryVisualData)];
    }

    return;
  }

  WebStateList* itemWebStateList = self.webStateList;
  if (index == WebStateList::kInvalidIndex) {
    if (pinned) {
      return;
    }
    // If this is a search result, it may contain items from other windows or
    // from the inactive browser - check inactive browser and other windows
    // before giving up.
    BrowserList* browserList =
        BrowserListFactory::GetForBrowserState(self.browserState);
    Browser* browser = GetBrowserForTabWithId(
        browserList, itemID, self.browserState->IsOffTheRecord());

    if (!browser) {
      return;
    }

    if (browser->IsInactive()) {
      base::RecordAction(
          base::UserMetricsAction("MobileTabGridOpenInactiveTabSearchResult"));
      index = itemWebStateList->count();
      MoveTabToBrowser(itemID, self.browser, index);
    } else {
      // Other windows case.
      itemWebStateList = browser->GetWebStateList();
      index = GetWebStateIndex(itemWebStateList,
                               WebStateSearchCriteria{
                                   .identifier = itemID,
                                   .pinned_state = PinnedState::kNonPinned,
                               });
      SceneState* targetSceneState = browser->GetSceneState();
      SceneState* currentSceneState = self.browser->GetSceneState();

      UISceneActivationRequestOptions* options =
          [[UISceneActivationRequestOptions alloc] init];
      options.requestingScene = currentSceneState.scene;

      [[UIApplication sharedApplication]
          requestSceneSessionActivation:targetSceneState.scene.session
                           userActivity:nil
                                options:options
                           errorHandler:^(NSError* error) {
                             LOG(ERROR) << base::SysNSStringToUTF8(
                                 error.localizedDescription);
                             NOTREACHED();
                           }];
    }
  }

  web::WebState* selectedWebState = itemWebStateList->GetWebStateAt(index);
  LogPriceDropMetrics(selectedWebState);

  base::TimeDelta timeSinceLastActivation =
      base::Time::Now() - selectedWebState->GetLastActiveTime();
  base::UmaHistogramCustomTimes(
      "IOS.TabGrid.TabSelected.TimeSinceLastActivation",
      timeSinceLastActivation, base::Minutes(1), base::Days(24), 50);

  // Don't attempt a no-op activation. Normally this is not an issue, but it's
  // possible that this method (-selectItemWithID:) is being called as part of
  // a WebStateListObserver callback, in which case even a no-op activation
  // will cause a CHECK().
  if (index == itemWebStateList->active_index()) {
    // In search mode the consumer doesn't have any information about the
    // selected item. So even if the active webstate is the same as the one that
    // is being selected, make sure that the consumer update its selected item.
    [self.consumer selectItemWithID:itemID];
    return;
  } else {
    base::RecordAction(
        base::UserMetricsAction("MobileTabGridMoveToExistingTab"));
  }

  // Avoid a reentrant activation. This is a fix for crbug.com/1134663, although
  // ignoring the selection at this point may do weird things.
  if (itemWebStateList->IsMutating()) {
    return;
  }

  // It should be safe to activate here.
  itemWebStateList->ActivateWebStateAt(index);
}

- (BOOL)isItemWithIDSelected:(web::WebStateID)itemID {
  int index = GetWebStateIndex(self.webStateList,
                               WebStateSearchCriteria{.identifier = itemID});
  if (index == WebStateList::kInvalidIndex) {
    return NO;
  }
  return index == self.webStateList->active_index();
}

- (void)setPinState:(BOOL)pinState forItemWithID:(web::WebStateID)itemID {
  SetWebStatePinnedState(self.webStateList, itemID, pinState);
}

- (void)closeItemWithID:(web::WebStateID)itemID {
  [self.gridConsumer setPageIdleStatus:NO];
  int index = GetWebStateIndex(self.webStateList,
                               WebStateSearchCriteria{
                                   .identifier = itemID,
                               });
  if (index != WebStateList::kInvalidIndex) {
    self.webStateList->CloseWebStateAt(index, WebStateList::CLOSE_USER_ACTION);
    return;
  }

  // `index` is `WebStateList::kInvalidIndex`, so `itemID` should be a search
  // result from a different window. Since this item is not from the current
  // browser, no UI updates will be sent to the current grid. Notify the current
  // grid consumer about the change.
  [self.consumer removeItemWithID:itemID selectedItemID:web::WebStateID()];
  base::RecordAction(
      base::UserMetricsAction("MobileTabGridSearchCloseTabFromAnotherWindow"));

  BrowserList* browserList =
      BrowserListFactory::GetForBrowserState(self.browserState);
  Browser* browser = GetBrowserForTabWithId(
      browserList, itemID, self.browserState->IsOffTheRecord());

  // If this tab is still associated with another browser, remove it from the
  // associated web state list.
  if (browser) {
    WebStateList* itemWebStateList = browser->GetWebStateList();
    index = GetWebStateIndex(itemWebStateList,
                             WebStateSearchCriteria{
                                 .identifier = itemID,
                                 .pinned_state = PinnedState::kNonPinned,
                             });
    itemWebStateList->CloseWebStateAt(index, WebStateList::CLOSE_USER_ACTION);
  }
}

- (void)closeItemsWithIDs:(const std::set<web::WebStateID>&)itemIDs {
  auto itemsCount = itemIDs.size();
  base::UmaHistogramCounts100("IOS.TabGrid.Selection.CloseTabs", itemsCount);
  RecordTabGridCloseTabsCount(itemsCount);

  WebStateList* webStateList = self.webStateList;
  {
    WebStateList::ScopedBatchOperation lock =
        webStateList->StartBatchOperation();
    for (const web::WebStateID itemID : itemIDs) {
      const int index = GetWebStateIndex(
          webStateList,
          WebStateSearchCriteria{.identifier = itemID,
                                 .pinned_state = PinnedState::kNonPinned});
      if (index != WebStateList::kInvalidIndex) {
        TabSwitcherItem* itemToRemove =
            [[TabSwitcherItem alloc] initWithIdentifier:itemID];
        GridItemIdentifier* identifierToRemove =
            [GridItemIdentifier tabIdentifier:itemToRemove];
        [_selectedEditingItems removeItem:identifierToRemove];
        webStateList->CloseWebStateAt(index, WebStateList::CLOSE_USER_ACTION);
      }
    }
  }

  const bool allTabsClosed = webStateList->empty();
  if (allTabsClosed) {
    if (!self.browserState->IsOffTheRecord()) {
      base::RecordAction(base::UserMetricsAction(
          "MobileTabGridSelectionCloseAllRegularTabsConfirmed"));
    } else {
      base::RecordAction(base::UserMetricsAction(
          "MobileTabGridSelectionCloseAllIncognitoTabsConfirmed"));
    }
  }
}

- (void)closeAllItems {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (void)saveAndCloseAllItems {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (void)undoCloseAllItems {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (void)discardSavedClosedItems {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (NSArray<UIMenuElement*>*)addToButtonMenuElementsForItems:
    (const std::set<web::WebStateID>&)itemIDs {
  if (!self.browser) {
    return nil;
  }

  NSMutableArray<UIMenuElement*>* actions = [[NSMutableArray alloc] init];

  ActionFactory* actionFactory = [[ActionFactory alloc]
      initWithScenario:kMenuScenarioHistogramTabGridAddTo];

  __weak BaseGridMediator* weakSelf = self;

  // Copy the set of items, so that the following block can use it.
  std::set<web::WebStateID> itemIDsCopy = itemIDs;
  UIAction* bookmarkAction = [actionFactory actionToBookmarkWithBlock:^{
    [weakSelf addItemsWithIDsToBookmarks:itemIDsCopy];
  }];
  // Bookmarking can be disabled from prefs (from an enterprise policy),
  // if that's the case grey out the option in the menu.
  BOOL isEditBookmarksEnabled =
      self.browser->GetBrowserState()->GetPrefs()->GetBoolean(
          bookmarks::prefs::kEditBookmarksEnabled);
  if (!isEditBookmarksEnabled) {
    bookmarkAction.attributes = UIMenuElementAttributesDisabled;
  }

  if (base::FeatureList::IsEnabled(kTabGroupsInGrid)) {
    ProceduralBlock createTabGroupActionBlock = ^{
      BOOL incognito = [weakSelf isIncognitoBrowser];
      [weakSelf.delegate showTabGroupCreationWithWithIdentifiers:itemIDsCopy
                                                       incognito:incognito];
    };
    UIAction* addToNewTabGroupAction = [actionFactory
        actionToAddTabsToNewGroupWithTabsNumber:itemIDs.size()
                                          block:createTabGroupActionBlock];
    [actions addObject:[UIMenu menuWithTitle:@""
                                       image:nil
                                  identifier:nil
                                     options:UIMenuOptionsDisplayInline
                                    children:@[ addToNewTabGroupAction ]]];
  }

  [actions addObject:[actionFactory actionToAddToReadingListWithBlock:^{
             [weakSelf addItemsWithIDsToReadingList:itemIDsCopy];
           }]];

  [actions addObject:bookmarkAction];

  return actions;
}

- (void)searchItemsWithText:(NSString*)searchText {
  TabsSearchService* searchService =
      TabsSearchServiceFactory::GetForBrowserState(self.browserState);
  const std::u16string& searchTerm = base::SysNSStringToUTF16(searchText);
  searchService->Search(
      searchTerm,
      base::BindOnce(^(
          std::vector<TabsSearchService::TabsSearchBrowserResults> results) {
        NSMutableArray* currentBrowserItems = [[NSMutableArray alloc] init];
        NSMutableArray* remainingItems = [[NSMutableArray alloc] init];
        for (const TabsSearchService::TabsSearchBrowserResults& browserResults :
             results) {
          for (web::WebState* webState : browserResults.web_states) {
            GridItemIdentifier* item = [GridItemIdentifier
                tabIdentifier:[[WebStateTabSwitcherItem alloc]
                                  initWithWebState:webState]];

            if (browserResults.browser == self.browser) {
              [currentBrowserItems addObject:item];
            } else {
              [remainingItems addObject:item];
            }
          }
        }

        NSArray* allItems = nil;
        // If there are results from Browsers other than the current one,
        // append those results to the end.
        if (remainingItems.count) {
          allItems = [currentBrowserItems
              arrayByAddingObjectsFromArray:remainingItems];
        } else {
          allItems = currentBrowserItems;
        }
        [self.consumer populateItems:allItems selectedItemID:web::WebStateID()];
      }));
}

- (void)resetToAllItems {
  [self populateConsumerItems];
}

- (void)fetchSearchHistoryResultsCountForText:(NSString*)searchText
                                   completion:(void (^)(size_t))completion {
  TabsSearchService* search_service =
      TabsSearchServiceFactory::GetForBrowserState(self.browserState);
  const std::u16string& searchTerm = base::SysNSStringToUTF16(searchText);
  search_service->SearchHistory(searchTerm,
                                base::BindOnce(^(size_t resultCount) {
                                  completion(resultCount);
                                }));
}

#pragma mark GridCommands helpers

- (void)insertNewItemAtIndex:(NSUInteger)index withURL:(const GURL&)newTabURL {
  // The incognito mediator's Browser is briefly set to nil after the last
  // incognito tab is closed.  This occurs because the incognito BrowserState
  // needs to be destroyed to correctly clear incognito browsing data.  Don't
  // attempt to create a new WebState with a nil BrowserState.
  if (!self.browser) {
    return;
  }

  // There are some circumstances where a new tab insertion can be erroniously
  // triggered while another web state list mutation is happening. To ensure
  // those bugs don't become crashes, check that the web state list is OK to
  // mutate.
  if (self.webStateList->IsMutating()) {
    // Shouldn't have happened!
    DCHECK(false) << "Reentrant web state insertion!";
    return;
  }

  DCHECK(self.browserState);
  web::WebState::CreateParams params(self.browserState);
  std::unique_ptr<web::WebState> webState = web::WebState::Create(params);

  web::NavigationManager::WebLoadParams loadParams(newTabURL);
  loadParams.transition_type = ui::PAGE_TRANSITION_TYPED;
  webState->GetNavigationManager()->LoadURLWithParams(loadParams);

  int webStateListIndex = [self webStateListIndexFromItemIndex:index];

  self.webStateList->InsertWebState(
      std::move(webState),
      WebStateList::InsertionParams::AtIndex(webStateListIndex).Activate());
}

#pragma mark - TabCollectionDragDropHandler

- (NSArray<UIDragItem*>*)allSelectedDragItems {
  NSMutableArray<UIDragItem*>* dragItems = [[NSMutableArray alloc] init];
  for (GridItemIdentifier* itemID in _selectedEditingItems.itemsIdentifiers) {
    switch (itemID.type) {
      case GridItemType::Tab: {
        UIDragItem* dragItem =
            [self dragItemForItemWithID:itemID.tabSwitcherItem.identifier];
        if (dragItem) {
          [dragItems addObject:dragItem];
        }
        break;
      }
      case GridItemType::Group:
        // TODO(crbug.com/1501837) : Add the drag action for tab groups.
        break;
      case GridItemType::SuggestedActions:
        // Suggested actions items are not dragable and not stored in
        // `_selectedEditingItems`.
        NOTREACHED_NORETURN();
    }
  }
  return dragItems;
}

- (UIDragItem*)dragItemForItem:(TabSwitcherItem*)item {
  return [self dragItemForItemWithID:item.identifier];
}

- (void)dragWillBeginForItem:(TabSwitcherItem*)item {
  _dragItemID = item.identifier;
}

- (void)dragSessionDidEnd {
  _dragItemID = web::WebStateID();
  // Update buttons as the number of items or the number of selected items might
  // have changed.
  [self.toolbarsMutator setButtonsEnabled:YES];
  [self configureToolbarsButtons];
}

- (UIDropOperation)dropOperationForDropSession:(id<UIDropSession>)session {
  UIDragItem* dragItem = session.localDragSession.items.firstObject;

  // Tab move operations only originate from Chrome so a local object is used.
  // Local objects allow synchronous drops, whereas NSItemProvider only allows
  // asynchronous drops.
  if ([dragItem.localObject isKindOfClass:[TabInfo class]]) {
    TabInfo* tabInfo = static_cast<TabInfo*>(dragItem.localObject);
    // If the dropped tab is from the same Chrome window and has been removed,
    // cancel the drop operation.
    if (_dragItemID == tabInfo.tabID &&
        GetWebStateIndex(self.webStateList,
                         WebStateSearchCriteria{
                             .identifier = tabInfo.tabID,
                             .pinned_state = PinnedState::kNonPinned,
                         }) == WebStateList::kInvalidIndex) {
      return UIDropOperationCancel;
    }
    if (self.browserState->IsOffTheRecord() == tabInfo.incognito) {
      return UIDropOperationMove;
    }

    // Tabs of different profiles (regular/incognito) cannot be dropped.
    return UIDropOperationForbidden;
  }

  // All URLs originating from Chrome create a new tab (as opposed to moving a
  // tab).
  if ([dragItem.localObject isKindOfClass:[NSURL class]]) {
    return UIDropOperationCopy;
  }

  // URLs are accepted when drags originate from outside Chrome.
  NSArray<NSString*>* acceptableTypes = @[ UTTypeURL.identifier ];
  if ([session hasItemsConformingToTypeIdentifiers:acceptableTypes]) {
    return UIDropOperationCopy;
  }

  // Other UTI types such as image data or file data cannot be dropped.
  return UIDropOperationForbidden;
}

- (void)dropItem:(UIDragItem*)dragItem
               toIndex:(NSUInteger)destinationIndex
    fromSameCollection:(BOOL)fromSameCollection {
  // Tab move operations only originate from Chrome so a local object is used.
  // Local objects allow synchronous drops, whereas NSItemProvider only allows
  // asynchronous drops.
  if ([dragItem.localObject isKindOfClass:[TabInfo class]]) {
    TabInfo* tabInfo = static_cast<TabInfo*>(dragItem.localObject);
    if (!fromSameCollection) {
      // Try to unpin the tab. If the returned index is invalid that means the
      // tab lives in another Browser.
      int tabIndex = WebStateList::kInvalidIndex;
      if (IsPinnedTabsEnabled()) {
        tabIndex = SetWebStatePinnedState(self.webStateList, tabInfo.tabID,
                                          /*pin_state=*/false);
      }
      if (tabIndex == WebStateList::kInvalidIndex) {
        // Move tab across Browsers.
        base::UmaHistogramEnumeration(kUmaGridViewDragOrigin,
                                      DragItemOrigin::kOtherBrwoser);
        MoveTabToBrowser(tabInfo.tabID, self.browser, destinationIndex);
        return;
      }
      base::UmaHistogramEnumeration(kUmaGridViewDragOrigin,
                                    DragItemOrigin::kSameBrowser);
    } else {
      base::UmaHistogramEnumeration(kUmaGridViewDragOrigin,
                                    DragItemOrigin::kSameCollection);
    }

    // Reorder tab within same grid.
    [self moveItemWithID:tabInfo.tabID toIndex:destinationIndex];
    return;
  }
  base::UmaHistogramEnumeration(kUmaGridViewDragOrigin, DragItemOrigin::kOther);

  // Handle URLs from within Chrome synchronously using a local object.
  if ([dragItem.localObject isKindOfClass:[URLInfo class]]) {
    URLInfo* droppedURL = static_cast<URLInfo*>(dragItem.localObject);
    [self insertNewItemAtIndex:destinationIndex withURL:droppedURL.URL];
    return;
  }
}

- (void)dropItemFromProvider:(NSItemProvider*)itemProvider
                     toIndex:(NSUInteger)destinationIndex
          placeholderContext:
              (id<UICollectionViewDropPlaceholderContext>)placeholderContext {
  if (![itemProvider canLoadObjectOfClass:[NSURL class]]) {
    [placeholderContext deletePlaceholder];
    return;
  }

  auto loadHandler =
      ^(__kindof id<NSItemProviderReading> providedItem, NSError* error) {
        dispatch_async(dispatch_get_main_queue(), ^{
          [placeholderContext deletePlaceholder];
          NSURL* droppedURL = static_cast<NSURL*>(providedItem);
          [self insertNewItemAtIndex:destinationIndex
                             withURL:net::GURLWithNSURL(droppedURL)];
        });
      };
  [itemProvider loadObjectOfClass:[NSURL class] completionHandler:loadHandler];
}

#pragma mark - Private

// Calls `-populateItems:selectedItemID:` on the consumer.
- (void)populateConsumerItems {
  if (!self.webStateList) {
    return;
  }
  [self.consumer populateItems:CreateItems(self.webStateList)
                selectedItemID:GetActiveNonPinnedTabID(self.webStateList)];
}

// Adds an observations to every non-pinned WebState.
- (void)addWebStateObservations {
  int firstIndex =
      IsPinnedTabsEnabled() ? self.webStateList->pinned_tabs_count() : 0;
  for (int i = firstIndex; i < self.webStateList->count(); i++) {
    web::WebState* webState = self.webStateList->GetWebStateAt(i);
    _scopedWebStateObservation->AddObservation(webState);
  }
}

// Returns a SnapshotStorage for the current browser.
- (SnapshotStorage*)snapshotStorage {
  if (!self.browser) {
    return nil;
  }
  return SnapshotBrowserAgent::FromBrowser(self.browser)->snapshot_storage();
}

- (void)addItemsWithIDsToReadingList:(const std::set<web::WebStateID>&)itemIDs {
  [self.delegate dismissPopovers];

  base::UmaHistogramCounts100("IOS.TabGrid.Selection.AddToReadingList",
                              itemIDs.size());

  NSArray<URLWithTitle*>* URLs = [self urlsWithTitleFromItemIDs:itemIDs];

  ReadingListAddCommand* command =
      [[ReadingListAddCommand alloc] initWithURLs:URLs];
  ReadingListBrowserAgent* readingListBrowserAgent =
      ReadingListBrowserAgent::FromBrowser(self.browser);
  readingListBrowserAgent->AddURLsToReadingList(command.URLs);
}

- (void)addItemsWithIDsToBookmarks:(const std::set<web::WebStateID>&)itemIDs {
  id<BookmarksCommands> bookmarkHandler =
      HandlerForProtocol(_browser->GetCommandDispatcher(), BookmarksCommands);

  if (!bookmarkHandler) {
    return;
  }
  [self.delegate dismissPopovers];
  base::RecordAction(
      base::UserMetricsAction("MobileTabGridAddedMultipleNewBookmarks"));
  base::UmaHistogramCounts100("IOS.TabGrid.Selection.AddToBookmarks",
                              itemIDs.size());

  NSArray<URLWithTitle*>* URLs = [self urlsWithTitleFromItemIDs:itemIDs];

  [bookmarkHandler bookmarkWithFolderChooser:URLs];
}

- (NSArray<URLWithTitle*>*)urlsWithTitleFromItemIDs:
    (const std::set<web::WebStateID>&)itemIDs {
  NSMutableArray<URLWithTitle*>* URLs = [[NSMutableArray alloc] init];
  for (const web::WebStateID itemID : itemIDs) {
    TabItem* item = GetTabItem(self.webStateList,
                               WebStateSearchCriteria{
                                   .identifier = itemID,
                                   .pinned_state = PinnedState::kNonPinned,
                               });
    URLWithTitle* URL = [[URLWithTitle alloc] initWithURL:item.URL
                                                    title:item.title];
    [URLs addObject:URL];
  }
  return URLs;
}

// Converts the WebStateList indexes to the collection view's item indexes by
// shifting indexes by the number of pinned WebStates.
- (NSUInteger)itemIndexFromWebStateListIndex:(int)index {
  if (!IsPinnedTabsEnabled()) {
    return index;
  }

  // If WebStateList's index is invalid or it's inside of pinned WebStates
  // range return invalid index.
  if (index == WebStateList::kInvalidIndex ||
      index < self.webStateList->pinned_tabs_count()) {
    return NSNotFound;
  }

  return index - self.webStateList->pinned_tabs_count();
}

// Converts the collection view's item indexes to the WebStateList indexes by
// shifting indexes by the number of pinned WebStates.
- (int)webStateListIndexFromItemIndex:(NSUInteger)index {
  if (!IsPinnedTabsEnabled()) {
    return index;
  }

  if (index == NSNotFound) {
    return WebStateList::kInvalidIndex;
  }

  return index + self.webStateList->pinned_tabs_count();
}

// Inserts/removes a non pinned item to/from the collection.
- (void)changePinnedStateForWebState:(web::WebState*)webState
                             atIndex:(int)index {
  if ([self isPinnedWebState:index]) {
    [self.consumer removeItemWithID:webState->GetUniqueIdentifier()
                     selectedItemID:GetActiveNonPinnedTabID(self.webStateList)];

    _scopedWebStateObservation->RemoveObservation(webState);
  } else {
    TabSwitcherItem* item =
        [[WebStateTabSwitcherItem alloc] initWithWebState:webState];
    NSUInteger itemIndex = [self itemIndexFromWebStateListIndex:index];
    [self.consumer insertItem:[GridItemIdentifier tabIdentifier:item]
                      atIndex:itemIndex
               selectedItemID:GetActiveNonPinnedTabID(self.webStateList)];

    _scopedWebStateObservation->AddObservation(webState);
  }
}

- (BOOL)isPinnedWebState:(int)index {
  if (IsPinnedTabsEnabled() && self.webStateList->IsWebStatePinnedAt(index)) {
    return YES;
  }
  return NO;
}

// Updates toolbars when the number of web state might be changed.
- (void)updateToolbarAfterNumberOfItemsChanged {
  if (self.currentMode == TabGridModeSelection && self.webStateList->empty()) {
    // Exit selection mode if there are no more tabs.
    self.currentMode = TabGridModeNormal;
  } else {
    // Update toolbar's buttons as the number of tabs have probably changed so
    // the options changed (ex: "Undo" may be available now).
    [self configureToolbarsButtons];
  }
}

// Returns a drag item for the given `itemID`.
- (UIDragItem*)dragItemForItemWithID:(web::WebStateID)itemID {
  web::WebState* webState = GetWebState(
      self.webStateList, WebStateSearchCriteria{
                             .identifier = itemID,
                             .pinned_state = PinnedState::kNonPinned,
                         });
  return CreateTabDragItem(webState);
}

- (BOOL)isIncognitoBrowser {
  return static_cast<BOOL>(self.browserState->IsOffTheRecord());
}

#pragma mark - TabGridPageMutator

- (void)currentlySelectedGrid:(BOOL)selected {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (void)switchToMode:(TabGridMode)mode {
  self.currentMode = mode;
}

#pragma mark - TabGridToolbarsGridDelegate

- (void)closeAllButtonTapped:(id)sender {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (void)doneButtonTapped:(id)sender {
  // Tapping Done when in selection mode, should only return back to the normal
  // mode.
  if (self.currentMode == TabGridModeSelection) {
    self.currentMode = TabGridModeNormal;
    // Records action when user exit the selection mode.
    base::RecordAction(base::UserMetricsAction("MobileTabGridSelectionDone"));
  } else {
    [self.toolbarTabGridDelegate doneButtonTapped:sender];
  }
}

- (void)newTabButtonTapped:(id)sender {
  NOTREACHED_NORETURN() << "Should be implemented in a subclass.";
}

- (void)selectAllButtonTapped:(id)sender {
  NSUInteger selectedItemsCount = _selectedEditingItems.tabsCount;
  BOOL allItemsSelected =
      static_cast<int>(selectedItemsCount) ==
      (self.webStateList->count() - self.webStateList->pinned_tabs_count());

  // Deselect all items if they are all already selected.
  if (allItemsSelected) {
    [_selectedEditingItems removeAllItems];
    base::RecordAction(
        base::UserMetricsAction("MobileTabGridSelectionDeselectAll"));
  } else {
    NSArray<GridItemIdentifier*>* identifiers = CreateItems(self.webStateList);
    for (GridItemIdentifier* identifier in identifiers) {
      [self addToSelectionItemID:identifier];
    }
    base::RecordAction(
        base::UserMetricsAction("MobileTabGridSelectionSelectAll"));
  }
  [self.consumer reload];
  [self configureToolbarsButtons];
}

- (void)searchButtonTapped:(id)sender {
  self.currentMode = TabGridModeSearch;
  base::RecordAction(base::UserMetricsAction("MobileTabGridSearchTabs"));
}

- (void)cancelSearchButtonTapped:(id)sender {
  base::RecordAction(base::UserMetricsAction("MobileTabGridCancelSearchTabs"));
  self.currentMode = TabGridModeNormal;
}

- (void)closeSelectedTabs:(id)sender {
  [self.delegate dismissPopovers];

  std::set<web::WebStateID> selectedIDs;
  for (GridItemIdentifier* identifier in _selectedEditingItems
           .itemsIdentifiers) {
    // TODO(crbug.com/1501837): Add the tab groups closing.
    if (identifier.type == GridItemType::Tab) {
      selectedIDs.insert(identifier.tabSwitcherItem.identifier);
    }
  }
  [self.delegate
      showCloseItemsConfirmationActionSheetWithBaseGridMediator:self
                                                        itemIDs:selectedIDs
                                                         anchor:sender];
}

- (void)shareSelectedTabs:(id)sender {
  [self.delegate dismissPopovers];

  base::RecordAction(
      base::UserMetricsAction("MobileTabGridSelectionShareTabs"));
  base::UmaHistogramCounts100("IOS.TabGrid.Selection.ShareTabs",
                              _selectedEditingItems.sharableItemsCount);
  [self.delegate baseGridMediator:self
                        shareURLs:[_selectedEditingItems selectedTabsURLs]
                           anchor:sender];
}

- (void)selectTabsButtonTapped:(id)sender {
  self.currentMode = TabGridModeSelection;
  [self configureToolbarsButtons];
  base::RecordAction(base::UserMetricsAction("MobileTabGridSelectTabs"));
}

#pragma mark - GridViewControllerMutator

- (void)userTappedOnItemID:(GridItemIdentifier*)itemID {
  CHECK(itemID.type == GridItemType::Group || itemID.type == GridItemType::Tab);
  if (self.currentMode == TabGridModeSelection) {
    if ([self isItemSelected:itemID]) {
      [self removeFromSelectionItemID:itemID];
    } else {
      [self addToSelectionItemID:itemID];
    }
  }
}

- (void)addToSelectionItemID:(GridItemIdentifier*)itemID {
  CHECK(itemID.type == GridItemType::Tab || itemID.type == GridItemType::Group);
  if (self.currentMode != TabGridModeSelection) {
    base::debug::DumpWithoutCrashing();
    return;
  }
  [_selectedEditingItems addItem:itemID];
  [self configureToolbarsButtons];
}

- (void)removeFromSelectionItemID:(GridItemIdentifier*)itemID {
  CHECK(itemID.type == GridItemType::Tab || itemID.type == GridItemType::Group);
  if (self.currentMode != TabGridModeSelection) {
    return;
  }

  [_selectedEditingItems removeItem:itemID];
  [self configureToolbarsButtons];
}

- (void)closeItemID:(web::WebStateID)itemID {
  [self closeItemWithID:itemID];
}

#pragma mark - BaseGridMediatorItemProvider

- (BOOL)isItemSelected:(GridItemIdentifier*)itemID {
  return [_selectedEditingItems containItem:itemID];
}

@end
