// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/base_grid_mediator.h"

#import "base/containers/contains.h"
#import "components/commerce/core/commerce_feature_list.h"
#import "ios/chrome/browser/commerce/model/shopping_persisted_data_tab_helper.h"
#import "ios/chrome/browser/shared/model/browser/test/test_browser.h"
#import "ios/chrome/browser/shared/model/browser_state/test_chrome_browser_state.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_opener.h"
#import "ios/chrome/browser/tabs/model/features.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/grid_item_identifier.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/grid_mediator_test.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/incognito/incognito_grid_mediator.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/regular/regular_grid_mediator.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/toolbars/tab_grid_toolbars_configuration.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/toolbars/test/fake_tab_grid_toolbars_mediator.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_switcher_item.h"
#import "ios/chrome/browser/ui/tab_switcher/test/fake_tab_collection_consumer.h"
#import "ios/web/public/test/fakes/fake_web_frames_manager.h"
#import "ios/web/public/test/fakes/fake_web_state.h"
#import "ios/web/public/web_state_id.h"
#import "testing/gtest/include/gtest/gtest.h"
#import "testing/gtest_mac.h"

namespace {

// BaseGridMediatorTest is parameterized on this enum to test all children
// mediator.
enum GridMediatorType { TEST_REGULAR_MEDIATOR, TEST_INCOGNITO_MEDIATOR };

const char kPriceTrackingWithOptimizationGuideParam[] =
    "price_tracking_with_optimization_guide";
const char kHasPriceDropUserAction[] = "Commerce.TabGridSwitched.HasPriceDrop";
const char kHasNoPriceDropUserAction[] = "Commerce.TabGridSwitched.NoPriceDrop";

}  // namespace

class BaseGridMediatorTest
    : public GridMediatorTestClass,
      public ::testing::WithParamInterface<GridMediatorType> {
 public:
  BaseGridMediatorTest() {}
  ~BaseGridMediatorTest() override {}

  void SetUp() override {
    GridMediatorTestClass::SetUp();
    if (GetParam() == TEST_INCOGNITO_MEDIATOR) {
      mediator_ = [[IncognitoGridMediator alloc] init];
    } else {
      mediator_ = [[RegularGridMediator alloc] init];
    }
    mediator_.consumer = consumer_;
    mediator_.browser = browser_.get();
    mediator_.toolbarsMutator = fake_toolbars_mediator_;
    [mediator_ currentlySelectedGrid:YES];
  }

  void TearDown() override {
    // Forces the IncognitoGridMediator to removes its Observer from
    // WebStateList before the Browser is destroyed.
    mediator_.browser = nullptr;
    mediator_ = nil;
    GridMediatorTestClass::TearDown();
  }

 protected:
  BaseGridMediator* mediator_;
};

// Variation on BaseGridMediatorTest which enable PriceDropIndicatorsFlag.
class BaseGridMediatorWithPriceDropIndicatorsTest
    : public BaseGridMediatorTest {
 protected:
  void InitializeFeatureFlags() override {
    scoped_feature_list_.InitWithFeaturesAndParameters(
        {{commerce::kCommercePriceTracking,
          {{kPriceTrackingWithOptimizationGuideParam, "true"}}}},
        {});
  }

  void SetFakePriceDrop(web::WebState* web_state) {
    ShoppingPersistedDataTabHelper::PriceDrop price_drop;
    price_drop.current_price = @"$5";
    price_drop.previous_price = @"$10";
    price_drop.url = web_state->GetLastCommittedURL();
    price_drop.timestamp = base::Time::Now();
    ShoppingPersistedDataTabHelper::FromWebState(web_state)
        ->SetPriceDropForTesting(
            std::make_unique<ShoppingPersistedDataTabHelper::PriceDrop>(
                std::move(price_drop)));
  }
};

#pragma mark - Consumer tests

// Tests drag and dropping an item that has been closed.
TEST_P(BaseGridMediatorTest, DragAndDropClosedItem) {
  std::unique_ptr<web::FakeWebState> web_state =
      CreateFakeWebStateWithURL(GURL("https://google.com"));
  web::WebStateID item_identifier = web_state.get()->GetUniqueIdentifier();
  browser_->GetWebStateList()->InsertWebState(
      std::move(web_state), WebStateList::InsertionParams::AtIndex(1));

  TabSwitcherItem* itemToSelect =
      [[TabSwitcherItem alloc] initWithIdentifier:item_identifier];
  [mediator_ switchToMode:TabGridModeSelection];
  [mediator_
      addToSelectionItemID:[GridItemIdentifier tabIdentifier:itemToSelect]];

  browser_->GetWebStateList()->CloseWebStateAt(1,
                                               WebStateList::CLOSE_USER_ACTION);
  EXPECT_EQ(0UL, [mediator_ allSelectedDragItems].count);
}

