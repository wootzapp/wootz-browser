// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/app_list/views/app_list_bubble_apps_collections_page.h"

#include <utility>

#include "ash/app_list/test/app_list_test_helper.h"
#include "ash/app_list/views/app_list_bubble_view.h"
#include "ash/app_list/views/apps_collections_dismiss_dialog.h"
#include "ash/app_list/views/apps_grid_context_menu.h"
#include "ash/app_list/views/search_result_page_anchored_dialog.h"
#include "ash/public/cpp/app_list/app_list_features.h"
#include "ash/test/ash_test_base.h"
#include "base/test/scoped_feature_list.h"
#include "ui/compositor/layer.h"
#include "ui/compositor/layer_animator.h"
#include "ui/compositor/scoped_animation_duration_scale_mode.h"
#include "ui/compositor/test/layer_animation_stopped_waiter.h"
#include "ui/compositor/test/test_utils.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/menu/menu_item_view.h"
#include "ui/views/controls/menu/submenu_view.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/test/widget_test.h"

namespace ash {
namespace {

class AppListBubbleAppsCollectionsPageTest : public AshTestBase {
 public:
  AppListBubbleAppsCollectionsPageTest() = default;

  // AshTestBase:
  void SetUp() override {
    scoped_feature_list_.InitWithFeatures(
        {app_list_features::kAppsCollections,
         app_list_features::kForceShowAppsCollections},
        {});
    AshTestBase::SetUp();
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_F(AppListBubbleAppsCollectionsPageTest,
       AppsCollectionsPageVisibleAfterQuicklyClearingSearch) {
  // Open the app list without animation.
  ASSERT_EQ(ui::ScopedAnimationDurationScaleMode::duration_multiplier(),
            ui::ScopedAnimationDurationScaleMode::ZERO_DURATION);
  auto* helper = GetAppListTestHelper();
  helper->ShowAppList();

  auto* apps_collections_page = helper->GetBubbleAppsCollectionsPage();
  ASSERT_TRUE(apps_collections_page->GetVisible());

  // Enable animations.
  ui::ScopedAnimationDurationScaleMode duration(
      ui::ScopedAnimationDurationScaleMode::NON_ZERO_DURATION);

  // Type a key to trigger the animation to transition to the search page.
  PressAndReleaseKey(ui::VKEY_A);
  ASSERT_TRUE(apps_collections_page->GetPageAnimationLayerForTest()
                  ->GetAnimator()
                  ->is_animating());

  // Before the animation completes, delete the search. This should abort
  // animations, animate back to the apps page, and leave the apps page visible.
  PressAndReleaseKey(ui::VKEY_BACK);
  ui::LayerAnimationStoppedWaiter().Wait(
      apps_collections_page->GetPageAnimationLayerForTest());
  EXPECT_TRUE(apps_collections_page->GetVisible());
  EXPECT_EQ(
      1.0f,
      apps_collections_page->scroll_view()->contents()->layer()->opacity());
}

TEST_F(AppListBubbleAppsCollectionsPageTest, AnimateHidePage) {
  // Open the app list without animation.
  ASSERT_EQ(ui::ScopedAnimationDurationScaleMode::duration_multiplier(),
            ui::ScopedAnimationDurationScaleMode::ZERO_DURATION);
  auto* helper = GetAppListTestHelper();
  helper->ShowAppList();

  auto* apps_collections_page = helper->GetBubbleAppsCollectionsPage();
  ASSERT_TRUE(apps_collections_page->GetVisible());

  // Enable animations.
  ui::ScopedAnimationDurationScaleMode duration(
      ui::ScopedAnimationDurationScaleMode::NON_ZERO_DURATION);

  // Type a key to trigger the animation to transition to the search page.
  PressAndReleaseKey(ui::VKEY_A);
  ui::Layer* layer = apps_collections_page->GetPageAnimationLayerForTest();
  ui::LayerAnimationStoppedWaiter().Wait(layer);

  // Ensure there is one more frame presented after animation finishes to allow
  // animation throughput data to be passed from cc to ui.
  layer->GetCompositor()->ScheduleFullRedraw();
  EXPECT_TRUE(ui::WaitForNextFrameToBePresented(layer->GetCompositor()));

  // Apps page is not visible.
  EXPECT_FALSE(apps_collections_page->GetVisible());
}

TEST_F(AppListBubbleAppsCollectionsPageTest, AnimateShowPage) {
  // Open the app list without animation.
  ASSERT_EQ(ui::ScopedAnimationDurationScaleMode::duration_multiplier(),
            ui::ScopedAnimationDurationScaleMode::ZERO_DURATION);
  auto* helper = GetAppListTestHelper();
  helper->ShowAppList();

  // Type a key switch to the search page.
  PressAndReleaseKey(ui::VKEY_A);

  auto* apps_collections_page = helper->GetBubbleAppsCollectionsPage();
  ASSERT_FALSE(apps_collections_page->GetVisible());

  // Enable animations.
  ui::ScopedAnimationDurationScaleMode duration(
      ui::ScopedAnimationDurationScaleMode::NON_ZERO_DURATION);

  // Press escape to trigger animation back to the apps page.
  PressAndReleaseKey(ui::VKEY_ESCAPE);
  ui::Layer* layer = apps_collections_page->GetPageAnimationLayerForTest();
  ui::LayerAnimationStoppedWaiter().Wait(layer);

  // Ensure there is one more frame presented after animation finishes to allow
  // animation throughput data to be passed from cc to ui.
  layer->GetCompositor()->ScheduleFullRedraw();
  EXPECT_TRUE(ui::WaitForNextFrameToBePresented(layer->GetCompositor()));

  // Apps page is visible.
  EXPECT_TRUE(apps_collections_page->GetVisible());
}

TEST_F(AppListBubbleAppsCollectionsPageTest, DismissNudgeIsVisible) {
  // Open the app list without animation.
  ASSERT_EQ(ui::ScopedAnimationDurationScaleMode::duration_multiplier(),
            ui::ScopedAnimationDurationScaleMode::ZERO_DURATION);
  auto* helper = GetAppListTestHelper();
  helper->ShowAppList();

  AppListToastContainerView* toast_container =
      helper->GetBubbleAppsCollectionsPage()->GetToastContainerViewForTest();
  EXPECT_TRUE(toast_container->IsToastVisible());
}

TEST_F(AppListBubbleAppsCollectionsPageTest, ShowAppsPageAfterDismissingNudge) {
  // Open the app list without animation.
  ASSERT_EQ(ui::ScopedAnimationDurationScaleMode::duration_multiplier(),
            ui::ScopedAnimationDurationScaleMode::ZERO_DURATION);
  auto* helper = GetAppListTestHelper();
  helper->ShowAppList();

  auto* apps_collections_page = helper->GetBubbleAppsCollectionsPage();
  AppListToastContainerView* toast_container =
      apps_collections_page->GetToastContainerViewForTest();
  EXPECT_TRUE(toast_container->IsToastVisible());

  // Click on close button to dismiss the toast.
  LeftClickOn(toast_container->GetToastButton());
  EXPECT_FALSE(toast_container->IsToastVisible());

  // Apps collections page is not visible.
  EXPECT_FALSE(apps_collections_page->GetVisible());
}

TEST_F(AppListBubbleAppsCollectionsPageTest,
       CancelDismissDialogAfterAttempingSort) {
  // Open the app list without animation.
  ASSERT_EQ(ui::ScopedAnimationDurationScaleMode::duration_multiplier(),
            ui::ScopedAnimationDurationScaleMode::ZERO_DURATION);
  auto* helper = GetAppListTestHelper();
  helper->ShowAppList();

  auto* apps_collections_page = helper->GetBubbleAppsCollectionsPage();
  AppsGridContextMenu* context_menu =
      apps_collections_page->context_menu_for_test();
  EXPECT_FALSE(context_menu->IsMenuShowing());

  // Get a point in `apps_collections_page` that doesn't have an item on it.
  const gfx::Point empty_space =
      apps_collections_page->GetBoundsInScreen().CenterPoint();

  // Open the menu to test the alphabetical sort option.
  GetEventGenerator()->MoveMouseTo(empty_space);
  GetEventGenerator()->ClickRightButton();
  EXPECT_TRUE(context_menu->IsMenuShowing());

  // Open the Reorder by Name submenu.
  views::MenuItemView* reorder_option =
      context_menu->root_menu_item_view()->GetSubmenu()->GetMenuItemAt(1);
  ASSERT_EQ(reorder_option->title(), u"Name");
  LeftClickOn(reorder_option);

  ASSERT_TRUE(helper->GetBubbleSearchPageDialog());

  views::Widget* widget = helper->GetBubbleSearchPageDialog()->widget();
  views::WidgetDelegate* widget_delegate = widget->widget_delegate();
  views::test::WidgetDestroyedWaiter widget_waiter(widget);
  LeftClickOn(static_cast<AppsCollectionsDismissDialog*>(widget_delegate)
                  ->cancel_button_for_test());
  widget_waiter.Wait();

  // Apps collections page is still visible.
  EXPECT_TRUE(apps_collections_page->GetVisible());
  EXPECT_EQ(AppListSortOrder::kCustom, helper->model()->requested_sort_order());
}

TEST_F(AppListBubbleAppsCollectionsPageTest, ShowAppsPageAfterSorting) {
  // Open the app list without animation.
  ASSERT_EQ(ui::ScopedAnimationDurationScaleMode::duration_multiplier(),
            ui::ScopedAnimationDurationScaleMode::ZERO_DURATION);
  auto* helper = GetAppListTestHelper();
  helper->ShowAppList();

  auto* apps_collections_page = helper->GetBubbleAppsCollectionsPage();
  AppsGridContextMenu* context_menu =
      apps_collections_page->context_menu_for_test();
  EXPECT_FALSE(context_menu->IsMenuShowing());

  // Get a point in `apps_collections_page` that doesn't have an item on it.
  const gfx::Point empty_space =
      apps_collections_page->GetBoundsInScreen().CenterPoint();

  // Open the menu to test the alphabetical sort option.
  GetEventGenerator()->MoveMouseTo(empty_space);
  GetEventGenerator()->ClickRightButton();
  EXPECT_TRUE(context_menu->IsMenuShowing());

  // Open the Reorder by Name submenu.
  views::MenuItemView* reorder_option =
      context_menu->root_menu_item_view()->GetSubmenu()->GetMenuItemAt(1);
  ASSERT_EQ(reorder_option->title(), u"Name");
  LeftClickOn(reorder_option);

  ASSERT_TRUE(helper->GetBubbleSearchPageDialog());

  views::Widget* widget = helper->GetBubbleSearchPageDialog()->widget();
  views::WidgetDelegate* widget_delegate = widget->widget_delegate();
  views::test::WidgetDestroyedWaiter widget_waiter(widget);
  LeftClickOn(static_cast<AppsCollectionsDismissDialog*>(widget_delegate)
                  ->accept_button_for_test());
  widget_waiter.Wait();

  // Apps collections page is not visible.
  EXPECT_FALSE(apps_collections_page->GetVisible());
  EXPECT_EQ(AppListSortOrder::kNameAlphabetical,
            helper->model()->requested_sort_order());
}

}  // namespace
}  // namespace ash
