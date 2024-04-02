// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <memory>
#include <vector>

#include "ash/accessibility/accessibility_controller.h"
#include "ash/accessibility/magnifier/docked_magnifier_controller.h"
#include "ash/accessibility/test_accessibility_controller_client.h"
#include "ash/constants/ash_features.h"
#include "ash/constants/ash_pref_names.h"
#include "ash/display/window_tree_host_manager.h"
#include "ash/keyboard/ui/keyboard_ui_controller.h"
#include "ash/public/cpp/accelerators.h"
#include "ash/public/cpp/test/shell_test_api.h"
#include "ash/public/cpp/window_properties.h"
#include "ash/root_window_controller.h"
#include "ash/screen_util.h"
#include "ash/shell.h"
#include "ash/shell_delegate.h"
#include "ash/style/close_button.h"
#include "ash/style/system_toast_style.h"
#include "ash/system/toast/toast_manager_impl.h"
#include "ash/test/ash_test_base.h"
#include "ash/test/ash_test_util.h"
#include "ash/test_shell_delegate.h"
#include "ash/wm/desks/desks_controller.h"
#include "ash/wm/desks/desks_test_util.h"
#include "ash/wm/desks/desks_util.h"
#include "ash/wm/desks/legacy_desk_bar_view.h"
#include "ash/wm/mru_window_tracker.h"
#include "ash/wm/overview/overview_controller.h"
#include "ash/wm/overview/overview_drop_target.h"
#include "ash/wm/overview/overview_focus_cycler.h"
#include "ash/wm/overview/overview_grid.h"
#include "ash/wm/overview/overview_grid_test_api.h"
#include "ash/wm/overview/overview_group_item.h"
#include "ash/wm/overview/overview_item.h"
#include "ash/wm/overview/overview_item_view.h"
#include "ash/wm/overview/overview_session.h"
#include "ash/wm/overview/overview_test_base.h"
#include "ash/wm/overview/overview_test_util.h"
#include "ash/wm/overview/overview_utils.h"
#include "ash/wm/overview/overview_window_drag_controller.h"
#include "ash/wm/overview/scoped_overview_transform_window.h"
#include "ash/wm/snap_group/snap_group.h"
#include "ash/wm/snap_group/snap_group_controller.h"
#include "ash/wm/splitview/faster_split_view.h"
#include "ash/wm/splitview/split_view_constants.h"
#include "ash/wm/splitview/split_view_controller.h"
#include "ash/wm/splitview/split_view_divider.h"
#include "ash/wm/splitview/split_view_divider_view.h"
#include "ash/wm/splitview/split_view_overview_session.h"
#include "ash/wm/splitview/split_view_utils.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "ash/wm/tablet_mode/tablet_mode_controller_test_api.h"
#include "ash/wm/window_cycle/window_cycle_controller.h"
#include "ash/wm/window_cycle/window_cycle_list.h"
#include "ash/wm/window_cycle/window_cycle_view.h"
#include "ash/wm/window_mini_view.h"
#include "ash/wm/window_resizer.h"
#include "ash/wm/window_state.h"
#include "ash/wm/window_util.h"
#include "ash/wm/wm_constants.h"
#include "ash/wm/wm_event.h"
#include "ash/wm/wm_metrics.h"
#include "ash/wm/workspace/multi_window_resize_controller.h"
#include "ash/wm/workspace/workspace_event_handler.h"
#include "ash/wm/workspace_controller_test_api.h"
#include "base/check_op.h"
#include "base/run_loop.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/time/time.h"
#include "chromeos/ui/base/window_state_type.h"
#include "chromeos/ui/frame/caption_buttons/snap_controller.h"
#include "ui/aura/client/aura_constants.h"
#include "ui/aura/test/test_window_delegate.h"
#include "ui/base/cursor/mojom/cursor_type.mojom-shared.h"
#include "ui/base/hit_test.h"
#include "ui/compositor/scoped_animation_duration_scale_mode.h"
#include "ui/display/test/display_manager_test_api.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/keyboard_codes_posix.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/point_conversions.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/geometry/vector2d.h"
#include "ui/views/widget/widget.h"
#include "ui/wm/core/window_modality_controller.h"
#include "ui/wm/core/window_util.h"
#include "ui/wm/public/window_move_client.h"

namespace ash {

namespace {

using ::chromeos::WindowStateType;

using ui::mojom::CursorType;

using WindowCyclingDirection = WindowCycleController::WindowCyclingDirection;

SplitViewController* split_view_controller() {
  return SplitViewController::Get(Shell::GetPrimaryRootWindow());
}

SplitViewDivider* split_view_divider() {
  return split_view_controller()->split_view_divider();
}

gfx::Rect split_view_divider_bounds_in_screen() {
  return split_view_divider()->GetDividerBoundsInScreen(
      /*is_dragging=*/false);
}

const gfx::Rect work_area_bounds() {
  return display::Screen::GetScreen()->GetPrimaryDisplay().work_area();
}

void SwitchToTabletMode() {
  TabletModeControllerTestApi test_api;
  test_api.DetachAllMice();
  test_api.EnterTabletMode();
}

void ExitTabletMode() {
  TabletModeControllerTestApi().LeaveTabletMode();
}

gfx::Rect GetOverviewGridBounds() {
  OverviewSession* overview_session = GetOverviewSession();
  return overview_session
             ? OverviewGridTestApi(overview_session->grid_list()[0].get())
                   .bounds()
             : gfx::Rect();
}

void SnapOneTestWindow(
    aura::Window* window,
    WindowStateType state_type,
    float snap_ratio,
    WindowSnapActionSource snap_action_source = WindowSnapActionSource::kTest) {
  WindowState* window_state = WindowState::Get(window);
  const WindowSnapWMEvent snap_event(
      state_type == WindowStateType::kPrimarySnapped ? WM_EVENT_SNAP_PRIMARY
                                                     : WM_EVENT_SNAP_SECONDARY,
      snap_ratio, snap_action_source);
  window_state->OnWMEvent(&snap_event);
  EXPECT_EQ(state_type, window_state->GetStateType());
}

// Verifies that `window` is in split view overview, where `window` is
// excluded from overview, and overview occupies the work area opposite of
// `window`. Returns the corresponding `SplitViewOverviewSession` if exits and
// nullptr otherwise. `faster_split_screen_setup` specifies whether the
// `SplitViewOverviewSession` is initiated by faster split screen set up or not,
// where behaviors differ such as overview widget.
SplitViewOverviewSession* VerifySplitViewOverviewSession(
    aura::Window* window,
    bool faster_split_screen_setup = true) {
  auto* overview_controller = OverviewController::Get();
  EXPECT_TRUE(overview_controller->InOverviewSession());
  EXPECT_FALSE(
      overview_controller->overview_session()->IsWindowInOverview(window));

  SplitViewOverviewSession* split_view_overview_session =
      RootWindowController::ForWindow(window)->split_view_overview_session();
  EXPECT_TRUE(split_view_overview_session);
  gfx::Rect expected_grid_bounds = work_area_bounds();
  expected_grid_bounds.Subtract(window->GetBoundsInScreen());

  if (split_view_divider()->divider_widget()) {
    expected_grid_bounds.Subtract(split_view_divider_bounds_in_screen());
  }

  // Clamp the length on the side that can be shrunk by resizing to avoid going
  // below the threshold i.e. 1/3 of the corresponding work area length.
  const bool is_horizontal = IsLayoutHorizontal(Shell::GetPrimaryRootWindow());
  const int min_length = (is_horizontal ? work_area_bounds().width()
                                        : work_area_bounds().height()) /
                         3;
  if (is_horizontal) {
    expected_grid_bounds.set_width(
        std::max(expected_grid_bounds.width(), min_length));
  } else {
    expected_grid_bounds.set_height(
        std::max(expected_grid_bounds.height(), min_length));
  }

  if (!Shell::Get()->IsInTabletMode()) {
    EXPECT_EQ(expected_grid_bounds, GetOverviewGridBounds());
  }

  EXPECT_TRUE(expected_grid_bounds.Contains(GetOverviewGridBounds()));

  if (!Shell::Get()->IsInTabletMode() && faster_split_screen_setup) {
    auto* overview_grid = GetOverviewGridForRoot(window->GetRootWindow());
    EXPECT_TRUE(overview_grid->faster_splitview_widget());
    EXPECT_FALSE(overview_grid->no_windows_widget());
    EXPECT_FALSE(overview_grid->GetSaveDeskButtonContainer());
    EXPECT_FALSE(overview_grid->desks_bar_view());
  }

  return split_view_overview_session;
}

// Maximize the snapped window which will exit the split view session. This is
// used in preparation for the next round of testing.
void MaximizeToClearTheSession(aura::Window* window) {
  WindowState* window_state = WindowState::Get(window);
  window_state->Maximize();
  SplitViewOverviewSession* split_view_overview_session =
      RootWindowController::ForWindow(window)->split_view_overview_session();
  EXPECT_FALSE(split_view_overview_session);
}

// Selects the overview item for `window`.
void ClickOverviewItem(ui::test::EventGenerator* event_generator,
                       aura::Window* window) {
  event_generator->MoveMouseTo(gfx::ToRoundedPoint(
      GetOverviewItemForWindow(window)->GetTransformedBounds().CenterPoint()));
  event_generator->ClickLeftButton();
}

// Drag the given group `item` to the `screen_location`. This is added before
// the event handling of the middle seam is done.
void DragGroupItemToPoint(OverviewItemBase* item,
                          const gfx::Point& screen_location,
                          ui::test::EventGenerator* event_generator,
                          bool by_touch_gestures,
                          bool drop) {
  DCHECK(item);

  gfx::Point location =
      gfx::ToRoundedPoint(item->target_bounds().CenterPoint());
  // TODO(michelefan): Use the center point of the `overview_item` after
  // implementing or defining the event handling for the middle seam area.
  location.Offset(/*delta_x=*/5, /*delta_y=*/5);
  event_generator->set_current_screen_location(location);
  if (by_touch_gestures) {
    event_generator->PressTouch();
    event_generator->MoveTouchBy(50, 0);
    event_generator->MoveTouch(screen_location);
    if (drop) {
      event_generator->ReleaseTouch();
    }
  } else {
    event_generator->PressLeftButton();
    Shell::Get()->cursor_manager()->SetDisplay(
        display::Screen::GetScreen()->GetDisplayNearestPoint(screen_location));
    event_generator->MoveMouseTo(screen_location);
    if (drop) {
      event_generator->ReleaseLeftButton();
    }
  }
}

// Returns true if the union bounds of `w1`, `w2` and the split view
// divider (if exists) equal to the bounds of the work area and false
// otherwise.
bool UnionBoundsEqualToWorkAreaBounds(aura::Window* w1, aura::Window* w2) {
  gfx::Rect union_bounds;
  union_bounds.Union(w1->GetBoundsInScreen());
  union_bounds.Union(w2->GetBoundsInScreen());
  const auto divider_bounds = split_view_divider()->divider_widget()
                                  ? split_view_divider_bounds_in_screen()
                                  : gfx::Rect();
  union_bounds.Union(divider_bounds);
  return union_bounds == work_area_bounds();
}

}  // namespace

// -----------------------------------------------------------------------------
// FasterSplitScreenTest:

// Test fixture to verify faster split screen feature.

class FasterSplitScreenTest : public OverviewTestBase {
 public:
  FasterSplitScreenTest() {
    scoped_feature_list_.InitWithFeatures(
        /*enabled_features=*/{features::kFasterSplitScreenSetup,
                              features::kOsSettingsRevampWayfinding},
        /*disabled_features=*/{});
  }
  FasterSplitScreenTest(const FasterSplitScreenTest&) = delete;
  FasterSplitScreenTest& operator=(const FasterSplitScreenTest&) = delete;
  ~FasterSplitScreenTest() override = default;

  // AshTestBase:
  void SetUp() override {
    OverviewTestBase::SetUp();
    WindowCycleList::SetDisableInitialDelayForTesting(true);
  }

 protected:
  base::HistogramTester histogram_tester_;

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

// Tests that if the user disables the pref for snap window suggestions, we
// don't start partial overview.
TEST_F(FasterSplitScreenTest, DisableSnapWindowSuggestionsPref) {
  PrefService* pref =
      Shell::Get()->session_controller()->GetActivePrefService();

  pref->SetBoolean(prefs::kSnapWindowSuggestions, false);
  ASSERT_FALSE(pref->GetBoolean(prefs::kSnapWindowSuggestions));

  // Snap a window. Test we don't start overview.
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(Shell::Get()->overview_controller()->InOverviewSession());
}

TEST_F(FasterSplitScreenTest, Basic) {
  // Create two test windows, snap `w1`. Test `w1` is snapped and excluded from
  // overview while `w2` is in overview.
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  auto* overview_controller = Shell::Get()->overview_controller();
  EXPECT_TRUE(
      overview_controller->overview_session()->IsWindowInOverview(w2.get()));

  // Select `w2` from overview. Test `w2` auto snaps.
  ClickOverviewItem(GetEventGenerator(), w2.get());
  WaitForOverviewExitAnimation();
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w2.get())->GetStateType());
  EXPECT_FALSE(overview_controller->InOverviewSession());

  // Create a new `w3` and snap it to the left. Test it doesn't start overview.
  std::unique_ptr<aura::Window> w3(CreateTestWindow());
  SnapOneTestWindow(w3.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(overview_controller->InOverviewSession());

  // Create a new `w4` and snap it to the right. Test it doesn't start overview.
  std::unique_ptr<aura::Window> w4(CreateTestWindow());
  SnapOneTestWindow(w4.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(overview_controller->InOverviewSession());
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w4.get())->GetStateType());

  // Test all the other window states remain the same.
  EXPECT_EQ(WindowStateType::kPrimarySnapped,
            WindowState::Get(w1.get())->GetStateType());
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w2.get())->GetStateType());
  EXPECT_EQ(WindowStateType::kPrimarySnapped,
            WindowState::Get(w3.get())->GetStateType());

  // Enter overview normally. Test that no windows widget will not show.
  ToggleOverview();
  auto* overview_grid = GetOverviewGridForRoot(w1->GetRootWindow());
  EXPECT_FALSE(overview_grid->no_windows_widget());
  EXPECT_FALSE(overview_grid->faster_splitview_widget());
}

// Tests that on one window snapped, `SnapGroupController` starts
// `SplitViewOverviewSession` (snap group creation session).
TEST_F(FasterSplitScreenTest, CloseSnappedWindowEndsSplitViewOverviewSession) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());

  // Snap `w1` to the left. Test that we are in split view overview, excluding
  // `w1` and taking half the screen.
  SnapOneTestWindow(w1.get(),
                    /*state_type=*/WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  // Close `w1`. Test that we end overview.
  w1.reset();
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
}

// Tests that faster split screen can only start with certain snap action
// sources.
TEST_F(FasterSplitScreenTest, SnapActionSourceLimitations) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());

  struct {
    WindowSnapActionSource snap_action_source;
    bool should_show_partial_overview;
  } kTestCases[]{
      {WindowSnapActionSource::kSnapByWindowLayoutMenu,
       /*should_show_partial_overview=*/true},
      {WindowSnapActionSource::kDragWindowToEdgeToSnap,
       /*should_show_partial_overview=*/true},
      {WindowSnapActionSource::kLongPressCaptionButtonToSnap,
       /*should_show_partial_overview=*/true},
      {WindowSnapActionSource::kLacrosSnapButtonOrWindowLayoutMenu,
       /*should_show_partial_overview=*/true},
      {WindowSnapActionSource::kKeyboardShortcutToSnap,
       /*should_show_partial_overview=*/false},
      {WindowSnapActionSource::kSnapByWindowStateRestore,
       /*should_show_partial_overview=*/false},
      {WindowSnapActionSource::kSnapByFullRestoreOrDeskTemplateOrSavedDesk,
       /*should_show_partial_overview=*/false},
  };

  for (const auto test_case : kTestCases) {
    SnapOneTestWindow(w1.get(), WindowStateType::kSecondarySnapped,
                      chromeos::kDefaultSnapRatio,
                      test_case.snap_action_source);
    EXPECT_EQ(test_case.should_show_partial_overview, IsInOverviewSession());
    MaximizeToClearTheSession(w1.get());
  }
}

TEST_F(FasterSplitScreenTest, CycleSnap) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  auto* window_state = WindowState::Get(w1.get());

  // Cycle snap to the left.
  const WindowSnapWMEvent cycle_snap_primary(WM_EVENT_CYCLE_SNAP_PRIMARY);
  window_state->OnWMEvent(&cycle_snap_primary);
  auto* overview_controller = Shell::Get()->overview_controller();
  EXPECT_FALSE(overview_controller->InOverviewSession());

  // Cycle snap to the right.
  const WindowSnapWMEvent cycle_snap_secondary(WM_EVENT_CYCLE_SNAP_SECONDARY);
  window_state->OnWMEvent(&cycle_snap_secondary);
  EXPECT_FALSE(overview_controller->InOverviewSession());
}

TEST_F(FasterSplitScreenTest, EndSplitViewOverviewSession) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  // Drag `w1` out of split view. Test it ends overview.
  const gfx::Rect window_bounds(w1->GetBoundsInScreen());
  const gfx::Point drag_point(window_bounds.CenterPoint().x(),
                              window_bounds.y() + 10);
  auto* event_generator = GetEventGenerator();
  event_generator->set_current_screen_location(drag_point);
  event_generator->DragMouseBy(10, 10);
  EXPECT_FALSE(Shell::Get()->overview_controller()->InOverviewSession());

  // Snap then minimize the window. Test it ends overview.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  WMEvent minimize_event(WM_EVENT_MINIMIZE);
  WindowState::Get(w1.get())->OnWMEvent(&minimize_event);
  EXPECT_FALSE(Shell::Get()->overview_controller()->InOverviewSession());

  // Snap then close the window. Test it ends overview.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  w1.reset();
  EXPECT_FALSE(Shell::Get()->overview_controller()->InOverviewSession());
}

TEST_F(FasterSplitScreenTest, ResizeSplitViewOverviewAndWindow) {
  UpdateDisplay("900x600");
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  const gfx::Rect initial_bounds(w1->GetBoundsInScreen());

  // Drag the right edge of the window to resize the window and overview at the
  // same time. Test that the bounds are updated.
  const gfx::Point start_point(w1->GetBoundsInScreen().right_center());
  auto* generator = GetEventGenerator();
  generator->set_current_screen_location(start_point);

  // Resize to less than 1/3. Test we don't end overview.
  const auto drag_point1 =
      gfx::Point(work_area_bounds().width() * chromeos::kOneThirdSnapRatio - 10,
                 start_point.y());
  generator->DragMouseTo(drag_point1);
  gfx::Rect expected_window_bounds(initial_bounds);
  expected_window_bounds.set_width(drag_point1.x());
  EXPECT_EQ(expected_window_bounds, w1->GetBoundsInScreen());
  VerifySplitViewOverviewSession(w1.get());

  // Resize to greater than 2/3. Test we don't end overview.
  const auto drag_point2 =
      gfx::Point(work_area_bounds().width() * chromeos::kTwoThirdSnapRatio + 10,
                 start_point.y());
  generator->DragMouseTo(drag_point2);
  expected_window_bounds.set_width(drag_point2.x());
  EXPECT_EQ(expected_window_bounds, w1->GetBoundsInScreen());
  VerifySplitViewOverviewSession(w1.get());
}

// Tests that drag to snap window -> resize window -> snap window again restores
// to the default snap ratio. Regression test for b/315039407.
TEST_F(FasterSplitScreenTest, ResizeThenDragToSnap) {
  auto get_drag_point = [](aura::Window* window) -> gfx::Point {
    const gfx::Rect window_bounds = window->GetBoundsInScreen();
    return {window_bounds.CenterPoint().x(), window_bounds.y() + 10};
  };

  // Create `w2` first, as `w1` will be created on top and we want to drag it.
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  std::unique_ptr<aura::Window> w1(CreateAppWindow());

  // Drag to snap `w1` to 1/2.
  auto* event_generator = GetEventGenerator();
  event_generator->set_current_screen_location(get_drag_point(w1.get()));
  event_generator->DragMouseTo(0, 100);
  WindowState* window_state = WindowState::Get(w1.get());
  EXPECT_EQ(WindowStateType::kPrimarySnapped, window_state->GetStateType());
  const gfx::Rect work_area(work_area_bounds());
  const gfx::Rect snapped_bounds(0, 0, work_area.width() / 2,
                                 work_area.height());
  EXPECT_EQ(snapped_bounds, w1->GetBoundsInScreen());

  // Resize `w1` to an arbitrary size not 1/2.
  event_generator->set_current_screen_location(snapped_bounds.right_center());
  event_generator->DragMouseBy(100, 10);
  EXPECT_EQ(WindowStateType::kPrimarySnapped, window_state->GetStateType());
  EXPECT_NE(snapped_bounds, w1->GetBoundsInScreen());

  // Drag `w1` to unsnap and skip overview pairing.
  event_generator->set_current_screen_location(get_drag_point(w1.get()));
  event_generator->DragMouseBy(10, 10);
  EXPECT_FALSE(IsInOverviewSession());
  EXPECT_EQ(WindowStateType::kNormal, window_state->GetStateType());
  EXPECT_NE(snapped_bounds, w1->GetBoundsInScreen());

  // Drag to snap `w1` again. Test it snaps to 1/2.
  event_generator->set_current_screen_location(get_drag_point(w1.get()));
  event_generator->DragMouseTo(0, 100);
  EXPECT_EQ(snapped_bounds, w1->GetBoundsInScreen());

  // Resize `w1` to an arbitrary size not 1/2 again.
  event_generator->set_current_screen_location(snapped_bounds.right_center());
  event_generator->DragMouseBy(-100, 10);
  EXPECT_EQ(WindowStateType::kPrimarySnapped, window_state->GetStateType());
  EXPECT_NE(snapped_bounds, w1->GetBoundsInScreen());

  // Drag to snap `w2`. Test it snaps to 1/2.
  event_generator->set_current_screen_location(get_drag_point(w2.get()));
  event_generator->DragMouseTo(0, 100);
  EXPECT_EQ(WindowStateType::kPrimarySnapped,
            WindowState::Get(w2.get())->GetStateType());
  EXPECT_EQ(snapped_bounds, w2->GetBoundsInScreen());
}

