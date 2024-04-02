// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/autofill/bottom_sheet/virtual_card_enrollment_bottom_sheet_view_controller.h"

#import "build/branding_buildflags.h"
#import "components/autofill/core/browser/payments/payments_service_url.h"
#import "components/grit/components_scaled_resources.h"
#import "ios/chrome/browser/net/model/crurl.h"
#import "ios/chrome/browser/shared/ui/table_view/cells/table_view_detail_icon_item.h"
#import "ios/chrome/browser/shared/ui/util/uikit_ui_util.h"
#import "ios/chrome/common/ui/colors/semantic_color_names.h"
#import "ios/chrome/common/ui/confirmation_alert/confirmation_alert_action_handler.h"
#import "ios/chrome/common/ui/util/text_view_util.h"
#import "url/gurl.h"

static NSString* kDetailIconCellIdentifier = @"DetailIconCell";

namespace {

// The padding above and below the logo image.
CGFloat const kLogoPadding = 16;

// The padding above and below the illustration image.
CGFloat const kIllustrationPadding = 20;

// The extra padding on the left and right of the title and explanatory message.
CGFloat const kTitlePadding = 24;

// The spacing between vertically stacked elements.
CGFloat const kVerticalSpacingMedium = 16;

// The credit card corner radius.
CGFloat const kCreditCardCellCornerRadius = 10;

// The credit card cell height.
CGFloat const kCreditCardCellHeight = 64;

}  // namespace

@interface VirtualCardEnrollmentBottomSheetViewController () <
    UITableViewDataSource,
    UITableViewDelegate,
    UITextViewDelegate,
    ConfirmationAlertActionHandler>
@end

@implementation VirtualCardEnrollmentBottomSheetViewController {
  VirtualCardEnrollmentBottomSheetData* _bottomSheetData;

  UITextView* _explanatoryMessageView;
  UIStackView* _customUnderTitleView;
}

- (void)viewDidLoad {
  self.actionHandler = self;

  // Prevent extra spacing at the top of the bottom sheet content.
  self.topAlignedLayout = YES;

  // Set the spacing between the two stack views.
  self.customSpacing = kVerticalSpacingMedium;

  // Remove extra space between the scroll view bottom and last legal message.
  self.customScrollViewBottomInsets = 0;

  // Hide the "Done" button in the navigation bar.
  self.showDismissBarButton = NO;

  self.aboveTitleView = [self createAboveTitleStackView];

  _customUnderTitleView = [self createUnderTitleView];
  [self addLegalMessages:_bottomSheetData.paymentServerLegalMessageLines];
  [self addLegalMessages:_bottomSheetData.issuerLegalMessageLines];

  self.underTitleView = _customUnderTitleView;

  [super viewDidLoad];
}

#pragma mark - VirtualCardEnrollmentBottomSheetConsumer

// Set the virtual card enrollment bottom sheet data.
- (void)setCardData:(VirtualCardEnrollmentBottomSheetData*)data {
  _bottomSheetData = data;
  self.primaryActionString = data.acceptActionText;
  self.secondaryActionString = data.cancelActionText;
}

#pragma mark - ConfirmationAlertActionHandler

- (void)confirmationAlertPrimaryAction {
  // Accept button was clicked.
  [self.delegate didAccept];
}

- (void)confirmationAlertSecondaryAction {
  // Dismiss button was clicked.
  [self.delegate didCancel];
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView*)tableView
    numberOfRowsInSection:(NSInteger)section {
  return 1;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
  return 1;
}

- (UITableViewCell*)tableView:(UITableView*)tableView
        cellForRowAtIndexPath:(NSIndexPath*)indexPath {
  TableViewDetailIconCell* cell =
      [tableView dequeueReusableCellWithIdentifier:kDetailIconCellIdentifier];

  cell.selectionStyle = UITableViewCellSelectionStyleNone;
  cell.backgroundColor = [UIColor colorNamed:kSecondaryBackgroundColor];
  cell.userInteractionEnabled = NO;
  cell.accessibilityIdentifier =
      _bottomSheetData.creditCard.cardNameAndLastFourDigits;
  [cell.textLabel
      setText:_bottomSheetData.creditCard.cardNameAndLastFourDigits];
  [cell setDetailText:_bottomSheetData.creditCard.cardDetails];
  [cell setIconImage:_bottomSheetData.creditCard.icon
            tintColor:nil
      backgroundColor:cell.backgroundColor
         cornerRadius:kCreditCardCellCornerRadius];
  [cell setTextLayoutConstraintAxis:UILayoutConstraintAxisVertical];

  return cell;
}

#pragma mark Private

