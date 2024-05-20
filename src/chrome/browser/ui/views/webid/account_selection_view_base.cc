// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/webid/account_selection_view_base.h"

#include "chrome/browser/image_fetcher/image_decoder_impl.h"
#include "chrome/browser/ui/views/controls/hover_button.h"
#include "chrome/browser/ui/views/webid/account_selection_bubble_view.h"
#include "chrome/grit/generated_resources.h"
#include "components/image_fetcher/core/image_fetcher_impl.h"
#include "components/vector_icons/vector_icons.h"
#include "third_party/blink/public/mojom/webid/federated_auth_request.mojom.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/border.h"
#include "ui/views/controls/styled_label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/layout/flex_layout_view.h"
#include "ui/views/style/typography.h"
#include "ui/views/widget/widget_observer.h"
#include "url/gurl.h"

namespace {

// safe_zone_diameter/icon_size as defined in
// https://www.w3.org/TR/appmanifest/#icon-masks
constexpr float kMaskableWebIconSafeZoneRatio = 0.8f;

// Selects string for disclosure text based on passed-in `privacy_policy_url`
// and `terms_of_service_url`.
int SelectDisclosureTextResourceId(const GURL& privacy_policy_url,
                                   const GURL& terms_of_service_url) {
  if (privacy_policy_url.is_empty()) {
    return terms_of_service_url.is_empty()
               ? IDS_ACCOUNT_SELECTION_DATA_SHARING_CONSENT_NO_PP_OR_TOS
               : IDS_ACCOUNT_SELECTION_DATA_SHARING_CONSENT_NO_PP;
  }

  return terms_of_service_url.is_empty()
             ? IDS_ACCOUNT_SELECTION_DATA_SHARING_CONSENT_NO_TOS
             : IDS_ACCOUNT_SELECTION_DATA_SHARING_CONSENT;
}

gfx::ImageSkia CreateCircleCroppedImage(const gfx::ImageSkia& original_image,
                                        int image_size) {
  return gfx::CanvasImageSource::MakeImageSkia<CircleCroppedImageSkiaSource>(
      original_image, original_image.width() * kMaskableWebIconSafeZoneRatio,
      image_size);
}

}  // namespace

constexpr net::NetworkTrafficAnnotationTag kTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("fedcm_account_profile_image_fetcher",
                                        R"(
        semantics {
          sender: "Profile image fetcher for FedCM Account chooser on desktop."
          description:
            "Retrieves profile images for user's accounts in the FedCM login"
            "flow."
          trigger:
            "Triggered when FedCM API is called and account chooser shows up."
            "The accounts shown are ones for which the user has previously"
            "signed into the identity provider."
          data:
            "Account picture URL of user account, provided by the identity"
            "provider."
          destination: WEBSITE
          internal {
            contacts {
                email: "web-identity-eng@google.com"
            }
          }
          user_data {
            type: USER_CONTENT
          }
          last_reviewed: "2024-01-25"
        }
        policy {
          cookies_allowed: NO
          setting:
            "You can enable or disable this feature in chrome://settings, under"
            "'Privacy and security', then 'Site Settings', and finally"
            "'Third party sign-in'."
          policy_exception_justification:
            "Not implemented. This is a feature that sites use for"
            "Federated Sign-In, for which we do not have an Enterprise policy."
        })");

void LetterCircleCroppedImageSkiaSource::Draw(gfx::Canvas* canvas) {
  monogram::DrawMonogramInCanvas(canvas, size().width(), size().width(),
                                 letter_, SK_ColorWHITE, SK_ColorGRAY);
}

void CircleCroppedImageSkiaSource::Draw(gfx::Canvas* canvas) {
  const int canvas_edge_size = size().width();

  // Center the avatar in the canvas.
  const int x = (canvas_edge_size - avatar_.width()) / 2;
  const int y = (canvas_edge_size - avatar_.height()) / 2;

  SkPath circular_mask;
  circular_mask.addCircle(SkIntToScalar(canvas_edge_size / 2),
                          SkIntToScalar(canvas_edge_size / 2),
                          SkIntToScalar(canvas_edge_size / 2));
  canvas->ClipPath(circular_mask, true);
  canvas->DrawImageInt(avatar_, x, y);
}