TEST_F(FasterSplitScreenTest, ResizeAndAutoSnap) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  const gfx::Rect initial_bounds(w1->GetBoundsInScreen());
  ASSERT_TRUE(OverviewController::Get()->InOverviewSession());

  auto* generator = GetEventGenerator();
  generator->set_current_screen_location(
      w1->GetBoundsInScreen().right_center());
  const int drag_x = 100;
  generator->DragMouseBy(drag_x, 0);
  ASSERT_TRUE(OverviewController::Get()->InOverviewSession());

  gfx::Rect expected_window_bounds(initial_bounds);
  expected_window_bounds.set_width(initial_bounds.width() + drag_x);
  EXPECT_EQ(expected_window_bounds, w1->GetBoundsInScreen());

  gfx::Rect expected_grid_bounds(work_area_bounds());
  expected_grid_bounds.Subtract(w1->GetBoundsInScreen());
  EXPECT_EQ(expected_grid_bounds, GetOverviewGridBounds());

  // Create a window and test that it auto snaps.
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w3.get())->GetStateType());
  EXPECT_EQ(expected_grid_bounds, w3->GetBoundsInScreen());
}

// Verify the window focus behavior both when activing a window or skipping
// pairing in partial overview.
// 1. When activating a window in partial overview, the chosen window will be
// the activated one upon exit;
// 2. When skipping pairing in partial overview, the snapped window will still
// be the activated one if it was activated before entering
// `SplitViewOverviewSession`.
TEST_F(FasterSplitScreenTest, SnappedWindowFocusTest) {
  UpdateDisplay("800x600");
  std::unique_ptr<aura::Window> w2(CreateAppWindow(gfx::Rect(200, 100)));
  std::unique_ptr<aura::Window> w1(CreateAppWindow(gfx::Rect(100, 100)));
  ASSERT_TRUE(wm::IsActiveWindow(w1.get()));

  auto* event_generator = GetEventGenerator();
  for (const bool skip_pairing : {true, false}) {
    SnapOneTestWindow(w1.get(), chromeos::WindowStateType::kSecondarySnapped,
                      chromeos::kDefaultSnapRatio);
    VerifySplitViewOverviewSession(w1.get());

    auto* w2_overview_item = GetOverviewItemForWindow(w2.get());
    EXPECT_TRUE(w2_overview_item);

    const auto w2_overview_item_bounds = w2_overview_item->target_bounds();
    const gfx::Point click_point =
        skip_pairing
            ? gfx::ToRoundedPoint(w2_overview_item_bounds.bottom_right()) +
                  gfx::Vector2d(20, 20)
            : gfx::ToRoundedPoint(w2_overview_item_bounds.CenterPoint());

    event_generator->MoveMouseTo(click_point);
    event_generator->ClickLeftButton();

    EXPECT_EQ(wm::IsActiveWindow(w1.get()), skip_pairing);
    EXPECT_FALSE(IsInOverviewSession());
    MaximizeToClearTheSession(w1.get());
  }
}

TEST_F(FasterSplitScreenTest, DragToPartialOverview) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  ToggleOverview();
  OverviewSession* overview_session =
      OverviewController::Get()->overview_session();
  ASSERT_TRUE(overview_session);
  EXPECT_TRUE(overview_session->IsWindowInOverview(w1.get()));
  EXPECT_TRUE(overview_session->IsWindowInOverview(w2.get()));

  // Drag `w1` to enter partial overview.
  auto* event_generator = GetEventGenerator();
  DragGroupItemToPoint(GetOverviewItemForWindow(w1.get()), gfx::Point(0, 0),
                       event_generator,
                       /*by_touch_gestures=*/false, /*drop=*/true);
  EXPECT_EQ(WindowStateType::kPrimarySnapped,
            WindowState::Get(w1.get())->GetStateType());
  VerifySplitViewOverviewSession(w1.get(), /*faster_split_screen_setup=*/false);
  EXPECT_TRUE(overview_session->IsWindowInOverview(w2.get()));

  // Select `w2`. Test it snaps and we end overview.
  ClickOverviewItem(event_generator, w2.get());
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w2.get())->GetStateType());
  EXPECT_EQ(WindowStateType::kPrimarySnapped,
            WindowState::Get(w1.get())->GetStateType());
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
}

// Tests that when clicking or tapping on the empty area during faster split
// screen setup session, overview will end.
TEST_F(FasterSplitScreenTest, SkipPairingInOverviewWhenActivatingTheEmptyArea) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());

  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  ASSERT_EQ(1u, GetOverviewSession()->grid_list().size());

  auto* w2_overview_item = GetOverviewItemForWindow(w2.get());
  EXPECT_TRUE(w2_overview_item);
  const gfx::Point outside_point =
      gfx::ToRoundedPoint(
          w2_overview_item->GetTransformedBounds().bottom_right()) +
      gfx::Vector2d(20, 20);

  // Verify that clicking on an empty area in overview will exit the paring.
  auto* event_generator = GetEventGenerator();
  event_generator->MoveMouseTo(outside_point);
  event_generator->ClickLeftButton();
  EXPECT_FALSE(IsInOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);

  // Verify that tapping on an empty area in overview will exit the paring.
  MaximizeToClearTheSession(w1.get());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  event_generator->MoveTouch(outside_point);
  event_generator->PressTouch();
  event_generator->ReleaseTouch();
  EXPECT_FALSE(IsInOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
}

// Tests that when clicking or tapping on the snapped window on the `HTCLIENT`
// or `HTCAPTION` area during faster split screen setup session, overview will
// end.
TEST_F(FasterSplitScreenTest, SkipPairingWhenActivatingTheSnappedWindow) {
  UpdateDisplay("800x600");
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  aura::test::TestWindowDelegate delegate;

  auto* event_generator = GetEventGenerator();

  // Snap `w1`. Test that moving the mouse around won't end overview
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  event_generator->MoveMouseTo(w1->GetBoundsInScreen().CenterPoint());
  EXPECT_TRUE(IsInOverviewSession());
  MaximizeToClearTheSession(w1.get());

  // Build test cases to verify that overview will end when clicking or tapping
  // on the window caption or client area.
  struct {
    int window_component;
    bool is_click_event;
  } kTestCases[]{
      {/*window_component*/ HTCLIENT, /*is_click_event=*/true},
      {/*window_component*/ HTCAPTION, /*is_click_event=*/true},
      {/*window_component*/ HTCLIENT, /*is_click_event=*/false},
      {/*window_component*/ HTCAPTION, /*is_click_event=*/false},
  };

  for (const auto& test_case : kTestCases) {
    SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                      chromeos::kDefaultSnapRatio);
    VerifySplitViewOverviewSession(w1.get());
    delegate.set_window_component(test_case.window_component);
    if (test_case.is_click_event) {
      event_generator->ClickLeftButton();
    } else {
      event_generator->PressTouch();
      event_generator->ReleaseTouch();
    }
    EXPECT_FALSE(IsInOverviewSession());
    MaximizeToClearTheSession(w1.get());
  }
}

TEST_F(FasterSplitScreenTest, SkipPairingOnKeyEvent) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());

  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  ASSERT_EQ(1u, GetOverviewSession()->grid_list().size());

  // Test that Esc key exits overview.
  PressAndReleaseKey(ui::VKEY_ESCAPE, ui::EF_NONE);
  OverviewController* overview_controller = OverviewController::Get();
  EXPECT_FALSE(overview_controller->InOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);

  // Test that Alt + Tab exits overview.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());
  PressAndReleaseKey(ui::VKEY_TAB, ui::EF_ALT_DOWN);
  EXPECT_FALSE(overview_controller->InOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
  EXPECT_TRUE(Shell::Get()->window_cycle_controller()->IsCycling());
}

TEST_F(FasterSplitScreenTest, SkipPairingToast) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  auto* overview_grid = GetOverviewGridForRoot(w1->GetRootWindow());
  ASSERT_TRUE(overview_grid);
  auto* faster_split_view = overview_grid->GetFasterSplitView();
  ASSERT_TRUE(faster_split_view);
  LeftClickOn(faster_split_view->GetDismissButton());

  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
}

TEST_F(FasterSplitScreenTest, DontStartPartialOverviewAfterSkippingPairing) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  // Press Esc key to skip pairing.
  PressAndReleaseKey(ui::VKEY_ESCAPE, ui::EF_NONE);
  OverviewController* overview_controller = OverviewController::Get();
  EXPECT_FALSE(overview_controller->InOverviewSession());

  // Snap `w2`. Since `w1` is snapped to primary, it doesn't start partial
  // overview. wm::ActivateWindow(w2.get());
  SnapOneTestWindow(w2.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(overview_controller->InOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
  EXPECT_EQ(WindowState::Get(w2.get())->GetStateType(),
            WindowStateType::kSecondarySnapped);
}

TEST_F(FasterSplitScreenTest, DontStartPartialOverviewAfterClosingWindow) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  // Select `w2` to auto-snap it.
  ClickOverviewItem(GetEventGenerator(), w2.get());

  // Close `w2`, then open and snap a new `w3`. Test we don't start partial
  // overview.
  w2.reset();
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  SnapOneTestWindow(w3.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
}

TEST_F(FasterSplitScreenTest, StartPartialOverviewForMinimizedWindow) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  // Now minimize `w1`, so visually there is no primary snapped window.
  WindowState::Get(w1.get())->Minimize();

  // Now snap `w2` to secondary. Since `w1` is minimized, it starts partial
  // overview.
  SnapOneTestWindow(w2.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w2.get());
}

// Tests that when activating an already snapped window, cannot snap toast will
// not show by mistake. See b/323391799 for details.
TEST_F(FasterSplitScreenTest,
       DoNotShowCannotSnapToastWhenActivatingTheSnappedWindow) {
  UpdateDisplay("800x600");
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kKeyboardShortcutToSnap);
  ASSERT_TRUE(WindowState::Get(w1.get())->IsSnapped());

  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w2.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kDragWindowToEdgeToSnap);
  EXPECT_FALSE(IsInOverviewSession());

  wm::ActivateWindow(w1.get());
  EXPECT_FALSE(ToastManager::Get()->IsToastShown(kAppCannotSnapToastId));
}

