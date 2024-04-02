// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/plus_addresses/plus_address_creation_dialog_delegate.h"

#include <optional>

#include "base/check_is_test.h"
#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/run_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "build/branding_buildflags.h"
#include "chrome/browser/ui/plus_addresses/plus_address_creation_controller.h"
#include "chrome/browser/ui/plus_addresses/plus_address_creation_controller_desktop.h"
#include "chrome/browser/ui/plus_addresses/plus_address_creation_view.h"
#include "chrome/browser/ui/views/chrome_layout_provider.h"
#include "chrome/grit/theme_resources.h"
#include "components/constrained_window/constrained_window_views.h"
#include "components/plus_addresses/features.h"
#include "components/plus_addresses/plus_address_types.h"
#include "components/strings/grit/components_strings.h"
#include "components/web_modal/web_contents_modal_dialog_manager.h"
#include "components/web_modal/web_contents_modal_dialog_manager_delegate.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/referrer.h"
#include "ui/base/interaction/element_identifier.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/window_open_disposition.h"
#include "ui/color/color_id.h"
#include "ui/events/base_event_utils.h"
#include "ui/gfx/text_constants.h"
#include "ui/gfx/vector_icon_types.h"
#include "ui/views/background.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/styled_label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/style/typography.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/widget/widget.h"
#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
#include "components/plus_addresses/resources/vector_icons.h"
#else
#include "components/vector_icons/vector_icons.h"
#endif

namespace plus_addresses {

namespace {
const float kDescriptionWidthPercent = 0.8;
const int kPlusAddressLabelVerticalMargin = 10;
const int kPlusAddressLogoWidth = 100;
#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
const gfx::VectorIcon& kLogoIcon = plus_addresses::kPlusAddressesLogoIcon;
#else
const gfx::VectorIcon& kLogoIcon = vector_icons::kProductIcon;
#endif
}  // namespace

DEFINE_CLASS_ELEMENT_IDENTIFIER_VALUE(PlusAddressCreationView,
                                      kPlusAddressDescriptionTextElementId);
DEFINE_CLASS_ELEMENT_IDENTIFIER_VALUE(PlusAddressCreationView,
                                      kPlusAddressErrorTextElementId);
DEFINE_CLASS_ELEMENT_IDENTIFIER_VALUE(PlusAddressCreationView,
                                      kPlusAddressConfirmButtonElementId);
DEFINE_CLASS_ELEMENT_IDENTIFIER_VALUE(PlusAddressCreationView,
                                      kPlusAddressCancelButtonElementId);
DEFINE_CLASS_ELEMENT_IDENTIFIER_VALUE(PlusAddressCreationView,
                                      kPlusAddressSuggestedEmailElementId);

PlusAddressCreationDialogDelegate::PlusAddressCreationDialogDelegate(
    base::WeakPtr<PlusAddressCreationController> controller,
    content::WebContents* web_contents,
    const std::string& primary_email_address)
    : views::BubbleDialogDelegate(/*anchor_view=*/nullptr,
                                  views::BubbleBorder::Arrow::NONE),
      controller_(controller),
      web_contents_(web_contents) {
  // This delegate is owned & deleted by the PlusAddressCreationController.
  SetOwnedByWidget(false);
  RegisterDeleteDelegateCallback(base::BindOnce(
      [](base::WeakPtr<PlusAddressCreationController> controller) {
        controller->OnDialogDestroyed();
      },
      controller));
  SetModalType(ui::MODAL_TYPE_CHILD);
  set_fixed_width(views::LayoutProvider::Get()->GetDistanceMetric(
      views::DISTANCE_MODAL_DIALOG_PREFERRED_WIDTH));

  std::unique_ptr<views::View> primary_view =
      views::Builder<views::BoxLayoutView>()
          .SetOrientation(views::BoxLayout::Orientation::kVertical)
          .Build();

  // Create hero image.
  primary_view->AddChildView(
      views::Builder<views::ImageView>()
          .SetImage(ui::ImageModel::FromVectorIcon(kLogoIcon, ui::kColorIcon,
                                                   kPlusAddressLogoWidth))
          .Build());

  // Add title view.
  primary_view->AddChildView(
      views::Builder<views::StyledLabel>()
          .SetHorizontalAlignment(gfx::ALIGN_CENTER)
          .SetTextContext(views::style::STYLE_PRIMARY)
          .SetText(l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_TITLE))
          .SetTextContext(views::style::CONTEXT_DIALOG_TITLE)
          .SetDefaultTextStyle(views::style::STYLE_BODY_1_BOLD)
          .Build());