- (UIStackView*)createAboveTitleStackView {
  UIStackView* aboveTitleStackView =
      [[UIStackView alloc] initWithFrame:CGRectZero];
  aboveTitleStackView.layoutMarginsRelativeArrangement = YES;
  aboveTitleStackView.layoutMargins =
      UIEdgeInsetsMake(0, kTitlePadding, 0, kTitlePadding);
  aboveTitleStackView.axis = UILayoutConstraintAxisVertical;
  aboveTitleStackView.spacing = kVerticalSpacingMedium;

  [aboveTitleStackView
      addArrangedSubview:[[UIView alloc]
                             initWithFrame:CGRectMake(0, 0, 0, kLogoPadding)]];

  [aboveTitleStackView addArrangedSubview:[self createGooglePayLogoView]];
  CGFloat logoIllustrationSpacerHeight =
      kLogoPadding + kIllustrationPadding - aboveTitleStackView.spacing;
  [aboveTitleStackView
      addArrangedSubview:
          [self createVerticalSpacerView:logoIllustrationSpacerHeight]];
  [aboveTitleStackView addArrangedSubview:[self createIllustrationView]];
  [aboveTitleStackView
      addArrangedSubview:[self createVerticalSpacerView:kIllustrationPadding]];
  [aboveTitleStackView addArrangedSubview:[self createTitleLabel]];
  _explanatoryMessageView = [self createExplanatoryMessageTextView];
  [aboveTitleStackView addArrangedSubview:_explanatoryMessageView];
  return aboveTitleStackView;
}

- (UIImageView*)createGooglePayLogoView {
  UIImageView* logoImageTitleView =
      [[UIImageView alloc] initWithImage:[self googlePayBadgeImage]];
  logoImageTitleView.contentMode = UIViewContentModeCenter;
  return logoImageTitleView;
}

- (UIView*)createVerticalSpacerView:(CGFloat)height {
  return [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, height)];
}

// Returns the google pay badge image corresponding to the current
// UIUserInterfaceStyle (light/dark mode).
- (UIImage*)googlePayBadgeImage {
  // IDR_AUTOFILL_GOOGLE_PAY_DARK only exists in official builds.
#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
  return self.traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark
             ? NativeImage(IDR_AUTOFILL_GOOGLE_PAY_DARK)
             : NativeImage(IDR_AUTOFILL_GOOGLE_PAY);
#else
  return NativeImage(IDR_AUTOFILL_GOOGLE_PAY);
#endif
}

- (UIImageView*)createIllustrationView {
  UIImageView* illustrationImageView = [[UIImageView alloc]
      initWithImage:[UIImage
                        imageNamed:@"virtual_card_enrollment_illustration"]];
  illustrationImageView.contentMode = UIViewContentModeCenter;
  return illustrationImageView;
}

- (UILabel*)createTitleLabel {
  UILabel* titleLabel = [[UILabel alloc] initWithFrame:CGRectZero];
  titleLabel.text = _bottomSheetData.title;
  titleLabel.numberOfLines = 0;  // Allow multiple lines.
  titleLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleHeadline];
  titleLabel.textColor = [UIColor colorNamed:kTextPrimaryColor];
  titleLabel.textAlignment = NSTextAlignmentCenter;
  return titleLabel;
}

- (UITextView*)createExplanatoryMessageTextView {
  UITextView* explanatoryMessageView = CreateUITextViewWithTextKit1();
  explanatoryMessageView.scrollEnabled = NO;
  explanatoryMessageView.editable = NO;
  explanatoryMessageView.delegate = self;
  explanatoryMessageView.translatesAutoresizingMaskIntoConstraints = NO;
  explanatoryMessageView.textContainerInset = UIEdgeInsetsZero;
  explanatoryMessageView.linkTextAttributes =
      @{NSForegroundColorAttributeName : [UIColor colorNamed:kBlueColor]};
  explanatoryMessageView.attributedText =
      [self attributedTextForExplanatoryMessage];
  return explanatoryMessageView;
}

- (NSAttributedString*)attributedTextForExplanatoryMessage {
  NSRange rangeOfLearnMore = [_bottomSheetData.explanatoryMessage
      rangeOfString:_bottomSheetData.learnMoreLinkText];
  NSMutableParagraphStyle* centeredTextStyle =
      [[NSMutableParagraphStyle alloc] init];
  centeredTextStyle.alignment = NSTextAlignmentCenter;
  NSMutableAttributedString* attributedText = [[NSMutableAttributedString alloc]
      initWithString:_bottomSheetData.explanatoryMessage
          attributes:@{
            NSParagraphStyleAttributeName : centeredTextStyle,
            NSForegroundColorAttributeName :
                [UIColor colorNamed:kTextSecondaryColor]
          }];
  [attributedText addAttribute:NSLinkAttributeName
                         value:@"unused"
                         range:rangeOfLearnMore];
  return attributedText;
}