// Tests that the consumer is populated after the tab model is set on the
// mediator.
TEST_P(BaseGridMediatorTest, ConsumerPopulateItems) {
  EXPECT_EQ(3UL, consumer_.items.size());
  EXPECT_EQ(original_selected_identifier_, consumer_.selectedItemID);
}

// Tests that the consumer is notified when a web state is inserted.
TEST_P(BaseGridMediatorTest, ConsumerInsertItem) {
  ASSERT_EQ(3UL, consumer_.items.size());
  std::unique_ptr<web::FakeWebState> web_state =
      CreateFakeWebStateWithURL(GURL());
  web::WebStateID item_identifier = web_state.get()->GetUniqueIdentifier();
  browser_->GetWebStateList()->InsertWebState(
      std::move(web_state), WebStateList::InsertionParams::AtIndex(1));
  EXPECT_EQ(4UL, consumer_.items.size());
  // The same ID should be selected after the insertion, since the new web state
  // wasn't selected.
  EXPECT_EQ(original_selected_identifier_, consumer_.selectedItemID);
  EXPECT_EQ(item_identifier, consumer_.items[1]);
  EXPECT_FALSE(base::Contains(original_identifiers_, item_identifier));
}

// Tests that the consumer is notified when a web state is removed.
// The selected web state at index 1 is removed. The web state originally
// at index 2 should be the new selected item.
TEST_P(BaseGridMediatorTest, ConsumerRemoveItem) {
  browser_->GetWebStateList()->CloseWebStateAt(1, WebStateList::CLOSE_NO_FLAGS);
  EXPECT_EQ(2UL, consumer_.items.size());
  // Expect that a different web state is selected now.
  EXPECT_NE(original_selected_identifier_, consumer_.selectedItemID);
}

// Tests that the consumer is notified when the active web state is changed.
TEST_P(BaseGridMediatorTest, ConsumerUpdateSelectedItem) {
  EXPECT_EQ(original_selected_identifier_, consumer_.selectedItemID);
  browser_->GetWebStateList()->ActivateWebStateAt(2);
  EXPECT_EQ(
      browser_->GetWebStateList()->GetWebStateAt(2)->GetUniqueIdentifier(),
      consumer_.selectedItemID);
}

// Tests that the consumer is notified when a web state is replaced.
// The selected item is replaced, so the new selected item id should be the
// id of the new item.
TEST_P(BaseGridMediatorTest, ConsumerReplaceItem) {
  std::unique_ptr<web::FakeWebState> new_web_state =
      CreateFakeWebStateWithURL(GURL());
  web::WebStateID new_item_identifier = new_web_state->GetUniqueIdentifier();
  @autoreleasepool {
    browser_->GetWebStateList()->ReplaceWebStateAt(1, std::move(new_web_state));
  }
  EXPECT_EQ(3UL, consumer_.items.size());
  EXPECT_EQ(new_item_identifier, consumer_.selectedItemID);
  EXPECT_EQ(new_item_identifier, consumer_.items[1]);
  EXPECT_FALSE(base::Contains(original_identifiers_, new_item_identifier));
}

// Tests that the consumer is notified when a web state is moved.
TEST_P(BaseGridMediatorTest, ConsumerMoveItem) {
  web::WebStateID item1 = consumer_.items[1];
  web::WebStateID item2 = consumer_.items[2];
  browser_->GetWebStateList()->MoveWebStateAt(1, 2);
  EXPECT_EQ(item1, consumer_.items[2]);
  EXPECT_EQ(item2, consumer_.items[1]);
}

#pragma mark - Command tests

// Tests that the active index is updated when `-selectItemWithID:` is called.
// Tests that the consumer's selected index is updated.
TEST_P(BaseGridMediatorTest, SelectItemCommand) {
  // Previous selected index is 1.
  web::WebStateID identifier =
      browser_->GetWebStateList()->GetWebStateAt(2)->GetUniqueIdentifier();
  [mediator_ selectItemWithID:identifier pinned:NO];
  EXPECT_EQ(2, browser_->GetWebStateList()->active_index());
  EXPECT_EQ(identifier, consumer_.selectedItemID);
}