BrandIconImageView::BrandIconImageView(
    base::OnceCallback<void(const GURL&, const gfx::ImageSkia&)> add_image,
    int image_size,
    bool should_circle_crop,
    std::optional<SkColor> background_color)
    : add_image_(std::move(add_image)),
      image_size_(image_size),
      should_circle_crop_(should_circle_crop),
      background_color_(background_color) {}

BrandIconImageView::~BrandIconImageView() = default;

void BrandIconImageView::FetchImage(
    const GURL& icon_url,
    image_fetcher::ImageFetcher& image_fetcher) {
  image_fetcher::ImageFetcherParams params(kTrafficAnnotation,
                                           kImageFetcherUmaClient);
  image_fetcher.FetchImage(
      icon_url,
      base::BindOnce(&BrandIconImageView::OnImageFetched,
                     weak_ptr_factory_.GetWeakPtr(), icon_url),
      std::move(params));
}

void BrandIconImageView::CropAndSetImage(const gfx::ImageSkia& original_image) {
  constexpr int kBorderRadius = 10;
  gfx::ImageSkia cropped_image =
      should_circle_crop_
          ? CreateCircleCroppedImage(original_image, image_size_)
          : gfx::ImageSkiaOperations::CreateResizedImage(
                original_image, skia::ImageOperations::RESIZE_BEST,
                gfx::Size(image_size_, image_size_));
  if (background_color_) {
    cropped_image = gfx::ImageSkiaOperations::CreateImageWithCircleBackground(
        kBorderRadius, *background_color_, cropped_image);
  }
  SetImage(ui::ImageModel::FromImageSkia(cropped_image));
}

void BrandIconImageView::OnImageFetched(
    const GURL& image_url,
    const gfx::Image& image,
    const image_fetcher::RequestMetadata& metadata) {
  if (image.Width() != image.Height() ||
      image.Width() < AccountSelectionView::GetBrandIconMinimumSize()) {
    return;
  }
  gfx::ImageSkia skia_image = image.AsImageSkia();
  CropAndSetImage(skia_image);

  // TODO(crbug.com/327509202): This stops the crashes but should fix to prevent
  // this from crashing in the first place.
  if (!add_image_) {
    return;
  }
  std::move(add_image_).Run(image_url, skia_image);
}

BEGIN_METADATA(BrandIconImageView)
END_METADATA

class AccountImageView : public views::ImageView {
  METADATA_HEADER(AccountImageView, views::ImageView)

 public:
  AccountImageView() = default;

  AccountImageView(const AccountImageView&) = delete;
  AccountImageView& operator=(const AccountImageView&) = delete;
  ~AccountImageView() override = default;

  // Fetch image and set it on AccountImageView.
  void FetchAccountImage(const content::IdentityRequestAccount& account,
                         image_fetcher::ImageFetcher& image_fetcher,
                         int image_size) {
    image_fetcher::ImageFetcherParams params(kTrafficAnnotation,
                                             kImageFetcherUmaClient);

    // OnImageFetched() is a member of AccountImageView so that the callback
    // is cancelled in the case that AccountImageView is destroyed prior to
    // the callback returning.
    image_fetcher.FetchImage(account.picture,
                             base::BindOnce(&AccountImageView::OnImageFetched,
                                            weak_ptr_factory_.GetWeakPtr(),
                                            base::UTF8ToUTF16(account.name),
                                            &image_fetcher, image_size),
                             std::move(params));
  }

 private:
  void OnImageFetched(const std::u16string& account_name,
                      image_fetcher::ImageFetcher* image_fetcher,
                      int image_size,
                      const gfx::Image& image,
                      const image_fetcher::RequestMetadata& metadata) {
    gfx::ImageSkia avatar;
    if (image.IsEmpty()) {
      std::u16string letter = account_name;
      if (letter.length() > 0) {
        letter = base::i18n::ToUpper(account_name.substr(0, 1));
      }
      avatar = gfx::CanvasImageSource::MakeImageSkia<
          LetterCircleCroppedImageSkiaSource>(letter, image_size);
    } else {
      avatar =
          gfx::CanvasImageSource::MakeImageSkia<CircleCroppedImageSkiaSource>(
              image.AsImageSkia(), std::nullopt, image_size);
    }
    SetImage(ui::ImageModel::FromImageSkia(avatar));
  }

