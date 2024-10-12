// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_CHROMEOS_MAGIC_BOOST_MAGIC_BOOST_OPT_IN_CARD_H_
#define CHROME_BROWSER_UI_CHROMEOS_MAGIC_BOOST_MAGIC_BOOST_OPT_IN_CARD_H_

#include "base/memory/raw_ptr.h"
#include "chrome/browser/ui/views/editor_menu/utils/pre_target_handler_view.h"
#include "ui/base/metadata/metadata_header_macros.h"

namespace views {
class MdTextButton;
class UniqueWidgetPtr;
}  // namespace views

namespace chromeos {

// The Magic Boost opt-in card view.
class MagicBoostOptInCard : public chromeos::editor_menu::PreTargetHandlerView {
  METADATA_HEADER(MagicBoostOptInCard,
                  chromeos::editor_menu::PreTargetHandlerView)

 public:
  MagicBoostOptInCard();
  MagicBoostOptInCard(const MagicBoostOptInCard&) = delete;
  MagicBoostOptInCard& operator=(const MagicBoostOptInCard&) = delete;
  ~MagicBoostOptInCard() override;

  // Creates a widget that contains a `MagicBoostOptInCard`, configured with the
  // given `anchor_view_bounds`.
  static views::UniqueWidgetPtr CreateWidget(
      const gfx::Rect& anchor_view_bounds);

  // Returns the host widget's name.
  static const char* GetWidgetName();

  // Updates the bounds of the widget to the given `anchor_view_bounds`.
  void UpdateWidgetBounds(const gfx::Rect& anchor_view_bounds);

  // views::View:
  void RequestFocus() override;

 private:
  raw_ptr<views::MdTextButton> secondary_button_ = nullptr;
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_CHROMEOS_MAGIC_BOOST_MAGIC_BOOST_OPT_IN_CARD_H_
