// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/notification_center/views/pinned_notification_view.h"

#include "ash/public/cpp/ash_view_ids.h"
#include "ash/public/cpp/system_notification_builder.h"
#include "ash/resources/vector_icons/vector_icons.h"
#include "ash/style/icon_button.h"
#include "ash/style/pill_button.h"
#include "ash/test/ash_test_base.h"
#include "base/test/metrics/histogram_tester.h"
#include "ui/message_center/public/cpp/notification.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/test/views_test_utils.h"
#include "ui/views/view_utils.h"
#include "ui/views/widget/widget.h"

namespace ash {

namespace {

views::ImageView* GetIcon(views::View* notification_view) {
  return views::AsViewClass<views::ImageView>(
      notification_view->GetViewByID(VIEW_ID_PINNED_NOTIFICATION_ICON));
}

views::Label* GetTitleLabel(views::View* notification_view) {
  return views::AsViewClass<views::Label>(
      notification_view->GetViewByID(VIEW_ID_PINNED_NOTIFICATION_TITLE_LABEL));
}

views::Label* GetSubtitleLabel(views::View* notification_view) {
  return views::AsViewClass<views::Label>(notification_view->GetViewByID(
      VIEW_ID_PINNED_NOTIFICATION_SUBTITLE_LABEL));
}

views::Label* GetShortcutLabel(views::View* notification_view) {
  return views::AsViewClass<views::Label>(notification_view->GetViewByID(
      VIEW_ID_PINNED_NOTIFICATION_SHORTCUT_LABEL));
}

views::Label* GetShortcutDividerLabel(views::View* notification_view) {
  return views::AsViewClass<views::Label>(notification_view->GetViewByID(
      VIEW_ID_PINNED_NOTIFICATION_SHORTCUT_DIVIDER_LABEL));
}

PillButton* GetPillButton(views::View* notification_view) {
  return views::AsViewClass<PillButton>(
      notification_view->GetViewByID(VIEW_ID_PINNED_NOTIFICATION_PILL_BUTTON));
}

IconButton* GetPrimaryIconButton(views::View* notification_view) {
  return views::AsViewClass<IconButton>(notification_view->GetViewByID(
      VIEW_ID_PINNED_NOTIFICATION_PRIMARY_ICON_BUTTON));
}

IconButton* GetSecondaryIconButton(views::View* notification_view) {
  return views::AsViewClass<IconButton>(notification_view->GetViewByID(
      VIEW_ID_PINNED_NOTIFICATION_SECONDARY_ICON_BUTTON));
}

// Sample constants to use in the created test views.
const std::u16string sample_text = u"sample";
raw_ptr<const gfx::VectorIcon> sample_icon = &kPinnedIcon;

// Histogram names.
constexpr char kPinnedNotificationShownWithoutIconCount[] =
    "Ash.NotifierFramework.PinnedSystemNotification.ShownWithoutIcon";
constexpr char kPinnedNotificationShownWithoutTitleCount[] =
    "Ash.NotifierFramework.PinnedSystemNotification.ShownWithoutTitle";

}  // namespace

// Unit test to verify that `PinnedNotificationView` elements are created by
// providing different parameters. Visit go/ongoing-processes-variations to
// access screenshots of the different view configurations.
using PinnedNotificationViewTest = AshTestBase;

// Tests that the mandatory fields, which are title and icon, are created.
TEST_F(PinnedNotificationViewTest, Default) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  ASSERT_TRUE(GetTitleLabel(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  ASSERT_TRUE(GetSubtitleLabel(pinned_notification_view));
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetShortcutLabel(pinned_notification_view));
  EXPECT_FALSE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_FALSE(GetPillButton(pinned_notification_view));
  EXPECT_FALSE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the mandatory fields and a subtitle are created.
TEST_F(PinnedNotificationViewTest, Subtitle) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetMessage(sample_text)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  ASSERT_TRUE(GetTitleLabel(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  ASSERT_TRUE(GetSubtitleLabel(pinned_notification_view));
  EXPECT_TRUE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetShortcutLabel(pinned_notification_view));
  EXPECT_FALSE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_FALSE(GetPillButton(pinned_notification_view));
  EXPECT_FALSE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the mandatory fields and a shortcut text label are created.
TEST_F(PinnedNotificationViewTest, ShortcutText) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  message_center::RichNotificationData data;
  data.shortcut_text = sample_text;

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetOptionalFields(data)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_TRUE(GetShortcutLabel(pinned_notification_view));
  EXPECT_FALSE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_FALSE(GetPillButton(pinned_notification_view));
  EXPECT_FALSE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the mandatory fields and a pill button are created.
TEST_F(PinnedNotificationViewTest, PillButton) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  message_center::RichNotificationData data;
  data.buttons.emplace_back(message_center::ButtonInfo(/*title=*/sample_text));

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetOptionalFields(data)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetShortcutLabel(pinned_notification_view));
  EXPECT_FALSE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_TRUE(GetPillButton(pinned_notification_view));
  EXPECT_FALSE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the mandatory fields and a shortcut text + a pill button are
// created. A shortcut divider should also be created.
TEST_F(PinnedNotificationViewTest, ShortcutTextAndPillButton) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  message_center::RichNotificationData data;
  data.shortcut_text = sample_text;
  data.buttons.emplace_back(message_center::ButtonInfo(/*title=*/sample_text));

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetOptionalFields(data)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_TRUE(GetShortcutLabel(pinned_notification_view));
  EXPECT_TRUE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_TRUE(GetPillButton(pinned_notification_view));
  EXPECT_FALSE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the mandatory fields and an icon button are created.
TEST_F(PinnedNotificationViewTest, OneIconButton) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  message_center::RichNotificationData data;
  data.buttons.emplace_back(message_center::ButtonInfo(
      /*vector_icon=*/sample_icon, /*accessible_name=*/sample_text));

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetOptionalFields(data)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetShortcutLabel(pinned_notification_view));
  EXPECT_FALSE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_FALSE(GetPillButton(pinned_notification_view));
  EXPECT_TRUE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the mandatory fields and two icon buttons are created.
TEST_F(PinnedNotificationViewTest, TwoIconButtons) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  message_center::RichNotificationData data;
  data.buttons.emplace_back(message_center::ButtonInfo(
      /*vector_icon=*/sample_icon, /*accessible_name=*/sample_text));
  data.buttons.emplace_back(message_center::ButtonInfo(
      /*vector_icon=*/sample_icon, /*accessible_name=*/sample_text));

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetOptionalFields(data)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetShortcutLabel(pinned_notification_view));
  EXPECT_FALSE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_FALSE(GetPillButton(pinned_notification_view));
  EXPECT_TRUE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_TRUE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the mandatory fields and a shortcut text + two icon buttons are
