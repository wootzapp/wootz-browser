// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_GAME_DASHBOARD_GAME_DASHBOARD_MAIN_MENU_VIEW_H_
#define ASH_GAME_DASHBOARD_GAME_DASHBOARD_MAIN_MENU_VIEW_H_

#include <string>

#include "ash/ash_export.h"
#include "base/memory/raw_ptr.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/bubble/bubble_dialog_delegate_view.h"

namespace views {
class BoxLayoutView;
}  // namespace views

namespace ash {

class AnchoredNudge;
class FeatureTile;
class GameDashboardContext;
class IconButton;
class PillButton;
class Switch;

// GameDashboardMainMenuView is the expanded menu view attached to the game
// dashboard button.
class ASH_EXPORT GameDashboardMainMenuView
    : public views::BubbleDialogDelegateView {
  METADATA_HEADER(GameDashboardMainMenuView, views::BubbleDialogDelegateView)

 public:
  explicit GameDashboardMainMenuView(GameDashboardContext* context);

  GameDashboardMainMenuView(const GameDashboardMainMenuView&) = delete;
  GameDashboardMainMenuView& operator=(const GameDashboardMainMenuView) =
      delete;
  ~GameDashboardMainMenuView() override;

  // Updates `record_game_tile_` to the stop button. Called when
  // `CaptureModeController` has started a recording session. If
  // `is_recording_game_window` is true, then the `context_`'s game_window is
  // being recorded.
  void OnRecordingStarted(bool is_recording_game_window);

  // Updates `record_game_tile_` to the default state. Called when
  // `CaptureModeController` has ended a recording session or was aborted.
  void OnRecordingEnded();

  // Updates the `record_game_tile_`'s sub-label with `duration`, showing the
  // recording duration.
  void UpdateRecordingDuration(const std::u16string& duration);

  // Updates the `game_controls_tile_` states, sub-label and tooltip text.
  void UpdateGameControlsTile();

 private:
  friend class GameDashboardContextTestApi;

  class GameControlsDetailsRow;

  // Callbacks for the tiles and buttons in the main menu view.
  // Handles showing and hiding the toolbar.
  void OnToolbarTilePressed();
  // Handles toggling the game recording.
  void OnRecordGameTilePressed();
  // Handles taking a screenshot of the game window when pressed.
  void OnScreenshotTilePressed();

  // Callbacks for the buttons in the settings view.
  // Handles going back to the main menu view when pressed.
  void OnSettingsBackButtonPressed();
  // Handles toggling the welcome dialog preference and updating the Switch
  // state.
  void OnWelcomeDialogSwitchPressed();

  // Handles functions for Game Controls buttons.
  void OnGameControlsTilePressed();

  // Handles when the Screen Size Settings is pressed.
  void OnScreenSizeSettingsButtonPressed();
  // Opens the feedback form with preset information.
  void OnFeedbackButtonPressed();
  // Opens the help center for more info about Game Dashboard.
  void OnHelpButtonPressed();
  // Opens up the Game Dashboard Settings.
  void OnSettingsButtonPressed();

  // Creates the `main_menu_container_` and adds all rows of views pertaining to
  // the main menu view to it.
  void AddMainMenuViews();

  // Adds a row of shortcut tiles to the `main_menu_container_` for users to
  // quickly access common functionality.
  void AddShortcutTilesRow();

  // Adds feature details rows, for example, including Game Controls or window
  // size to the `main_menu_container_`.
  void MaybeAddArcFeatureRows();

  // Adds Game Controls feature tile in `container` if it is the ARC game window
  // and Game Controls is available.
  void MaybeAddGameControlsTile(views::View* container);

  // Adds menu controls row for Game Controls.
  void AddGameControlsDetailsRow(views::View* container);

  // Adds a row to access a settings page controlling the screen size if the
  // given game window is an ARC app.
  void AddScreenSizeSettingsRow(views::View* container);

  // Adds the dashboard cluster (containing feedback, settings, and help
  // buttons) to the `main_menu_container_`.
  void AddUtilityClusterRow();

  // views::View:
  void VisibilityChanged(views::View* starting_from, bool is_visible) override;

  // Updates the `record_game_tile_` UI. If `is_recording_game_window` is
  // true, then the tile will change to a stop button, otherwise it will show
  // the default UI.
  void UpdateRecordGameTile(bool is_recording_game_window);

  // Adds pulse animation and an education nudge for
  // `game_controls_setup_button_` if it exists and `is_o4c` is false. `is_o4c`
  // is true if the ARC game is optimized for ChromeOS.
  void MaybeDecorateSetupButton(bool is_o4c);
  // Performs pulse animation for `game_controls_setup_button_`.
  void PerformPulseAnimationForSetupButton(int pulse_count);
  // Shows education nudge for `game_controls_setup_button_`.
  void ShowNudgeForSetupButton();

  // Creates the `settings_view_container_` and adds all rows pertaining to the
  // settings view to it.
  void AddSettingsViews();

  // Adds a row displaying the title and back button.
  void AddSettingsTitleRow();

  // Adds a row displaying the welcome dialog setting.
  void AddWelcomeDialogSettingsRow();

  // Updates the accessible name for the `welcome_dialog_settings_switch_`.
  void OnWelcomeDialogSwitchStateChanged(bool is_enabled);

  // Gets UI elements from Game Controls details row.
  PillButton* GetGameControlsSetupButton();
  Switch* GetGameControlsFeatureSwitch();

  // For test to access the nudge ID in the anonymous namespace.
  AnchoredNudge* GetGameControlsSetupNudgeForTesting();

  // views::Views:
  void OnThemeChanged() override;

  // Allows this class to access `GameDashboardContext` owned functions/objects.
  const raw_ptr<GameDashboardContext> context_;

  // Container holding all views displayed in the main menu view.
  raw_ptr<views::BoxLayoutView> main_menu_container_ = nullptr;

  // Container holding all views displayed in the settings view.
  raw_ptr<views::BoxLayoutView> settings_view_container_ = nullptr;

  // Shortcut Tiles:
  // Toolbar button to toggle the `GameDashboardToolbarView`.
  raw_ptr<FeatureTile> toolbar_tile_ = nullptr;

  // Game controls button to toggle the Game Controls UI hint.
  raw_ptr<FeatureTile> game_controls_tile_ = nullptr;

  // Record game button to start recording the game window. This will open the
  // screen capture tool, allowing the user to select recording options.
  raw_ptr<FeatureTile> record_game_tile_ = nullptr;

  // Game Controls details row to configure Game Controls.
  raw_ptr<GameControlsDetailsRow> game_controls_details_ = nullptr;

  // The `Switch` representing toggling the welcome dialog within the settings.
  raw_ptr<Switch> welcome_dialog_settings_switch_ = nullptr;

  // Back button in the `settings_view_container_`. Visible for testing.
  raw_ptr<IconButton> settings_view_back_button_ = nullptr;

  // Layer for setup button pulse animation.
  std::unique_ptr<ui::Layer> gc_setup_button_pulse_layer_;
};

}  // namespace ash

#endif  // ASH_GAME_DASHBOARD_GAME_DASHBOARD_MAIN_MENU_VIEW_H_