TEST_F(FasterSplitScreenTest, DontStartPartialOverviewForFloatedWindow) {
  // Snap 2 test windows in place.
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  // To simulate the CUJ when a user selects a window from overview, activate
  // and snap `w2`.
  wm::ActivateWindow(w2.get());
  SnapOneTestWindow(w2.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(Shell::Get()->overview_controller()->InOverviewSession());

  // Create a 3rd floated window on top of `w2`.
  std::unique_ptr<aura::Window> floated_window = CreateAppWindow();
  PressAndReleaseKey(ui::VKEY_F, ui::EF_ALT_DOWN | ui::EF_COMMAND_DOWN);
  EXPECT_TRUE(WindowState::Get(floated_window.get())->IsFloated());
  EXPECT_TRUE(
      w2->GetBoundsInScreen().Contains(floated_window->GetBoundsInScreen()));

  // Open a 4th window and snap it on top of `w1`. Test we don't start partial
  // overview.
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  SnapOneTestWindow(w3.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(Shell::Get()->overview_controller()->InOverviewSession());
}

// Tests that partial overview will not be triggered if the window to be snapped
// is only one window for the active desk and on the current display.
TEST_F(FasterSplitScreenTest, DontStartPartiOverviewIfThereIsOnlyOneWindow) {
  UpdateDisplay("900x600, 901+0-900x600");
  ASSERT_EQ(Shell::GetAllRootWindows().size(), 2u);

  DesksController* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kButton);
  ASSERT_EQ(2u, desks_controller->desks().size());
  Desk* desk0 = desks_controller->GetDeskAtIndex(0);
  Desk* desk1 = desks_controller->GetDeskAtIndex(1);

  std::unique_ptr<aura::Window> w1(
      CreateAppWindow(gfx::Rect(10, 20, 200, 100)));

  // Create the 2nd window and move it to another desk.
  std::unique_ptr<aura::Window> w2(
      CreateAppWindow(gfx::Rect(100, 20, 200, 100)));
  ASSERT_EQ(desks_util::GetDeskForContext(w1.get()), desk0);
  ASSERT_EQ(desks_util::GetDeskForContext(w2.get()), desk0);
  desks_controller->MoveWindowFromActiveDeskTo(
      w2.get(), desk1, w2->GetRootWindow(),
      DesksMoveWindowFromActiveDeskSource::kShortcut);
  ASSERT_EQ(desks_util::GetDeskForContext(w2.get()), desk1);

  // Create the 3rd window on the 2nd display.
  std::unique_ptr<aura::Window> w3(
      CreateAppWindow(gfx::Rect(1000, 20, 200, 100)));

  // Verify that snapping `w1` won't trigger partial overview.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_FALSE(IsInOverviewSession());
}

// Tests that only when there is a non-occluded window snapped on the opposite
// side should we skip showing partial overview on window snapped. This test
// focuses on the window layout setup **with** intersections.
TEST_F(FasterSplitScreenTest,
       OppositeSnappedWindowOcclusionWithIntersectionsTest) {
  UpdateDisplay("800x600");
  // Window Layout before snapping `w1` to the primary snapped position:
  // `w2` is snapped on the secondary snapped position;
  // `w3` is stacked above `w2` with intersections.
  //
  //                  +-----------+
  //          +-------|-+         |
  //          |       | |         |
  //          |   w3  | |   w2    |
  //          |       | |         |
  //          +-------|-+         |
  //                  +-----------+
  //
  // For the window layout setup above, we should show partial overview
  // when snapping `w1` by the desired snap action source.

  // Snap `w2` to the secondary snapped location without triggering faster split
  // screen to get window layout setup ready.
  std::unique_ptr<aura::Window> w2 = CreateAppWindow();
  SnapOneTestWindow(w2.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kKeyboardShortcutToSnap);
  ASSERT_TRUE(w2->IsVisible());

  // Create `w3` with bounds that intersect with `w2`.
  std::unique_ptr<aura::Window> w3 =
      CreateAppWindow(gfx::Rect(350, 200, 150, 200));
  ASSERT_TRUE(w3->IsVisible());
  EXPECT_TRUE(w3->GetBoundsInScreen().Intersects(w2->GetBoundsInScreen()));

  // Create and snap `w1` to the primary snapped position and expect to trigger
  // the faster split screen setup.
  std::unique_ptr<aura::Window> w1 = CreateAppWindow();
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  ASSERT_TRUE(w1->IsVisible());
  VerifySplitViewOverviewSession(w1.get());

  // Activate `w2` to bring it to the front and snap it to the primary
  // snapped location without triggering faster split screen in preparation for
  // the next round of testing. `w2` is fully visible now.
  wm::ActivateWindow(w2.get());
  SnapOneTestWindow(w2.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kKeyboardShortcutToSnap);
  EXPECT_FALSE(IsInOverviewSession());

  // Snap `w1` to secondary snapped position with desired snap action source to
  // trigger faster split screen setup, with `w1` occupying the primary snapped
  // position, partial overview shouldn't start.
  SnapOneTestWindow(w1.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  EXPECT_FALSE(IsInOverviewSession());
}

// Tests that only when there is a non-occluded window snapped on the opposite
// side should we skip showing partial overview on window snapped. This test
// focuses on the window layout setup **without** intersections.
TEST_F(FasterSplitScreenTest,
       OppositeSnappedWindowOcclusionWithoutIntersectionsTest) {
  UpdateDisplay("800x600");
  // Window Layout before snapping `w1` to the primary snapped position:
  // `w2` is snapped on the secondary snapped position;
  // `w3` is stacked above `w2` without intersections.
  //
  //              +-----------+
  //              |    +---+  |
  //              |    | w3|  |
  //              |    +---+  |
  //              |    w2     |
  //              |           |
  //              +-----------+
  //
  // For the window layout setup above, we should show partial overview
  // when snapping `w1` by the desired snap action source.

  // Snap `w2` to the secondary snapped location without triggering faster split
  // screen to get window layout setup ready.
  std::unique_ptr<aura::Window> w2 = CreateAppWindow();
  SnapOneTestWindow(w2.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kKeyboardShortcutToSnap);
  ASSERT_TRUE(w2->IsVisible());

  // Create `w3` with bounds confined by the bounds `w2`.
  std::unique_ptr<aura::Window> w3 =
      CreateAppWindow(gfx::Rect(550, 45, 50, 50));
  ASSERT_TRUE(w3->IsVisible());
  EXPECT_TRUE(w2->GetBoundsInScreen().Contains(w3->GetBoundsInScreen()));

  // Create and snap `w1` to the primary snapped position and expect to trigger
  // the faster split screen setup.
  std::unique_ptr<aura::Window> w1 = CreateAppWindow();
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  ASSERT_TRUE(w1->IsVisible());
  VerifySplitViewOverviewSession(w1.get());

  // Activate `w2` to bring it to the front and snap it to the primary
  // snapped location without triggering faster split screen in preparation for
  // the next round of testing. `w2` is fully visible now.
  wm::ActivateWindow(w2.get());
  SnapOneTestWindow(w2.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kKeyboardShortcutToSnap);
  EXPECT_FALSE(IsInOverviewSession());

  // Snap `w1` to secondary snapped position with desired snap action source to
  // trigger faster split screen setup, with `w1` occupying the primary snapped
  // position, partial overview shouldn't start.
  SnapOneTestWindow(w1.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  EXPECT_FALSE(IsInOverviewSession());
}

TEST_F(FasterSplitScreenTest, NoCrashOnDisplayChange) {
  UpdateDisplay("800x600,1000x600");
  display::test::DisplayManagerTestApi display_manager_test(display_manager());

  // Snap `window` on the second display. Test its bounds are updated.
  std::unique_ptr<aura::Window> window1(
      CreateTestWindowInShellWithBounds(gfx::Rect(900, 0, 100, 100)));
  std::unique_ptr<aura::Window> window2(
      CreateTestWindowInShellWithBounds(gfx::Rect(1000, 0, 100, 100)));
  SnapOneTestWindow(window1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  ASSERT_EQ(display_manager_test.GetSecondaryDisplay().id(),
            display::Screen::GetScreen()
                ->GetDisplayNearestWindow(window1.get())
                .id());
  const gfx::Rect work_area(
      display_manager_test.GetSecondaryDisplay().work_area());
  EXPECT_EQ(gfx::Rect(800, 0, work_area.width() / 2, work_area.height()),
            window1->GetBoundsInScreen());
  VerifySplitViewOverviewSession(window1.get());

  // Disconnect the second display. Test no crash.
  UpdateDisplay("800x600");
  base::RunLoop().RunUntilIdle();
}

// Tests that autosnapping a window with minimum size doesn't crash. Regression
// test for http://b/324483718.
TEST_F(FasterSplitScreenTest, SnapWindowWithMinimumSize) {
  UpdateDisplay("800x600");
  std::unique_ptr<aura::Window> w1(CreateAppWindow());

  // 1 - Test min size > 1/3 scenario.
  // Set `w2` min size to be > 1/3 of the display width.
  aura::test::TestWindowDelegate delegate;
  std::unique_ptr<aura::Window> w2(CreateTestWindowInShellWithDelegate(
      &delegate, /*id=*/-1, gfx::Rect(800, 600)));
  int min_width = 396;
  delegate.set_minimum_size(gfx::Size(min_width, 0));

  // Snap `w1` to primary 2/3.
  WindowState* window_state = WindowState::Get(w1.get());
  const WindowSnapWMEvent snap_type(
      WM_EVENT_SNAP_PRIMARY, chromeos::kTwoThirdSnapRatio,
      /*snap_action_source=*/WindowSnapActionSource::kTest);
  window_state->OnWMEvent(&snap_type);
  ASSERT_TRUE(OverviewController::Get()->InOverviewSession());

  // Select `w2` from overview.
  auto* event_generator = GetEventGenerator();
  event_generator->MoveMouseTo(
      gfx::ToRoundedPoint(GetOverviewItemForWindow(w2.get())
                              ->GetTransformedBounds()
                              .CenterPoint()));
  event_generator->ClickLeftButton();

  // Test it gets snapped at its minimum size.
  EXPECT_EQ(min_width, w2->GetBoundsInScreen().width());

  MaximizeToClearTheSession(w2.get());

  // 2 - Test min size > 1/2 scenario.
  // Set `w2` min size to be > 1/2 of the display width.
  min_width = 450;
  delegate.set_minimum_size(gfx::Size(min_width, 0));

  // Snap `w1` to primary 1/2.
  const WindowSnapWMEvent snap_default(
      WM_EVENT_SNAP_PRIMARY, chromeos::kDefaultSnapRatio,
      /*snap_action_source=*/WindowSnapActionSource::kTest);
  window_state->OnWMEvent(&snap_default);
  ASSERT_TRUE(OverviewController::Get()->InOverviewSession());

  // Select `w2` from overview.
  event_generator->MoveMouseTo(
      gfx::ToRoundedPoint(GetOverviewItemForWindow(w2.get())
                              ->GetTransformedBounds()
                              .CenterPoint()));
  event_generator->ClickLeftButton();

  // Test it gets snapped at its minimum size.
  EXPECT_EQ(min_width, w2->GetBoundsInScreen().width());
}

// Tests we start partial overview if there's an opposite snapped window on
// another display.
TEST_F(FasterSplitScreenTest, OppositeSnappedWindowOnOtherDisplay) {
  UpdateDisplay("800x600,801+0-800x600");

  // Create 3 test windows, with `w3` on display 2.
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  std::unique_ptr<aura::Window> w3(
      CreateAppWindow(gfx::Rect(900, 0, 100, 100)));
  std::unique_ptr<aura::Window> w4(
      CreateAppWindow(gfx::Rect(1000, 0, 100, 100)));

  // Snap `w1` to primary on display 1.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  display::Screen* screen = display::Screen::GetScreen();
  auto display_list = screen->GetAllDisplays();
  ASSERT_EQ(display_list[0], screen->GetDisplayNearestWindow(w1.get()));

  // Test we start partial overview.
  EXPECT_TRUE(IsInOverviewSession());
  EXPECT_TRUE(
      RootWindowController::ForWindow(w1.get())->split_view_overview_session());

  // Select `w2` to snap on the first display.
  ClickOverviewItem(GetEventGenerator(), w2.get());
  EXPECT_EQ(display_list[0], screen->GetDisplayNearestWindow(w2.get()));

  // Snap `w3` to secondary on display 2.
  SnapOneTestWindow(w3.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  ASSERT_EQ(display_list[1], screen->GetDisplayNearestWindow(w3.get()));

  // Test we start partial overview since no window is snapped on display 2.
  EXPECT_TRUE(IsInOverviewSession());
  EXPECT_TRUE(
      RootWindowController::ForWindow(w3.get())->split_view_overview_session());
}

// Tests that the snapped window bounds will be refreshed on display changes to
// preserve the snap ratio.
TEST_F(FasterSplitScreenTest, WindowBoundsRefreshedOnDisplayChanges) {
  UpdateDisplay("900x600");
  std::unique_ptr<aura::Window> window1(CreateAppWindow());
  std::unique_ptr<aura::Window> window2(CreateAppWindow());
  SnapOneTestWindow(window1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kTwoThirdSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  VerifySplitViewOverviewSession(window1.get());
  ASSERT_EQ(WindowState::Get(window1.get())->snap_ratio(),
            chromeos::kTwoThirdSnapRatio);
  const auto work_area_bounds_1 = work_area_bounds();
  ASSERT_EQ(
      window1->GetBoundsInScreen(),
      gfx::Rect(0, 0, work_area_bounds_1.width() * chromeos::kTwoThirdSnapRatio,
                work_area_bounds_1.height()));

  UpdateDisplay("1200x600");
  VerifySplitViewOverviewSession(window1.get());
  EXPECT_EQ(WindowState::Get(window1.get())->snap_ratio(),
            chromeos::kTwoThirdSnapRatio);
  const auto work_area_bounds_2 = work_area_bounds();
  EXPECT_EQ(
      window1->GetBoundsInScreen(),
      gfx::Rect(0, 0, work_area_bounds_2.width() * chromeos::kTwoThirdSnapRatio,
                work_area_bounds_2.height()));
}

// Tests that the grid and faster splitview widget is updated on keyboard
// and work area bounds changes.
TEST_F(FasterSplitScreenTest, KeyboardAndWorkAreaBoundsChanges) {
  std::unique_ptr<aura::Window> window1(CreateAppWindow());
  std::unique_ptr<aura::Window> window2(CreateAppWindow());
  SnapOneTestWindow(window1.get(), chromeos::WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  VerifySplitViewOverviewSession(window1.get());

  // Show the virtual keyboard. Test we refresh the grid and widget bounds.
  SetVirtualKeyboardEnabled(true);
  auto* keyboard_controller = keyboard::KeyboardUIController::Get();
  keyboard_controller->ShowKeyboard(true);
  VerifySplitViewOverviewSession(window1.get());
  EXPECT_EQ(chromeos::WindowStateType::kPrimarySnapped,
            WindowState::Get(window1.get())->GetStateType());
  auto* overview_grid = GetOverviewGridForRoot(window1->GetRootWindow());
  EXPECT_TRUE(GetOverviewGridBounds().Contains(
      overview_grid->GetFasterSplitView()->GetBoundsInScreen()));

  // Hide the virtual keyboard. Test we refresh the grid and widget bounds.
  keyboard_controller->HideKeyboardByUser();
  VerifySplitViewOverviewSession(window1.get());
  EXPECT_EQ(chromeos::WindowStateType::kPrimarySnapped,
            WindowState::Get(window1.get())->GetStateType());
  EXPECT_TRUE(GetOverviewGridBounds().Contains(
      overview_grid->GetFasterSplitView()->GetBoundsInScreen()));

  // Show the docked magnifier, which ends overview.
  auto* docked_magnifier_controller =
      Shell::Get()->docked_magnifier_controller();
  docked_magnifier_controller->SetEnabled(/*enabled=*/true);
  EXPECT_FALSE(IsInOverviewSession());
  // TODO(sophiewen): Consider testing no faster splitview widget.
}

// Test to verify that there will be no crash when dragging the snapped window
// out without resizing the window see crash in b/321111182.
TEST_F(FasterSplitScreenTest, NoCrashWhenDraggingTheSnappedWindow) {
  std::unique_ptr<aura::Window> window1(CreateAppWindow());
  std::unique_ptr<aura::Window> window2(CreateAppWindow());
  SnapOneTestWindow(window1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kTwoThirdSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  VerifySplitViewOverviewSession(window1.get());

  std::unique_ptr<WindowResizer> resizer(CreateWindowResizer(
      window1.get(), gfx::PointF(), HTCAPTION, wm::WINDOW_MOVE_SOURCE_MOUSE));
  resizer->Drag(gfx::PointF(500, 100), /*event_flags=*/0);
  WindowState* window_state = WindowState::Get(window1.get());
  EXPECT_TRUE(window_state->is_dragged());
  resizer->CompleteDrag();
  EXPECT_FALSE(window_state->IsSnapped());
}

// Tests that after a minimized window gets auto-snapped, dragging the window
// won't lead to crash. See crash at http://b/324483508.
TEST_F(FasterSplitScreenTest,
       NoCrashWhenDraggingTheAutoSnappedWindowThatWasPreviouslyMinimized) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(
      CreateAppWindow(gfx::Rect(100, 100, 100, 100)));
  WindowState* w2_window_state = WindowState::Get(w2.get());
  w2_window_state->Minimize();
  ASSERT_TRUE(w2_window_state->IsMinimized());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  VerifySplitViewOverviewSession(w1.get());

  auto* w2_overview_item = GetOverviewItemForWindow(w2.get());
  auto* event_generator = GetEventGenerator();
  event_generator->set_current_screen_location(
      gfx::ToRoundedPoint(w2_overview_item->target_bounds().CenterPoint()));
  event_generator->ClickLeftButton();
  EXPECT_EQ(w2_window_state->GetStateType(),
            WindowStateType::kSecondarySnapped);

  std::unique_ptr<WindowResizer> resizer(CreateWindowResizer(
      w2.get(), gfx::PointF(), HTCAPTION, wm::WINDOW_MOVE_SOURCE_MOUSE));
  resizer->Drag(gfx::PointF(500, 100), /*event_flags=*/0);
  EXPECT_TRUE(w2_window_state->is_dragged());
  resizer->CompleteDrag();
  EXPECT_FALSE(w2_window_state->IsSnapped());
}

// Verifies the issue to snap a window in overview is working properly. see
// b/322893408.
TEST_F(FasterSplitScreenTest, EnterOverviewSnappingWindow) {
  std::unique_ptr<aura::Window> window1(
      CreateAppWindow(gfx::Rect(20, 20, 200, 100)));
  std::unique_ptr<aura::Window> windo2(
      CreateAppWindow(gfx::Rect(10, 10, 200, 100)));

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kOverviewButton);
  ASSERT_TRUE(IsInOverviewSession());

  auto* overview_item = GetOverviewItemForWindow(window1.get());
  auto* event_generator = GetEventGenerator();
  event_generator->MoveMouseTo(
      gfx::ToRoundedPoint(overview_item->target_bounds().CenterPoint()));
  event_generator->PressLeftButton();
  event_generator->DragMouseTo(0, 0);
  event_generator->ReleaseLeftButton();
  EXPECT_TRUE(IsInOverviewSession());
}

// Verifies that there will be no crash when transitioning the
// `SplitViewOverviewSession` between clamshell and tablet mode.
TEST_F(FasterSplitScreenTest, ClamshellTabletTransitionOneSnappedWindow) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(w1.get());

  SwitchToTabletMode();
  EXPECT_TRUE(split_view_divider()->divider_widget());
  auto observed_windows = split_view_divider()->observed_windows();
  EXPECT_EQ(1u, observed_windows.size());
  EXPECT_EQ(w1.get(), observed_windows.front());

  TabletModeControllerTestApi().LeaveTabletMode();
}

TEST_F(FasterSplitScreenTest, ClamshellTabletTransitionTwoSnappedWindows) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  // Select the second window from overview to snap it.
  ClickOverviewItem(GetEventGenerator(), w2.get());
  EXPECT_FALSE(split_view_divider()->divider_widget());

  SwitchToTabletMode();
  EXPECT_TRUE(split_view_divider()->divider_widget());
  auto observed_windows = split_view_divider()->observed_windows();
  EXPECT_EQ(2u, observed_windows.size());
  // TODO(b/312229933): Determine whether the order of `observed_windows_`
  // matters.
  EXPECT_TRUE(UnionBoundsEqualToWorkAreaBounds(w1.get(), w2.get()));

  TabletModeControllerTestApi().LeaveTabletMode();
}

// Tests that there will be no overlap between two windows on window layout
// setup complete. It used to happen because the minimum size of the window was
// never taken into account. See http://b/324631432 for more details.
TEST_F(FasterSplitScreenTest,
       NoOverlapAfterSnapRatioVariesToAccommodateForMinimumSize) {
  UpdateDisplay("900x600");

  std::unique_ptr<aura::Window> window1(CreateAppWindow());

  // Create `window2` with window minimum size above 1/3 of the work area.
  aura::test::TestWindowDelegate delegate2;
  std::unique_ptr<aura::Window> window2(CreateTestWindowInShellWithDelegate(
      &delegate2, /*id=*/-1, gfx::Rect(600, 300)));
  delegate2.set_minimum_size(gfx::Size(400, 200));

  SnapOneTestWindow(window2.get(), WindowStateType::kSecondarySnapped,
                    chromeos::kOneThirdSnapRatio);
  VerifySplitViewOverviewSession(window2.get());

  auto* item1 = GetOverviewItemForWindow(window1.get());
  auto* event_generator = GetEventGenerator();
  event_generator->MoveMouseTo(
      gfx::ToRoundedPoint(item1->target_bounds().CenterPoint()));
  event_generator->ClickLeftButton();
  WaitForOverviewExitAnimation();
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());

  // Verify that the snap ratio of `window2` will be adjusted to accommodate for
  // the window minimum size.
  WindowState* window2_state = WindowState::Get(window2.get());
  ASSERT_TRUE(window2_state->snap_ratio());
  EXPECT_EQ(window2_state->GetStateType(), WindowStateType::kSecondarySnapped);
  EXPECT_GT(window2_state->snap_ratio().value(), chromeos::kOneThirdSnapRatio);

  // Verify that the auto snap ratio of `window1` will be adjusted as well.
  WindowState* window1_state = WindowState::Get(window1.get());
  ASSERT_TRUE(window1_state->snap_ratio());
  EXPECT_EQ(window1_state->GetStateType(), WindowStateType::kPrimarySnapped);
  EXPECT_LT(window1_state->snap_ratio().value(), chromeos::kTwoThirdSnapRatio);

  // Both windows will fit within the work are with no overlap
  EXPECT_EQ(window1->GetBoundsInScreen().width() +
                window2->GetBoundsInScreen().width(),
            work_area_bounds().width());
}

// Tests that double tap to swap windows doesn't crash after transition to
// tablet mode (b/308216746).
TEST_F(FasterSplitScreenTest, NoCrashWhenDoubleTapAfterTransition) {
  // Use non-zero to start an animation, which will notify
  // `SplitViewOverviewSession::OnWindowBoundsChanged()`.
  ui::ScopedAnimationDurationScaleMode test_duration_mode(
      ui::ScopedAnimationDurationScaleMode::NON_ZERO_DURATION);
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  SwitchToTabletMode();
  EXPECT_TRUE(split_view_divider()->divider_widget());

  // Double tap on the divider. This will start a drag and notify
  // SplitViewOverviewSession.
  const gfx::Point divider_center =
      split_view_divider()
          ->GetDividerBoundsInScreen(/*is_dragging=*/false)
          .CenterPoint();
  GetEventGenerator()->GestureTapAt(divider_center);
  GetEventGenerator()->GestureTapAt(divider_center);
}

TEST_F(FasterSplitScreenTest, BasicTabKeyNavigation) {
  std::unique_ptr<aura::Window> window2(CreateTestWindow());
  std::unique_ptr<aura::Window> window1(CreateTestWindow());

  const WindowSnapWMEvent snap_event(WM_EVENT_SNAP_PRIMARY,
                                     WindowSnapActionSource::kTest);
  WindowState::Get(window1.get())->OnWMEvent(&snap_event);
  ASSERT_TRUE(IsInOverviewSession());

  // Tab until we get to the first overview item.
  SendKeyUntilOverviewItemIsFocused(ui::VKEY_TAB);
  const std::vector<std::unique_ptr<OverviewItemBase>>& overview_windows =
      GetOverviewItemsForRoot(0);
  EXPECT_EQ(overview_windows[0]->GetWindow(), GetOverviewFocusedWindow());

  OverviewFocusCycler* focus_cycler = GetOverviewSession()->focus_cycler();
  OverviewGrid* grid = GetOverviewSession()->grid_list()[0].get();

  // Tab to the toast dismiss button.
  PressAndReleaseKey(ui::VKEY_TAB);
  ASSERT_TRUE(IsInOverviewSession());
  EXPECT_EQ(grid->GetFasterSplitView()->GetDismissButton(),
            focus_cycler->focused_view()->GetView());

  // Tab to the settings button.
  PressAndReleaseKey(ui::VKEY_TAB);
  ASSERT_TRUE(IsInOverviewSession());
  EXPECT_EQ(grid->GetFasterSplitView()->settings_button(),
            focus_cycler->focused_view());

  // Note we use `PressKeyAndModifierKeys()` to send modifier and key separately
  // to simulate real user input.

  // Shift + Tab reverse tabs to the dismiss button.
  auto* event_generator = GetEventGenerator();
  event_generator->PressKeyAndModifierKeys(ui::VKEY_TAB, ui::EF_SHIFT_DOWN);
  ASSERT_TRUE(IsInOverviewSession());
  EXPECT_EQ(grid->GetFasterSplitView()->GetDismissButton(),
            focus_cycler->focused_view()->GetView());

  // Shift + Tab reverse tabs to the overview item.
  event_generator->PressKeyAndModifierKeys(ui::VKEY_TAB, ui::EF_SHIFT_DOWN);
  ASSERT_TRUE(IsInOverviewSession());
  EXPECT_EQ(overview_windows[0]->GetWindow(), GetOverviewFocusedWindow());
}

// Tests that the chromevox keys work as expected.
TEST_F(FasterSplitScreenTest, TabbingChromevox) {
  Shell::Get()->accessibility_controller()->spoken_feedback().SetEnabled(true);

  std::unique_ptr<aura::Window> window2(CreateTestWindow());
  std::unique_ptr<aura::Window> window1(CreateTestWindow());

  const WindowSnapWMEvent snap_event(WM_EVENT_SNAP_PRIMARY,
                                     WindowSnapActionSource::kTest);
  auto* event_generator = GetEventGenerator();

  enum class TestCase { kDismissButton, kSettingsButton };
  const auto kTestCases = {TestCase::kDismissButton, TestCase::kSettingsButton};

  for (auto test_case : kTestCases) {
    WindowState::Get(window1.get())->OnWMEvent(&snap_event);
    ASSERT_TRUE(OverviewController::Get()->InOverviewSession());

    // Note we use `PressKeyAndModifierKeys()` to send modifier and key
    // separately to simulate real user input.

    // Search + Right moves to the first overview item.
    event_generator->PressKeyAndModifierKeys(ui::VKEY_RIGHT,
                                             ui::EF_COMMAND_DOWN);
    const std::vector<std::unique_ptr<OverviewItemBase>>& overview_windows =
        GetOverviewItemsForRoot(0);
    EXPECT_EQ(overview_windows[0]->GetWindow(), GetOverviewFocusedWindow());

    // Search + Right moves to the dismiss button.
    event_generator->PressKeyAndModifierKeys(ui::VKEY_RIGHT,
                                             ui::EF_COMMAND_DOWN);
    OverviewGrid* grid = GetOverviewSession()->grid_list()[0].get();
    OverviewFocusCycler* focus_cycler = GetOverviewSession()->focus_cycler();
    EXPECT_EQ(grid->GetFasterSplitView()->GetDismissButton(),
              focus_cycler->focused_view()->GetView());

    // Search + Right moves to the settings button.
    event_generator->PressKeyAndModifierKeys(ui::VKEY_RIGHT,
                                             ui::EF_COMMAND_DOWN);
    EXPECT_EQ(grid->GetFasterSplitView()->settings_button(),
              focus_cycler->focused_view());

    if (test_case == TestCase::kSettingsButton) {
      // Search + Space activates the settings button.
      event_generator->PressKeyAndModifierKeys(ui::VKEY_SPACE,
                                               ui::EF_COMMAND_DOWN);
      EXPECT_FALSE(IsInOverviewSession());
    } else {
      // Search + Left moves back to the dismiss button.
      event_generator->PressKeyAndModifierKeys(ui::VKEY_LEFT,
                                               ui::EF_COMMAND_DOWN);
      EXPECT_EQ(grid->GetFasterSplitView()->GetDismissButton(),
                focus_cycler->focused_view()->GetView());

      // Search + Space activates the dismiss button.
      event_generator->PressKeyAndModifierKeys(ui::VKEY_SPACE,
                                               ui::EF_COMMAND_DOWN);
      EXPECT_FALSE(IsInOverviewSession());
    }
  }
}

TEST_F(FasterSplitScreenTest, AccessibilityFocusAnnotator) {
  auto window1 = CreateTestWindow(gfx::Rect(100, 100));
  auto window0 = CreateTestWindow(gfx::Rect(100, 100));

  // Snap `window0`, so it is excluded from the overview list.
  SnapOneTestWindow(window0.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kDragWindowToEdgeToSnap);

  auto* focus_widget = views::Widget::GetWidgetForNativeWindow(
      GetOverviewSession()->GetOverviewFocusWindow());
  ASSERT_TRUE(focus_widget);
  OverviewGrid* grid = GetOverviewSession()->grid_list()[0].get();
  ASSERT_FALSE(grid->desks_widget());
  ASSERT_FALSE(grid->GetSaveDeskForLaterButton());
  auto* faster_splitview_widget = grid->faster_splitview_widget();
  ASSERT_TRUE(faster_splitview_widget);

  // Overview items are in MRU order, so the expected order in the grid list is
  // the reverse creation order.
  auto* item_widget1 = GetOverviewItemForWindow(window1.get())->item_widget();

  // Order should be [focus_widget, item_widget1, faster_splitview_widget].
  CheckA11yOverrides("focus", focus_widget,
                     /*expected_previous=*/faster_splitview_widget,
                     /*expected_next=*/item_widget1);
  CheckA11yOverrides("item1", item_widget1, /*expected_previous=*/focus_widget,
                     /*expected_next=*/faster_splitview_widget);
  CheckA11yOverrides("splitview", faster_splitview_widget,
                     /*expected_previous=*/item_widget1,
                     /*expected_next=*/focus_widget);
}

// Tests the histograms for the split view overview session exit points are
// recorded correctly in clamshell.
TEST_F(FasterSplitScreenTest,
       SplitViewOverviewSessionExitPointClamshellHistograms) {
  const auto kWindowLayoutCompleteOnSessionExit =
      BuildWindowLayoutCompleteOnSessionExitHistogram();
  const auto kSplitViewOverviewSessionExitPoint =
      BuildSplitViewOverviewExitPointHistogramName(
          WindowSnapActionSource::kDragWindowToEdgeToSnap);

  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());

  // Verify the initial count for the histogram.
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/true,
                                      /*expected_count=*/0);
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/false,
                                      /*expected_count=*/0);

  // Set up the splitview overview session and select a window in the partial
  // overview to complete the window layout.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kDragWindowToEdgeToSnap);
  VerifySplitViewOverviewSession(w1.get());
  auto* event_generator = GetEventGenerator();
  ClickOverviewItem(event_generator, w2.get());
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/true,
                                      /*expected_count=*/1);
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/false,
                                      /*expected_count=*/0);
  histogram_tester_.ExpectBucketCount(
      kSplitViewOverviewSessionExitPoint,
      SplitViewOverviewSessionExitPoint::kCompleteByActivating,
      /*expected_count=*/1);
  MaximizeToClearTheSession(w1.get());
  MaximizeToClearTheSession(w2.get());

  // Set up the splitview overview session and click an empty area to skip the
  // pairing.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kDragWindowToEdgeToSnap);
  VerifySplitViewOverviewSession(w1.get());
  auto* item2 = GetOverviewItemForWindow(w2.get());
  gfx::Point outside_point =
      gfx::ToRoundedPoint(item2->target_bounds().bottom_right());
  outside_point.Offset(/*delta_x=*/5, /*delta_y=*/5);
  event_generator->MoveMouseTo(outside_point);
  event_generator->ClickLeftButton();
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/true,
                                      /*expected_count=*/1);
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/false,
                                      /*expected_count=*/1);
  histogram_tester_.ExpectBucketCount(kSplitViewOverviewSessionExitPoint,
                                      SplitViewOverviewSessionExitPoint::kSkip,
                                      /*expected_count=*/1);
  MaximizeToClearTheSession(w1.get());
  MaximizeToClearTheSession(w2.get());

  // Set up the splitview overview session, create a 3rd window to be
  // auto-snapped and complete the window layout.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kDragWindowToEdgeToSnap);
  VerifySplitViewOverviewSession(w1.get());
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/true,
                                      /*expected_count=*/2);
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/false,
                                      /*expected_count=*/1);
  histogram_tester_.ExpectBucketCount(
      kSplitViewOverviewSessionExitPoint,
      SplitViewOverviewSessionExitPoint::kCompleteByActivating,
      /*expected_count=*/2);
  MaximizeToClearTheSession(w1.get());
  MaximizeToClearTheSession(w3.get());

  // Set up the splitview overview session and press escape key to skip pairing.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kDragWindowToEdgeToSnap);
  VerifySplitViewOverviewSession(w1.get());
  event_generator->PressAndReleaseKey(ui::VKEY_ESCAPE);
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/true,
                                      /*expected_count=*/2);
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/false,
                                      /*expected_count=*/2);
  histogram_tester_.ExpectBucketCount(kSplitViewOverviewSessionExitPoint,
                                      SplitViewOverviewSessionExitPoint::kSkip,
                                      /*expected_count=*/2);
  MaximizeToClearTheSession(w1.get());
  MaximizeToClearTheSession(w2.get());

  // Set up the splitview overview session and close the snapped window to exit
  // the session.
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kDragWindowToEdgeToSnap);
  VerifySplitViewOverviewSession(w1.get());
  w1.reset();
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/true,
                                      /*expected_count=*/2);
  histogram_tester_.ExpectBucketCount(kWindowLayoutCompleteOnSessionExit,
                                      /*sample=*/false,
                                      /*expected_count=*/2);
  histogram_tester_.ExpectBucketCount(
      kSplitViewOverviewSessionExitPoint,
      SplitViewOverviewSessionExitPoint::kWindowDestroy,
      /*expected_count=*/1);
}