// created. A shortcut divider should also be created.
TEST_F(PinnedNotificationViewTest, ShortcutTextAndIconButtons) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  message_center::RichNotificationData data;
  data.shortcut_text = sample_text;
  data.buttons.emplace_back(message_center::ButtonInfo(
      /*vector_icon=*/sample_icon, /*accessible_name=*/sample_text));
  data.buttons.emplace_back(message_center::ButtonInfo(
      /*vector_icon=*/sample_icon, /*accessible_name=*/sample_text));

  auto notification =
      ash::SystemNotificationBuilder()
          .SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetOptionalFields(data)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetIcon(pinned_notification_view));
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_TRUE(GetShortcutLabel(pinned_notification_view));
  EXPECT_TRUE(GetShortcutDividerLabel(pinned_notification_view));
  EXPECT_FALSE(GetPillButton(pinned_notification_view));
  EXPECT_TRUE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_TRUE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests the invalid case where two `ButtonInfo` objects are sent but one has a
// `title` to create a `PillButton` and the other only has a `vector_icon` and
// `accessible_name` to create an `IconButton`. In these cases, only the first
// button will be created and the second `ButtonInfo` object will be ignored.
TEST_F(PinnedNotificationViewTest, InvalidButtonInfo) {
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  ash::SystemNotificationBuilder notification_builder;
  message_center::RichNotificationData data;

  // Provide the data to create an `IconButton` and then a `PillButton`.
  data.buttons.emplace_back(message_center::ButtonInfo(
      /*vector_icon=*/sample_icon, /*accessible_name=*/sample_text));
  data.buttons.emplace_back(message_center::ButtonInfo(/*title=*/sample_text));

  auto notification =
      notification_builder.SetId("id")
          .SetCatalogName(NotificationCatalogName::kTestCatalogName)
          .SetSmallImage(*sample_icon)
          .SetTitle(sample_text)
          .SetOptionalFields(data)
          .Build(
              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that only a single `IconButton` was created.
  EXPECT_FALSE(GetPillButton(pinned_notification_view));
  EXPECT_TRUE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));

  // Provide the data to create a `PillButton` and then an `IconButton`.
  data.buttons.clear();
  data.buttons.emplace_back(message_center::ButtonInfo(/*title=*/sample_text));
  data.buttons.emplace_back(message_center::ButtonInfo(
      /*vector_icon=*/sample_icon, /*accessible_name=*/sample_text));

  notification = notification_builder.SetId("id")
                     .SetCatalogName(NotificationCatalogName::kTestCatalogName)
                     .SetSmallImage(*sample_icon)
                     .SetTitle(sample_text)
                     .SetOptionalFields(data)
                     .Build(
                         /*keep_timestamp=*/false);

  pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that only a single `PillButton` was created.
  EXPECT_TRUE(GetPillButton(pinned_notification_view));
  EXPECT_FALSE(GetPrimaryIconButton(pinned_notification_view));
  EXPECT_FALSE(GetSecondaryIconButton(pinned_notification_view));
}