// Tests that the active index is updated when `-selectItemWithID:` is called.
// Tests that the consumer's selected index is updated with pinned state.
TEST_P(BaseGridMediatorTest, SelectPinnedItemCommand) {
  if (GetParam() == TEST_INCOGNITO_MEDIATOR || !IsPinnedTabsEnabled()) {
    // Test only available in non-incognito when pinned tabs are enabled.
    return;
  }
  WebStateList* web_state_list = browser_->GetWebStateList();
  web::WebStateID identifier_0 =
      web_state_list->GetWebStateAt(0)->GetUniqueIdentifier();
  web::WebStateID identifier_1 =
      web_state_list->GetWebStateAt(1)->GetUniqueIdentifier();
  web::WebStateID identifier_2 =
      web_state_list->GetWebStateAt(2)->GetUniqueIdentifier();
  [mediator_ setPinState:YES forItemWithID:identifier_0];
  ASSERT_EQ(1, browser_->GetWebStateList()->active_index());
  ASSERT_EQ(identifier_1, consumer_.selectedItemID);

  [mediator_ selectItemWithID:identifier_0 pinned:YES];

  EXPECT_EQ(0, browser_->GetWebStateList()->active_index());
  EXPECT_EQ(identifier_0, consumer_.selectedItemID);

  [mediator_ selectItemWithID:identifier_2 pinned:NO];

  EXPECT_EQ(2, browser_->GetWebStateList()->active_index());
  EXPECT_EQ(identifier_2, consumer_.selectedItemID);

  // Selecting the pinned one with pinned = NO fails.
  [mediator_ selectItemWithID:identifier_0 pinned:NO];

  EXPECT_EQ(2, browser_->GetWebStateList()->active_index());
  EXPECT_EQ(identifier_2, consumer_.selectedItemID);
}

// Tests the pinned tab command.
TEST_P(BaseGridMediatorTest, PinItemCommand) {
  if (GetParam() == TEST_INCOGNITO_MEDIATOR || !IsPinnedTabsEnabled()) {
    // Test only available in non-incognito when pinned tabs are enabled.
    return;
  }
  WebStateList* web_state_list = browser_->GetWebStateList();
  // At first the second web state is active.
  ASSERT_EQ(1, web_state_list->active_index());
  ASSERT_EQ(0, web_state_list->pinned_tabs_count());

  web::WebStateID selected_identifier =
      web_state_list->GetWebStateAt(1)->GetUniqueIdentifier();
  web::WebStateID identifier =
      web_state_list->GetWebStateAt(2)->GetUniqueIdentifier();

  [mediator_ setPinState:YES forItemWithID:identifier];

  // The pinned web state moved to the first position, moving the others.
  EXPECT_EQ(1, web_state_list->pinned_tabs_count());
  EXPECT_EQ(2, web_state_list->active_index());
  EXPECT_EQ(selected_identifier, consumer_.selectedItemID);

  [mediator_ setPinState:NO forItemWithID:identifier];

  // The pinned web state moves back to the end of the WebStateList.
  EXPECT_EQ(0, web_state_list->pinned_tabs_count());
  EXPECT_EQ(1, web_state_list->active_index());
  EXPECT_EQ(selected_identifier, consumer_.selectedItemID);
}

// Tests that the WebStateList count is decremented when
// `-closeItemWithID:` is called.
// Tests that the consumer's item count is also decremented.
TEST_P(BaseGridMediatorTest, CloseItemCommand) {
  // Previously there were 3 items.
  web::WebStateID identifier =
      browser_->GetWebStateList()->GetWebStateAt(0)->GetUniqueIdentifier();
  [mediator_ closeItemWithID:identifier];
  EXPECT_EQ(2, browser_->GetWebStateList()->count());
  EXPECT_EQ(2UL, consumer_.items.size());
}

