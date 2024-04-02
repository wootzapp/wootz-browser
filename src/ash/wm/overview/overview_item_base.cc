// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wm/overview/overview_item_base.h"

#include <vector>

#include "ash/public/cpp/window_properties.h"
#include "ash/shell.h"
#include "ash/wm/desks/desks_util.h"
#include "ash/wm/overview/overview_constants.h"
#include "ash/wm/overview/overview_controller.h"
#include "ash/wm/overview/overview_grid.h"
#include "ash/wm/overview/overview_grid_event_handler.h"
#include "ash/wm/overview/overview_group_item.h"
#include "ash/wm/overview/overview_item.h"
#include "ash/wm/overview/overview_session.h"
#include "ash/wm/overview/overview_utils.h"
#include "ash/wm/overview/scoped_overview_animation_settings.h"
#include "ash/wm/snap_group/snap_group.h"
#include "ash/wm/snap_group/snap_group_controller.h"
#include "ash/wm/splitview/split_view_utils.h"
#include "ash/wm/window_util.h"
#include "ash/wm/wm_constants.h"
#include "base/memory/raw_ptr.h"

namespace ash {

OverviewItemBase::OverviewItemBase(OverviewSession* overview_session,
                                   OverviewGrid* overview_grid,
                                   aura::Window* root_window)
    : root_window_(root_window),
      overview_session_(overview_session),
      overview_grid_(overview_grid) {}

OverviewItemBase::~OverviewItemBase() = default;

// static
std::unique_ptr<OverviewItemBase> OverviewItemBase::Create(
    aura::Window* window,
    OverviewSession* overview_session,
    OverviewGrid* overview_grid) {
  SnapGroupController* snap_group_controller = SnapGroupController::Get();
  if (snap_group_controller) {
    if (SnapGroup* snap_group =
            snap_group_controller->GetSnapGroupForGivenWindow(window)) {
      return std::make_unique<OverviewGroupItem>(
          std::vector<raw_ptr<aura::Window, VectorExperimental>>{
              snap_group->window1(), snap_group->window2()},
          overview_session, overview_grid);
    }
  }

  return std::make_unique<OverviewItem>(window, overview_session, overview_grid,
                                        /*destruction_delegate=*/overview_grid,
                                        /*event_handler_delegate=*/nullptr,
                                        /*eligible_for_shadow_config=*/true);
}

bool OverviewItemBase::IsDragItem() const {
  return overview_session_->GetCurrentDraggedOverviewItem() == this;
}

void OverviewItemBase::RefreshShadowVisuals(bool shadow_visible) {
  // Shadow is normally turned off during animations and reapplied when on
  // animation complete. On destruction, `shadow_` is cleaned up before
  // `transform_window_`, which may call this function, so early exit if
  // `shadow_` is nullptr.
  if (!shadow_) {
    return;
  }

  const gfx::RectF shadow_bounds_in_screen = target_bounds_;
  auto* shadow_layer = shadow_->GetLayer();
  if (!shadow_visible || shadow_bounds_in_screen.IsEmpty()) {
    shadow_layer->SetVisible(false);
    return;
  }

  shadow_layer->SetVisible(true);

  gfx::Rect shadow_content_bounds(
      gfx::ToRoundedRect(shadow_bounds_in_screen).size());
  shadow_->SetContentBounds(shadow_content_bounds);
  shadow_->SetRoundedCornerRadius(
      window_util::GetMiniWindowRoundedCornerRadius());
}

void OverviewItemBase::UpdateShadowTypeForDrag(bool is_dragging) {
  shadow_->SetType(is_dragging ? kDraggedShadowType : kDefaultShadowType);
}

void OverviewItemBase::HandleGestureEventForTabletModeLayout(
    ui::GestureEvent* event,
    OverviewItemBase* event_source_item) {
  const gfx::PointF location = event->details().bounding_box_f().CenterPoint();
  OverviewGridEventHandler* grid_event_handler =
      overview_grid()->grid_event_handler();
  const bool is_drag_item = IsDragItem();
  switch (event->type()) {
    case ui::ET_SCROLL_FLING_START:
      if (is_drag_item) {
        HandleFlingStartEvent(location, event->details().velocity_x(),
                              event->details().velocity_y());
      } else {
        grid_event_handler->OnGestureEvent(event);
      }
      break;
    case ui::ET_GESTURE_SCROLL_BEGIN:
      if (std::abs(event->details().scroll_y_hint()) >
          std::abs(event->details().scroll_x_hint())) {
        HandlePressEvent(location, /*from_touch_gesture=*/true,
                         event_source_item);
      } else {
        grid_event_handler->OnGestureEvent(event);
      }
      break;
    case ui::ET_GESTURE_SCROLL_UPDATE:
      if (is_drag_item) {
        HandleDragEvent(location);
      } else {
        grid_event_handler->OnGestureEvent(event);
      }
      break;
    case ui::ET_GESTURE_SCROLL_END:
      if (is_drag_item) {
        HandleReleaseEvent(location);
      } else {
        grid_event_handler->OnGestureEvent(event);
      }
      break;
    case ui::ET_GESTURE_LONG_PRESS:
      HandlePressEvent(location, /*from_touch_gesture=*/true,
                       event_source_item);
      HandleLongPressEvent(location);
      break;
    case ui::ET_GESTURE_TAP:
      HandleTapEvent(location, event_source_item);
      break;
    case ui::ET_GESTURE_END:
      HandleGestureEndEvent();
      break;
    default:
      grid_event_handler->OnGestureEvent(event);
      break;
  }
}

void OverviewItemBase::HandleMouseEvent(const ui::MouseEvent& event,
                                        OverviewItemBase* event_source_item) {
  if (!overview_session_->CanProcessEvent(this, /*from_touch_gesture=*/false)) {
    return;
  }

  // `event.target()` will be null if we use search+space on this item with
  // chromevox on. Accessibility API will synthesize a mouse event in that case
  // without a target. We just use the centerpoint of the item so that
  // search+space will select the item, leaving overview.
  const gfx::PointF screen_location =
      event.target() ? event.target()->GetScreenLocationF(event)
                     : gfx::PointF(GetWindowsUnionScreenBounds().CenterPoint());
  switch (event.type()) {
    case ui::ET_MOUSE_PRESSED:
      HandlePressEvent(screen_location, /*from_touch_gesture=*/false,
                       event_source_item);
      break;
    case ui::ET_MOUSE_RELEASED:
      HandleReleaseEvent(screen_location);
      break;
    case ui::ET_MOUSE_DRAGGED:
      HandleDragEvent(screen_location);
      break;
    default:
      NOTREACHED();
      break;
  }
}

void OverviewItemBase::HandleGestureEvent(ui::GestureEvent* event,
                                          OverviewItemBase* event_source_item) {
  if (!overview_session_->CanProcessEvent(this, /*from_touch_gesture=*/true)) {
    event->StopPropagation();
    event->SetHandled();
    return;
  }

  if (ShouldUseTabletModeGridLayout()) {
    HandleGestureEventForTabletModeLayout(event, event_source_item);
    return;
  }

  const gfx::PointF location = event->details().bounding_box_f().CenterPoint();
  switch (event->type()) {
    case ui::ET_GESTURE_TAP_DOWN:
      HandlePressEvent(location, /*from_touch_gesture=*/true,
                       event_source_item);
      break;
    case ui::ET_GESTURE_SCROLL_UPDATE:
      HandleDragEvent(location);
      break;
    case ui::ET_SCROLL_FLING_START:
      HandleFlingStartEvent(location, event->details().velocity_x(),
                            event->details().velocity_y());
      break;
    case ui::ET_GESTURE_SCROLL_END:
      HandleReleaseEvent(location);
      break;
    case ui::ET_GESTURE_LONG_PRESS:
      HandleLongPressEvent(location);
      break;
    case ui::ET_GESTURE_TAP:
      HandleTapEvent(location, event_source_item);
      break;
    case ui::ET_GESTURE_END:
      HandleGestureEndEvent();
      break;
    default:
      break;
  }
}

views::Widget::InitParams OverviewItemBase::CreateOverviewItemWidgetParams(
    aura::Window* parent_window,
    const std::string& widget_name,
    bool accept_events) const {
  views::Widget::InitParams params;
  params.type = views::Widget::InitParams::TYPE_POPUP;
  params.ownership = views::Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET;
  params.opacity = views::Widget::InitParams::WindowOpacity::kTranslucent;
  params.name = widget_name;
  params.activatable = views::Widget::InitParams::Activatable::kDefault;
  params.accept_events = accept_events;
  params.parent = parent_window;
  params.init_properties_container.SetProperty(kHideInDeskMiniViewKey, true);
  return params;
}

void OverviewItemBase::ConfigureTheShadow() {
  shadow_ = SystemShadow::CreateShadowOnNinePatchLayer(kDefaultShadowType);
  auto* shadow_layer = shadow_->GetLayer();
  auto* widget_layer = item_widget_->GetLayer();
  widget_layer->Add(shadow_layer);
  widget_layer->StackAtBottom(shadow_layer);
  shadow_->ObserveColorProviderSource(item_widget_.get());
}

void OverviewItemBase::HandleDragEvent(const gfx::PointF& location_in_screen) {
  if (IsDragItem()) {
    overview_session_->Drag(this, location_in_screen);
  }
}

void OverviewItemBase::HandlePressEvent(const gfx::PointF& location_in_screen,
                                        bool from_touch_gesture,
                                        OverviewItemBase* event_source_item) {
  // No need to start the drag again if already in a drag. This can happen if we
  // switch fingers midway through a drag.
  if (!IsDragItem()) {
    StartDrag();
    overview_session_->InitiateDrag(this, location_in_screen,
                                    /*is_touch_dragging=*/from_touch_gesture,
                                    event_source_item);
  }
}

void OverviewItemBase::HandleReleaseEvent(
    const gfx::PointF& location_in_screen) {
  if (IsDragItem()) {
    overview_session_->CompleteDrag(this, location_in_screen);
  }
}

void OverviewItemBase::HandleLongPressEvent(
    const gfx::PointF& location_in_screen) {
  if (IsDragItem() &&
      (ShouldAllowSplitView() || (desks_util::ShouldDesksBarBeCreated() &&
                                  overview_grid_->IsDesksBarViewActive()))) {
    overview_session_->StartNormalDragMode(location_in_screen);
  }
}

void OverviewItemBase::HandleFlingStartEvent(
    const gfx::PointF& location_in_screen,
    float velocity_x,
    float velocity_y) {
  overview_session_->Fling(this, location_in_screen, velocity_x, velocity_y);
}

void OverviewItemBase::HandleTapEvent(const gfx::PointF& location_in_screen,
                                      OverviewItemBase* event_source_item) {
  if (IsDragItem()) {
    overview_session_->ActivateDraggedWindow();
    return;
  }

  overview_session_->SelectWindow(event_source_item);
}

void OverviewItemBase::HandleGestureEndEvent() {
  if (IsDragItem()) {
    // Gesture end events come from a long press getting canceled. Long press
    // alters the stacking order, so on gesture end, make sure we restore the
    // stacking order on the next reposition.
    set_should_restack_on_animation_end(true);
    overview_session_->ResetDraggedWindowGesture();
  }
}

}  // namespace ash