// Integration test of the `SplitViewOverviewSession` exit point with drag to
// snap action source. Verify that the end-to-end metric is recorded correctly.
TEST_F(FasterSplitScreenTest, KeyMetricsIntegrationTest_DragToSnap) {
  UpdateDisplay("800x600");

  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());

  const auto kSplitViewOverviewSessionExitPoint =
      BuildSplitViewOverviewExitPointHistogramName(
          WindowSnapActionSource::kDragWindowToEdgeToSnap);
  histogram_tester_.ExpectBucketCount(
      kSplitViewOverviewSessionExitPoint,
      SplitViewOverviewSessionExitPoint::kCompleteByActivating,
      /*expected_count=*/0);

  // Drag a window to snap on the primary snapped position and verify the
  // metrics.
  std::unique_ptr<WindowResizer> resizer(CreateWindowResizer(
      w1.get(), gfx::PointF(), HTCAPTION, wm::WINDOW_MOVE_SOURCE_MOUSE));
  resizer->Drag(gfx::PointF(0, 400), /*event_flags=*/0);
  resizer->CompleteDrag();
  resizer.reset();
  SplitViewOverviewSession* split_view_overview_session =
      VerifySplitViewOverviewSession(w1.get());
  EXPECT_EQ(split_view_overview_session->snap_action_source_for_testing(),
            WindowSnapActionSource::kDragWindowToEdgeToSnap);
  auto* event_generator = GetEventGenerator();
  ClickOverviewItem(event_generator, w2.get());
  histogram_tester_.ExpectBucketCount(
      kSplitViewOverviewSessionExitPoint,
      SplitViewOverviewSessionExitPoint::kCompleteByActivating,
      /*expected_count=*/1);

  MaximizeToClearTheSession(w1.get());
  MaximizeToClearTheSession(w2.get());

  // Drag a window to snap on the secondary snapped position and verify the
  // metrics.
  resizer = CreateWindowResizer(w1.get(), gfx::PointF(), HTCAPTION,
                                wm::WINDOW_MOVE_SOURCE_MOUSE);
  resizer->Drag(gfx::PointF(800, 0), /*event_flags=*/0);
  resizer->CompleteDrag();
  resizer.reset();
  split_view_overview_session = VerifySplitViewOverviewSession(w1.get());
  EXPECT_EQ(split_view_overview_session->snap_action_source_for_testing(),
            WindowSnapActionSource::kDragWindowToEdgeToSnap);

  auto* item2 = GetOverviewItemForWindow(w2.get());
  gfx::Point outside_point =
      gfx::ToRoundedPoint(item2->target_bounds().bottom_right());
  outside_point.Offset(/*delta_x=*/5, /*delta_y=*/5);
  event_generator->MoveMouseTo(outside_point);
  event_generator->ClickLeftButton();
  histogram_tester_.ExpectBucketCount(kSplitViewOverviewSessionExitPoint,
                                      SplitViewOverviewSessionExitPoint::kSkip,
                                      /*expected_count=*/1);
  MaximizeToClearTheSession(w1.get());
}

// Integration test of the `SplitViewOverviewSession` exit point with window
// size button as the snap action source. Verify that the end-to-end metric is
// recorded correctly.
TEST_F(FasterSplitScreenTest, KeyMetricsIntegrationTest_WindowSizeButton) {
  UpdateDisplay("800x600");

  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());

  struct SnapRequestWithActionSource {
    chromeos::SnapController::SnapRequestSource request_source;
    WindowSnapActionSource snap_action_source;
  } kTestCases[]{
      {chromeos::SnapController::SnapRequestSource::kWindowLayoutMenu,
       WindowSnapActionSource::kSnapByWindowLayoutMenu},
      {chromeos::SnapController::SnapRequestSource::kSnapButton,
       WindowSnapActionSource::kLongPressCaptionButtonToSnap},
  };

  for (const auto test_case : kTestCases) {
    const auto kSplitViewOverviewSessionExitPoint =
        BuildSplitViewOverviewExitPointHistogramName(
            test_case.snap_action_source);
    histogram_tester_.ExpectBucketCount(
        kSplitViewOverviewSessionExitPoint,
        SplitViewOverviewSessionExitPoint::kCompleteByActivating,
        /*expected_count=*/0);

    auto commit_snap = [&]() {
      chromeos::SnapController::Get()->CommitSnap(
          w1.get(), chromeos::SnapDirection::kSecondary,
          chromeos::kDefaultSnapRatio, test_case.request_source);
      SplitViewOverviewSession* split_view_overview_session =
          VerifySplitViewOverviewSession(w1.get());
      EXPECT_TRUE(split_view_overview_session);
      EXPECT_EQ(split_view_overview_session->snap_action_source_for_testing(),
                test_case.snap_action_source);
    };

    commit_snap();
    auto* event_generator = GetEventGenerator();
    ClickOverviewItem(GetEventGenerator(), w2.get());
    histogram_tester_.ExpectBucketCount(
        kSplitViewOverviewSessionExitPoint,
        SplitViewOverviewSessionExitPoint::kCompleteByActivating,
        /*expected_count=*/1);
    MaximizeToClearTheSession(w1.get());
    MaximizeToClearTheSession(w2.get());

    commit_snap();
    auto* item2 = GetOverviewItemForWindow(w2.get());
    gfx::Point outside_point =
        gfx::ToRoundedPoint(item2->target_bounds().bottom_right());
    outside_point.Offset(/*delta_x=*/5, /*delta_y=*/5);
    event_generator->MoveMouseTo(outside_point);
    event_generator->ClickLeftButton();

    histogram_tester_.ExpectBucketCount(
        kSplitViewOverviewSessionExitPoint,
        SplitViewOverviewSessionExitPoint::kSkip,
        /*expected_count=*/1);
    MaximizeToClearTheSession(w1.get());
  }
}

// Tests that the `OverviewStartAction` will be recorded correctly in uma for
// the faster split screen setup.
TEST_F(FasterSplitScreenTest, OverviewStartActionHistogramTest) {
  constexpr char kOverviewStartActionHistogram[] = "Ash.Overview.StartAction";
  // Verify the initial count for the histogram.
  histogram_tester_.ExpectBucketCount(
      kOverviewStartActionHistogram,
      OverviewStartAction::kFasterSplitScreenSetup,
      /*expected_count=*/0);
  std::unique_ptr<aura::Window> window1(CreateAppWindow());
  std::unique_ptr<aura::Window> window2(CreateAppWindow());
  SnapOneTestWindow(window1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  VerifySplitViewOverviewSession(window1.get());
  histogram_tester_.ExpectBucketCount(
      kOverviewStartActionHistogram,
      OverviewStartAction::kFasterSplitScreenSetup,
      /*expected_count=*/1);
}

// Tests that a11y alert will be announced upon entering the faster split screen
// setup session.
TEST_F(FasterSplitScreenTest, A11yAlertOnEnteringFaterSplitScreenSetup) {
  TestAccessibilityControllerClient client;
  std::unique_ptr<aura::Window> window1(CreateAppWindow());
  std::unique_ptr<aura::Window> window2(CreateAppWindow());
  EXPECT_NE(AccessibilityAlert::FASTER_SPLIT_SCREEN_SETUP,
            client.last_a11y_alert());
  SnapOneTestWindow(window1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio);
  EXPECT_EQ(AccessibilityAlert::FASTER_SPLIT_SCREEN_SETUP,
            client.last_a11y_alert());
}

// Tests that there will be no crash when dragging a snapped window in overview
// toward the edge. In this case, the overview components will become too small
// to meet the minimum requirement of the fundamental UI layer such as shadow.
// See the regression behavior in http://b/324478757.
TEST_F(FasterSplitScreenTest, NoCrashWhenDraggingSnappedWindowToEdge) {
  std::unique_ptr<aura::Window> window1(
      CreateAppWindow(gfx::Rect(0, 0, 200, 100)));
  std::unique_ptr<aura::Window> window2(
      CreateAppWindow(gfx::Rect(100, 100, 200, 100)));
  SnapOneTestWindow(window1.get(), WindowStateType::kPrimarySnapped,
                    chromeos::kDefaultSnapRatio,
                    WindowSnapActionSource::kSnapByWindowLayoutMenu);
  WaitForOverviewEntered();
  VerifySplitViewOverviewSession(window1.get());

  // Drag the snapped window towards the edge of the work area and verify that
  // there is no crash.
  auto* event_generator = GetEventGenerator();
  event_generator->set_current_screen_location(
      window1.get()->GetBoundsInScreen().right_center());
  gfx::Point drag_end_point = work_area_bounds().right_center();
  drag_end_point.Offset(/*delta_x=*/-10, 0);
  event_generator->PressLeftButton();
  event_generator->MoveMouseTo(drag_end_point);

  // Verify that shadow exists for overview item.
  auto* overview_item2 = GetOverviewItemForWindow(window2.get());
  const auto shadow_content_bounds =
      overview_item2->get_shadow_content_bounds_for_testing();
  EXPECT_FALSE(shadow_content_bounds.IsEmpty());

  VerifySplitViewOverviewSession(window1.get());
  EXPECT_TRUE(WindowState::Get(window1.get())->is_dragged());
}

// -----------------------------------------------------------------------------
// SnapGroupTest:

// A test fixture to test the snap group feature.
class SnapGroupTest : public FasterSplitScreenTest {
 public:
  SnapGroupTest() {
    scoped_feature_list_.InitWithFeatures(/*enabled_features=*/
                                          {features::kSnapGroup,
                                           features::kSameAppWindowCycle},
                                          /*disabled_features=*/{});
  }
  SnapGroupTest(const SnapGroupTest&) = delete;
  SnapGroupTest& operator=(const SnapGroupTest&) = delete;
  ~SnapGroupTest() override = default;

  // AshTestBase:
  void SetUp() override {
    AshTestBase::SetUp();
    WindowCycleList::SetDisableInitialDelayForTesting(true);
  }

  void SnapOneTestWindow(aura::Window* window, WindowStateType state_type) {
    UpdateDisplay("800x600");
    WindowState* window_state = WindowState::Get(window);
    const WindowSnapWMEvent snap_type(
        state_type == WindowStateType::kPrimarySnapped
            ? WM_EVENT_SNAP_PRIMARY
            : WM_EVENT_SNAP_SECONDARY,
        /*snap_action_source=*/WindowSnapActionSource::kTest);
    window_state->OnWMEvent(&snap_type);
    EXPECT_EQ(state_type, window_state->GetStateType());
  }

  void SnapTwoTestWindows(aura::Window* window1,
                          aura::Window* window2,
                          bool horizontal = true) {
    CHECK_NE(window1, window2);
    // Snap `window1` to trigger the overview session shown on the other side of
    // the screen.
    SnapOneTestWindow(
        window1,
        /*state_type=*/chromeos::WindowStateType::kPrimarySnapped);
    WaitForOverviewEntered();
    VerifySplitViewOverviewSession(window1);

    // When the first window is snapped, it takes exactly half the width.
    gfx::Rect expected_bounds(work_area_bounds());
    gfx::Rect left_bounds, right_bounds;
    expected_bounds.SplitVertically(&left_bounds, &right_bounds);
    EXPECT_EQ(left_bounds, window1->GetBoundsInScreen());

    // The `window2` gets selected in the overview will be snapped to the
    // non-occupied snap position and the overview session will end.
    ClickOverviewItem(GetEventGenerator(), window2);
    WaitForOverviewExitAnimation();
    EXPECT_EQ(WindowState::Get(window2)->GetStateType(),
              chromeos::WindowStateType::kSecondarySnapped);
    EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
    EXPECT_FALSE(RootWindowController::ForWindow(window1)
                     ->split_view_overview_session());

    auto* snap_group_controller = SnapGroupController::Get();
    ASSERT_TRUE(snap_group_controller);
    EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(window1, window2));

    // The split view divider will show on two windows snapped.
    EXPECT_TRUE(split_view_divider()->divider_widget());

    EXPECT_EQ(chromeos::kDefaultSnapRatio,
              *WindowState::Get(window1)->snap_ratio());
    EXPECT_EQ(chromeos::kDefaultSnapRatio,
              *WindowState::Get(window2)->snap_ratio());

    // Now that two windows are snapped, the divider is between them.
    gfx::Rect divider_bounds(
        split_view_divider()->GetDividerBoundsInScreen(/*is_dragging=*/false));
    left_bounds.set_width(left_bounds.width() - divider_bounds.width() / 2);
    right_bounds.set_x(right_bounds.x() + divider_bounds.width() / 2);
    right_bounds.set_width(right_bounds.width() - divider_bounds.width() / 2);

    EXPECT_EQ(left_bounds.width(), window1->GetBoundsInScreen().width());
    EXPECT_EQ(right_bounds.width(), window2->GetBoundsInScreen().width());
  }

  void CompleteWindowCycling() {
    WindowCycleController* window_cycle_controller =
        Shell::Get()->window_cycle_controller();
    window_cycle_controller->CompleteCycling();
    EXPECT_FALSE(window_cycle_controller->IsCycling());
  }

  void CycleWindow(WindowCyclingDirection direction, int steps) {
    WindowCycleController* window_cycle_controller =
        Shell::Get()->window_cycle_controller();
    for (int i = 0; i < steps; i++) {
      window_cycle_controller->HandleCycleWindow(direction);
      EXPECT_TRUE(window_cycle_controller->IsCycling());
    }
  }

  // TODO(michelefan): Consider put this test util in a base class or test file.
  std::unique_ptr<aura::Window> CreateTestWindowWithAppID(
      std::string app_id_key) {
    std::unique_ptr<aura::Window> window = CreateTestWindow();
    window->SetProperty(kAppIDKey, std::move(app_id_key));
    return window;
  }

  std::unique_ptr<aura::Window> CreateTransientChildWindow(
      aura::Window* transient_parent,
      gfx::Rect child_window_bounds) {
    auto child = CreateTestWindow(child_window_bounds);
    wm::AddTransientChild(transient_parent, child.get());
    return child;
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

// Tests that the creation and removal of snap group.
TEST_F(SnapGroupTest, AddAndRemoveSnapGroupTest) {
  auto* snap_group_controller = SnapGroupController::Get();
  const auto& snap_groups = snap_group_controller->snap_groups_for_testing();
  const auto& window_to_snap_group_map =
      snap_group_controller->window_to_snap_group_map_for_testing();
  EXPECT_EQ(snap_groups.size(), 0u);
  EXPECT_EQ(window_to_snap_group_map.size(), 0u);

  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  std::unique_ptr<aura::Window> w3(CreateTestWindow());

  SnapTwoTestWindows(w1.get(), w2.get());
  EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));
  EXPECT_FALSE(snap_group_controller->AddSnapGroup(w1.get(), w3.get()));

  EXPECT_EQ(snap_groups.size(), 1u);
  EXPECT_EQ(window_to_snap_group_map.size(), 2u);
  const auto iter1 = window_to_snap_group_map.find(w1.get());
  ASSERT_TRUE(iter1 != window_to_snap_group_map.end());
  const auto iter2 = window_to_snap_group_map.find(w2.get());
  ASSERT_TRUE(iter2 != window_to_snap_group_map.end());
  auto* snap_group = snap_groups.back().get();
  EXPECT_EQ(iter1->second, snap_group);
  EXPECT_EQ(iter2->second, snap_group);

  ASSERT_TRUE(snap_group_controller->RemoveSnapGroup(snap_group));
  ASSERT_TRUE(snap_groups.empty());
  ASSERT_TRUE(window_to_snap_group_map.empty());
}

// Tests that the corresponding snap group will be removed when one of the
// windows in the snap group gets destroyed.
TEST_F(SnapGroupTest, WindowDestroyTest) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  SnapGroupController* snap_group_controller = SnapGroupController::Get();
  ASSERT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));
  const auto& snap_groups = snap_group_controller->snap_groups_for_testing();
  const auto& window_to_snap_group_map =
      snap_group_controller->window_to_snap_group_map_for_testing();
  EXPECT_EQ(snap_groups.size(), 1u);
  EXPECT_EQ(window_to_snap_group_map.size(), 2u);

  // Destroy one window in the snap group and the entire snap group will be
  // removed.
  w1.reset();
  EXPECT_TRUE(snap_groups.empty());
  EXPECT_TRUE(window_to_snap_group_map.empty());
}