// Tests that the `Ash.NotifierFramework.PinnedSystemNotification`
// `ShownWithoutIcon` and `ShownWithoutTitle` metrics properly record when a
// pinned notification view is created without an icon or title.
TEST_F(PinnedNotificationViewTest, ShownWithoutIconOrTitleMetrics) {
  base::HistogramTester histogram_tester;
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  auto catalog_name = NotificationCatalogName::kTestCatalogName;

  auto notification = ash::SystemNotificationBuilder()
                          .SetId("id")
                          .SetCatalogName(catalog_name)
                          .Build(
                              /*keep_timestamp=*/false);

  widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that the appropriate metrics were recorded.
  histogram_tester.ExpectBucketCount(kPinnedNotificationShownWithoutIconCount,
                                     catalog_name, 1);
  histogram_tester.ExpectBucketCount(kPinnedNotificationShownWithoutTitleCount,
                                     catalog_name, 1);
}

// Tests that the notification `title` and `subtitle` fields can be updated.
TEST_F(PinnedNotificationViewTest, UpdateWithNotification) {
  base::HistogramTester histogram_tester;
  std::unique_ptr<views::Widget> widget = CreateFramelessTestWidget();

  auto catalog_name = NotificationCatalogName::kTestCatalogName;

  auto notification = ash::SystemNotificationBuilder()
                          .SetId("id")
                          .SetCatalogName(catalog_name)
                          .SetSmallImage(*sample_icon)
                          .SetTitle(sample_text)
                          .SetMessage(sample_text)
                          .Build(
                              /*keep_timestamp=*/false);

  PinnedNotificationView* pinned_notification_view = widget->SetContentsView(
      std::make_unique<PinnedNotificationView>(notification));

  // Test that appropriate notification elements were created.
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_TRUE(GetSubtitleLabel(pinned_notification_view)->GetVisible());

  // Set an empty `title` and `subtitle` and update the notification view.
  notification.set_title(std::u16string());
  notification.set_message(std::u16string());
  pinned_notification_view->UpdateWithNotification(notification);

  // Ensure that the notification elements's visibility was properly updated.
  EXPECT_TRUE(GetTitleLabel(pinned_notification_view)->GetVisible());
  EXPECT_FALSE(GetSubtitleLabel(pinned_notification_view)->GetVisible());

  // Ensure the `ShownWithoutTitle` metric was recorded after updating the
  // notification with an empty `title`.
  histogram_tester.ExpectBucketCount(kPinnedNotificationShownWithoutTitleCount,
                                     catalog_name, 1);
}

}  // namespace ash
