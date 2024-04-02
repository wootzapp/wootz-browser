// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_MAHI_MAHI_MENU_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_MAHI_MAHI_MENU_VIEW_H_

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/chromeos/mahi/mahi_browser_util.h"
#include "chrome/browser/ui/views/editor_menu/utils/pre_target_handler_view.h"
#include "ui/base/metadata/metadata_header_macros.h"

namespace views {
class FlexLayoutView;
class ImageButton;
class LabelButton;
class UniqueWidgetPtr;
}  // namespace views

namespace chromeos::mahi {

// A bubble style view to show Mahi Menu.
class MahiMenuView : public chromeos::editor_menu::PreTargetHandlerView {
  METADATA_HEADER(MahiMenuView, chromeos::editor_menu::PreTargetHandlerView)

 public:
  MahiMenuView();

  MahiMenuView(const MahiMenuView&) = delete;
  MahiMenuView& operator=(const MahiMenuView&) = delete;

  ~MahiMenuView() override;

  // chromeos::editor_menu::PreTargetHandlerView:
  void RequestFocus() override;

  // Creates a menu widget that contains a `MahiMenuView`, configured with the
  // given `anchor_view_bounds`.
  static views::UniqueWidgetPtr CreateWidget(
      const gfx::Rect& anchor_view_bounds);

  // Updates the bounds of the view according to the given `anchor_view_bounds`.
  void UpdateBounds(const gfx::Rect& anchor_view_bounds);

  views::LabelButton* summary_button_for_test() { return summary_button_; }
  views::LabelButton* outline_button_for_test() { return outline_button_; }

 private:
  // Buttons callback.
  void OnButtonPressed(::mahi::ButtonType button_type);

  std::unique_ptr<views::FlexLayoutView> CreateInputContainer();

  raw_ptr<views::ImageButton> settings_button_ = nullptr;
  raw_ptr<views::LabelButton> summary_button_ = nullptr;
  raw_ptr<views::LabelButton> outline_button_ = nullptr;

  base::WeakPtrFactory<MahiMenuView> weak_ptr_factory_{this};
};

}  // namespace chromeos::mahi

#endif  // CHROME_BROWSER_UI_VIEWS_MAHI_MAHI_MENU_VIEW_H_