  views::StyledLabel* description_paragraph = primary_view->AddChildView(
      views::Builder<views::StyledLabel>()
          .SetHorizontalAlignment(gfx::ALIGN_CENTER)
          .SetTextContext(views::style::CONTEXT_DIALOG_BODY_TEXT)
          .Build());

  // Split the difference on both sides of the description.
  int horizontal_margin = (1 - kDescriptionWidthPercent) *
                          ChromeLayoutProvider::Get()->GetDistanceMetric(
                              views::DISTANCE_MODAL_DIALOG_PREFERRED_WIDTH) /
                          2;
  description_paragraph->SetProperty(
      views::kMarginsKey,
      gfx::Insets::TLBR(0, horizontal_margin, 0, horizontal_margin));
  description_paragraph->SetProperty(views::kElementIdentifierKey,
                                     kPlusAddressDescriptionTextElementId);

  // Set the description text & update the styling.
  std::vector<size_t> description_offsets;
  // Prepend the settings link text with a newline to render it on one line.
  std::u16string settings_text =
      base::StrCat({u"\n", l10n_util::GetStringUTF16(
                               IDS_PLUS_ADDRESS_MODAL_PLUS_ADDRESS_LINK_TEXT)});
  description_paragraph->SetText(l10n_util::GetStringFUTF16(
      IDS_PLUS_ADDRESS_MODAL_PLUS_ADDRESS_DESCRIPTION_START, {settings_text},
      &description_offsets));

  gfx::Range settings_text_range(
      description_offsets[0], description_offsets[0] + settings_text.length());
  views::StyledLabel::RangeStyleInfo settings_text_style =
      views::StyledLabel::RangeStyleInfo::CreateForLink(base::BindRepeating(
          &PlusAddressCreationDialogDelegate::OpenSettingsLink,
          // Safe because this delegate outlives the Widget (and this view).
          base::Unretained(this), web_contents));
  description_paragraph->AddStyleRange(settings_text_range,
                                       settings_text_style);

  // Add the primary email address separately to avoid width constriction.
  views::StyledLabel* primary_email_address_view = primary_view->AddChildView(
      views::Builder<views::StyledLabel>()
          .SetHorizontalAlignment(gfx::ALIGN_CENTER)
          .SetTextContext(views::style::CONTEXT_DIALOG_BODY_TEXT)
          .Build());

  // Set the primary email address & update the styling.
  std::vector<size_t> email_address_offsets;
  std::u16string u16_primary_email_address =
      base::UTF8ToUTF16(primary_email_address);
  primary_email_address_view->SetText(l10n_util::GetStringFUTF16(
      IDS_PLUS_ADDRESS_MODAL_PLUS_ADDRESS_DESCRIPTION_END,
      {u16_primary_email_address}, &email_address_offsets));

  views::StyledLabel::RangeStyleInfo email_address_style;
  email_address_style.text_style = views::style::TextStyle::STYLE_EMPHASIZED;
  primary_email_address_view->AddStyleRange(
      gfx::Range(email_address_offsets[0],
                 email_address_offsets[0] + u16_primary_email_address.length()),
      email_address_style);

  // Create a bubble for the plus address to be displayed in.
  ChromeLayoutProvider* provider = ChromeLayoutProvider::Get();
  const int kRectangleRadius =
      provider->GetCornerRadiusMetric(views::ShapeContextTokens::kDialogRadius);

  std::unique_ptr<views::Background> background =
      views::CreateThemedRoundedRectBackground(
          // TODO(crbug.com/1467623) - Replace with color from the mocks.
          ui::kColorSubtleEmphasisBackground, kRectangleRadius);
  plus_address_label_ = primary_view->AddChildView(
      views::Builder<views::Label>()
          .SetText(l10n_util::GetStringUTF16(
              IDS_PLUS_ADDRESS_MODAL_PROPOSED_PLUS_ADDRESS_PLACEHOLDER))
          .SetTextContext(views::style::CONTEXT_DIALOG_BODY_TEXT)
          .SetTextStyle(views::style::STYLE_PRIMARY)
          .SetBackground(std::move(background))
          .Build());
  plus_address_label_->SetProperty(
      views::kMarginsKey,
      gfx::Insets::TLBR(kPlusAddressLabelVerticalMargin, 0,
                        kPlusAddressLabelVerticalMargin, 0));
  plus_address_label_->SetProperty(views::kElementIdentifierKey,
                                   kPlusAddressSuggestedEmailElementId);
  plus_address_label_->SetSelectable(true);
  plus_address_label_->SetLineHeight(2 * plus_address_label_->GetLineHeight());

