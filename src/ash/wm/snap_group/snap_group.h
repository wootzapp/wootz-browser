// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_WM_SNAP_GROUP_SNAP_GROUP_H_
#define ASH_WM_SNAP_GROUP_SNAP_GROUP_H_

#include <optional>

#include "ash/wm/splitview/layout_divider_controller.h"
#include "ash/wm/splitview/split_view_divider.h"
#include "ash/wm/window_positioning_utils.h"
#include "ash/wm/window_state_observer.h"
#include "base/memory/raw_ptr.h"
#include "base/time/time.h"
#include "ui/aura/window_observer.h"
#include "ui/display/display_observer.h"
#include "ui/wm/public/activation_change_observer.h"

namespace aura {
class Window;
}  // namespace aura

namespace ash {

class ScopedOverviewHideWindows;

// Takes over snap group management after the creation in `SplitViewController`.
// Observes window and window state changes. Implements the
// `LayoutDividerController` interface to allow synchronized resizing of the
// windows within the group. The creation will eventually be done in
// `SnapGroupController` after the major window layout architecture is complete.
// `sticky_creation_time` specifies the creation time of a prior Snap Group from
// which the current one was derived using the Snap to Replace feature.  If this
// value is not specified, it means that the current Snap Group was not created
// from a previous one.
class SnapGroup : public aura::WindowObserver,
                  public WindowStateObserver,
                  public LayoutDividerController,
                  public display::DisplayObserver,
                  public wm::ActivationChangeObserver {
 public:
  SnapGroup(aura::Window* window1,
            aura::Window* window2,
            std::optional<base::TimeTicks> sticky_creation_time);
  SnapGroup(const SnapGroup&) = delete;
  SnapGroup& operator=(const SnapGroup&) = delete;
  ~SnapGroup() override;

  aura::Window* window1() const { return window1_; }
  aura::Window* window2() const { return window2_; }
  SplitViewDivider* snap_group_divider() { return &snap_group_divider_; }

  // Cleans up prior to deletion. Must be called before the object is destroyed.
  void Shutdown();

  // These functions return the snapped window in the specified snap position
  // (left/top or right/bottom) based on the display's orientation.
  //
  // In primary screen orientation:
  //  - `GetPhysicallyLeftOrTopWindow()` returns the `window1_`;
  //  - `GetPhysicallyRightOrBottomWindow()` returns the `window2_`.
  //
  // In non-primary screen orientation:
  //  - `GetPhysicallyLeftOrTopWindow()` returns the `window2_`;
  //  - `GetPhysicallyRightOrBottomWindow()` returns the `window1_`.
  aura::Window* GetPhysicallyLeftOrTopWindow();
  aura::Window* GetPhysicallyRightOrBottomWindow();

  // Gets the window snapped at `snap_type`.
  const aura::Window* GetWindowOfSnapViewType(SnapViewType snap_type) const;

  void ShowDivider();
  void HideDivider();

  // Returns true if snap group is configured in a vertical split-screen layout.
  // Returns false otherwise.
  bool IsSnapGroupLayoutHorizontal();

  // Unified helper to handle mouse/touch events received from
  // `ToplevelWindowEventHandler` to hide `snap_group_divider_` when either of
  // the windows becomes unsnapped.
  void OnLocatedEvent(ui::LocatedEvent* event);

  // Returns the topmost window in the snap group.
  aura::Window* GetTopMostWindowInGroup() const;

  // Minimizes the windows in the snap group.
  void MinimizeWindows();

  // aura::WindowObserver:
  void OnWindowDestroying(aura::Window* window) override;
  void OnWindowParentChanged(aura::Window* window,
                             aura::Window* parent) override;

  // WindowStateObserver:
  void OnPreWindowStateTypeChange(WindowState* window_state,
                                  chromeos::WindowStateType old_type) override;

  // LayoutDividerController:
  aura::Window* GetRootWindow() override;
  void StartResizeWithDivider(const gfx::Point& location_in_screen) override;
  void UpdateResizeWithDivider(const gfx::Point& location_in_screen) override;
  bool EndResizeWithDivider(const gfx::Point& location_in_screen) override;
  void OnResizeEnding() override;
  void OnResizeEnded() override;
  void SwapWindows() override;
  gfx::Rect GetSnappedWindowBoundsInScreen(
      SnapPosition snap_position,
      aura::Window* window_for_minimum_size,
      float snap_ratio,
      bool account_for_divider_width) const override;
  SnapPosition GetPositionOfSnappedWindow(
      const aura::Window* window) const override;

  // display::DisplayObserver:
  void OnDisplayMetricsChanged(const display::Display& display,
                               uint32_t metrics) override;

  // wm::ActivationChangeObserver:
  void OnWindowActivated(ActivationReason reason,
                         aura::Window* gained_active,
                         aura::Window* lost_active) override;

 private:
  friend class SnapGroupController;

  // Observes the windows that are added in the `this`.
  void StartObservingWindows();

  // Stops observing the windows when the `this` gets destructed.
  void StopObservingWindows();

  // Updates the bounds of windows in `this`. 'account_for_divider_width'
  // determines whether to adjust the snapped windows' bounds to accommodate the
  // divider.
  void UpdateGroupWindowsBounds(bool account_for_divider_width);

  // Updates the bounds of the given snapped window.
  // - Adjusts window bounds to accommodate the divider if
  // 'account_for_divider_width' is true.
  // - Prioritizes 'snap_ratio' (predetermined snap ratio for the snapped
  // window) over the snap ratio retrieved from the window's state if set.
  void UpdateSnappedWindowBounds(aura::Window* window,
                                 bool account_for_divider_width,
                                 std::optional<float> snap_ratio);

  // Adjusts snapped windows and divider bounds to match the given
  // `primary_snap_ratio`. Note the windows and divider must fit the work area.
  void ApplyPrimarySnapRatio(float primary_snap_ratio);

  // Refreshes the window and divider bounds. Note `this` may be destroyed if
  // the windows are no longer valid for a snap group.
  void RefreshSnapGroup();

  // Hides scoped windows in a snap group in partial overview, restores their
  // visibility when partial overview ends.
  void OnOverviewModeStarting();
  void OnOverviewModeEnding();

  // True while the snap group is being moved due to parent window change.
  bool is_moving_snap_group_ = false;

  // Within a snap group, the divider appears as a widget positioned between the
  // two snapped windows. It serves a dual purpose: signifying the group
  // connection and enabling simultaneous resizing of both windows. In terms of
  // stacking order, `snap_group_divider_` is the bottom-most transient child of
  // the top-most window of the two windows.
  SplitViewDivider snap_group_divider_;

  std::unique_ptr<ScopedOverviewHideWindows> hide_windows_in_partial_overview_;

  // Window that has state type of `chromeos::WindowStateType::kPrimarySnapped`.
  // Physically it is left/top for primary screen orientation, however it will
  // be right/bottom for secondary screen orientation.
  raw_ptr<aura::Window> window1_;

  // Window that has state type of
  // `chromeos::WindowStateType::kSecondarySnapped`. Physically it is
  // right/bottom for primary screen orientation, however it will be left/top
  // for secondary screen orientation.
  raw_ptr<aura::Window> window2_;

  // Tracks the timestamp of the original Snap Group's creation time, preserved
  // when using 'Snap to Replace'.
  const base::TimeTicks carry_over_creation_time_;

  // Tracks the actual creation time of `this` without carrying over the
  // creation time from a previous Snap Group when using Snap to Replace i.e.
  // the two snapped windows remain unchanged throughout its existence.
  const base::TimeTicks actual_creation_time_;

  // True if the shutting down process has been triggered.
  bool is_shutting_down_ = false;
};

}  // namespace ash

#endif  // ASH_WM_SNAP_GROUP_SNAP_GROUP_H_
