// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/picker/views/picker_list_item_view.h"

#include <memory>
#include <string>
#include <utility>

#include "ash/ash_element_identifiers.h"
#include "ash/bubble/bubble_utils.h"
#include "ash/picker/views/picker_item_view.h"
#include "ash/style/style_util.h"
#include "ash/style/typography.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/models/image_model.h"
#include "ui/chromeos/styles/cros_tokens_color_mappings.h"
#include "ui/color/color_id.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/size.h"
#include "ui/views/border.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/layout/flex_layout_view.h"
#include "ui/views/layout/layout_manager.h"
#include "ui/views/layout/layout_types.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/view_utils.h"

namespace ash {
namespace {

constexpr auto kPickerListItemBorderInsets = gfx::Insets::TLBR(8, 16, 8, 8);

constexpr gfx::Size kLeadingIconSizeDip(20, 20);
constexpr int kImageDisplayHeight = 72;
constexpr auto kLeadingIconRightPadding = gfx::Insets::TLBR(0, 0, 0, 16);

}  // namespace

PickerListItemView::PickerListItemView(SelectItemCallback select_item_callback)
    : PickerItemView(std::move(select_item_callback),
                     FocusIndicatorStyle::kFocusBar) {
  SetLayoutManager(std::make_unique<views::FlexLayout>());
  auto* item_contents =
      AddChildView(views::Builder<views::FlexLayoutView>()
                       .SetOrientation(views::LayoutOrientation::kHorizontal)
                       .SetCrossAxisAlignment(views::LayoutAlignment::kStart)
                       .SetCanProcessEventsWithinSubtree(false)
                       .Build());

  leading_container_ = item_contents->AddChildView(
      views::Builder<views::FlexLayoutView>()
          .SetOrientation(views::LayoutOrientation::kVertical)
          .SetCrossAxisAlignment(views::LayoutAlignment::kStart)
          .Build());

  auto* main_container = item_contents->AddChildView(
      views::Builder<views::FlexLayoutView>()
          .SetOrientation(views::LayoutOrientation::kVertical)
          .Build());
  primary_container_ =
      main_container->AddChildView(std::make_unique<views::FlexLayoutView>());
  secondary_container_ =
      main_container->AddChildView(std::make_unique<views::FlexLayoutView>());

  SetBorder(views::CreateEmptyBorder(kPickerListItemBorderInsets));
  SetProperty(views::kElementIdentifierKey,
              kPickerSearchResultsListItemElementId);
}

PickerListItemView::~PickerListItemView() = default;

void PickerListItemView::SetPrimaryText(const std::u16string& primary_text) {
  primary_container_->RemoveAllChildViews();
  primary_container_->AddChildView(
      bubble_utils::CreateLabel(TypographyToken::kCrosBody2, primary_text,
                                cros_tokens::kCrosSysOnSurface));
  SetAccessibleName(primary_text);
}

void PickerListItemView::SetPrimaryImage(
    std::unique_ptr<views::ImageView> primary_image) {
  primary_container_->RemoveAllChildViews();
  auto* image_view = primary_container_->AddChildView(std::move(primary_image));
  image_view->SetCanProcessEventsWithinSubtree(false);
  const gfx::Size original_size = image_view->GetImageModel().Size();
  if (original_size.height() > 0) {
    image_view->SetImageSize(gfx::ScaleToRoundedSize(
        original_size,
        static_cast<float>(kImageDisplayHeight) / original_size.height()));
  }
  // TODO: b/316936418 - Get accessible name for image contents.
  SetAccessibleName(u"image contents");
}

void PickerListItemView::SetLeadingIcon(const ui::ImageModel& icon) {
  leading_container_->RemoveAllChildViews();
  leading_container_->AddChildView(
      views::Builder<views::ImageView>()
          .SetImageSize(kLeadingIconSizeDip)
          .SetImage(icon)
          .SetCanProcessEventsWithinSubtree(false)
          .SetProperty(views::kMarginsKey, kLeadingIconRightPadding)
          .Build());
}

void PickerListItemView::SetSecondaryText(
    const std::u16string& secondary_text) {
  secondary_container_->RemoveAllChildViews();
  secondary_container_->AddChildView(bubble_utils::CreateLabel(
      TypographyToken::kCrosAnnotation2, secondary_text,
      cros_tokens::kCrosSysOnSurfaceVariant));
}

std::u16string PickerListItemView::GetPrimaryTextForTesting() const {
  if (primary_container_->children().empty()) {
    return u"";
  }
  if (const auto* label = views::AsViewClass<views::Label>(
          primary_container_->children().front().get())) {
    return label->GetText();
  }
  return u"";
}

ui::ImageModel PickerListItemView::GetPrimaryImageForTesting() const {
  if (primary_container_->children().empty()) {
    return ui::ImageModel();
  }
  if (const auto* image = views::AsViewClass<views::ImageView>(
          primary_container_->children().front().get())) {
    return image->GetImageModel();
  }
  return ui::ImageModel();
}

BEGIN_METADATA(PickerListItemView)
END_METADATA

}  // namespace ash