  // Create and hide label for bug report instruction.
  std::vector<size_t> error_link_offsets;
  std::u16string error_link_text =
      l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_ERROR_REPORT_LINK_TEXT);
  error_report_label_ = primary_view->AddChildView(
      views::Builder<views::StyledLabel>()
          .SetHorizontalAlignment(gfx::ALIGN_CENTER)
          .SetText(l10n_util::GetStringFUTF16(
              IDS_PLUS_ADDRESS_MODAL_REPORT_ERROR_INSTRUCTION_DESKTOP,
              {error_link_text}, &error_link_offsets))
          .SetTextContext(views::style::CONTEXT_BUBBLE_FOOTER)
          .SetDefaultTextStyle(views::style::STYLE_HINT)
          .SetVisible(false)
          .Build());
  error_report_label_->SetProperty(
      views::kMarginsKey,
      gfx::Insets::TLBR(kPlusAddressLabelVerticalMargin, 0,
                        kPlusAddressLabelVerticalMargin, 0));
  error_report_label_->SetProperty(views::kElementIdentifierKey,
                                   kPlusAddressErrorTextElementId);
  // Update style for error link.
  gfx::Range error_link_range(error_link_offsets[0],
                              error_link_offsets[0] + error_link_text.length());
  views::StyledLabel::RangeStyleInfo error_link_text_style =
      views::StyledLabel::RangeStyleInfo::CreateForLink(base::BindRepeating(
          &PlusAddressCreationDialogDelegate::OpenErrorReportLink,
          // Safe because this delegate outlives the Widget (and this view).
          base::Unretained(this), web_contents));
  error_report_label_->AddStyleRange(error_link_range, error_link_text_style);

  // Avoid using the builtin DialogDelegate buttons so that we can use
  // GetWidget()->Close() to close the UI when ready.
  SetButtons(ui::DIALOG_BUTTON_NONE);

  // Initialize buttons.
  views::BoxLayoutView* buttons_view = primary_view->AddChildView(
      views::Builder<views::BoxLayoutView>()
          .SetOrientation(views::BoxLayout::Orientation::kHorizontal)
          .SetMainAxisAlignment(views::BoxLayout::MainAxisAlignment::kEnd)
          .SetBetweenChildSpacing(provider->GetDistanceMetric(
              views::DistanceMetric::DISTANCE_RELATED_BUTTON_HORIZONTAL))
          .Build());

  cancel_button_ =
      buttons_view->AddChildView(std::make_unique<views::MdTextButton>(
          base::BindRepeating(
              &PlusAddressCreationDialogDelegate::HandleButtonPress,
              // Safe because this delegate outlives the Widget (and this view).
              base::Unretained(this), PlusAddressViewButtonType::kCancel),
          l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_CANCEL_TEXT)

              ));
  cancel_button_->SetTooltipText(
      l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_CANCEL_TEXT));
  cancel_button_->SetAccessibleName(
      l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_CANCEL_TEXT));
  cancel_button_->SizeToPreferredSize();
  cancel_button_->SetProperty(views::kElementIdentifierKey,
                              kPlusAddressCancelButtonElementId);

  confirm_button_ =
      buttons_view->AddChildView(std::make_unique<views::MdTextButton>(
          base::BindRepeating(
              &PlusAddressCreationDialogDelegate::HandleButtonPress,
              // Safe because this delegate outlives the Widget (and this view).
              base::Unretained(this), PlusAddressViewButtonType::kConfirm),
          l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_OK_TEXT)));
  confirm_button_->SetTooltipText(
      l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_OK_TEXT));
  confirm_button_->SetAccessibleName(
      l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_MODAL_OK_TEXT));
  confirm_button_->SizeToPreferredSize();
  confirm_button_->SetStyle(ui::ButtonStyle::kProminent);
  confirm_button_->SetEnabled(false);
  confirm_button_->SetProperty(views::kElementIdentifierKey,
                               kPlusAddressConfirmButtonElementId);

  SetContentsView(std::move(primary_view));
}