// Tests that when `-addNewItem` is called, the WebStateList count is
// incremented, the `active_index` is at the end of WebStateList, the new
// web state has no opener, and the URL is the New Tab Page.
// Tests that the consumer has added an item with the correct identifier.
TEST_P(BaseGridMediatorTest, AddNewItemAtEndCommand) {
  // Previously there were 3 items and the selected index was 1.
  [mediator_ addNewItem];
  EXPECT_EQ(4, browser_->GetWebStateList()->count());
  EXPECT_EQ(3, browser_->GetWebStateList()->active_index());
  web::WebState* web_state = browser_->GetWebStateList()->GetWebStateAt(3);
  ASSERT_TRUE(web_state);
  EXPECT_EQ(web_state->GetBrowserState(), browser_state_.get());
  EXPECT_FALSE(web_state->HasOpener());
  // The URL of pending item (i.e. kChromeUINewTabURL) will not be returned
  // here because WebState doesn't load the URL until it's visible and
  // NavigationManager::GetVisibleURL requires WebState::IsLoading to be true
  // to return pending item's URL.
  EXPECT_EQ("", web_state->GetVisibleURL().spec());
  web::WebStateID identifier = web_state->GetUniqueIdentifier();
  EXPECT_FALSE(base::Contains(original_identifiers_, identifier));
  // Consumer checks.
  EXPECT_EQ(4UL, consumer_.items.size());
  EXPECT_EQ(identifier, consumer_.selectedItemID);
  EXPECT_EQ(identifier, consumer_.items[3]);
}

// Tests that when `-insertNewItemAtIndex:` is called, the WebStateList
// count is incremented, the `active_index` is the newly added index, the new
// web state has no opener, and the URL is the new tab page.
// Checks that the consumer has added an item with the correct identifier.
TEST_P(BaseGridMediatorTest, InsertNewItemCommand) {
  // Previously there were 3 items and the selected index was 1.
  [mediator_ insertNewItemAtIndex:0];
  EXPECT_EQ(4, browser_->GetWebStateList()->count());
  EXPECT_EQ(0, browser_->GetWebStateList()->active_index());
  web::WebState* web_state = browser_->GetWebStateList()->GetWebStateAt(0);
  ASSERT_TRUE(web_state);
  EXPECT_EQ(web_state->GetBrowserState(), browser_state_.get());
  EXPECT_FALSE(web_state->HasOpener());
  // The URL of pending item (i.e. kChromeUINewTabURL) will not be returned
  // here because WebState doesn't load the URL until it's visible and
  // NavigationManager::GetVisibleURL requires WebState::IsLoading to be true
  // to return pending item's URL.
  EXPECT_EQ("", web_state->GetVisibleURL().spec());
  web::WebStateID identifier = web_state->GetUniqueIdentifier();
  EXPECT_FALSE(base::Contains(original_identifiers_, identifier));
  // Consumer checks.
  EXPECT_EQ(4UL, consumer_.items.size());
  EXPECT_EQ(identifier, consumer_.selectedItemID);
  EXPECT_EQ(identifier, consumer_.items[0]);
}

// Tests that `-insertNewItemAtIndex:` is a no-op if the mediator's browser
// is nullptr.
TEST_P(BaseGridMediatorTest, InsertNewItemWithNoBrowserCommand) {
  mediator_.browser = nullptr;
  ASSERT_EQ(3, browser_->GetWebStateList()->count());
  ASSERT_EQ(1, browser_->GetWebStateList()->active_index());
  [mediator_ insertNewItemAtIndex:0];
  EXPECT_EQ(3, browser_->GetWebStateList()->count());
  EXPECT_EQ(1, browser_->GetWebStateList()->active_index());
}

// Tests that when `-moveItemFromIndex:toIndex:` is called, there is no change
// in the item count in WebStateList, but that the constituent web states
// have been reordered.
TEST_P(BaseGridMediatorTest, MoveItemCommand) {
  // Capture ordered original IDs.
  std::vector<web::WebStateID> pre_move_ids;
  for (int i = 0; i < 3; i++) {
    web::WebState* web_state = browser_->GetWebStateList()->GetWebStateAt(i);
    pre_move_ids.push_back(web_state->GetUniqueIdentifier());
  }
  web::WebStateID pre_move_selected_id =
      pre_move_ids[browser_->GetWebStateList()->active_index()];
  // Items start ordered [A, B, C].
  [mediator_ moveItemWithID:consumer_.items[0] toIndex:2];
  // Items should now be ordered [B, C, A] -- the pre-move identifiers should
  // still be in this order.
  // Item count hasn't changed.
  EXPECT_EQ(3, browser_->GetWebStateList()->count());
  // Active index has moved -- it was 1, now 0.
  EXPECT_EQ(0, browser_->GetWebStateList()->active_index());
  // Identifier at 0, 1, 2 should match the original_identifier_ at 1, 2, 0.
  for (int index = 0; index < 2; index++) {
    web::WebState* web_state =
        browser_->GetWebStateList()->GetWebStateAt(index);
    ASSERT_TRUE(web_state);
    web::WebStateID identifier = web_state->GetUniqueIdentifier();
    EXPECT_EQ(identifier, pre_move_ids[(index + 1) % 3]);
    EXPECT_EQ(identifier, consumer_.items[index]);
  }
  EXPECT_EQ(pre_move_selected_id, consumer_.selectedItemID);
}