// Tests that if one window in the snap group is actiaved, the stacking order of
// the other window in the snap group will be updated to be right below the
// activated window i.e. the two windows in the snap group will be placed on
// top.
TEST_F(SnapGroupTest, WindowStackingOrderTest) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  std::unique_ptr<aura::Window> w3(CreateTestWindow());

  SnapTwoTestWindows(w1.get(), w2.get());
  ASSERT_TRUE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w1.get(), w2.get()));

  wm::ActivateWindow(w3.get());

  // Actiave one of the windows in the snap group.
  wm::ActivateWindow(w1.get());

  MruWindowTracker::WindowList window_list =
      Shell::Get()->mru_window_tracker()->BuildMruWindowList(kActiveDesk);
  EXPECT_EQ(std::vector<vector_experimental_raw_ptr<aura::Window>>(
                {w1.get(), w3.get(), w2.get()}),
            window_list);

  // `w3` is stacked below `w2` even though the activation order of `w3` is
  // before `w2`.
  EXPECT_TRUE(window_util::IsStackedBelow(w3.get(), w2.get()));
}

// Tests that on one window snapped in clamshell mode, the overview will be
// shown on the other side of the screen. When activating a window in overview,
// the window gets activated will be auto-snapped and the overview session will
// end. Close one window will end the split view mode.
TEST_F(SnapGroupTest, ClamshellSplitViewBasicFunctionalities) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  w1.reset();
  EXPECT_FALSE(split_view_controller()->InSplitViewMode());
}

// Tests that when there is one snapped window and overview open, creating a new
// window, i.e. by clicking the shelf icon, will auto-snap it.
// TODO(michelefan): Re-enable this test after the divider refactor work is
// done.
TEST_F(SnapGroupTest, DISABLED_AutoSnapNewWindow) {
  // Snap `w1` to start split view overview session.
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  SnapOneTestWindow(w1.get(),
                    /*state_type=*/WindowStateType::kPrimarySnapped);
  VerifySplitViewOverviewSession(w1.get());

  // Create a new `w2`. Test it auto-snaps and forms a snap group with `w1`.
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w2.get())->GetStateType());
  EXPECT_TRUE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w1.get(), w2.get()));

  // Create a new `w3` and snap it on top of `w1` and `w2`'s group. Test it
  // starts overview.
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  SnapOneTestWindow(w3.get(),
                    /*state_type=*/WindowStateType::kPrimarySnapped);
  EXPECT_TRUE(OverviewController::Get()->InOverviewSession());
  EXPECT_TRUE(
      RootWindowController::ForWindow(w3.get())->split_view_overview_session());

  // TODO(b/296935443): Currently SplitViewController calculates the snap bounds
  // based on `split_view_divider_`, which may be created for the snap group
  // underneath `w3`'s split view overview session, so we won't verify
  // overview is exactly the remaining work area of `w3` yet.
  EXPECT_TRUE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w1.get(), w2.get()));
}

// TODO(b/326481241): Currently it's not possible to swap windows since
// `SplitViewController` still manages the windows and updates the bounds in a
// `SnapGroup`. This will just check that double tap still works after
// conversion.
TEST_F(SnapGroupTest, DoubleTapDivider) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  auto* snap_group = SnapGroupController::Get()->GetTopmostSnapGroup();
  EXPECT_TRUE(snap_group);
  auto* new_primary_window = snap_group->window1();
  auto* new_secondary_window = snap_group->window2();

  // Switch to tablet mode. Test that double tap on the divider swaps the
  // windows.
  SwitchToTabletMode();
  EXPECT_EQ(new_primary_window, split_view_controller()->primary_window());
  EXPECT_EQ(new_secondary_window, split_view_controller()->secondary_window());
  EXPECT_TRUE(split_view_controller()->split_view_divider()->divider_widget());
  const gfx::Point divider_center =
      split_view_controller()
          ->split_view_divider()
          ->GetDividerBoundsInScreen(/*is_dragging=*/false)
          .CenterPoint();
  GetEventGenerator()->GestureTapAt(divider_center);
  GetEventGenerator()->GestureTapAt(divider_center);
  EXPECT_EQ(new_secondary_window, split_view_controller()->primary_window());
  EXPECT_EQ(new_primary_window, split_view_controller()->secondary_window());
}

TEST_F(SnapGroupTest, DontAutoSnapNewWindowOutsideSplitViewOverview) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  EXPECT_TRUE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w1.get(), w2.get()));
  EXPECT_FALSE(
      RootWindowController::ForWindow(w1.get())->split_view_overview_session());
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());

  // Open a third window. Test it does *not* snap.
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  EXPECT_FALSE(WindowState::Get(w3.get())->IsSnapped());
  EXPECT_TRUE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w1.get(), w2.get()));
  EXPECT_TRUE(split_view_divider()->divider_widget());
}

// Tests that removing a display during split view overview session doesn't
// crash.
TEST_F(SnapGroupTest, RemoveDisplay) {
  UpdateDisplay("800x600,801+0-800x600");
  display::test::DisplayManagerTestApi display_manager_test(display_manager());

  // Snap `window` on the second display to start split view overview session.
  std::unique_ptr<aura::Window> window1(
      CreateTestWindowInShellWithBounds(gfx::Rect(900, 0, 100, 100)));
  std::unique_ptr<aura::Window> window2(
      CreateTestWindowInShellWithBounds(gfx::Rect(1000, 0, 100, 100)));
  WindowState* window_state = WindowState::Get(window1.get());
  const WindowSnapWMEvent snap_type(
      WM_EVENT_SNAP_PRIMARY,
      /*snap_action_source=*/WindowSnapActionSource::kTest);
  window_state->OnWMEvent(&snap_type);
  ASSERT_EQ(display_manager_test.GetSecondaryDisplay().id(),
            display::Screen::GetScreen()
                ->GetDisplayNearestWindow(window1.get())
                .id());
  EXPECT_EQ(WindowStateType::kPrimarySnapped, window_state->GetStateType());
  EXPECT_TRUE(OverviewController::Get()->InOverviewSession());
  EXPECT_TRUE(RootWindowController::ForWindow(window1.get())
                  ->split_view_overview_session());

  // Disconnect the second display. Test no crash.
  UpdateDisplay("800x600");
  base::RunLoop().RunUntilIdle();
}

// Tests the snap ratio is updated correctly when resizing the windows in a snap
// group with the split view divider.
TEST_F(SnapGroupTest, SnapRatioTest) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  const gfx::Point hover_location =
      split_view_divider_bounds_in_screen().CenterPoint();
  split_view_divider()->StartResizeWithDivider(hover_location);
  const auto end_point =
      hover_location + gfx::Vector2d(-work_area_bounds().width() / 6, 0);
  split_view_divider()->ResizeWithDivider(end_point);
  split_view_divider()->EndResizeWithDivider(end_point);
  EXPECT_TRUE(split_view_controller()->InSplitViewMode());
  EXPECT_NEAR(chromeos::kOneThirdSnapRatio,
              WindowState::Get(w1.get())->snap_ratio().value(),
              /*abs_error=*/0.1);
  EXPECT_NEAR(chromeos::kTwoThirdSnapRatio,
              WindowState::Get(w2.get())->snap_ratio().value(),
              /*abs_error=*/0.1);
}

// Tests that the windows in a snap group can be resized to an arbitrary
// location with the split view divider.
TEST_F(SnapGroupTest, ResizeWithSplitViewDividerToArbitraryLocations) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  for (const int distance_delta : {-10, 6, -15}) {
    const auto w1_cached_bounds = w1.get()->GetBoundsInScreen();
    const auto w2_cached_bounds = w2.get()->GetBoundsInScreen();

    const gfx::Point hover_location =
        split_view_divider_bounds_in_screen().CenterPoint();
    split_view_divider()->StartResizeWithDivider(hover_location);
    split_view_divider()->ResizeWithDivider(hover_location +
                                            gfx::Vector2d(distance_delta, 0));
    EXPECT_TRUE(split_view_controller()->InSplitViewMode());

    // TODO(michelefan): Consolidate the bounds update / calculation with the
    // existence of divider between clamshell and tablet mode. Change
    // `EXPECT_NEAR` back to `EXPECT_EQ`.
    const int abs_error = kSplitviewDividerShortSideLength / 2;
    EXPECT_NEAR(w1_cached_bounds.width() + distance_delta,
                w1.get()->GetBoundsInScreen().width(), abs_error);
    EXPECT_NEAR(w2_cached_bounds.width() - distance_delta,
                w2.get()->GetBoundsInScreen().width(), abs_error);
    EXPECT_NEAR(w1.get()->GetBoundsInScreen().width() +
                    w2.get()->GetBoundsInScreen().width() +
                    kSplitviewDividerShortSideLength,
                work_area_bounds().width(), abs_error);
  }
}

// Tests that when snapping a snapped window to the same snapped state, the
// overview session will not be triggered. The Overview session will be
// triggered when the snapped window is being snapped to the other snapped
// state.
TEST_F(SnapGroupTest, TwoWindowsSnappedTest) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  // Snap the primary window again as the primary window, the overview session
  // won't be triggered.
  SnapOneTestWindow(w1.get(),
                    /*state_type=*/WindowStateType::kPrimarySnapped);
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
  auto* snap_group_controller = SnapGroupController::Get();
  EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));

  // Snap the current primary window as the secondary window, the overview
  // session will be triggered.
  SnapOneTestWindow(w1.get(),
                    /*state_type=*/WindowStateType::kSecondarySnapped);
  EXPECT_TRUE(OverviewController::Get()->InOverviewSession());
  EXPECT_FALSE(
      snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));

  // Select the other window in overview to form a snap group and exit overview.
  ClickOverviewItem(GetEventGenerator(), w2.get());
  WaitForOverviewExitAnimation();
}

// Tests that there is no crash when work area changed after snapping two
// windows. Docked mananifier is used as an example to trigger the work area
// change.
TEST_F(SnapGroupTest, WorkAreaChangeTest) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  auto* docked_mangnifier_controller =
      Shell::Get()->docked_magnifier_controller();
  docked_mangnifier_controller->SetEnabled(/*enabled=*/true);
}

// Tests that a snap group and the split view divider will be will be
// automatically created on two windows snapped in the clamshell mode. The snap
// group will be removed together with the split view divider on destroying of
// one window in the snap group.
TEST_F(SnapGroupTest, AutomaticallyCreateGroupOnTwoWindowsSnappedInClamshell) {
  auto* snap_group_controller = SnapGroupController::Get();
  ASSERT_TRUE(snap_group_controller);
  const auto& snap_groups = snap_group_controller->snap_groups_for_testing();
  const auto& window_to_snap_group_map =
      snap_group_controller->window_to_snap_group_map_for_testing();
  EXPECT_TRUE(snap_groups.empty());
  EXPECT_TRUE(window_to_snap_group_map.empty());

  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  EXPECT_EQ(snap_groups.size(), 1u);
  EXPECT_EQ(window_to_snap_group_map.size(), 2u);

  std::unique_ptr<aura::Window> w3(CreateTestWindow());
  wm::ActivateWindow(w2.get());
  EXPECT_TRUE(window_util::IsStackedBelow(w3.get(), w1.get()));

  w1.reset();
  EXPECT_FALSE(split_view_divider()->divider_widget());
  EXPECT_TRUE(snap_groups.empty());
  EXPECT_TRUE(window_to_snap_group_map.empty());
}

// Tests that the split view divider will be stacked on top of both windows in
// the snap group and that on a third window activated the split view divider
// will be stacked below the newly activated window.
TEST_F(SnapGroupTest, DividerStackingOrderTest) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  wm::ActivateWindow(w1.get());

  SplitViewDivider* divider = split_view_divider();
  auto* divider_widget = divider->divider_widget();
  aura::Window* divider_window = divider_widget->GetNativeWindow();
  EXPECT_TRUE(window_util::IsStackedBelow(w2.get(), w1.get()));
  EXPECT_TRUE(window_util::IsStackedBelow(w1.get(), divider_window));
  EXPECT_TRUE(window_util::IsStackedBelow(w2.get(), divider_window));

  std::unique_ptr<aura::Window> w3(
      CreateTestWindow(gfx::Rect(100, 200, 300, 400)));
  EXPECT_TRUE(window_util::IsStackedBelow(divider_window, w3.get()));
  EXPECT_TRUE(window_util::IsStackedBelow(w1.get(), divider_window));
  EXPECT_TRUE(window_util::IsStackedBelow(w2.get(), w1.get()));

  wm::ActivateWindow(w2.get());
  EXPECT_TRUE(window_util::IsStackedBelow(w3.get(), w1.get()));
  EXPECT_TRUE(window_util::IsStackedBelow(w1.get(), w2.get()));
  EXPECT_TRUE(window_util::IsStackedBelow(w2.get(), divider_window));
}

// Tests that divider will be closely tied to the windows in a snap group, which
// will also apply on transient window added.
TEST_F(SnapGroupTest, DividerStackingOrderWithTransientWindow) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  wm::ActivateWindow(w1.get());

  SplitViewDivider* divider = split_view_divider();
  auto* divider_widget = divider->divider_widget();
  ASSERT_TRUE(divider_widget);
  aura::Window* divider_window = divider_widget->GetNativeWindow();
  EXPECT_TRUE(window_util::IsStackedBelow(w2.get(), w1.get()));
  EXPECT_TRUE(window_util::IsStackedBelow(w1.get(), divider_window));
  EXPECT_TRUE(window_util::IsStackedBelow(w2.get(), divider_window));

  auto w1_transient =
      CreateTransientChildWindow(w1.get(), gfx::Rect(100, 200, 200, 200));
  w1_transient->SetProperty(aura::client::kModalKey, ui::MODAL_TYPE_WINDOW);
  wm::SetModalParent(w1_transient.get(), w1.get());
  EXPECT_TRUE(window_util::IsStackedBelow(divider_window, w1_transient.get()));
}

// Tests the overall stacking order with two transient windows each of which
// belongs to a window in snap group is expected. The tests is to verify the
// transient windows issue showed in http://b/297448600#comment2.
TEST_F(SnapGroupTest, DividerStackingOrderWithTwoTransientWindows) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  SplitViewDivider* divider = split_view_divider();
  auto* divider_widget = divider->divider_widget();
  ASSERT_TRUE(divider_widget);
  aura::Window* divider_window = divider_widget->GetNativeWindow();
  ASSERT_TRUE(window_util::IsStackedBelow(w1.get(), w2.get()));
  ASSERT_TRUE(window_util::IsStackedBelow(w1.get(), divider_window));
  ASSERT_TRUE(window_util::IsStackedBelow(w2.get(), divider_window));

  // By default `w1_transient` is `MODAL_TYPE_NONE`, meaning that the associated
  // `w1` interactable.
  std::unique_ptr<aura::Window> w1_transient(
      CreateTransientChildWindow(w1.get(), gfx::Rect(10, 20, 20, 30)));

  // Add transient window for `w2` and making it not interactable by setting it
  // with the type of `ui::MODAL_TYPE_WINDOW`.
  std::unique_ptr<aura::Window> w2_transient(
      CreateTransientChildWindow(w2.get(), gfx::Rect(200, 20, 20, 30)));
  w2_transient->SetProperty(aura::client::kModalKey, ui::MODAL_TYPE_WINDOW);
  wm::SetModalParent(w2_transient.get(), w2.get());

  // The expected stacking order is as follows:
  //                    TOP
  // `w2_transient`      |
  //      |              |
  //   divider           |
  //      |              |
  //     `w2`            |
  //      |              |
  // `w1_transient`      |
  //      |              |
  //     `w1`            |
  //                   BOTTOM
  EXPECT_TRUE(window_util::IsStackedBelow(divider_window, w2_transient.get()));
  EXPECT_TRUE(
      window_util::IsStackedBelow(w1_transient.get(), w2_transient.get()));
  EXPECT_TRUE(window_util::IsStackedBelow(w1_transient.get(), divider_window));
}

// Tests that the union bounds of the primary window, secondary window in a snap
// group and the split view divider will be equal to the work area bounds both
// in horizontal and vertical split view mode.
TEST_F(SnapGroupTest, SplitViewDividerBoundsTest) {
  for (const auto is_display_horizontal_layout : {true, false}) {
    // Need to explicitly create two windows otherwise to snap a snapped window
    // on the same position won't trigger the overview session.
    std::unique_ptr<aura::Window> w1(CreateTestWindow());
    std::unique_ptr<aura::Window> w2(CreateTestWindow());
    SnapTwoTestWindows(w1.get(), w2.get(), is_display_horizontal_layout);
    EXPECT_TRUE(UnionBoundsEqualToWorkAreaBounds(w1.get(), w2.get()));
  }
}

TEST_F(SnapGroupTest, OverviewEnterExitBasic) {
  UpdateDisplay("800x600");

  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get(), /*horizontal=*/true);

  // Verify that full overview session is expected when starting overview from
  // accelerator and that split view divider will not be available.
  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  WaitForOverviewEnterAnimation();
  EXPECT_TRUE(overview_controller->overview_session());
  EXPECT_EQ(GetOverviewGridBounds(), work_area_bounds());
  EXPECT_FALSE(split_view_divider()->divider_widget());
  EXPECT_EQ(WindowStateType::kPrimarySnapped,
            WindowState::Get(w1.get())->GetStateType());
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w2.get())->GetStateType());

  // Verify that the snap group is restored with two windows snapped and that
  // the split view divider becomes available on overview exit.
  ToggleOverview();
  EXPECT_FALSE(overview_controller->overview_session());
  SnapGroupController* snap_group_controller = SnapGroupController::Get();
  EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));
  EXPECT_EQ(WindowStateType::kPrimarySnapped,
            WindowState::Get(w1.get())->GetStateType());
  EXPECT_EQ(WindowStateType::kSecondarySnapped,
            WindowState::Get(w2.get())->GetStateType());
  EXPECT_TRUE(split_view_divider()->divider_widget());
}

// Tests that partial overview is shown on the other side of the screen on one
// window snapped.
TEST_F(SnapGroupTest, PartialOverview) {
  UpdateDisplay("800x600");
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());

  for (const auto& snap_state :
       {WindowStateType::kPrimarySnapped, WindowStateType::kSecondarySnapped}) {
    SnapOneTestWindow(w1.get(), snap_state);
    WaitForOverviewEnterAnimation();
    EXPECT_TRUE(OverviewController::Get()->overview_session());
    EXPECT_NE(GetOverviewGridBounds(), work_area_bounds());
    EXPECT_NEAR(GetOverviewGridBounds().width(),
                work_area_bounds().width() / 2.f,
                kSplitviewDividerShortSideLength / 2.f);
  }
}

// Tests that the group item will be created properly and that the snap group
// will be represented as one group item in overview.
TEST_F(SnapGroupTest, OverviewGroupItemCreationBasic) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  WaitForOverviewEnterAnimation();
  ASSERT_TRUE(overview_controller->overview_session());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  EXPECT_EQ(overview_grid->window_list().size(), 2u);
}

// Tests that if one of the windows in a snap group gets destroyed in overview,
// the overview group item will only host the other window. If both of the
// windows get destroyed, the corresponding overview group item will be removed
// from the overview grid.
TEST_F(SnapGroupTest, WindowDestructionInOverview) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  WaitForOverviewEnterAnimation();
  ASSERT_TRUE(overview_controller->overview_session());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  ASSERT_EQ(overview_grid->window_list().size(), 2u);

  // On one window in snap group destroying, the group item will host the other
  // window.
  w2.reset();
  ASSERT_TRUE(overview_grid);
  EXPECT_EQ(overview_grid->window_list().size(), 2u);

  // On the only remaining window in snap group destroying, the group item will
  // be removed from the overview grid.
  w1.reset();
  ASSERT_TRUE(overview_grid);
  EXPECT_EQ(overview_grid->window_list().size(), 1u);
}

// Tests that the rounded corners of the remaining item in the snap group on
// window destruction will be refreshed so that the exposed corners will be
// rounded corners.
TEST_F(SnapGroupTest, RefreshVisualsOnWindowDestructionInOverview) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  ASSERT_TRUE(overview_controller->overview_session());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& overview_items = overview_grid->window_list();
  ASSERT_EQ(overview_items.size(), 2u);

  w2.reset();
  ASSERT_TRUE(overview_grid);
  EXPECT_EQ(overview_grid->window_list().size(), 2u);

  for (const auto& overview_item : overview_items) {
    const gfx::RoundedCornersF rounded_corners =
        overview_item->GetRoundedCorners();
    EXPECT_NEAR(rounded_corners.upper_left(), kWindowMiniViewCornerRadius,
                /*abs_error=*/0.01);
    EXPECT_NEAR(rounded_corners.upper_right(), kWindowMiniViewCornerRadius,
                /*abs_error=*/0.01);
    EXPECT_NEAR(rounded_corners.lower_right(), kWindowMiniViewCornerRadius,
                /*abs_error=*/0.01);
    EXPECT_NEAR(rounded_corners.lower_left(), kWindowMiniViewCornerRadius,
                /*abs_error=*/0.01);
  }
}