PlusAddressCreationDialogDelegate::~PlusAddressCreationDialogDelegate() {
  plus_address_label_ = nullptr;
}

bool PlusAddressCreationDialogDelegate::ShouldShowCloseButton() const {
  return true;
}

void PlusAddressCreationDialogDelegate::OnWidgetInitialized() {
  if (views::BubbleFrameView* frame = GetBubbleFrameView(); frame) {
    frame->close_button()->SetCallback(
        views::Button::PressedCallback(base::BindRepeating(
            &PlusAddressCreationDialogDelegate::HandleButtonPress,
            // Safe because this outlives the BubbleFrameView.
            base::Unretained(this), PlusAddressViewButtonType::kClose)));
  }
}

void PlusAddressCreationDialogDelegate::OpenSettingsLink(
    content::WebContents* web_contents) {
  if (web_contents && !features::kPlusAddressManagementUrl.Get().empty()) {
    web_contents->OpenURL(content::OpenURLParams(
        GURL(features::kPlusAddressManagementUrl.Get()), content::Referrer(),
        WindowOpenDisposition::NEW_FOREGROUND_TAB,
        ui::PageTransition::PAGE_TRANSITION_LINK,
        /*is_renderer_initiated=*/false));
  }
}

// TODO(b/313670457) Test open link behaviors when migrate to Kombucha.
void PlusAddressCreationDialogDelegate::OpenErrorReportLink(
    content::WebContents* web_contents) {
  if (web_contents && !features::kPlusAddressErrorReportUrl.Get().empty()) {
    web_contents->OpenURL(content::OpenURLParams(
        GURL(features::kPlusAddressErrorReportUrl.Get()), content::Referrer(),
        WindowOpenDisposition::NEW_FOREGROUND_TAB,
        ui::PageTransition::PAGE_TRANSITION_LINK,
        /*is_renderer_initiated=*/false));
  }
}

void PlusAddressCreationDialogDelegate::ShowReserveResult(
    const PlusProfileOrError& maybe_plus_profile) {
  CHECK(plus_address_label_);

  if (maybe_plus_profile.has_value()) {
    plus_address_label_->SetText(
        base::UTF8ToUTF16(maybe_plus_profile->plus_address));
    confirm_button_->SetEnabled(true);
  } else {
    ShowErrorStateUI();
  }
}

void PlusAddressCreationDialogDelegate::ShowConfirmResult(
    const PlusProfileOrError& maybe_plus_profile) {
  CHECK(plus_address_label_);
  CHECK(GetWidget());
  CHECK(GetBubbleFrameView());

  // Stop indicating loading now that we have the server response.
  GetBubbleFrameView()->SetProgress(std::nullopt);

  if (maybe_plus_profile.has_value()) {
    GetWidget()->CloseWithReason(
        views::Widget::ClosedReason::kAcceptButtonClicked);
  } else {
    ShowErrorStateUI();
    confirm_button_->SetEnabled(false);
  }
}

void PlusAddressCreationDialogDelegate::HandleButtonPress(
    PlusAddressViewButtonType type) {
  CHECK(GetBubbleFrameView());

  switch (type) {
    case PlusAddressViewButtonType::kConfirm: {
      controller_->OnConfirmed();
      // Show a progress bar that loops until the Confirm request is resolved.
      GetBubbleFrameView()->SetProgress(-1);
      return;
    }
    case PlusAddressViewButtonType::kCancel: {
      controller_->OnCanceled();
      GetWidget()->CloseWithReason(
          views::Widget::ClosedReason::kCancelButtonClicked);
      return;
    }
    case PlusAddressViewButtonType::kClose: {
      controller_->OnCanceled();
      GetWidget()->CloseWithReason(
          views::Widget::ClosedReason::kCloseButtonClicked);
      return;
    }
  }
}

void PlusAddressCreationDialogDelegate::ShowErrorStateUI() {
  CHECK(GetWidget() && web_contents_);
  plus_address_label_->SetVisible(false);
  // Show the error report instructions.
  error_report_label_->SetVisible(true);
  // Update the size of modal.
  constrained_window::UpdateWebContentsModalDialogPosition(
      GetWidget(),
      web_modal::WebContentsModalDialogManager::FromWebContents(web_contents_)
          ->delegate()
          ->GetWebContentsModalDialogHost());
}

}  // namespace plus_addresses