// Tests that when `-searchItemsWithText:` is called, there is no change in the
// items in WebStateList and the correct items are populated by the consumer.
TEST_P(BaseGridMediatorTest, SearchItemsWithTextCommand) {
  // Capture ordered original IDs.
  std::vector<web::WebStateID> pre_search_ids;
  for (int i = 0; i < 3; i++) {
    web::WebState* web_state = browser_->GetWebStateList()->GetWebStateAt(i);
    pre_search_ids.push_back(web_state->GetUniqueIdentifier());
  }
  web::WebStateID expected_result_identifier =
      browser_->GetWebStateList()->GetWebStateAt(2)->GetUniqueIdentifier();

  [mediator_ searchItemsWithText:@"hello"];

  // Only one result should be found.
  EXPECT_TRUE(WaitForConsumerUpdates(1UL));
  EXPECT_EQ(expected_result_identifier, consumer_.items[0]);

  // Web states count should not change.
  EXPECT_EQ(3, browser_->GetWebStateList()->count());
  // Active index should not change.
  EXPECT_EQ(1, browser_->GetWebStateList()->active_index());
  // The order of the items should be the same.
  for (int i = 0; i < 3; i++) {
    web::WebState* web_state = browser_->GetWebStateList()->GetWebStateAt(i);
    ASSERT_TRUE(web_state);
    web::WebStateID identifier = web_state->GetUniqueIdentifier();
    EXPECT_EQ(identifier, pre_search_ids[i]);
  }
}

// Tests that when `-resetToAllItems:` is called, the consumer gets all the
// items from items in WebStateList and correct item selected.
TEST_P(BaseGridMediatorTest, resetToAllItems) {
  ASSERT_EQ(3, browser_->GetWebStateList()->count());
  ASSERT_EQ(3UL, consumer_.items.size());

  [mediator_ searchItemsWithText:@"hello"];
  // Only 1 result is in the consumer after the search is done.
  ASSERT_TRUE(WaitForConsumerUpdates(1UL));

  [mediator_ resetToAllItems];
  // consumer should revert back to have the items from the webstate list.
  ASSERT_TRUE(WaitForConsumerUpdates(3UL));
  // Active index should not change.
  EXPECT_EQ(original_selected_identifier_, consumer_.selectedItemID);

  // The order of the items on the consumer be the exact same order as the in
  // WebStateList.
  for (int i = 0; i < 3; i++) {
    web::WebState* web_state = browser_->GetWebStateList()->GetWebStateAt(i);
    ASSERT_TRUE(web_state);
    web::WebStateID identifier = web_state->GetUniqueIdentifier();
    EXPECT_EQ(identifier, consumer_.items[i]);
  }
}

TEST_P(BaseGridMediatorWithPriceDropIndicatorsTest,
       TestSelectItemWithNoPriceDrop) {
  web::WebState* web_state_to_select =
      browser_->GetWebStateList()->GetWebStateAt(2);
  // No need to set a null price drop - it will be null by default.
  [mediator_ selectItemWithID:web_state_to_select->GetUniqueIdentifier()
                       pinned:NO];
  EXPECT_EQ(1, user_action_tester_.GetActionCount(kHasNoPriceDropUserAction));
  EXPECT_EQ(0, user_action_tester_.GetActionCount(kHasPriceDropUserAction));
}

TEST_P(BaseGridMediatorWithPriceDropIndicatorsTest,
       TestSelectItemWithPriceDrop) {
  web::WebState* web_state_to_select =
      browser_->GetWebStateList()->GetWebStateAt(2);
  // Add a fake price drop.
  SetFakePriceDrop(web_state_to_select);
  [mediator_ selectItemWithID:web_state_to_select->GetUniqueIdentifier()
                       pinned:NO];
  EXPECT_EQ(1, user_action_tester_.GetActionCount(kHasPriceDropUserAction));
  EXPECT_EQ(0, user_action_tester_.GetActionCount(kHasNoPriceDropUserAction));
}