// Tests that when one of the window in snap group gets destroyed in overview,
// the other window will restore its bounds properly when been activated to exit
// overview.
TEST_F(SnapGroupTest, RemainingWindowBoundsRestoreAfterDestructionInOverview) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  std::unique_ptr<aura::Window> w3(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  ASSERT_TRUE(split_view_divider()->divider_widget());
  const gfx::Size w1_size_before_overview = w1->GetBoundsInScreen().size();

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  ASSERT_TRUE(overview_controller->InOverviewSession());
  EXPECT_FALSE(w1->transform().IsIdentity());
  EXPECT_FALSE(w2->transform().IsIdentity());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  ASSERT_EQ(overview_grid->window_list().size(), 2u);

  // On one window in snap group destroying, the group item will host the other
  // window.
  w2.reset();
  ASSERT_TRUE(overview_grid);
  EXPECT_EQ(overview_grid->window_list().size(), 2u);

  ClickOverviewItem(GetEventGenerator(), w1.get());
  EXPECT_FALSE(overview_controller->InOverviewSession());
  const gfx::Size w1_size_after_overview = w1->GetBoundsInScreen().size();

  // Verify that the size of `w1` on overview exit is equal to that of before
  // entering overview plus `kSplitviewDividerShortSideLength / 2`.
  EXPECT_EQ(
      w1_size_before_overview.width() + kSplitviewDividerShortSideLength / 2,
      w1_size_after_overview.width());
  EXPECT_EQ(w1_size_before_overview.height(), w1_size_after_overview.height());

  // Verify that the transform is identity.
  EXPECT_TRUE(w1->transform().IsIdentity());
}

// Tests that the individual items within the same group will be hosted by the
// same overview group item.
TEST_F(SnapGroupTest, OverviewItemTest) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  OverviewSession* overview_session = overview_controller->overview_session();
  ASSERT_TRUE(overview_session);

  EXPECT_EQ(overview_session->GetOverviewItemForWindow(w1.get()),
            overview_session->GetOverviewItemForWindow(w2.get()));
}

// Tests that the size of the `OverviewItem`s hosted by the `OverviewGroupItem`
// will correspond to the actual window layout.
TEST_F(SnapGroupTest, ReflectSnapRatioInOverviewGroupItem) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  ASSERT_TRUE(split_view_divider()->divider_widget());
  const gfx::Point hover_location =
      split_view_divider_bounds_in_screen().CenterPoint();
  split_view_divider()->StartResizeWithDivider(hover_location);
  const gfx::Vector2d drag_delta(-work_area_bounds().width() / 6, 0);
  const auto end_point = hover_location + drag_delta;
  split_view_divider()->ResizeWithDivider(end_point);
  split_view_divider()->EndResizeWithDivider(end_point);
  EXPECT_TRUE(split_view_controller()->InSplitViewMode());
  EXPECT_NEAR(chromeos::kOneThirdSnapRatio,
              WindowState::Get(w1.get())->snap_ratio().value(),
              /*abs_error=*/0.01);
  EXPECT_NEAR(chromeos::kTwoThirdSnapRatio,
              WindowState::Get(w2.get())->snap_ratio().value(),
              /*abs_error=*/0.01);

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  OverviewSession* overview_session = overview_controller->overview_session();
  ASSERT_TRUE(overview_session);

  OverviewGroupItem* overview_group_item =
      static_cast<OverviewGroupItem*>(GetOverviewItemForWindow(w1.get()));
  ASSERT_TRUE(overview_group_item);

  const auto& overview_items =
      overview_group_item->overview_items_for_testing();
  ASSERT_EQ(overview_items.size(), 2u);

  // Since `w1` is roughly half the width of `w2`, verify that `item1_bounds` is
  // also half the width of `item2_bounds`.
  const gfx::Rect item1_bounds =
      overview_items[0]->item_widget()->GetWindowBoundsInScreen();
  const gfx::Rect item2_bounds =
      overview_items[1]->item_widget()->GetWindowBoundsInScreen();
  const float size_ratio =
      static_cast<float>(item1_bounds.width()) / item2_bounds.width();
  EXPECT_NEAR(size_ratio, 0.5, /*abs_error=*/0.01);
}

// Tests the individual close functionality of the `OverviewGroupItem` by
// clicking on the close button of each overview item.
TEST_F(SnapGroupTest, CloseIndividualWindowByCloseButton) {
  ScopedOverviewTransformWindow::SetImmediateCloseForTests(/*immediate=*/true);
  std::unique_ptr<aura::Window> w0(CreateAppWindow());
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  SnapTwoTestWindows(w0.get(), w1.get());
  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());
  OverviewGroupItem* overview_group_item =
      static_cast<OverviewGroupItem*>(GetOverviewItemForWindow(w0.get()));
  ASSERT_TRUE(overview_group_item);

  const auto& overview_items =
      overview_group_item->overview_items_for_testing();
  ASSERT_EQ(overview_items.size(), 2u);

  // Since the window will be deleted in overview, release the ownership to
  // avoid double deletion.
  w0.release();

  auto* event_generator = GetEventGenerator();
  const CloseButton* w0_close_button =
      overview_items[0]->overview_item_view()->close_button();
  event_generator->MoveMouseTo(
      w0_close_button->GetBoundsInScreen().CenterPoint());
  event_generator->ClickLeftButton();

  // Use the run loop so that to wait until the window is closed.
  base::RunLoop().RunUntilIdle();

  // Verify that only one item remains to be hosted by the group item.
  ASSERT_EQ(overview_items.size(), 1u);

  // Verify that the visuals of the remaining item will be refreshed with four
  // rounded corners applied.
  const gfx::RoundedCornersF rounded_corners =
      GetOverviewItemForWindow(w1.get())->GetRoundedCorners();
  EXPECT_NEAR(rounded_corners.upper_left(), kWindowMiniViewCornerRadius,
              /*abs_error=*/1);
  EXPECT_NEAR(rounded_corners.upper_right(), kWindowMiniViewCornerRadius,
              /*abs_error=*/1);
  EXPECT_NEAR(rounded_corners.lower_right(), kWindowMiniViewCornerRadius,
              /*abs_error=*/1);
  EXPECT_NEAR(rounded_corners.lower_left(), kWindowMiniViewCornerRadius,
              /*abs_error=*/1);
}

// Tests that the overview group item will be closed when focused in overview
// with `Ctrl + W`.
// TODO(michelefan@): Re-purpose this test. Currently disabled due to product
// decision change.
TEST_F(SnapGroupTest, DISABLED_CtrlPlusWToCloseFocusedGroupInOverview) {
  // Explicitly enable immediate close so that we can directly close the
  // window(s) without waiting the delayed task to be completed in
  // `ScopedOverviewTransformWindow::Close()`.
  ScopedOverviewTransformWindow::SetImmediateCloseForTests(/*immediate=*/true);
  std::unique_ptr<aura::Window> w0(CreateAppWindow());
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  SnapTwoTestWindows(w0.get(), w1.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());
  OverviewSession* overview_session = overview_controller->overview_session();
  ASSERT_TRUE(GetOverviewItemForWindow(w0.get()));

  SendKeyUntilOverviewItemIsFocused(ui::VKEY_TAB);
  EXPECT_TRUE(overview_session->focus_cycler()->GetFocusedItem());

  // Since the window will be deleted in overview, release the ownership to
  // avoid double deletion.
  w0.release();
  w1.release();
  SendKey(ui::VKEY_TAB, GetEventGenerator(), ui::EF_CONTROL_DOWN);

  // Verify that both windows in the snap group will be deleted.
  EXPECT_FALSE(w0.get());
  EXPECT_FALSE(w1.get());
}

// Tests that the minimized windows in a snap group will be shown as a single
// group item in overview.
TEST_F(SnapGroupTest, MinimizedSnapGroupInOverview) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());

  SnapGroupController::Get()->MinimizeTopMostSnapGroup();

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  ASSERT_TRUE(overview_controller->overview_session());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  EXPECT_EQ(overview_grid->window_list().size(), 1u);
}

// Tests that the bounds on the overview group item as well as the individual
// overview item hosted by the group item will be set correctly.
TEST_F(SnapGroupTest, OverviewItemBoundsTest) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get());
  ASSERT_TRUE(wm::IsActiveWindow(w2.get()));

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests);
  OverviewSession* overview_session = overview_controller->overview_session();
  ASSERT_TRUE(overview_session);

  // The cumulative sum of the bounds while iterating through the individual
  // items hosted by the overview item should always be inside the group item
  // widget target bounds.
  auto* overview_group_item =
      overview_session->GetOverviewItemForWindow(w1.get());
  const gfx::RectF& group_item_bounds = overview_group_item->target_bounds();
  gfx::RectF cumulative_bounds;
  for (aura::Window* window : overview_group_item->GetWindows()) {
    auto* overview_item = overview_session->GetOverviewItemForWindow(window);
    cumulative_bounds.Union(overview_item->target_bounds());
    EXPECT_GT(cumulative_bounds.width(), 0u);
    EXPECT_TRUE(group_item_bounds.Contains(cumulative_bounds));
  }
}

// Tests the rounded corners will be applied to the exposed corners of the
// overview group item.
TEST_F(SnapGroupTest, OverviewGroupItemRoundedCorners) {
  std::unique_ptr<aura::Window> window0 = CreateAppWindow();
  std::unique_ptr<aura::Window> window1 = CreateAppWindow();
  std::unique_ptr<aura::Window> window2 = CreateAppWindow(gfx::Rect(100, 100));
  SnapTwoTestWindows(window0.get(), window1.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 2u);
  for (const auto& overview_item : window_list) {
    EXPECT_EQ(overview_item->GetRoundedCorners(),
              gfx::RoundedCornersF(kWindowMiniViewCornerRadius));
  }
}

// Tests the rounded corners will be applied to the exposed corners of the
// overview group item if the corresponding snap group is minimized.
TEST_F(SnapGroupTest, MinimizedSnapGroupRoundedCornersInOverview) {
  std::unique_ptr<aura::Window> w0(CreateAppWindow());
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow(gfx::Rect(100, 100)));
  SnapTwoTestWindows(w0.get(), w1.get());

  SnapGroupController::Get()->MinimizeTopMostSnapGroup();

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->overview_session());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 2u);
  for (const auto& overview_item : window_list) {
    EXPECT_EQ(overview_item->GetRoundedCorners(),
              gfx::RoundedCornersF(kWindowMiniViewCornerRadius));
  }
}

// Tests that the shadow for the group item in overview will be applied on the
// group-level.
TEST_F(SnapGroupTest, OverviewGroupItemShadow) {
  std::unique_ptr<aura::Window> w0(CreateAppWindow());
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow(gfx::Rect(100, 100)));
  SnapTwoTestWindows(w0.get(), w1.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->overview_session());
  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 2u);

  // Wait until the post task to `UpdateRoundedCornersAndShadow()` triggered in
  // `OverviewController::DelayedUpdateRoundedCornersAndShadow()` is finished.
  ShellTestApi().WaitForOverviewAnimationState(
      OverviewAnimationState::kEnterAnimationComplete);
  base::RunLoop().RunUntilIdle();
  for (const auto& overview_item : window_list) {
    const auto shadow_content_bounds =
        overview_item->get_shadow_content_bounds_for_testing();
    ASSERT_FALSE(shadow_content_bounds.IsEmpty());
    EXPECT_EQ(shadow_content_bounds.size(),
              gfx::ToRoundedSize(overview_item->target_bounds().size()));
  }
}

// Tests that when one of the windows in the snap group gets destroyed in
// overview the shadow contents bounds on the remaining item get updated
// correctly.
TEST_F(SnapGroupTest, CorrectShadowBoundsOnRemainingItemInOverview) {
  std::unique_ptr<aura::Window> w0(CreateAppWindow());
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  SnapTwoTestWindows(w0.get(), w1.get());

  OverviewController* overview_controller = Shell::Get()->overview_controller();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->overview_session());
  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 1u);

  w0.reset();
  EXPECT_EQ(window_list.size(), 1u);

  // Verify that the shadow bounds will be refreshed to fit with the remaining
  // item.
  auto& overview_item = window_list[0];
  const auto shadow_content_bounds =
      overview_item->get_shadow_content_bounds_for_testing();
  EXPECT_EQ(shadow_content_bounds.size(),
            gfx::ToRoundedSize(overview_item->target_bounds().size()));
}

// Tests the basic functionality of focus cycling in overview through tabbing,
// the overview group item will be focused and activated as a group
TEST_F(SnapGroupTest, DISABLED_OverviewGroupItemFocusCycling) {
  std::unique_ptr<aura::Window> window0 = CreateAppWindow();
  std::unique_ptr<aura::Window> window1 = CreateAppWindow();
  std::unique_ptr<aura::Window> window2 = CreateAppWindow(gfx::Rect(100, 100));
  SnapTwoTestWindows(window0.get(), window1.get());
  EXPECT_TRUE(window_util::IsStackedBelow(window0.get(), window1.get()));

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 2u);

  // Overview items to be cycled:
  // [window0, window1], window2
  SendKeyUntilOverviewItemIsFocused(ui::VKEY_TAB);
  auto* event_generator = GetEventGenerator();
  SendKey(ui::VKEY_TAB, event_generator, ui::EF_NONE, /*count=*/2);
  SendKey(ui::VKEY_RETURN, event_generator);
  EXPECT_FALSE(overview_controller->InOverviewSession());
  MruWindowTracker* mru_window_tracker = Shell::Get()->mru_window_tracker();
  aura::Window* top_most_window = window_util::GetTopMostWindow(
      mru_window_tracker->BuildMruWindowList(DesksMruType::kActiveDesk));
  EXPECT_EQ(top_most_window, window2.get());

  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  EXPECT_TRUE(overview_controller->InOverviewSession());

  // Overview items to be cycled:
  // window2, [window0, window1]
  SendKeyUntilOverviewItemIsFocused(ui::VKEY_TAB);
  SendKey(ui::VKEY_TAB, event_generator, ui::EF_NONE, /*count=*/2);
  SendKey(ui::VKEY_RETURN, event_generator);
  EXPECT_FALSE(overview_controller->InOverviewSession());
  top_most_window = window_util::GetTopMostWindow(
      mru_window_tracker->BuildMruWindowList(DesksMruType::kActiveDesk));
  EXPECT_EQ(top_most_window, window1.get());
}

// Tests the basic functionality of activating a group item in overview with
// mouse or touch. Overview will exit upon mouse/touch release and the overview
// item that directly handles the event will be activated.
TEST_F(SnapGroupTest, GroupItemActivation) {
  std::unique_ptr<aura::Window> window0 = CreateAppWindow();
  std::unique_ptr<aura::Window> window1 = CreateAppWindow();
  SnapTwoTestWindows(window0.get(), window1.get());
  // Pre-check that `window1` is the active window between the windows in the
  // snap group.
  ASSERT_TRUE(wm::IsActiveWindow(window1.get()));
  std::unique_ptr<aura::Window> window2 = CreateAppWindow(gfx::Rect(100, 100));
  ASSERT_TRUE(wm::IsActiveWindow(window2.get()));

  struct {
    bool use_touch;
    gfx::Vector2d offset;
    raw_ptr<aura::Window> expected_activated_window;
  } kTestCases[]{
      {false, gfx::Vector2d(-5, -5), window0.get()},
      {true, gfx::Vector2d(-5, -5), window0.get()},
      {false, gfx::Vector2d(5, 5), window1.get()},
      {true, gfx::Vector2d(5, 5), window1.get()},
  };

  OverviewController* overview_controller = OverviewController::Get();
  auto* event_generator = GetEventGenerator();

  for (const auto& test : kTestCases) {
    overview_controller->StartOverview(OverviewStartAction::kTests,
                                       OverviewEnterExitType::kImmediateEnter);
    ASSERT_TRUE(overview_controller->InOverviewSession());

    const auto* overview_grid =
        GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
    ASSERT_TRUE(overview_grid);
    const auto& window_list = overview_grid->window_list();
    ASSERT_EQ(window_list.size(), 2u);

    OverviewSession* overview_session = overview_controller->overview_session();
    auto* overview_item =
        overview_session->GetOverviewItemForWindow(window0.get());
    const auto hover_point =
        gfx::ToRoundedPoint(overview_item->target_bounds().CenterPoint()) +
        test.offset;
    event_generator->set_current_screen_location(hover_point);
    if (test.use_touch) {
      event_generator->PressTouch();
      event_generator->ReleaseTouch();
    } else {
      event_generator->ClickLeftButton();
    }

    EXPECT_FALSE(overview_controller->InOverviewSession());

    // Verify that upon mouse/touch release, the snap group will be brought to
    // the front with the expected activated.
    EXPECT_TRUE(wm::IsActiveWindow(test.expected_activated_window));
  }
}

// Tests the basic drag and drop functionality for overview group item with both
// mouse and touch events. The group item will be dropped to its original
// position before drag started.
TEST_F(SnapGroupTest, DragAndDropBasic) {
  // Explicitly create another desk so that the virtual desk bar won't expand
  // from zero-state to expanded-state when dragging starts.
  auto* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kButton);
  ASSERT_EQ(2u, desks_controller->desks().size());

  std::unique_ptr<aura::Window> window0 = CreateAppWindow();
  std::unique_ptr<aura::Window> window1 = CreateAppWindow();
  SnapTwoTestWindows(window0.get(), window1.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 1u);

  OverviewSession* overview_session = overview_controller->overview_session();
  auto* overview_item =
      overview_session->GetOverviewItemForWindow(window0.get());
  auto* event_generator = GetEventGenerator();
  const auto target_bounds_before_dragging = overview_item->target_bounds();

  for (const bool by_touch : {false, true}) {
    DragGroupItemToPoint(
        overview_item,
        Shell::GetPrimaryRootWindow()->GetBoundsInScreen().CenterPoint(),
        event_generator, by_touch, /*drop=*/false);
    EXPECT_NE(overview_item->target_bounds(), target_bounds_before_dragging);

    if (by_touch) {
      event_generator->ReleaseTouch();
    } else {
      event_generator->ReleaseLeftButton();
    }

    EXPECT_TRUE(overview_controller->InOverviewSession());

    // Verify that `overview_item` is dropped to its old position before
    // dragging.
    EXPECT_EQ(overview_item->target_bounds(), target_bounds_before_dragging);
  }
}

// Tests that the bounds of the drop target for `OverviewGroupItem` will match
// that of the corresponding item which the drop target is a placeholder for.
TEST_F(SnapGroupTest, DropTargetBoundsForGroupItem) {
  auto* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kButton);
  ASSERT_EQ(2u, desks_controller->desks().size());

  std::unique_ptr<aura::Window> window0 = CreateAppWindow();
  std::unique_ptr<aura::Window> window1 = CreateAppWindow();
  SnapTwoTestWindows(window0.get(), window1.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());

  aura::Window* primary_root_window = Shell::GetPrimaryRootWindow();
  auto* overview_grid = GetOverviewGridForRoot(primary_root_window);
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 1u);

  OverviewSession* overview_session = overview_controller->overview_session();
  auto* overview_item =
      overview_session->GetOverviewItemForWindow(window0.get());
  auto* event_generator = GetEventGenerator();
  const gfx::RectF target_bounds_before_dragging =
      overview_item->target_bounds();

  for (const bool by_touch : {true}) {
    DragGroupItemToPoint(
        overview_item,
        Shell::GetPrimaryRootWindow()->GetBoundsInScreen().CenterPoint(),
        event_generator, by_touch, /*drop=*/false);
    EXPECT_TRUE(overview_controller->InOverviewSession());

    auto* drop_target = overview_grid->drop_target();
    EXPECT_TRUE(drop_target);

    // Verify that the bounds of the `drop_target` will be the same as the
    // `target_bounds_before_dragging`.
    EXPECT_EQ(gfx::RectF(drop_target->item_widget()->GetWindowBoundsInScreen()),
              target_bounds_before_dragging);
    if (by_touch) {
      event_generator->ReleaseTouch();
    } else {
      event_generator->ReleaseLeftButton();
    }
  }
}