  base::WeakPtrFactory<AccountImageView> weak_ptr_factory_{this};
};

BEGIN_METADATA(AccountImageView)
END_METADATA

AccountSelectionViewBase::AccountSelectionViewBase(
    content::WebContents* web_contents,
    AccountSelectionViewBase::Observer* observer,
    views::WidgetObserver* widget_observer,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : web_contents_(web_contents),
      widget_observer_(widget_observer),
      observer_(observer) {
  image_fetcher_ = std::make_unique<image_fetcher::ImageFetcherImpl>(
      std::make_unique<ImageDecoderImpl>(), std::move(url_loader_factory));
}

AccountSelectionViewBase::AccountSelectionViewBase() = default;
AccountSelectionViewBase::~AccountSelectionViewBase() {}

void AccountSelectionViewBase::SetLabelProperties(views::Label* label) {
  label->SetMultiLine(true);
  label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  label->SetAllowCharacterBreak(true);
  label->SetProperty(
      views::kFlexBehaviorKey,
      views::FlexSpecification(views::MinimumFlexSizeRule::kScaleToZero,
                               views::MaximumFlexSizeRule::kUnbounded,
                               /*adjust_height_for_width=*/true));
}

std::unique_ptr<views::View> AccountSelectionViewBase::CreateAccountRow(
    const content::IdentityRequestAccount& account,
    const IdentityProviderDisplayData& idp_display_data,
    bool should_hover,
    bool should_include_idp,
    bool is_modal_dialog,
    int additional_vertical_padding) {
  int avatar_size = is_modal_dialog ? kModalAvatarSize : kDesiredAvatarSize;
  views::style::TextStyle account_name_style =
      is_modal_dialog ? views::style::STYLE_BODY_3_MEDIUM
                      : views::style::STYLE_PRIMARY;
  views::style::TextStyle account_email_style =
      is_modal_dialog ? views::style::STYLE_BODY_5
                      : views::style::STYLE_SECONDARY;

  std::unique_ptr<views::View> avatar_view;
  auto account_image_view = std::make_unique<AccountImageView>();
  account_image_view->SetImageSize({avatar_size, avatar_size});
  CHECK(should_hover || !should_include_idp);
  if (should_hover) {
    if (should_include_idp) {
      account_image_view->FetchAccountImage(account, *image_fetcher_,
                                            avatar_size);
      // Introduce a border so that the IDP image is a bit past the account
      // image.
      account_image_view->SetBorder(views::CreateEmptyBorder(
          gfx::Insets::TLBR(/*top=*/0, /*left=*/0, /*bottom=*/kIdpBadgeOffset,
                            /*right=*/kIdpBadgeOffset)));
      // Put `account_image_view` into a FillLayout `background_container`.
      std::unique_ptr<views::View> background_container =
          std::make_unique<views::View>();
      background_container->SetUseDefaultFillLayout(true);
      background_container->AddChildView(std::move(account_image_view));

      // Put brand icon image view into a BoxLayout container.
      std::unique_ptr<views::BoxLayoutView> icon_container =
          std::make_unique<views::BoxLayoutView>();
      icon_container->SetMainAxisAlignment(views::LayoutAlignment::kEnd);
      icon_container->SetCrossAxisAlignment(views::LayoutAlignment::kEnd);

      // `web_contents_` may be nullptr in tests.
      SkColor background_color =
          web_contents_ ? web_contents_->GetColorProvider().GetColor(
                              ui::kColorDialogBackground)
                        : SK_ColorWHITE;
      std::unique_ptr<BrandIconImageView> brand_icon_image_view =
          std::make_unique<BrandIconImageView>(
              base::BindOnce(&AccountSelectionViewBase::AddIdpImage,
                             weak_ptr_factory_.GetWeakPtr()),
              kLargeAvatarBadgeSize, /*should_circle_crop=*/true,
              background_color);
      ConfigureBrandImageView(brand_icon_image_view.get(),
                              idp_display_data.idp_metadata.brand_icon_url);

      icon_container->AddChildView(std::move(brand_icon_image_view));

      // Put BoxLayout container into FillLayout container to stack the views.
      // This stacks the IDP icon on top of the background image.
      background_container->AddChildView(std::move(icon_container));

      avatar_view = std::move(background_container);
    } else {
      account_image_view->FetchAccountImage(account, *image_fetcher_,
                                            avatar_size);
      avatar_view = std::move(account_image_view);
    }
    std::unique_ptr<views::ImageView> arrow_icon_view = nullptr;
    if (is_modal_dialog) {
      constexpr int kArrowIconRightPadding = 8;
      arrow_icon_view = std::make_unique<views::ImageView>();
      arrow_icon_view->SetBorder(views::CreateEmptyBorder(
          gfx::Insets::TLBR(/*top=*/0, /*left=*/0, /*bottom=*/0,
                            /*right=*/kArrowIconRightPadding)));
      arrow_icon_view->SetImage(ui::ImageModel::FromVectorIcon(
          vector_icons::kSubmenuArrowIcon, ui::kColorIcon, kArrowIconSize));
    }

    // We can pass crefs to OnAccountSelected because the `observer_` owns the
    // data.
    std::u16string footer =
        should_include_idp ? idp_display_data.idp_etld_plus_one : u"";
    auto row = std::make_unique<HoverButton>(
        base::BindRepeating(
            &AccountSelectionViewBase::Observer::OnAccountSelected,
            base::Unretained(observer_), std::cref(account),
            std::cref(idp_display_data)),
        std::move(avatar_view),
        /*title=*/base::UTF8ToUTF16(account.name),
        /*subtitle=*/base::UTF8ToUTF16(account.email),
        /*secondary_view=*/std::move(arrow_icon_view),
        /*add_vertical_label_spacing=*/true, footer);
    row->SetBorder(views::CreateEmptyBorder(gfx::Insets::VH(
        /*vertical=*/additional_vertical_padding,
        /*horizontal=*/is_modal_dialog ? kModalHorizontalSpacing
                                       : kLeftRightPadding)));
    row->SetTitleTextStyle(account_name_style, ui::kColorDialogBackground,
                           /*color_id=*/std::nullopt);
    row->SetSubtitleTextStyle(views::style::CONTEXT_LABEL, account_email_style);
    if (should_include_idp) {
      row->SetFooterTextStyle(views::style::CONTEXT_LABEL, account_email_style);
    }
    return row;
  }
  account_image_view->FetchAccountImage(account, *image_fetcher_, avatar_size);
  auto row = std::make_unique<views::View>();
  row->SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kHorizontal,
      gfx::Insets::VH(
          /*vertical=*/kVerticalSpacing + additional_vertical_padding,
          /*horizontal=*/is_modal_dialog ? kModalHorizontalSpacing : 0),
      kLeftRightPadding));
  row->AddChildView(std::move(account_image_view));
  views::View* const text_column =
      row->AddChildView(std::make_unique<views::View>());
  text_column->SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical));

  // Add account name.
  views::StyledLabel* const account_name =
      text_column->AddChildView(std::make_unique<views::StyledLabel>());
  account_name->SetDefaultTextStyle(account_name_style);
  account_name->SetText(base::UTF8ToUTF16(account.name));
  account_name->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

  // Add account email.
  views::Label* const account_email =
      text_column->AddChildView(std::make_unique<views::Label>(
          base::UTF8ToUTF16(account.email),
          views::style::CONTEXT_DIALOG_BODY_TEXT, account_email_style));
  account_email->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

  return row;
}