// Ensures that when there is web states in normal mode, the toolbar
// configuration is correct.
TEST_P(BaseGridMediatorTest, TestToolbarsNormalModeWithWebstates) {
  EXPECT_EQ(3UL, consumer_.items.size());
  // Force the toolbar configuration by setting the view as currently selected.
  [mediator_ currentlySelectedGrid:YES];
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.closeAllButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.doneButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.newTabButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.searchButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.selectTabsButton);

  EXPECT_FALSE(fake_toolbars_mediator_.configuration.undoButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.deselectAllButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.selectAllButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.addToButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.closeSelectedTabsButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.shareButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.cancelSearchButton);
}

// Ensures that selection button are correctly enabled when pushing select tab
// button.
TEST_P(BaseGridMediatorTest, TestToolbarsSelectionModeWithoutSelection) {
  EXPECT_EQ(3UL, consumer_.items.size());
  [mediator_ selectTabsButtonTapped:nil];

  EXPECT_TRUE(fake_toolbars_mediator_.configuration.selectAllButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.doneButton);
  EXPECT_EQ(0u, fake_toolbars_mediator_.configuration.selectedItemsCount);

  EXPECT_FALSE(fake_toolbars_mediator_.configuration.closeAllButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.newTabButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.searchButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.selectTabsButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.undoButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.deselectAllButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.addToButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.closeSelectedTabsButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.shareButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.cancelSearchButton);
}

// Ensures that selection button are correctly enabled when pushing select tab
// button and the user selected one tab.
TEST_P(BaseGridMediatorTest, TestToolbarsSelectionModeWithSelection) {
  EXPECT_EQ(3UL, consumer_.items.size());
  [mediator_ selectTabsButtonTapped:nil];

  // Simulate a user who tapped on a tab.
  TabSwitcherItem* itemToTap =
      [[TabSwitcherItem alloc] initWithIdentifier:browser_->GetWebStateList()
                                                      ->GetWebStateAt(1)
                                                      ->GetUniqueIdentifier()];
  [mediator_ userTappedOnItemID:[GridItemIdentifier tabIdentifier:itemToTap]];

  EXPECT_TRUE(fake_toolbars_mediator_.configuration.selectAllButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.doneButton);
  EXPECT_EQ(1u, fake_toolbars_mediator_.configuration.selectedItemsCount);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.closeSelectedTabsButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.shareButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.addToButton);

  EXPECT_FALSE(fake_toolbars_mediator_.configuration.closeAllButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.newTabButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.searchButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.selectTabsButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.undoButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.deselectAllButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.cancelSearchButton);
}

// Tests that no updates to the toolbars happen when the mediator is not
// selected.
TEST_P(BaseGridMediatorTest, NoToolbarUpdateNotSelected) {
  EXPECT_EQ(3UL, consumer_.items.size());
  [mediator_ selectTabsButtonTapped:nil];

  EXPECT_TRUE(fake_toolbars_mediator_.configuration.selectAllButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.doneButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.closeSelectedTabsButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.shareButton);

  [mediator_ currentlySelectedGrid:NO];

  // Simulate a user who tapped on a tab.
  TabSwitcherItem* itemToTap =
      [[TabSwitcherItem alloc] initWithIdentifier:browser_->GetWebStateList()
                                                      ->GetWebStateAt(1)
                                                      ->GetUniqueIdentifier()];
  [mediator_ userTappedOnItemID:[GridItemIdentifier tabIdentifier:itemToTap]];

  // No update on the configuration as the mediator is no longer selected.
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.selectAllButton);
  EXPECT_TRUE(fake_toolbars_mediator_.configuration.doneButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.closeSelectedTabsButton);
  EXPECT_FALSE(fake_toolbars_mediator_.configuration.shareButton);
}

INSTANTIATE_TEST_SUITE_P(
    /* No InstantiationName */,
    BaseGridMediatorTest,
    ::testing::Values(GridMediatorType::TEST_REGULAR_MEDIATOR,
                      GridMediatorType::TEST_INCOGNITO_MEDIATOR));

INSTANTIATE_TEST_SUITE_P(
    /* No InstantiationName */,
    BaseGridMediatorWithPriceDropIndicatorsTest,
    ::testing::Values(GridMediatorType::TEST_REGULAR_MEDIATOR,
                      GridMediatorType::TEST_INCOGNITO_MEDIATOR));