- (UIStackView*)createUnderTitleView {
  UIStackView* underTitleView = [[UIStackView alloc] initWithFrame:CGRectZero];
  underTitleView.axis = UILayoutConstraintAxisVertical;
  underTitleView.spacing = kVerticalSpacingMedium;

  [underTitleView addArrangedSubview:[self createCardContainerTableView]];
  return underTitleView;
}

- (UITableView*)createCardContainerTableView {
  UITableView* cardContainerTable =
      [[UITableView alloc] initWithFrame:CGRectZero];
  cardContainerTable.rowHeight = kCreditCardCellHeight;
  cardContainerTable.separatorStyle = UITableViewCellSeparatorStyleNone;
  cardContainerTable.layer.cornerRadius = kCreditCardCellCornerRadius;
  [cardContainerTable registerClass:[TableViewDetailIconCell class]
             forCellReuseIdentifier:kDetailIconCellIdentifier];
  cardContainerTable.dataSource = self;
  cardContainerTable.delegate = self;
  [cardContainerTable.heightAnchor
      constraintEqualToConstant:kCreditCardCellHeight]
      .active = YES;
  return cardContainerTable;
}

- (void)addLegalMessages:(NSArray<SaveCardMessageWithLinks*>*)messages {
  for (SaveCardMessageWithLinks* message in messages) {
    UITextView* textView = CreateUITextViewWithTextKit1();
    textView.scrollEnabled = NO;
    textView.editable = NO;
    textView.delegate = self;
    textView.translatesAutoresizingMaskIntoConstraints = NO;
    textView.textContainerInset = UIEdgeInsetsZero;
    textView.linkTextAttributes =
        @{NSForegroundColorAttributeName : [UIColor colorNamed:kBlueColor]};
    textView.backgroundColor = UIColor.clearColor;
    textView.attributedText = [VirtualCardEnrollmentBottomSheetViewController
        attributedTextForText:message.messageText
                     linkUrls:message.linkURLs
                   linkRanges:message.linkRanges];
    [_customUnderTitleView addArrangedSubview:textView];
  }
}

+ (NSAttributedString*)attributedTextForText:(NSString*)text
                                    linkUrls:(std::vector<GURL>)linkURLs
                                  linkRanges:(NSArray*)linkRanges {
  NSMutableParagraphStyle* centeredTextStyle =
      [[NSMutableParagraphStyle alloc] init];
  centeredTextStyle.alignment = NSTextAlignmentCenter;
  NSDictionary* textAttributes = @{
    NSFontAttributeName :
        [UIFont preferredFontForTextStyle:UIFontTextStyleCaption2],
    NSForegroundColorAttributeName : [UIColor colorNamed:kTextSecondaryColor],
    NSParagraphStyleAttributeName : centeredTextStyle,
  };

  NSMutableAttributedString* attributedText =
      [[NSMutableAttributedString alloc] initWithString:text
                                             attributes:textAttributes];
  if (linkRanges) {
    [linkRanges enumerateObjectsUsingBlock:^(NSValue* rangeValue, NSUInteger i,
                                             BOOL* stop) {
      CrURL* crurl = [[CrURL alloc] initWithGURL:linkURLs[i]];
      if (!crurl || !crurl.gurl.is_valid()) {
        return;
      }
      [attributedText addAttribute:NSLinkAttributeName
                             value:crurl.nsurl
                             range:rangeValue.rangeValue];
    }];
  }
  return attributedText;
}

#pragma mark UITextViewDelegate

- (BOOL)textView:(UITextView*)textView
    shouldInteractWithURL:(NSURL*)URL
                  inRange:(NSRange)characterRange
              interaction:(UITextItemInteraction)interaction {
  if (textView == _explanatoryMessageView) {
    // The learn more link was clicked.
    [self.delegate
        didTapLinkURL:
            [[CrURL alloc]
                initWithGURL:autofill::payments::
                                 GetVirtualCardEnrollmentSupportUrl()]];
    return NO;
  } else {
    // A link in a legal message was clicked.
    [self.delegate didTapLinkURL:[[CrURL alloc] initWithNSURL:URL]];
    return NO;
  }
}

- (UIAction*)textView:(UITextView*)textView
    primaryActionForTextItem:(UITextItem*)textItem
               defaultAction:(UIAction*)defaultAction API_AVAILABLE(ios(17.0)) {
  CrURL* url = nil;
  if (textView == _explanatoryMessageView) {
    url = [[CrURL alloc]
        initWithGURL:autofill::payments::GetVirtualCardEnrollmentSupportUrl()];
  } else {
    url = [[CrURL alloc] initWithNSURL:textItem.link];
  }
  __weak VirtualCardEnrollmentBottomSheetViewController* weakSelf = self;
  return [UIAction actionWithHandler:^(UIAction* action) {
    [weakSelf.delegate didTapLinkURL:url];
  }];
}

@end
