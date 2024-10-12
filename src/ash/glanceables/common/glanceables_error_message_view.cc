// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/glanceables/common/glanceables_error_message_view.h"

#include <algorithm>
#include <memory>
#include <string>

#include "ash/glanceables/common/glanceables_view_id.h"
#include "ash/strings/grit/ash_strings.h"
#include "ash/style/typography.h"
#include "base/types/cxx23_to_underlying.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/chromeos/styles/cros_tokens_color_mappings.h"
#include "ui/compositor/layer.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/flex_layout_types.h"
#include "ui/views/layout/layout_types.h"
#include "ui/views/metadata/view_factory_internal.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"

namespace ash {
namespace {

constexpr int kErrorMessageViewSize = 34;
constexpr int kErrorMessageRoundedCornerRadius = kErrorMessageViewSize / 2;
constexpr gfx::Insets kButtonInsets = gfx::Insets::TLBR(8, 4, 8, 10);
constexpr gfx::Insets kLabelInsets = gfx::Insets::TLBR(0, 16, 0, 0);

class ActionLabelButton : public views::LabelButton {
  METADATA_HEADER(ActionLabelButton, views::LabelButton)

 public:
  ActionLabelButton(PressedCallback callback,
                    GlanceablesErrorMessageView::ButtonActionType type)
      : views::LabelButton(std::move(callback)) {
    int string_id;
    switch (type) {
      case GlanceablesErrorMessageView::ButtonActionType::kDismiss:
        string_id = IDS_GLANCEABLES_ERROR_DISMISS;
        break;
      case GlanceablesErrorMessageView::ButtonActionType::kReload:
        string_id = IDS_GLANCEABLES_ERROR_RELOAD;
        break;
    }
    SetText(l10n_util::GetStringUTF16(string_id));
    SetID(
        base::to_underlying(GlanceablesViewId::kGlanceablesErrorMessageButton));
    SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    SetProperty(views::kMarginsKey, kButtonInsets);
    SetEnabledTextColorIds(cros_tokens::kCrosSysPrimary);
    TypographyProvider::Get()->StyleLabel(TypographyToken::kCrosButton2,
                                          *label());
    label()->SetAutoColorReadabilityEnabled(false);
  }
  ~ActionLabelButton() override = default;
};

BEGIN_METADATA(ActionLabelButton)
END_METADATA

}  // namespace

GlanceablesErrorMessageView::GlanceablesErrorMessageView(
    views::Button::PressedCallback callback,
    const std::u16string& error_message,
    ButtonActionType type) {
  SetPaintToLayer();
  layer()->SetRoundedCornerRadius(
      gfx::RoundedCornersF(kErrorMessageRoundedCornerRadius));
  SetBackground(views::CreateThemedSolidBackground(
      cros_tokens::kCrosSysSystemOnBaseOpaque));
  SetID(base::to_underlying(GlanceablesViewId::kGlanceablesErrorMessageView));

  const auto* const typography_provider = TypographyProvider::Get();
  error_message_label_ = AddChildView(
      views::Builder<views::Label>()
          .SetID(base::to_underlying(
              GlanceablesViewId::kGlanceablesErrorMessageLabel))
          .SetEnabledColorId(cros_tokens::kCrosSysOnSurface)
          .SetFontList(typography_provider->ResolveTypographyToken(
              TypographyToken::kCrosAnnotation1))
          .SetLineHeight(typography_provider->ResolveLineHeight(
              TypographyToken::kCrosAnnotation1))
          .SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT)
          .SetText(error_message)
          .SetProperty(views::kMarginsKey, kLabelInsets)
          .SetProperty(
              views::kFlexBehaviorKey,
              views::FlexSpecification(views::MinimumFlexSizeRule::kScaleToZero,
                                       views::MaximumFlexSizeRule::kUnbounded))
          .Build());
  error_message_label_->SetAutoColorReadabilityEnabled(false);

  action_button_ = AddChildView(
      std::make_unique<ActionLabelButton>(std::move(callback), type));
}

void GlanceablesErrorMessageView::UpdateBoundsToContainer(
    const gfx::Rect& container_bounds) {
  gfx::Rect preferred_bounds(container_bounds);

  preferred_bounds.Inset(gfx::Insets::TLBR(
      preferred_bounds.height() - kErrorMessageViewSize, 0, 0, 0));

  SetBoundsRect(preferred_bounds);
}

std::u16string GlanceablesErrorMessageView::GetMessageForTest() const {
  return error_message_label_->GetText();
}

BEGIN_METADATA(GlanceablesErrorMessageView)
END_METADATA

}  // namespace ash
