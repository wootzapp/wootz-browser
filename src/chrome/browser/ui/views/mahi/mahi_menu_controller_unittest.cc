// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/mahi/mahi_menu_controller.h"

#include <memory>
#include <string>

#include "base/test/scoped_feature_list.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/chromeos/mahi/test/fake_mahi_web_contents_manager.h"
#include "chrome/browser/chromeos/mahi/test/scoped_mahi_web_contents_manager_for_testing.h"
#include "chrome/browser/ui/chromeos/read_write_cards/read_write_cards_ui_controller.h"
#include "chrome/browser/ui/views/editor_menu/utils/utils.h"
#include "chrome/test/views/chrome_views_test_base.h"
#include "chromeos/constants/chromeos_features.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/geometry/rect.h"

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "ash/constants/ash_switches.h"
#include "base/auto_reset.h"
#include "base/command_line.h"
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

namespace chromeos::mahi {

using ::testing::IsNull;

class MahiMenuControllerTest : public ChromeViewsTestBase {
 public:
  MahiMenuControllerTest() {
    menu_controller_ =
        std::make_unique<MahiMenuController>(read_write_cards_ui_controller_);

    scoped_mahi_web_contents_manager_ =
        std::make_unique<::mahi::ScopedMahiWebContentsManagerForTesting>(
            &fake_mahi_web_contents_manager_);
    // Sets the focused page's distillability to true so that it does not block
    // the menu widget's display.
    ChangePageDistillability(true);
  }

  MahiMenuControllerTest(const MahiMenuControllerTest&) = delete;
  MahiMenuControllerTest& operator=(const MahiMenuControllerTest&) = delete;

  ~MahiMenuControllerTest() override = default;

  MahiMenuController* menu_controller() { return menu_controller_.get(); }

  void ChangePageDistillability(bool value) {
    fake_mahi_web_contents_manager_.set_focused_web_content_is_distillable(
        value);
  }

 protected:
  ReadWriteCardsUiController read_write_cards_ui_controller_;

 private:
  std::unique_ptr<MahiMenuController> menu_controller_;

  ::mahi::FakeMahiWebContentsManager fake_mahi_web_contents_manager_;
  std::unique_ptr<::mahi::ScopedMahiWebContentsManagerForTesting>
      scoped_mahi_web_contents_manager_;

  base::test::ScopedFeatureList feature_list_{chromeos::features::kMahi};
#if BUILDFLAG(IS_CHROMEOS_ASH)
  base::AutoReset<bool> ignore_mahi_secret_key_ =
      ash::switches::SetIgnoreMahiSecretKeyForTest();
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)
};

// Tests the behavior of the controller when there's no text selected when
// `OnTextAvailable()` is triggered.
TEST_F(MahiMenuControllerTest, TextNotSelected) {
  EXPECT_FALSE(menu_controller()->menu_widget_for_test());

  // Menu widget should show when text is displayed.
  menu_controller()->OnTextAvailable(/*anchor_bounds=*/gfx::Rect(),
                                     /*selected_text=*/"",
                                     /*surrounding_text=*/"");

  EXPECT_TRUE(menu_controller()->menu_widget_for_test());
  EXPECT_TRUE(menu_controller()->menu_widget_for_test()->IsVisible());

  // Menu widget should hide when dismissed.
  menu_controller()->OnDismiss(/*is_other_command_executed=*/false);
  EXPECT_FALSE(menu_controller()->menu_widget_for_test());

  // If page is not distillable, then menu widget should not be triggered.
  ChangePageDistillability(false);
  menu_controller()->OnTextAvailable(/*anchor_bounds=*/gfx::Rect(),
                                     /*selected_text=*/"",
                                     /*surrounding_text=*/"");

  EXPECT_FALSE(menu_controller()->menu_widget_for_test());
}

// Tests the behavior of the controller when `OnAnchorBoundsChanged()` is
// triggered.
TEST_F(MahiMenuControllerTest, BoundsChanged) {
  EXPECT_FALSE(menu_controller()->menu_widget_for_test());

  gfx::Rect anchor_bounds = gfx::Rect(50, 50, 25, 100);
  menu_controller()->OnTextAvailable(anchor_bounds,
                                     /*selected_text=*/"",
                                     /*surrounding_text=*/"");
  auto* widget = menu_controller()->menu_widget_for_test();
  EXPECT_TRUE(widget);

  EXPECT_EQ(editor_menu::GetEditorMenuBounds(anchor_bounds,
                                             widget->GetContentsView()),
            widget->GetRestoredBounds());

  anchor_bounds = gfx::Rect(0, 50, 55, 80);

  // Widget should change bounds accordingly.
  menu_controller()->OnAnchorBoundsChanged(anchor_bounds);
  EXPECT_EQ(editor_menu::GetEditorMenuBounds(anchor_bounds,
                                             widget->GetContentsView()),
            widget->GetRestoredBounds());
}

// Tests the behavior of the controller when there's text selected when
// `OnTextAvailable()` is triggered.
TEST_F(MahiMenuControllerTest, TextSelected) {
  EXPECT_FALSE(read_write_cards_ui_controller_.widget_for_test());

  // Menu widget should show when text is displayed.
  menu_controller()->OnTextAvailable(/*anchor_bounds=*/gfx::Rect(),
                                     /*selected_text=*/"test selected text",
                                     /*surrounding_text=*/"");

  EXPECT_TRUE(read_write_cards_ui_controller_.widget_for_test());
  EXPECT_TRUE(read_write_cards_ui_controller_.widget_for_test()->IsVisible());
  EXPECT_TRUE(read_write_cards_ui_controller_.GetMahiViewForTest());

  // Menu widget should hide when dismissed.
  menu_controller()->OnDismiss(/*is_other_command_executed=*/false);
  EXPECT_FALSE(read_write_cards_ui_controller_.widget_for_test());
  EXPECT_FALSE(read_write_cards_ui_controller_.GetMahiViewForTest());
}

#if BUILDFLAG(IS_CHROMEOS_ASH)
class MahiMenuControllerFeatureKeyTest : public ChromeViewsTestBase {
 public:
  MahiMenuControllerFeatureKeyTest() {
    base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
    command_line->AppendSwitchASCII(ash::switches::kMahiFeatureKey, "hello");
  }

 private:
  base::test::ScopedFeatureList feature_list_{chromeos::features::kMahi};
};

TEST_F(MahiMenuControllerFeatureKeyTest, DoesNotShowWidgetIfFeatureKeyIsWrong) {
  ReadWriteCardsUiController read_write_cards_ui_controller;
  ::mahi::FakeMahiWebContentsManager fake_mahi_web_contents_manager;
  fake_mahi_web_contents_manager.set_focused_web_content_is_distillable(true);
  ::mahi::ScopedMahiWebContentsManagerForTesting
      scoped_mahi_web_contents_manager(&fake_mahi_web_contents_manager);
  MahiMenuController menu_controller(read_write_cards_ui_controller);

  menu_controller.OnTextAvailable(/*anchor_bounds=*/gfx::Rect(),
                                  /*selected_text=*/"",
                                  /*surrounding_text=*/"");

  EXPECT_THAT(menu_controller.menu_widget_for_test(), IsNull());
}
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

}  // namespace chromeos::mahi