// Tests the stacking order of the overview group item should be above other
// overview items while being dragged.
TEST_F(SnapGroupTest, StackingOrderWhileDraggingInOverview) {
  auto* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kButton);
  ASSERT_EQ(2u, desks_controller->desks().size());

  std::unique_ptr<aura::Window> w0 = CreateAppWindow();
  std::unique_ptr<aura::Window> w1 = CreateAppWindow();
  std::unique_ptr<aura::Window> w2 = CreateAppWindow(gfx::Rect(100, 100));
  SnapTwoTestWindows(w0.get(), w1.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());

  const auto* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 2u);

  OverviewSession* overview_session = overview_controller->overview_session();
  auto* group_item = overview_session->GetOverviewItemForWindow(w0.get());
  auto* group_item_widget = group_item->item_widget();
  auto* w2_item_pre_drag = GetOverviewItemForWindow(w2.get());
  EXPECT_TRUE(window_util::IsStackedBelow(
      w2_item_pre_drag->item_widget()->GetNativeWindow(),
      group_item_widget->GetNativeWindow()));

  // Initiate the first drag.
  auto* event_generator = GetEventGenerator();
  DragGroupItemToPoint(
      group_item,
      Shell::GetPrimaryRootWindow()->GetBoundsInScreen().CenterPoint(),
      event_generator, /*by_touch_gestures=*/false, /*drop=*/false);
  EXPECT_TRUE(overview_controller->InOverviewSession());

  auto* w2_item_during_drag = GetOverviewItemForWindow(w2.get());
  auto* w2_item_window_during_drag =
      w2_item_during_drag->item_widget()->GetNativeWindow();

  // Verify that the two windows together with the group item widget will be
  // stacked above the other overview item.
  EXPECT_TRUE(window_util::IsStackedBelow(
      w2_item_window_during_drag, group_item_widget->GetNativeWindow()));
  EXPECT_TRUE(
      window_util::IsStackedBelow(w2_item_window_during_drag, w0.get()));
  EXPECT_TRUE(
      window_util::IsStackedBelow(w2_item_window_during_drag, w1.get()));
  event_generator->ReleaseLeftButton();

  // Verify that the group item can be dragged again after completing the first
  // drag.
  DragGroupItemToPoint(
      group_item,
      Shell::GetPrimaryRootWindow()->GetBoundsInScreen().CenterPoint(),
      event_generator, /*by_touch_gestures=*/false, /*drop=*/true);
  EXPECT_TRUE(overview_controller->InOverviewSession());
}

// Tests that `OverviewGroupItem` is not snappable in overview when there are
// two windows hosted by it however when one of the windows gets destroyed in
// overview, the remaining item becomes snappable.
TEST_F(SnapGroupTest, GroupItemSnapBehaviorInOverview) {
  auto* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kButton);
  ASSERT_EQ(2u, desks_controller->desks().size());

  std::unique_ptr<aura::Window> window0 = CreateAppWindow();
  std::unique_ptr<aura::Window> window1 = CreateAppWindow();
  SnapTwoTestWindows(window0.get(), window1.get());

  OverviewController* overview_controller = OverviewController::Get();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());

  auto* overview_grid = GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 1u);

  OverviewSession* overview_session = overview_controller->overview_session();
  auto* overview_item =
      overview_session->GetOverviewItemForWindow(window0.get());
  auto* event_generator = GetEventGenerator();
  const auto target_bounds_before_dragging = overview_item->target_bounds();
  const auto drag_point =
      Shell::GetPrimaryRootWindow()->GetBoundsInScreen().left_center();
  DragGroupItemToPoint(overview_item, drag_point, event_generator,
                       /*by_touch_gestures=*/false, /*drop=*/true);

  DragGroupItemToPoint(overview_item, drag_point, event_generator,
                       /*by_touch_gestures=*/false, /*drop=*/true);
  EXPECT_FALSE(overview_item->get_cannot_snap_widget_for_testing());
  EXPECT_TRUE(overview_controller->InOverviewSession());

  // Verify that `overview_item` is dropped to its old position before
  // dragging.
  EXPECT_EQ(overview_item->target_bounds(), target_bounds_before_dragging);

  // Reset `window0` and verify that the remaining item becomes snappable.
  window0.reset();

  DragGroupItemToPoint(
      overview_session->GetOverviewItemForWindow(window1.get()), drag_point,
      event_generator, /*by_touch_gestures=*/false, /*drop=*/true);
  EXPECT_EQ(WindowState::Get(window1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
}

// Tests that the two windows contained in the overview group item will be moved
// from the original desk to another desk on drag complete and that the two
// windows will still be in a snap group. The divider will show up in the
// destination desk on target desk activated.
TEST_F(SnapGroupTest, DragOverviewGroupItemToAnotherDesk) {
  auto* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kButton);
  ASSERT_EQ(2u, desks_controller->desks().size());

  std::unique_ptr<aura::Window> window0 = CreateAppWindow();
  std::unique_ptr<aura::Window> window1 = CreateAppWindow();
  SnapTwoTestWindows(window0.get(), window1.get());

  OverviewController* overview_controller = Shell::Get()->overview_controller();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);
  ASSERT_TRUE(overview_controller->InOverviewSession());

  auto* overview_grid = GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 1u);
  const auto* desks_bar_view = overview_grid->desks_bar_view();
  ASSERT_TRUE(desks_bar_view);
  const auto& mini_views = desks_bar_view->mini_views();
  ASSERT_EQ(mini_views.size(), 2u);

  const Desk* desk0 = desks_controller->GetDeskAtIndex(0);
  const Desk* desk1 = desks_controller->GetDeskAtIndex(1);

  // Verify the initial conditions before dragging the item to another desk.
  ASSERT_EQ(desks_util::GetDeskForContext(window0.get()), desk0);
  ASSERT_EQ(desks_util::GetDeskForContext(window1.get()), desk0);

  // Test that both windows contained in the overview group item will be moved
  // to the another desk.
  DragGroupItemToPoint(
      overview_controller->overview_session()->GetOverviewItemForWindow(
          window0.get()),
      mini_views[1]->GetBoundsInScreen().CenterPoint(), GetEventGenerator(),
      /*by_touch_gestures=*/false,
      /*drop=*/true);
  EXPECT_TRUE(overview_controller->InOverviewSession());
  ASSERT_EQ(desks_util::GetDeskForContext(window0.get()), desk1);
  ASSERT_EQ(desks_util::GetDeskForContext(window1.get()), desk1);
  EXPECT_TRUE(SnapGroupController::Get()->AreWindowsInSnapGroup(window0.get(),
                                                                window1.get()));
  ActivateDesk(desk1);
  EXPECT_TRUE(split_view_divider()->divider_widget());
  EXPECT_EQ(desks_util::GetDeskForContext(
                split_view_divider()->divider_widget()->GetNativeWindow()),
            desk1);
}

// Verify that there will be no crash when dragging the group item with the
// existence of bubble widget to another desk in overview. See the crash at
// http://b/311255082.
TEST_F(SnapGroupTest,
       NoCrashWhenDraggingOverviewGroupItemWithBubbleToAnotherDesk) {
  auto* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kButton);
  ASSERT_EQ(2u, desks_controller->desks().size());

  std::unique_ptr<aura::Window> w0(CreateAppWindow(gfx::Rect(0, 0, 300, 300)));
  std::unique_ptr<aura::Window> w1(
      CreateAppWindow(gfx::Rect(500, 20, 200, 200)));
  SnapTwoTestWindows(w0.get(), w1.get());

  // Create a dummy view for the bubble, adding it to the `w0`.
  views::Widget* w0_widget = views::Widget::GetWidgetForNativeWindow(w0.get());
  auto* child_view =
      w0_widget->GetRootView()->AddChildView(std::make_unique<views::View>());
  child_view->SetBounds(100, 10, 20, 20);

  // Create a bubble widget that's anchored to `w0`.
  auto bubble_delegate = std::make_unique<views::BubbleDialogDelegateView>(
      child_view, views::BubbleBorder::TOP_RIGHT);

  // The line below is essential to make sure that the bubble doesn't get closed
  // when entering overview.
  bubble_delegate->set_close_on_deactivate(false);
  views::Widget* bubble_widget(views::BubbleDialogDelegateView::CreateBubble(
      std::move(bubble_delegate)));
  aura::Window* bubble_window = bubble_widget->GetNativeWindow();
  wm::AddTransientChild(w0.get(), bubble_window);

  bubble_widget->Show();
  EXPECT_TRUE(wm::HasTransientAncestor(bubble_window, w0.get()));

  // Verify that the bubble is created inside its anchor widget.
  EXPECT_TRUE(
      w0->GetBoundsInScreen().Contains(bubble_window->GetBoundsInScreen()));

  OverviewController* overview_controller = Shell::Get()->overview_controller();
  overview_controller->StartOverview(OverviewStartAction::kTests,
                                     OverviewEnterExitType::kImmediateEnter);

  auto* overview_grid = GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  ASSERT_TRUE(overview_grid);
  const auto& window_list = overview_grid->window_list();
  ASSERT_EQ(window_list.size(), 1u);
  const auto* desks_bar_view = overview_grid->desks_bar_view();
  ASSERT_TRUE(desks_bar_view);
  const auto& mini_views = desks_bar_view->mini_views();
  ASSERT_EQ(mini_views.size(), 2u);

  const Desk* desk0 = desks_controller->GetDeskAtIndex(0);
  const Desk* desk1 = desks_controller->GetDeskAtIndex(1);

  // Verify the initial conditions before dragging the item to another desk.
  ASSERT_EQ(desks_util::GetDeskForContext(w0.get()), desk0);
  ASSERT_EQ(desks_util::GetDeskForContext(w1.get()), desk0);

  // Test that both windows contained in the overview group item are contained
  // in `desk1` after the drag.
  DragGroupItemToPoint(
      overview_controller->overview_session()->GetOverviewItemForWindow(
          w0.get()),
      mini_views[1]->GetBoundsInScreen().CenterPoint(), GetEventGenerator(),
      /*by_touch_gestures=*/false,
      /*drop=*/true);
  EXPECT_TRUE(overview_controller->InOverviewSession());
  ASSERT_EQ(desks_util::GetDeskForContext(w0.get()), desk1);
  ASSERT_EQ(desks_util::GetDeskForContext(w1.get()), desk1);
  EXPECT_TRUE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w0.get(), w1.get()));
}

// Tests that the hit area of the split view divider can be outside of its
// bounds with the extra insets whose value is `kSplitViewDividerExtraInset`.
TEST_F(SnapGroupTest, SplitViewDividerEnlargedHitArea) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get(), /*horizontal=*/true);

  const gfx::Point cached_divider_center_point =
      split_view_divider_bounds_in_screen().CenterPoint();
  auto* event_generator = GetEventGenerator();
  gfx::Point hover_location =
      cached_divider_center_point -
      gfx::Vector2d(kSplitviewDividerShortSideLength / 2 +
                        kSplitViewDividerExtraInset / 2,
                    0);
  event_generator->MoveMouseTo(hover_location);
  event_generator->PressLeftButton();
  const auto move_vector = -gfx::Vector2d(50, 0);
  event_generator->MoveMouseTo(hover_location + move_vector);
  event_generator->ReleaseLeftButton();
  EXPECT_TRUE(split_view_controller()->InSplitViewMode());
  EXPECT_EQ(split_view_divider_bounds_in_screen().CenterPoint(),
            cached_divider_center_point + move_vector);
}

// Tests that by toggling the keyboard shortcut 'Search + Shift + G', the two
// snapped windows can be grouped or ungrouped.
TEST_F(SnapGroupTest, UseShortcutToGroupUnGroupWindows) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get(), /*horizontal=*/true);
  SnapGroupController* snap_group_controller = SnapGroupController::Get();
  EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));

  // Press the shortcut and the windows will be ungrouped.
  auto* event_generator = GetEventGenerator();
  event_generator->PressAndReleaseKey(ui::VKEY_G,
                                      ui::EF_SHIFT_DOWN | ui::EF_COMMAND_DOWN);
  EXPECT_FALSE(
      snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));

  // Press the shortcut again and the windows will be grouped.
  event_generator->PressAndReleaseKey(ui::VKEY_G,
                                      ui::EF_SHIFT_DOWN | ui::EF_COMMAND_DOWN);
  EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));
  EXPECT_TRUE(split_view_divider()->divider_widget());
}

// Tests that the windows in snap group can be toggled between been minimized
// and restored with the keyboard shortcut 'Search + Shift + D', the windows
// will be remained in a snap group through these operations.
TEST_F(SnapGroupTest, UseShortcutToMinimizeWindows) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get(), /*horizontal=*/true);

  SnapGroupController* snap_group_controller = SnapGroupController::Get();
  // Press the shortcut first time and the windows will be minimized.
  auto* event_generator = GetEventGenerator();
  event_generator->PressAndReleaseKey(ui::VKEY_D,
                                      ui::EF_SHIFT_DOWN | ui::EF_COMMAND_DOWN);
  EXPECT_TRUE(WindowState::Get(w1.get())->IsMinimized());
  EXPECT_TRUE(WindowState::Get(w2.get())->IsMinimized());
  EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));

  // Press the shortcut again and the windows will be unminimized.
  event_generator->PressAndReleaseKey(ui::VKEY_D,
                                      ui::EF_SHIFT_DOWN | ui::EF_COMMAND_DOWN);
  EXPECT_FALSE(WindowState::Get(w1.get())->IsMinimized());
  EXPECT_FALSE(WindowState::Get(w2.get())->IsMinimized());
  EXPECT_TRUE(snap_group_controller->AreWindowsInSnapGroup(w1.get(), w2.get()));
  EXPECT_TRUE(split_view_divider()->divider_widget());
}

TEST_F(SnapGroupTest, SkipPairingInOverviewWhenClickingEmptyArea) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());

  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped);
  WaitForOverviewEnterAnimation();
  OverviewController* overview_controller = OverviewController::Get();
  EXPECT_TRUE(overview_controller->InOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
  ASSERT_EQ(1u, GetOverviewSession()->grid_list().size());

  auto* w2_overview_item = GetOverviewItemForWindow(w2.get());
  EXPECT_TRUE(w2_overview_item);
  const gfx::Point outside_point =
      gfx::ToRoundedPoint(
          w2_overview_item->GetTransformedBounds().bottom_right()) +
      gfx::Vector2d(20, 20);

  // Verify that clicking on an empty area in overview will exit the paring.
  auto* event_generator = GetEventGenerator();
  event_generator->MoveMouseTo(outside_point);
  event_generator->ClickLeftButton();
  EXPECT_FALSE(overview_controller->InOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
  EXPECT_FALSE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w1.get(), w2.get()));
}

TEST_F(SnapGroupTest, SkipPairingInOverviewWithEscapeKey) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());

  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped);
  OverviewController* overview_controller = OverviewController::Get();
  EXPECT_TRUE(overview_controller->InOverviewSession());
  EXPECT_TRUE(GetOverviewSession()->IsWindowInOverview(w2.get()));
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
  ASSERT_EQ(1u, GetOverviewSession()->grid_list().size());

  GetEventGenerator()->PressAndReleaseKey(ui::VKEY_ESCAPE, ui::EF_NONE);
  EXPECT_FALSE(overview_controller->InOverviewSession());
  EXPECT_EQ(WindowState::Get(w1.get())->GetStateType(),
            WindowStateType::kPrimarySnapped);
  EXPECT_FALSE(
      SnapGroupController::Get()->AreWindowsInSnapGroup(w1.get(), w2.get()));
}

// Tests that when disallowing showing overview in clamshell with `kSnapGroup`
// enabled, the overview will not show on one window snapped. The overview will
// show when re-enabling showing overview.
TEST_F(SnapGroupTest, SnapWithoutShowingOverview) {
  SnapGroupController* snap_group_controller = SnapGroupController::Get();
  snap_group_controller->set_can_enter_overview_for_testing(
      /*can_enter_overview=*/false);

  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  std::unique_ptr<aura::Window> w3(CreateTestWindow());
  SnapOneTestWindow(w1.get(), WindowStateType::kPrimarySnapped);
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
  SnapOneTestWindow(w2.get(), WindowStateType::kSecondarySnapped);
  EXPECT_FALSE(OverviewController::Get()->InOverviewSession());
  w2.reset();

  snap_group_controller->set_can_enter_overview_for_testing(
      /*can_enter_overview=*/true);
  SnapOneTestWindow(w1.get(), WindowStateType::kSecondarySnapped);
  EXPECT_TRUE(OverviewController::Get()->InOverviewSession());
}

// Tests that the window list is reordered when there is snap group. The two
// windows will be adjacent with each other with primary snapped window put
// before secondary snapped window.
TEST_F(SnapGroupTest, WindowReorderInAltTab) {
  std::unique_ptr<aura::Window> window0(CreateTestWindowInShellWithId(0));
  std::unique_ptr<aura::Window> window1(CreateTestWindowInShellWithId(1));
  std::unique_ptr<aura::Window> window2(CreateTestWindowInShellWithId(2));
  SnapTwoTestWindows(window0.get(), window1.get());

  wm::ActivateWindow(window2.get());
  // Initial window activation order: window2, [window1, window0].
  ASSERT_TRUE(wm::IsActiveWindow(window2.get()));

  WindowCycleController* window_cycle_controller =
      Shell::Get()->window_cycle_controller();
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/1);

  const auto& windows =
      window_cycle_controller->window_cycle_list()->windows_for_testing();

  // Test that the two windows in a snap group are reordered to be adjacent
  // with each other to reflect the window layout with the revised order as :
  // window2, [window0, window1].
  ASSERT_EQ(windows.size(), 3u);
  EXPECT_EQ(windows.at(0), window2.get());
  EXPECT_EQ(windows.at(1), window0.get());
  EXPECT_EQ(windows.at(2), window1.get());
  CompleteWindowCycling();
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // With the activation of `window1`, `window0` will be inserted right before
  // `window1`.
  // The new window cycle list order as: [window0, window1], window2. Cycle
  // twice to focus on `window2`.
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/2);
  CompleteWindowCycling();
  EXPECT_TRUE(wm::IsActiveWindow(window2.get()));
}

// Tests that the number of views to be cycled through inside the mirror
// container view of window cycle view will be the number of free-form windows
// plus snap groups.
TEST_F(SnapGroupTest, WindowCycleViewTest) {
  std::unique_ptr<aura::Window> window0(CreateTestWindowInShellWithId(0));
  std::unique_ptr<aura::Window> window1(CreateTestWindowInShellWithId(1));
  std::unique_ptr<aura::Window> window2(CreateTestWindowInShellWithId(2));
  SnapTwoTestWindows(window0.get(), window1.get());

  WindowCycleController* window_cycle_controller =
      Shell::Get()->window_cycle_controller();
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/3);
  const auto* window_cycle_list = window_cycle_controller->window_cycle_list();
  const auto& windows = window_cycle_list->windows_for_testing();
  EXPECT_EQ(windows.size(), 3u);

  const WindowCycleView* cycle_view = window_cycle_list->cycle_view();
  ASSERT_TRUE(cycle_view);
  EXPECT_EQ(cycle_view->mirror_container_for_testing()->children().size(), 2u);
  CompleteWindowCycling();
}

// Tests that on window that belongs to a snap group destroying while cycling
// the window list with Alt + Tab, there will be no crash. The corresponding
// child mini view hosted by the group container view will be destroyed, the
// group container view will host the other child mini view.
TEST_F(SnapGroupTest, WindowInSnapGroupDestructionInAltTab) {
  std::unique_ptr<aura::Window> window0(CreateTestWindowInShellWithId(0));
  std::unique_ptr<aura::Window> window1(CreateTestWindowInShellWithId(1));
  std::unique_ptr<aura::Window> window2(CreateTestWindowInShellWithId(2));
  SnapTwoTestWindows(window0.get(), window1.get());

  WindowCycleController* window_cycle_controller =
      Shell::Get()->window_cycle_controller();
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/3);
  const auto* window_cycle_list = window_cycle_controller->window_cycle_list();
  const auto& windows = window_cycle_list->windows_for_testing();
  EXPECT_EQ(windows.size(), 3u);

  const WindowCycleView* cycle_view = window_cycle_list->cycle_view();
  ASSERT_TRUE(cycle_view);
  // Verify that the number of child views hosted by mirror container is two at
  // the beginning.
  EXPECT_EQ(cycle_view->mirror_container_for_testing()->children().size(), 2u);

  // Destroy `window0` which belongs to a snap group.
  window0.reset();
  // Verify that we should still be cycling.
  EXPECT_TRUE(window_cycle_controller->IsCycling());
  const auto* updated_window_cycle_list =
      window_cycle_controller->window_cycle_list();
  const auto& updated_windows =
      updated_window_cycle_list->windows_for_testing();
  // Verify that the updated windows list size decreased.
  EXPECT_EQ(updated_windows.size(), 2u);

  // Verify that the number of child views hosted by mirror container will still
  // be two.
  EXPECT_EQ(cycle_view->mirror_container_for_testing()->children().size(), 2u);
}

// Tests and verifies the steps it takes to focus on a window cycle item by
// tabbing and reverse tabbing. The focused item will be activated upon
// completion of window cycling.
TEST_F(SnapGroupTest, SteppingInWindowCycleView) {
  std::unique_ptr<aura::Window> window3 =
      CreateAppWindow(gfx::Rect(300, 300), AppType::CHROME_APP);
  std::unique_ptr<aura::Window> window2 =
      CreateAppWindow(gfx::Rect(200, 200), AppType::CHROME_APP);
  std::unique_ptr<aura::Window> window1 =
      CreateAppWindow(gfx::Rect(100, 100), AppType::BROWSER);
  std::unique_ptr<aura::Window> window0 =
      CreateAppWindow(gfx::Rect(10, 10), AppType::BROWSER);

  SnapTwoTestWindows(window0.get(), window1.get());
  EXPECT_TRUE(wm::IsActiveWindow(window1.get()));
  WindowState::Get(window3.get())->Activate();
  EXPECT_TRUE(wm::IsActiveWindow(window3.get()));

  // Window cycle list:
  // window3, [window0, window1], window2
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/2);
  CompleteWindowCycling();
  EXPECT_TRUE(wm::IsActiveWindow(window1.get()));

  // Window cycle list:
  // [window0, window1], window3, window2
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/1);
  CompleteWindowCycling();
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Window cycle list:
  // [window0, window1], window3, window2
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/3);
  CompleteWindowCycling();
  EXPECT_TRUE(wm::IsActiveWindow(window2.get()));

  // Window cycle list:
  // window2, [window0, window1], window3
  CycleWindow(WindowCyclingDirection::kBackward, /*steps=*/1);
  CompleteWindowCycling();
  EXPECT_TRUE(wm::IsActiveWindow(window3.get()));
}

