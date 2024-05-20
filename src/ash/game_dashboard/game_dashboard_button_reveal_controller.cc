// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/game_dashboard/game_dashboard_button_reveal_controller.h"

#include "ash/game_dashboard/game_dashboard_context.h"
#include "ash/game_dashboard/game_dashboard_utils.h"
#include "ash/wm/window_state.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "chromeos/ui/frame/immersive/immersive_fullscreen_controller.h"
#include "ui/aura/window.h"
#include "ui/display/screen.h"
#include "ui/gfx/geometry/point.h"
#include "ui/views/animation/animation_builder.h"
#include "ui/views/widget/widget.h"

namespace ash {

namespace {

// The delay in between the mouse stopping at the top edge of the screen and the
// game dashboard button revealing.
constexpr base::TimeDelta kMouseRevealDelay = base::Milliseconds(200);

constexpr base::TimeDelta kSlideAnimationDuration = base::Milliseconds(200);
constexpr base::TimeDelta kNoSlideAnimationDuration = base::Milliseconds(0);

}  // namespace

GameDashboardButtonRevealController::GameDashboardButtonRevealController(
    GameDashboardContext* context)
    : context_(context) {
  DCHECK(context_);
  context_->game_window()->AddPreTargetHandler(
      this, ui::EventTarget::Priority::kSystem);
  UpdateVisibility(/*target_visibility=*/false, /*animate=*/false);
}

GameDashboardButtonRevealController::~GameDashboardButtonRevealController() {
  UpdateVisibility(/*target_visibility=*/true, /*animate=*/false);
  context_->game_window()->RemovePreTargetHandler(this);
}

void GameDashboardButtonRevealController::UpdateVisibility(
    bool target_visibility,
    bool animate) {
  context_->SetGameDashboardButtonVisibility(/*visible=*/true);
  views::AnimationBuilder()
      .SetPreemptionStrategy(ui::LayerAnimator::PreemptionStrategy::
                                 IMMEDIATELY_ANIMATE_TO_NEW_TARGET)
      .OnEnded(
          base::BindOnce(&GameDashboardButtonRevealController::OnAnimationEnd,
                         weak_ptr_factory_.GetWeakPtr(), target_visibility))
      .Once()
      .SetDuration(animate ? kSlideAnimationDuration
                           : kNoSlideAnimationDuration)
      .SetTransform(
          context_->game_dashboard_button_widget()->GetLayer(),
          target_visibility
              ? gfx::Transform()
              : gfx::Transform::MakeTranslation(
                    /*tx=*/0,
                    /*ty=*/-game_dashboard_utils::GetFrameHeaderHeight(
                        context_->game_window())),
          gfx::Tween::EASE_OUT);
}

void GameDashboardButtonRevealController::OnMouseEvent(ui::MouseEvent* event) {
  const auto event_type = event->type();
  if (event_type != ui::ET_MOUSE_MOVED && event_type != ui::ET_MOUSE_RELEASED &&
      event_type != ui::ET_MOUSE_CAPTURE_CHANGED) {
    return;
  }

  // Start `top_edge_hover_timer_` to show the game dashboard button, if the
  // mouse cursor is within the top edge of the game window in fullscreen.
  const gfx::Point mouse_screen_location =
      event->target()->GetScreenLocation(*event);
  if (IsMouseWithinButtonRevealBounds(mouse_screen_location)) {
    if (!top_edge_hover_timer_.IsRunning()) {
      top_edge_hover_timer_.Start(
          FROM_HERE, kMouseRevealDelay, this,
          &GameDashboardButtonRevealController::OnTopEdgeHoverTimeout);
    }
    return;
  }

  // The mouse cursor is not within the top edge of the window.
  top_edge_hover_timer_.Stop();
  // If the main menu is closed, try to hide the game dashboard button.
  if (CanHideGameDashboardButton(mouse_screen_location)) {
    UpdateVisibility(/*target_visibility=*/false, /*animate=*/true);
  }
}

bool GameDashboardButtonRevealController::CanShowGameDashboardButton(
    const gfx::Point& mouse_screen_location) {
  return !context_->game_dashboard_button_widget()->IsVisible() &&
         IsMouseWithinButtonRevealBounds(mouse_screen_location);
}

bool GameDashboardButtonRevealController::CanHideGameDashboardButton(
    const gfx::Point& mouse_screen_location) {
  return !context_->IsMainMenuOpen() &&
         context_->game_dashboard_button_widget()->IsVisible() &&
         IsMouseOutsideHeaderBounds(mouse_screen_location);
}

bool GameDashboardButtonRevealController::IsMouseWithinButtonRevealBounds(
    const gfx::Point& mouse_screen_location) {
  gfx::Rect button_reveal_bounds = context_->game_window()->GetBoundsInScreen();
  button_reveal_bounds.set_height(
      chromeos::ImmersiveFullscreenController::kMouseRevealBoundsHeight);
  return button_reveal_bounds.Contains(mouse_screen_location);
}

bool GameDashboardButtonRevealController::IsMouseOutsideHeaderBounds(
    const gfx::Point& mouse_screen_location) {
  gfx::Rect header_bounds = context_->game_window()->GetBoundsInScreen();
  header_bounds.set_height(
      game_dashboard_utils::GetFrameHeaderHeight(context_->game_window()));
  // Allow the cursor to move slightly off the top-of-window views before hiding
  // the button. This matches the behavior in
  // `ImmersiveFullscreenController::UpdateLocatedEventRevealedLock()`.
  header_bounds.Inset(
      gfx::Insets::TLBR(0, 0,
                        -chromeos::ImmersiveFullscreenController::
                            kImmersiveFullscreenTopEdgeInset,
                        0));
  return !header_bounds.Contains(mouse_screen_location);
}

void GameDashboardButtonRevealController::OnTopEdgeHoverTimeout() {
  if (CanShowGameDashboardButton(
          display::Screen::GetScreen()->GetCursorScreenPoint())) {
    UpdateVisibility(/*target_visibility=*/true, /*animate=*/true);
  }
}

void GameDashboardButtonRevealController::OnAnimationEnd(
    bool target_visibility) {
  if (!target_visibility) {
    // The slide up animation has ended. Make the Game Dashboard button
    // widget not visible.
    context_->SetGameDashboardButtonVisibility(/*visible=*/false);
  }
}

}  // namespace ash