void AccountSelectionViewBase::AddIdpImage(const GURL& image_url,
                                           const gfx::ImageSkia& image) {
  brand_icon_images_[image_url] = image;
}

void AccountSelectionViewBase::ConfigureBrandImageView(
    BrandIconImageView* image_view,
    const GURL& brand_icon_url) {
  // Show placeholder brand icon prior to brand icon being fetched so that
  // header text wrapping does not change when brand icon is fetched.
  bool has_icon = brand_icon_url.is_valid();
  image_view->SetVisible(has_icon);
  if (!has_icon) {
    return;
  }

  auto it = brand_icon_images_.find(brand_icon_url);
  if (it != brand_icon_images_.end()) {
    image_view->CropAndSetImage(it->second);
    return;
  }

  image_view->FetchImage(brand_icon_url, *image_fetcher_);
}

std::unique_ptr<views::StyledLabel>
AccountSelectionViewBase::CreateDisclosureLabel(
    const IdentityProviderDisplayData& idp_display_data) {
  // It requires a StyledLabel so that we can add the links
  // to the privacy policy and terms of service URLs.
  std::unique_ptr<views::StyledLabel> disclosure_label =
      std::make_unique<views::StyledLabel>();
  disclosure_label->SetHorizontalAlignment(
      gfx::HorizontalAlignment::ALIGN_LEFT);

  // Set custom top margin for `disclosure_label` in order to take
  // (line_height - font_height) into account.
  disclosure_label->SetBorder(
      views::CreateEmptyBorder(gfx::Insets::TLBR(5, 0, 0, 0)));
  disclosure_label->SetDefaultTextStyle(views::style::STYLE_SECONDARY);

  const content::ClientMetadata& client_metadata =
      idp_display_data.client_metadata;
  int disclosure_resource_id = SelectDisclosureTextResourceId(
      client_metadata.privacy_policy_url, client_metadata.terms_of_service_url);

  // The order that the links are added to `link_data` should match the order of
  // the links in `disclosure_resource_id`.
  std::vector<std::pair<LinkType, GURL>> link_data;
  if (!client_metadata.privacy_policy_url.is_empty()) {
    link_data.emplace_back(LinkType::PRIVACY_POLICY,
                           client_metadata.privacy_policy_url);
  }
  if (!client_metadata.terms_of_service_url.is_empty()) {
    link_data.emplace_back(LinkType::TERMS_OF_SERVICE,
                           client_metadata.terms_of_service_url);
  }

  // Each link has both <ph name="BEGIN_LINK"> and <ph name="END_LINK">.
  std::vector<std::u16string> replacements = {
      idp_display_data.idp_etld_plus_one};
  replacements.insert(replacements.end(), link_data.size() * 2,
                      std::u16string());

  std::vector<size_t> offsets;
  const std::u16string disclosure_text = l10n_util::GetStringFUTF16(
      disclosure_resource_id, replacements, &offsets);
  disclosure_label->SetText(disclosure_text);

  size_t offset_index = 1u;
  for (const std::pair<LinkType, GURL>& link_data_item : link_data) {
    disclosure_label->AddStyleRange(
        gfx::Range(offsets[offset_index], offsets[offset_index + 1]),
        views::StyledLabel::RangeStyleInfo::CreateForLink(base::BindRepeating(
            &AccountSelectionViewBase::Observer::OnLinkClicked,
            base::Unretained(observer_), link_data_item.first,
            link_data_item.second)));
    offset_index += 2;
  }

  return disclosure_label;
}

base::WeakPtr<views::Widget> AccountSelectionViewBase::GetDialogWidget() {
  return dialog_widget_;
}

// static
net::NetworkTrafficAnnotationTag
AccountSelectionViewBase::GetTrafficAnnotation() {
  return kTrafficAnnotation;
}

bool AccountSelectionViewBase::CanFitInWebContents() {
  CHECK(web_contents_ && dialog_widget_);

  gfx::Size web_contents_size = web_contents_->GetSize();
  gfx::Size preferred_bubble_size =
      dialog_widget_->GetContentsView()->GetPreferredSize();

  // TODO(crbug.com/340368623): Figure out what to do when button flow modal
  // cannot fit in web contents. The offsets kRightMargin and kTopMargin pertain
  // to the bubble widget.
  return preferred_bubble_size.width() <
             (web_contents_size.width() - kRightMargin) &&
         preferred_bubble_size.height() <
             (web_contents_size.height() - kTopMargin);
}