// Tests that the exposed rounded corners of the cycling items are rounded
// corners. The visuals will be refreshed on window destruction that belongs to
// a snap group.
TEST_F(SnapGroupTest, WindowCycleItemRoundedCorners) {
  std::unique_ptr<aura::Window> window0 =
      CreateAppWindow(gfx::Rect(100, 200), AppType::BROWSER);
  std::unique_ptr<aura::Window> window1 =
      CreateAppWindow(gfx::Rect(200, 300), AppType::BROWSER);
  std::unique_ptr<aura::Window> window2 =
      CreateAppWindow(gfx::Rect(300, 400), AppType::BROWSER);
  SnapTwoTestWindows(window0.get(), window1.get());

  WindowCycleController* window_cycle_controller =
      Shell::Get()->window_cycle_controller();
  CycleWindow(WindowCyclingDirection::kForward, /*steps=*/3);
  EXPECT_TRUE(window_cycle_controller->IsCycling());
  const auto* window_cycle_list = window_cycle_controller->window_cycle_list();
  const auto* cycle_view = window_cycle_list->cycle_view();
  auto& cycle_item_views = cycle_view->cycle_views_for_testing();
  ASSERT_EQ(cycle_item_views.size(), 2u);
  for (ash::WindowMiniViewBase* cycle_item_view : cycle_item_views) {
    EXPECT_EQ(cycle_item_view->GetRoundedCorners(),
              gfx::RoundedCornersF(kWindowMiniViewCornerRadius));
  }

  // Destroy `window0` which belongs to a snap group while cycling.
  window0.reset();
  auto& new_cycle_item_views = cycle_view->cycle_views_for_testing();
  EXPECT_EQ(new_cycle_item_views.size(), 2u);

  // Verify that the visuals of the cycling items will be refreshed so that the
  // exposed corners will be rounded corners.
  for (ash::WindowMiniViewBase* cycle_item_view : new_cycle_item_views) {
    EXPECT_EQ(cycle_item_view->GetRoundedCorners(),
              gfx::RoundedCornersF(kWindowMiniViewCornerRadius));
  }
  CompleteWindowCycling();
}

// Tests that two windows in a snap group is allowed to be shown as group item
// view only if both of them belong to the same app as the mru window. If only
// one window belongs to the app, the representation of the window will be shown
// as the individual window cycle item view.
TEST_F(SnapGroupTest, SameAppWindowCycle) {
  struct app_id_pair {
    const char* trace_message;
    const std::string app_id_2;
    const std::string app_id_3;
    const size_t windows_size;
    const size_t cycle_views_count;
  } kTestCases[]{
      {/*trace_message=*/"Windows in snap group with same app id",
       /*app_id_2=*/"A", /*app_id_3=*/"A", /*windows_size=*/4u,
       /*cycle_views_count=*/3u},
      {/*trace_message=*/"Windows in snap group with different app ids",
       /*app_id_2=*/"A", /*app_id_3=*/"B", /*windows_size=*/3u,
       /*cycle_views_count=*/3u},
  };

  std::unique_ptr<aura::Window> w0(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w1(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w2(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w3(CreateTestWindowWithAppID(std::string("A")));
  SnapTwoTestWindows(w2.get(), w3.get());
  WindowCycleController* window_cycle_controller =
      Shell::Get()->window_cycle_controller();
  for (const auto& test_case : kTestCases) {
    w2->SetProperty(kAppIDKey, std::move(test_case.app_id_2));
    w3->SetProperty(kAppIDKey, std::move(test_case.app_id_3));

    wm::ActivateWindow(w2.get());
    ASSERT_TRUE(wm::IsActiveWindow(w2.get()));

    // Simulate pressing Alt + Backtick to trigger the same app cycling.
    auto* event_generator = GetEventGenerator();
    event_generator->PressKey(ui::VKEY_MENU, ui::EF_NONE);
    event_generator->PressAndReleaseKey(ui::VKEY_OEM_3, ui::EF_ALT_DOWN);

    const auto* window_cycle_list =
        window_cycle_controller->window_cycle_list();
    ASSERT_TRUE(window_cycle_list->same_app_only());

    // Verify the number of windows for the cycling.
    const auto& windows = window_cycle_list->windows_for_testing();
    EXPECT_EQ(windows.size(), test_case.windows_size);
    EXPECT_TRUE(window_cycle_controller->IsCycling());
    const auto* cycle_view = window_cycle_list->cycle_view();
    ASSERT_TRUE(cycle_view);

    // Verify the number of cycle views.
    auto& cycle_item_views = cycle_view->cycle_views_for_testing();
    EXPECT_EQ(cycle_item_views.size(), test_case.cycle_views_count);
    event_generator->ReleaseKey(ui::VKEY_MENU, ui::EF_NONE);
  }
}

// Tests and verifies that if one of the window in a snap group gets destroyed
// while doing same app window cycling the corresponding window cycle item view
// will be properly removed and re-configured with no crash.
TEST_F(SnapGroupTest, WindowDestructionDuringSameAppWindowCycle) {
  std::unique_ptr<aura::Window> w0(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w1(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w2(CreateTestWindowWithAppID(std::string("A")));
  SnapTwoTestWindows(w0.get(), w1.get());

  // Simulate pressing Alt + Backtick to trigger the same app cycling.
  auto* event_generator = GetEventGenerator();
  event_generator->PressKey(ui::VKEY_MENU, ui::EF_NONE);
  event_generator->PressAndReleaseKey(ui::VKEY_OEM_3, ui::EF_ALT_DOWN);

  WindowCycleController* window_cycle_controller =
      Shell::Get()->window_cycle_controller();
  const auto* window_cycle_list = window_cycle_controller->window_cycle_list();
  ASSERT_TRUE(window_cycle_list->same_app_only());
  const auto* cycle_view = window_cycle_list->cycle_view();
  ASSERT_TRUE(cycle_view);
  const auto& windows = window_cycle_list->windows_for_testing();
  EXPECT_EQ(windows.size(), 3u);
  w0.reset();

  // After the window destruction, the window cycle view is still available.
  ASSERT_TRUE(cycle_view);
  const auto& updated_windows = window_cycle_list->windows_for_testing();
  EXPECT_EQ(updated_windows.size(), 2u);
  CompleteWindowCycling();
}

// Tests that if a snap group is at the beginning of a window cycling list, the
// mru window will depend on the mru window between the two windows in the snap
// group, since the windows are reordered so that it reflects the actual window
// layout.
TEST_F(SnapGroupTest, MruWindowForSameApp) {
  // Generate 5 windows with 3 of them from app A and 2 of them from app B.
  std::unique_ptr<aura::Window> w0(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w1(CreateTestWindowWithAppID(std::string("B")));
  std::unique_ptr<aura::Window> w2(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w3(CreateTestWindowWithAppID(std::string("A")));
  std::unique_ptr<aura::Window> w4(CreateTestWindowWithAppID(std::string("B")));
  SnapTwoTestWindows(w0.get(), w1.get());

  // Specifically activate the secondary snapped window with app type B.
  wm::ActivateWindow(w1.get());

  // Simulate pressing Alt + Backtick to trigger the same app cycling.
  auto* event_generator = GetEventGenerator();
  event_generator->PressKey(ui::VKEY_MENU, ui::EF_NONE);
  event_generator->PressAndReleaseKey(ui::VKEY_OEM_3, ui::EF_ALT_DOWN);

  WindowCycleController* window_cycle_controller =
      Shell::Get()->window_cycle_controller();
  const auto* window_cycle_list = window_cycle_controller->window_cycle_list();
  ASSERT_TRUE(window_cycle_list->same_app_only());
  const auto& windows = window_cycle_list->windows_for_testing();

  // Verify that the windows in the list that are been cycled all belong to app
  // B.
  EXPECT_EQ(windows.size(), 2u);
  CompleteWindowCycling();
}

// Tests that after creating a snap group in clamshell, transition to tablet
// mode won't crash (b/288179725).
TEST_F(SnapGroupTest, NoCrashWhenRemovingGroupInTabletMode) {
  std::unique_ptr<aura::Window> w1(CreateTestWindow());
  std::unique_ptr<aura::Window> w2(CreateTestWindow());
  SnapTwoTestWindows(w1.get(), w2.get(), /*horizontal=*/true);

  SwitchToTabletMode();

  // Close w2. Test that the group is destroyed but we are still in split view.
  w2.reset();
  SnapGroupController* snap_group_controller =
      Shell::Get()->snap_group_controller();
  EXPECT_FALSE(snap_group_controller->GetSnapGroupForGivenWindow(w1.get()));
  EXPECT_FALSE(snap_group_controller->GetSnapGroupForGivenWindow(w2.get()));
  EXPECT_EQ(split_view_controller()->primary_window(), w1.get());
  EXPECT_TRUE(OverviewController::Get()->InOverviewSession());
}

// Tests that one snap group in clamshell will be converted to windows in tablet
// split view. When converted back to clamshell, the snap group will be
// restored.
TEST_F(SnapGroupTest, ClamshellTabletTransitionWithOneSnapGroup) {
  std::unique_ptr<aura::Window> window1(CreateTestWindowInShellWithId(0));
  std::unique_ptr<aura::Window> window2(CreateTestWindowInShellWithId(1));
  SnapTwoTestWindows(window1.get(), window2.get(), /*horizontal=*/true);
  EXPECT_TRUE(split_view_divider()->divider_widget());

  SwitchToTabletMode();
  EXPECT_TRUE(split_view_divider()->divider_widget());
  auto observed_windows = split_view_divider()->observed_windows();
  EXPECT_EQ(window1.get(), observed_windows.front());
  EXPECT_EQ(window2.get(), observed_windows.back());

  EXPECT_EQ(chromeos::kDefaultSnapRatio,
            *WindowState::Get(window1.get())->snap_ratio());
  EXPECT_EQ(chromeos::kDefaultSnapRatio,
            *WindowState::Get(window2.get())->snap_ratio());

  ExitTabletMode();
  EXPECT_TRUE(SnapGroupController::Get()->AreWindowsInSnapGroup(window1.get(),
                                                                window2.get()));
  EXPECT_EQ(chromeos::kDefaultSnapRatio,
            *WindowState::Get(window1.get())->snap_ratio());
  EXPECT_EQ(chromeos::kDefaultSnapRatio,
            *WindowState::Get(window2.get())->snap_ratio());
  EXPECT_TRUE(split_view_divider()->divider_widget());
}

// Tests that when converting to tablet mode with split view divider at an
// arbitrary location, the bounds of the two windows and the divider will be
// updated such that the snap ratio of the layout is one of the fixed snap
// ratios.
TEST_F(SnapGroupTest, ClamshellTabletTransitionGetClosestFixedRatio) {
  UpdateDisplay("900x600");
  std::unique_ptr<aura::Window> window1(CreateTestWindowInShellWithId(0));
  std::unique_ptr<aura::Window> window2(CreateTestWindowInShellWithId(1));
  SnapTwoTestWindows(window1.get(), window2.get(), /*horizontal=*/true);
  ASSERT_TRUE(split_view_divider()->divider_widget());
  EXPECT_EQ(*WindowState::Get(window1.get())->snap_ratio(),
            chromeos::kDefaultSnapRatio);

  // Build test cases to be used for divider dragging, with expected fixed ratio
  // and corresponding pixels shown in the ASCII diagram below:
  //   ┌────────────────┬────────────────┐
  //   │                │                │
  //   │                │                │
  //   │                │                │
  //   │                │                │
  //   │                │                │
  //   │                │                │
  //   │                │                │
  //   └─────────┬──────┼───────┬────────┘
  //             │      │       │
  // ratio:     1/3    1/2     2/3
  // pixel:     300    450     600      900

  struct {
    int distance_delta;
    float expected_snap_ratio;
  } kTestCases[]{{/*distance_delta=*/-200, chromeos::kOneThirdSnapRatio},
                 {/*distance_delta=*/400, chromeos::kTwoThirdSnapRatio},
                 {/*distance_delta=*/-180, chromeos::kDefaultSnapRatio}};

  auto* event_generator = GetEventGenerator();
  const gfx::Rect work_area_bounds_in_screen =
      screen_util::GetDisplayWorkAreaBoundsInScreenForActiveDeskContainer(
          split_view_controller()->root_window()->GetChildById(
              desks_util::GetActiveDeskContainerId()));
  for (const auto test_case : kTestCases) {
    event_generator->set_current_screen_location(
        split_view_divider_bounds_in_screen().CenterPoint());
    event_generator->DragMouseBy(test_case.distance_delta, 0);
    split_view_divider()->EndResizeWithDivider(
        event_generator->current_screen_location());
    SwitchToTabletMode();
    const auto current_divider_position =
        split_view_divider()
            ->GetDividerBoundsInScreen(/*is_dragging=*/false)
            .x();

    // We need to take into consideration of the variation introduced by the
    // divider shorter side length when calculating using snap ratio, i.e.
    // `kSplitviewDividerShortSideLength / 2`.
    const auto expected_divider_position = std::round(
        work_area_bounds_in_screen.width() * test_case.expected_snap_ratio -
        kSplitviewDividerShortSideLength / 2);

    // Verifies that the bounds of the windows and divider are updated correctly
    // such that snap ratio in the new window layout is expected.
    EXPECT_NEAR(current_divider_position, expected_divider_position,
                /*abs_error=*/1);
    EXPECT_NEAR(float(window1->GetBoundsInScreen().width()) /
                    work_area_bounds_in_screen.width(),
                test_case.expected_snap_ratio, /*abs_error=*/1);
    ExitTabletMode();
  }
}

TEST_F(SnapGroupTest, FeedbackButtonTest) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get(), /*horizontal=*/true);

  SplitViewDividerView* divider_view =
      split_view_divider()->divider_view_for_testing();
  auto* feedback_button = divider_view->feedback_button_for_testing();
  EXPECT_TRUE(feedback_button);

  // Verify that the feedback button is insivible by default.
  EXPECT_FALSE(feedback_button->GetVisible());

  // Test that the feedback button becomes visible upon hover on the divider.
  gfx::Point hover_location =
      split_view_divider_bounds_in_screen().CenterPoint();
  hover_location.Offset(0, -10);

  auto* event_generator = GetEventGenerator();
  event_generator->MoveMouseTo(hover_location);
  EXPECT_TRUE(feedback_button->GetVisible());

  // Test that the feedback button will be invisible when drag starts.
  event_generator->PressLeftButton();
  event_generator->MoveMouseBy(10, 0);
  EXPECT_FALSE(feedback_button->GetVisible());

  // Test that the feedback button will be visible again when drag ends.
  event_generator->ReleaseLeftButton();
  EXPECT_TRUE(feedback_button->GetVisible());

  // Test that open feedback dialog callback will be triggered.
  event_generator->MoveMouseTo(
      feedback_button->GetBoundsInScreen().CenterPoint());
  event_generator->ClickLeftButton();
  EXPECT_EQ(1, static_cast<TestShellDelegate*>(Shell::Get()->shell_delegate())
                   ->open_feedback_dialog_call_count());
}

// Tests that the cursor type gets updated to be resize cursor on mouse hovering
// on the split view divider excluding the feedback button.
TEST_F(SnapGroupTest, CursorUpdateTest) {
  std::unique_ptr<aura::Window> w1(CreateAppWindow());
  std::unique_ptr<aura::Window> w2(CreateAppWindow());
  SnapTwoTestWindows(w1.get(), w2.get(), /*horizontal=*/true);
  auto* divider = split_view_divider();
  ASSERT_TRUE(divider->divider_widget());

  auto divider_bounds = split_view_divider_bounds_in_screen();
  auto outside_point = split_view_divider_bounds_in_screen().CenterPoint();
  outside_point.Offset(-kSplitviewDividerShortSideLength * 5, 0);
  EXPECT_FALSE(divider_bounds.Contains(outside_point));

  auto* cursor_manager = Shell::Get()->cursor_manager();
  cursor_manager->SetCursor(CursorType::kPointer);

  // Test that the default cursor type when mouse is not hovered over the split
  // view divider.
  auto* event_generator = GetEventGenerator();
  event_generator->MoveMouseTo(outside_point);
  EXPECT_TRUE(cursor_manager->IsCursorVisible());
  EXPECT_FALSE(cursor_manager->IsCursorLocked());
  EXPECT_EQ(CursorType::kNull, cursor_manager->GetCursor().type());

  // Test that the cursor changed to resize cursor while hovering over the split
  // view divider.
  const auto delta_vector = gfx::Vector2d(0, -10);
  const gfx::Point cached_hover_point =
      divider_bounds.CenterPoint() + delta_vector;
  event_generator->MoveMouseTo(cached_hover_point);
  EXPECT_EQ(CursorType::kColumnResize, cursor_manager->GetCursor().type());

  // Test that after resizing, the cursor type is still the resize cursor.
  event_generator->PressLeftButton();
  const auto move_vector = gfx::Vector2d(20, 0);
  event_generator->MoveMouseTo(cached_hover_point + move_vector);
  event_generator->ReleaseLeftButton();
  EXPECT_EQ(CursorType::kColumnResize, cursor_manager->GetCursor().type());
  EXPECT_EQ(split_view_divider_bounds_in_screen().CenterPoint() + delta_vector,
            cached_hover_point + move_vector);

  // Test that when hovering over the feedback button, the cursor type changed
  // back to the default type.
  SplitViewDividerView* divider_view =
      split_view_divider()->divider_view_for_testing();
  auto* feedback_button = divider_view->feedback_button_for_testing();
  EXPECT_TRUE(feedback_button);
  event_generator->MoveMouseTo(divider_view->feedback_button_for_testing()
                                   ->GetBoundsInScreen()
                                   .CenterPoint());
  EXPECT_EQ(CursorType::kNull, cursor_manager->GetCursor().type());
}

// -----------------------------------------------------------------------------
// SnapGroupHistogramTest:

using SnapGroupHistogramTest = SnapGroupTest;

// Tests that the pipeline to get snap action source info all the way to be
// stored in the `SplitViewOverviewSession` is working. This test focuses on the
// snap action source with top-usage in clamshell.
TEST_F(SnapGroupHistogramTest, SnapActionSourcePipeline) {
  UpdateDisplay("800x600");
  std::unique_ptr<aura::Window> window1(CreateAppWindow(gfx::Rect(100, 100)));
  std::unique_ptr<aura::Window> window2(CreateAppWindow(gfx::Rect(200, 100)));

  // Drag a window to snap and verify the snap action source info.
  std::unique_ptr<WindowResizer> resizer(CreateWindowResizer(
      window1.get(), gfx::PointF(), HTCAPTION, wm::WINDOW_MOVE_SOURCE_MOUSE));
  resizer->Drag(gfx::PointF(0, 400), /*event_flags=*/0);
  resizer->CompleteDrag();
  resizer.reset();
  SplitViewOverviewSession* split_view_overview_session =
      VerifySplitViewOverviewSession(window1.get());
  EXPECT_EQ(split_view_overview_session->snap_action_source_for_testing(),
            WindowSnapActionSource::kDragWindowToEdgeToSnap);
  MaximizeToClearTheSession(window1.get());

  // Mock snap from window layout menu and verify the snap action source info.
  chromeos::SnapController::Get()->CommitSnap(
      window1.get(), chromeos::SnapDirection::kSecondary,
      chromeos::kDefaultSnapRatio,
      chromeos::SnapController::SnapRequestSource::kWindowLayoutMenu);
  split_view_overview_session = VerifySplitViewOverviewSession(window1.get());
  EXPECT_TRUE(split_view_overview_session);
  EXPECT_EQ(split_view_overview_session->snap_action_source_for_testing(),
            WindowSnapActionSource::kSnapByWindowLayoutMenu);
  MaximizeToClearTheSession(window1.get());

  // Mock snap from window snap button and verify the snap action source info.
  chromeos::SnapController::Get()->CommitSnap(
      window1.get(), chromeos::SnapDirection::kPrimary,
      chromeos::kDefaultSnapRatio,
      chromeos::SnapController::SnapRequestSource::kSnapButton);
  split_view_overview_session = VerifySplitViewOverviewSession(window1.get());
  EXPECT_TRUE(split_view_overview_session);
  EXPECT_EQ(split_view_overview_session->snap_action_source_for_testing(),
            WindowSnapActionSource::kLongPressCaptionButtonToSnap);
  MaximizeToClearTheSession(window1.get());
}

}  // namespace ash
