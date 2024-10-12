// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "base/i18n/number_formatting.h"
#import "base/ios/ios_util.h"
#import "base/strings/string_util.h"
#import "base/strings/sys_string_conversions.h"
#import "build/branding_buildflags.h"
#import "components/policy/core/common/policy_loader_ios_constants.h"
#import "components/policy/policy_constants.h"
#import "components/search_engines/search_engines_switches.h"
#import "components/signin/ios/browser/features.h"
#import "components/signin/public/base/consent_level.h"
#import "components/signin/public/base/signin_metrics.h"
#import "components/signin/public/base/signin_switches.h"
#import "components/strings/grit/components_strings.h"
#import "components/sync/base/user_selectable_type.h"
#import "components/sync/service/sync_prefs.h"
#import "components/unified_consent/pref_names.h"
#import "ios/chrome/browser/metrics/model/metrics_app_interface.h"
#import "ios/chrome/browser/policy/model/policy_earl_grey_utils.h"
#import "ios/chrome/browser/policy/model/policy_util.h"
#import "ios/chrome/browser/shared/public/features/features.h"
#import "ios/chrome/browser/shared/ui/elements/elements_constants.h"
#import "ios/chrome/browser/signin/model/capabilities_types.h"
#import "ios/chrome/browser/signin/model/fake_system_identity.h"
#import "ios/chrome/browser/signin/model/test_constants.h"
#import "ios/chrome/browser/ui/authentication/signin/signin_constants.h"
#import "ios/chrome/browser/ui/authentication/signin_earl_grey.h"
#import "ios/chrome/browser/ui/authentication/signin_earl_grey_ui_test_util.h"
#import "ios/chrome/browser/ui/authentication/signin_matchers.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_earl_grey.h"
#import "ios/chrome/browser/ui/first_run/first_run_app_interface.h"
#import "ios/chrome/browser/ui/first_run/first_run_constants.h"
#import "ios/chrome/browser/ui/search_engine_choice/search_engine_choice_constants.h"
#import "ios/chrome/browser/ui/search_engine_choice/search_engine_choice_earl_grey_ui_test_util.h"
#import "ios/chrome/browser/ui/settings/google_services/google_services_settings_constants.h"
#import "ios/chrome/browser/ui/settings/google_services/manage_sync_settings_constants.h"
#import "ios/chrome/browser/ui/settings/settings_app_interface.h"
#import "ios/chrome/common/ui/colors/semantic_color_names.h"
#import "ios/chrome/common/ui/promo_style/constants.h"
#import "ios/chrome/common/ui/table_view/table_view_cells_constants.h"
#import "ios/chrome/grit/ios_branded_strings.h"
#import "ios/chrome/grit/ios_strings.h"
#import "ios/chrome/test/earl_grey/chrome_actions.h"
#import "ios/chrome/test/earl_grey/chrome_earl_grey.h"
#import "ios/chrome/test/earl_grey/chrome_earl_grey_ui.h"
#import "ios/chrome/test/earl_grey/chrome_matchers.h"
#import "ios/chrome/test/earl_grey/chrome_test_case.h"
#import "ios/chrome/test/earl_grey/test_switches.h"
#import "ios/testing/earl_grey/app_launch_configuration.h"
#import "ios/testing/earl_grey/app_launch_manager.h"
#import "ios/testing/earl_grey/earl_grey_test.h"
#import "ui/base/device_form_factor.h"
#import "ui/base/l10n/l10n_util.h"
#import "ui/base/test/ios/ui_image_test_utils.h"

namespace {

// Type of FRE sign-in screen intent.
typedef NS_ENUM(NSUInteger, FRESigninIntent) {
  // FRE without enterprise policy.
  FRESigninIntentRegular,
  // FRE without forced sign-in policy.
  FRESigninIntentSigninForcedByPolicy,
  // FRE without disabled sign-in policy.
  FRESigninIntentSigninDisabledByPolicy,
  // FRE with an enterprise policy which is not explicitly handled by another
  // entry.
  FRESigninIntentSigninWithPolicy,
  // FRE with the SyncDisabled enterprise policy.
  FRESigninIntentSigninWithSyncDisabledPolicy,
  // FRE with no UMA link in the first screen.
  FRESigninIntentSigninWithUMAReportingDisabledPolicy,
};

NSString* const kSyncPassphrase = @"hello";

// Returns matcher for UMA manage link.
id<GREYMatcher> ManageUMALinkMatcher() {
  return grey_allOf(grey_accessibilityLabel(@"Manage"),
                    grey_sufficientlyVisible(), nil);
}

// Dismisses the remaining screens in FRE after the default browser screen.
void DismissDefaultBrowserAndOmniboxPositionSelectionScreens() {
  id<GREYMatcher> buttonMatcher = grey_allOf(
      grey_ancestor(grey_accessibilityID(
          first_run::kFirstRunDefaultBrowserScreenAccessibilityIdentifier)),
      grey_accessibilityTrait(UIAccessibilityTraitStaticText),
      grey_accessibilityLabel(l10n_util::GetNSString(
          IDS_IOS_FIRST_RUN_DEFAULT_BROWSER_SCREEN_SECONDARY_ACTION)),
      nil);

  [[[EarlGrey selectElementWithMatcher:buttonMatcher]
      assertWithMatcher:grey_notNil()] performAction:grey_tap()];

  if ([FirstRunAppInterface isOmniboxPositionChoiceEnabled]) {
    id<GREYMatcher> omniboxPositionScreenPrimaryButton = grey_allOf(
        grey_ancestor(grey_accessibilityID(
            first_run::
                kFirstRunOmniboxPositionChoiceScreenAccessibilityIdentifier)),
        grey_accessibilityID(kPromoStylePrimaryActionAccessibilityIdentifier),
        nil);

    [[[EarlGrey selectElementWithMatcher:omniboxPositionScreenPrimaryButton]
        assertWithMatcher:grey_notNil()] performAction:grey_tap()];
  }
}

}  // namespace

// Tests first run stages
@interface FirstRunTestCase : ChromeTestCase

@end

@implementation FirstRunTestCase

- (void)setUp {
  [[self class] testForStartup];
  [super setUp];

  GREYAssertNil([MetricsAppInterface setupHistogramTester],
                @"Failed to set up histogram tester.");

  // Because this test suite changes the state of Sync passwords, wait
  // until the engine is initialized before startup.
  [ChromeEarlGrey
      waitForSyncEngineInitialized:NO
                       syncTimeout:syncher::kSyncUKMOperationsTimeout];
}

- (void)tearDown {
  [SigninEarlGrey signOut];

  // Tests that use `addBookmarkWithSyncPassphrase` must ensure that Sync
  // data is cleared before tear down to reset the Sync password state.
  [ChromeEarlGrey
      waitForSyncEngineInitialized:NO
                       syncTimeout:syncher::kSyncUKMOperationsTimeout];
  [ChromeEarlGrey clearFakeSyncServerData];

  // Clear sync prefs for data types.
  [ChromeEarlGrey
      clearUserPrefWithName:syncer::SyncPrefs::GetPrefNameForTypeForTesting(
                                syncer::UserSelectableType::kTabs)];
  [ChromeEarlGrey
      clearUserPrefWithName:syncer::SyncPrefs::GetPrefNameForTypeForTesting(
                                syncer::UserSelectableType::kHistory)];

  // Clear MSBB consent.
  [ChromeEarlGrey
      clearUserPrefWithName:unified_consent::prefs::
                                kUrlKeyedAnonymizedDataCollectionEnabled];

  [super tearDown];
}

- (AppLaunchConfiguration)appConfigurationForTestCase {
  AppLaunchConfiguration config;

  config.additional_args.push_back(std::string("-") +
                                   test_switches::kSignInAtStartup);
  config.additional_args.push_back("-FirstRunForceEnabled");
  config.additional_args.push_back("true");
  // Relaunches the app at each test to rewind the startup state.
  config.relaunch_policy = ForceRelaunchByKilling;

  return config;
}

#pragma mark - Helper

- (void)relaunchAppWithBrowserSigninMode:(BrowserSigninMode)mode {
  std::string xmlPolicyValue("<integer>");
  xmlPolicyValue += base::NumberToString(static_cast<int>(mode));
  xmlPolicyValue += "</integer>";
  [self relaunchAppWithPolicyKey:policy::key::kBrowserSignin
                  xmlPolicyValue:xmlPolicyValue];
}

// Sets policy value and relaunches the app.
- (void)relaunchAppWithPolicyKey:(std::string)policyKey
                  xmlPolicyValue:(std::string)xmlPolicyValue {
  std::string policyData = std::string("<dict><key>") + policyKey + "</key>" +
                           xmlPolicyValue + "</dict>";
  // Configure the policy to force sign-in.
  AppLaunchConfiguration config = self.appConfigurationForTestCase;
  config.additional_args.push_back(
      "-" + base::SysNSStringToUTF8(kPolicyLoaderIOSConfigurationKey));
  config.additional_args.push_back(policyData);
  // Relaunch the app to take the configuration into account.
  [[AppLaunchManager sharedManager] ensureAppLaunchedWithConfiguration:config];
}

// Checks that the sign-in screen for enterprise is displayed.
- (void)verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
    (FRESigninIntent)FRESigninIntent {
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     first_run::kFirstRunSignInScreenAccessibilityIdentifier)]
      assertWithMatcher:grey_notNil()];
  NSString* title = nil;
  NSString* subtitle = nil;
  NSArray* disclaimerStrings = nil;
  switch (FRESigninIntent) {
    case FRESigninIntentRegular:
      title = l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_TITLE);
      subtitle = l10n_util::GetNSString(
          IDS_IOS_FIRST_RUN_SIGNIN_BENEFITS_SUBTITLE_SHORT);
      disclaimerStrings = @[
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TERMS_OF_SERVICE),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_METRIC_REPORTING),
      ];
      break;
    case FRESigninIntentSigninForcedByPolicy:
      title =
          l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_TITLE_SIGNIN_FORCED);
      subtitle = l10n_util::GetNSString(
          IDS_IOS_FIRST_RUN_SIGNIN_SUBTITLE_SIGNIN_FORCED);
      disclaimerStrings = @[
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_BROWSER_MANAGED),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TERMS_OF_SERVICE),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_METRIC_REPORTING),
      ];
      break;
    case FRESigninIntentSigninDisabledByPolicy:
      if ([ChromeEarlGrey isIPadIdiom]) {
        title =
            l10n_util::GetNSString(IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TITLE_IPAD);
      } else {
        title = l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TITLE_IPHONE);
      }
      subtitle =
          l10n_util::GetNSString(IDS_IOS_FIRST_RUN_WELCOME_SCREEN_SUBTITLE);
      disclaimerStrings = @[
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_BROWSER_MANAGED),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TERMS_OF_SERVICE),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_METRIC_REPORTING),
      ];
      break;
    case FRESigninIntentSigninWithSyncDisabledPolicy:
      title = l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_TITLE);
      // Note: With SyncDisabled, the "benefits" string is not used.
      subtitle =
          l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_SUBTITLE_SHORT);
      disclaimerStrings = @[
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_BROWSER_MANAGED),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TERMS_OF_SERVICE),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_METRIC_REPORTING),
      ];
      break;
    case FRESigninIntentSigninWithPolicy:
      title = l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_TITLE);
      subtitle = l10n_util::GetNSString(
          IDS_IOS_FIRST_RUN_SIGNIN_BENEFITS_SUBTITLE_SHORT);
      disclaimerStrings = @[
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_BROWSER_MANAGED),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TERMS_OF_SERVICE),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_METRIC_REPORTING),
      ];
      break;
    case FRESigninIntentSigninWithUMAReportingDisabledPolicy:
      title = l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_TITLE);
      subtitle = l10n_util::GetNSString(
          IDS_IOS_FIRST_RUN_SIGNIN_BENEFITS_SUBTITLE_SHORT);
      disclaimerStrings = @[
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_BROWSER_MANAGED),
        l10n_util::GetNSString(
            IDS_IOS_FIRST_RUN_WELCOME_SCREEN_TERMS_OF_SERVICE),
      ];
      break;
  }
  // Validate the Title text.
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(title),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
  // Validate the Subtitle text.
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(subtitle),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
  // Validate the Managed text.
  [self verifyDisclaimerFooterWithStrings:disclaimerStrings];
}

// Checks the disclaimer footer with the list of strings. `strings` can contain
// "BEGIN_LINK" and "END_LINK" for URL tags.
- (void)verifyDisclaimerFooterWithStrings:(NSArray*)strings {
  NSString* disclaimerText = [strings componentsJoinedByString:@" "];
  // Remove URL tags.
  disclaimerText =
      [disclaimerText stringByReplacingOccurrencesOfString:@"BEGIN_LINK"
                                                withString:@""];
  disclaimerText =
      [disclaimerText stringByReplacingOccurrencesOfString:@"END_LINK"
                                                withString:@""];
  // Check the footer.
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(disclaimerText),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
}

// Checks that the default browser screen is displayed.
- (void)verifyDefaultBrowserIsDisplayed {
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(
              first_run::kFirstRunDefaultBrowserScreenAccessibilityIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
}

- (void)acceptSyncOrHistory {
  // Accept the history opt-in screen.
  [[EarlGrey selectElementWithMatcher:
                 chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()]
      performAction:grey_tap()];
}

- (void)verifySyncOrHistoryEnabled:(BOOL)enabled {
  if (enabled) {
    GREYAssertTrue([ChromeEarlGrey isSyncHistoryDataTypeSelected],
                   @"History sync was unexpectedly disabled.");
  } else {
    GREYAssertFalse([ChromeEarlGrey isSyncHistoryDataTypeSelected],
                    @"History sync was unexpectedly enabled.");
  }
}

// Returns GREYElementInteraction for `matcher`, using `scrollViewMatcher` to
// scroll.
- (GREYElementInteraction*)
    elementInteractionWithGreyMatcher:(id<GREYMatcher>)matcher
                 scrollViewIdentifier:(NSString*)scrollViewIdentifier {
  id<GREYMatcher> scrollViewMatcher =
      grey_accessibilityID(scrollViewIdentifier);
  // Needs to scroll slowly to make sure to not miss a cell if it is not
  // currently on the screen. It should not be bigger than the visible part
  // of the collection view.
  id<GREYAction> searchAction = grey_scrollInDirection(kGREYDirectionDown, 200);
  return [[EarlGrey selectElementWithMatcher:matcher]
         usingSearchAction:searchAction
      onElementWithMatcher:scrollViewMatcher];
}

@end

// Test first run stages without search engine choice
@interface FirstRunWithoutSearchEngineChoiceTestCase : FirstRunTestCase

@end

@implementation FirstRunWithoutSearchEngineChoiceTestCase

- (AppLaunchConfiguration)appConfigurationForTestCase {
  AppLaunchConfiguration config = [super appConfigurationForTestCase];

  if ([self isRunningTest:@selector
            (testHistorySyncShownWithEquallyWeightedButtons)] ||
      [self isRunningTest:@selector
            (testHistorySyncShownWithoutMinorModeRestrictions)] ||
      [self
          isRunningTest:@selector
          (testHistorySyncShownWithEquallyWeightedButtonsOnCapabilitiesFetchTimeout
              )] ||
      [self
          isRunningTest:@selector
          (testHistorySyncShownWithEquallyWeightedButtonsOnCapabilitiesFetchTimeoutThenDeclined
              )]) {
    config.features_enabled.push_back(
        switches::kMinorModeRestrictionsForHistorySyncOptIn);
  }
  if ([self isRunningTest:@selector(testHistorySyncShownAfterSignIn)]) {
    config.features_disabled.push_back(
        switches::kMinorModeRestrictionsForHistorySyncOptIn);
  }

  return config;
}

#pragma mark - Tests

// Tests that the sentinel is written at the end of the first run.
- (void)testFRESentinel {
  [ChromeEarlGrey removeFirstRunSentinel];
  GREYAssertFalse([ChromeEarlGrey hasFirstRunSentinel],
                  @"First Run Sentinel not removed");
  [ChromeEarlGreyUI waitForAppToIdle];
  // Skip sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoSecondaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoSecondaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Omnibox position choice promo dismissal.
  if ([FirstRunAppInterface isOmniboxPositionChoiceEnabled]) {
    [[self elementInteractionWithGreyMatcher:
               chrome_test_util::SigninScreenPromoSecondaryButtonMatcher()
                        scrollViewIdentifier:
                            kPromoStyleScrollViewAccessibilityIdentifier]
        performAction:grey_tap()];
  }
  [ChromeEarlGreyUI waitForAppToIdle];
  // Tests that the sentinel file has been created.
  GREYAssertTrue([ChromeEarlGrey hasFirstRunSentinel],
                 @"First Run Sentinel not created");
}

// Tests FRE with UMA default value and without sign-in.
- (void)testWithUMACheckedAndNoSignin {
  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Skip sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoSecondaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed out.
  [SigninEarlGrey verifySignedOut];
}

// Tests FRE with UMA off and without sign-in.
- (void)testWithUMAUncheckedAndNoSignin {
  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Scroll down and open the UMA dialog.
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               ManageUMALinkMatcher(),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [ChromeEarlGreyUI waitForAppToIdle];
  // Turn off UMA.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::TableViewSwitchCell(
                                   kImproveChromeItemAccessibilityIdentifier,
                                   /*is_toggled_on=*/YES,
                                   /*enabled=*/YES)]
      performAction:chrome_test_util::TurnTableViewSwitchOn(NO)];
  // Close UMA dialog.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::NavigationBarDoneButton()]
      performAction:grey_tap()];
  // Skip sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is off.
  GREYAssertFalse(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly true by default.");
  // Check signed out.
  [SigninEarlGrey verifySignedOut];
}

// Tests FRE with UMA off, reopen UMA dialog and close the FRE without sign-in.
- (void)testUMAUncheckedWhenOpenedSecondTime {
  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Scroll down and open the UMA dialog.
  id<GREYMatcher> manageUMALinkMatcher =
      grey_allOf(ManageUMALinkMatcher(), grey_sufficientlyVisible(), nil);
  [[self elementInteractionWithGreyMatcher:manageUMALinkMatcher
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];

  // This wait is required because, on devices, EG-test may tap on the button
  // while it is sliding up, which cause the tap to misses the button.
  [ChromeEarlGreyUI waitForAppToIdle];
  // Turn off UMA.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::TableViewSwitchCell(
                                   kImproveChromeItemAccessibilityIdentifier,
                                   /*is_toggled_on=*/YES,
                                   /*enabled=*/YES)]
      performAction:chrome_test_util::TurnTableViewSwitchOn(NO)];
  // Close UMA dialog.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::NavigationBarDoneButton()]
      performAction:grey_tap()];
  // Open UMA dialog again.
  [[self elementInteractionWithGreyMatcher:manageUMALinkMatcher
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [ChromeEarlGreyUI waitForAppToIdle];
  // Check UMA off.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::TableViewSwitchCell(
                                   kImproveChromeItemAccessibilityIdentifier,
                                   /*is_toggled_on=*/NO,
                                   /*enabled=*/YES)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Close UMA dialog.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::NavigationBarDoneButton()]
      performAction:grey_tap()];
  // Skip sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is off.
  GREYAssertFalse(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly true by default.");
  // Check signed out.
  [SigninEarlGrey verifySignedOut];
}

// Tests to turn off UMA, and open the UMA dialog to turn it back on.
// TODO(crbug.com/40073685): Test fails on official builds.
#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
#define MAYBE_testUMAUncheckedAndCheckItAgain \
  DISABLED_testUMAUncheckedAndCheckItAgain
#else
#define MAYBE_testUMAUncheckedAndCheckItAgain testUMAUncheckedAndCheckItAgain
#endif
- (void)MAYBE_testUMAUncheckedAndCheckItAgain {
  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Scroll down and open the UMA dialog.
  id<GREYMatcher> manageUMALinkMatcher =
      grey_allOf(ManageUMALinkMatcher(), grey_sufficientlyVisible(), nil);
  [[self elementInteractionWithGreyMatcher:manageUMALinkMatcher
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [ChromeEarlGreyUI waitForAppToIdle];
  // Turn off UMA.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::TableViewSwitchCell(
                                   kImproveChromeItemAccessibilityIdentifier,
                                   /*is_toggled_on=*/YES,
                                   /*enabled=*/YES)]
      performAction:chrome_test_util::TurnTableViewSwitchOn(NO)];
  // Close UMA dialog.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::NavigationBarDoneButton()]
      performAction:grey_tap()];
  // Open UMA dialog again.
  [[self elementInteractionWithGreyMatcher:manageUMALinkMatcher
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [ChromeEarlGreyUI waitForAppToIdle];
  // Turn UMA back on.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::TableViewSwitchCell(
                                   kImproveChromeItemAccessibilityIdentifier,
                                   /*is_toggled_on=*/NO,
                                   /*enabled=*/YES)]
      performAction:chrome_test_util::TurnTableViewSwitchOn(YES)];
  // Close UMA dialog.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::NavigationBarDoneButton()]
      performAction:grey_tap()];
  // Skip sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed out.
  [SigninEarlGrey verifySignedOut];
}

// Tests FRE with UMA off and without sign-in.
- (void)testWithUMAUncheckedAndSignin {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Scroll down and open the UMA dialog.
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               ManageUMALinkMatcher(),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [ChromeEarlGreyUI waitForAppToIdle];
  // Turn off UMA.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::TableViewSwitchCell(
                                   kImproveChromeItemAccessibilityIdentifier,
                                   /*is_toggled_on=*/YES,
                                   /*enabled=*/YES)]
      performAction:chrome_test_util::TurnTableViewSwitchOn(NO)];
  // Close UMA dialog.
  [[EarlGrey
      selectElementWithMatcher:chrome_test_util::NavigationBarDoneButton()]
      performAction:grey_tap()];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Accept sync.
  [self acceptSyncOrHistory];
  // Check that UMA is off.
  GREYAssertFalse(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly true by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
  // Check sync is on.
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [ChromeEarlGreyUI openSettingsMenu];
  [self verifySyncOrHistoryEnabled:YES];
}

// Tests FRE with UMA default value and with sign-in.
- (void)testWithUMACheckedAndSignin {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Accept sync.
  [self acceptSyncOrHistory];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
  // Check sync is on.
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [ChromeEarlGreyUI openSettingsMenu];
  [self verifySyncOrHistoryEnabled:YES];
}

// Tests FRE with UMA default value, with sign-in and no sync.
- (void)testWithUMACheckedAndSigninAndNoSync {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Refuse sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
  // Check sync is off.
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [ChromeEarlGreyUI openSettingsMenu];
  [self verifySyncOrHistoryEnabled:NO];
}

#pragma mark - Enterprise

// Tests FRE with disabled sign-in policy.
- (void)testSignInDisabledByPolicy {
  // Configure the policy to disable SignIn.
  [self relaunchAppWithBrowserSigninMode:BrowserSigninMode::kDisabled];
  // Verify 2 steps FRE with disabled sign-in policy.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentSigninDisabledByPolicy];
  // Accept FRE.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed out.
  [SigninEarlGrey verifySignedOut];
}

// Tests forced sign-in policy, and accept sync.
- (void)testForceSigninByPolicy {
  // Configure the policy to force sign-in.
  [self relaunchAppWithBrowserSigninMode:BrowserSigninMode::kForced];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE with forced sign-in policy.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentSigninForcedByPolicy];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Accept sync.
  [self acceptSyncOrHistory];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
  // Check sync is on.
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [ChromeEarlGreyUI openSettingsMenu];
  [self verifySyncOrHistoryEnabled:YES];
  // Close settings.
  [[EarlGrey selectElementWithMatcher:chrome_test_util::SettingsDoneButton()]
      performAction:grey_tap()];
}

// Tests forced sign-in policy, and refuse sync.
- (void)testForceSigninByPolicyWithoutSync {
  // Configure the policy to force sign-in.
  [self relaunchAppWithBrowserSigninMode:BrowserSigninMode::kForced];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE with forced sign-in policy.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentSigninForcedByPolicy];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Refuse sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
  // Check sync is on.
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [ChromeEarlGreyUI openSettingsMenu];
  [self verifySyncOrHistoryEnabled:NO];
  // Close settings.
  [[EarlGrey selectElementWithMatcher:chrome_test_util::SettingsDoneButton()]
      performAction:grey_tap()];
}

// Tests sign-in with sync disabled policy.
- (void)testSyncDisabledByPolicy {
  [self relaunchAppWithPolicyKey:policy::key::kSyncDisabled
                  xmlPolicyValue:"<true/>"];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE with forced sign-in policy.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentSigninWithSyncDisabledPolicy];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
  // Check sync is on.
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [ChromeEarlGreyUI openSettingsMenu];
  [self verifySyncOrHistoryEnabled:NO];
}

// Tests enterprise policy wording on FRE when incognito policy is set.
- (void)testIncognitoPolicy {
  // Configure the policy to force sign-in.
  [self relaunchAppWithPolicyKey:policy::key::kIncognitoModeAvailability
                  xmlPolicyValue:"<integer>1</integer>"];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE with forced sign-in policy.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentSigninWithPolicy];
  // Refuse sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed out.
  [SigninEarlGrey verifySignedOut];
}

// Tests that the UMA link does not appear in  FRE when UMA is disabled by
// enterprise policy.
- (void)testUMADisabledByPolicy {
  // Configure the policy to disable UMA.
  [self relaunchAppWithPolicyKey:policy::key::kMetricsReportingEnabled
                  xmlPolicyValue:"<false/>"];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify 2 steps FRE with no UMA footer.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentSigninWithUMAReportingDisabledPolicy];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Accept sync.
  [self acceptSyncOrHistory];
  // Check that UMA is off.
  GREYAssertFalse(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly true by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
}

#pragma mark - Supervised User

// TODO(crbug.com/40070867): This test is failing.
// Tests FRE with UMA default value and with sign-in for a supervised user.
- (void)DISABLED_testWithUMACheckedAndSigninSupervised {
  // Add a fake supervised identity to the device.
  FakeSystemIdentity* fakeSupervisedIdentity =
      [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeSupervisedIdentity];
  [SigninEarlGrey setIsSubjectToParentalControls:YES
                                     forIdentity:fakeSupervisedIdentity];

  // Verify 2 steps FRE.
  [self verifyEnterpriseWelcomeScreenIsDisplayedWithFRESigninIntent:
            FRESigninIntentRegular];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Accept sync.
  [self acceptSyncOrHistory];
  // Check that UMA is on.
  GREYAssertTrue(
      [FirstRunAppInterface isUMACollectionEnabled],
      @"kMetricsReportingEnabled pref was unexpectedly false by default.");
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeSupervisedIdentity];
  // Check sync is on.
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [ChromeEarlGreyUI openSettingsMenu];
  [self verifySyncOrHistoryEnabled:YES];
}

// Tests that the History Sync Opt-In screen will have equally weighted button
// for users with minor mode restrictions.
- (void)testHistorySyncShownWithEquallyWeightedButtons {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Enforce minor mode restrictions.
  [SigninEarlGrey
      setCanShowHistorySyncOptInsWithoutMinorModeRestrictions:NO
                                                  forIdentity:fakeIdentity];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Verify that the History Sync Opt-In screen is shown.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Verify that the primary and secondary buttons have the same foreground and
  // background colors.
  NSString* foregroundColorName = kBlueColor;
  NSString* backgroundColorName = kBlueHaloColor;
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(
              chrome_test_util::ButtonWithForegroundColor(foregroundColorName),
              chrome_test_util::ButtonWithBackgroundColor(backgroundColorName),
              chrome_test_util::SigninScreenPromoPrimaryButtonMatcher(), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(
              chrome_test_util::ButtonWithForegroundColor(foregroundColorName),
              chrome_test_util::ButtonWithBackgroundColor(backgroundColorName),
              chrome_test_util::SigninScreenPromoSecondaryButtonMatcher(), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Accept History Sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that latency metrics are recorded.
  GREYAssertNil([MetricsAppInterface
                    expectUniqueSampleWithCount:1
                                      forBucket:false
                                   forHistogram:@"Signin.AccountCapabilities."
                                                @"ImmediatelyAvailable"],
                @"Incorrect immediate availability histogram");
  GREYAssertNil(
      [MetricsAppInterface
          expectTotalCount:1
              forHistogram:@"Signin.AccountCapabilities.UserVisibleLatency"],
      @"Failed to record user visibile latency histogram");
  GREYAssertNil(
      [MetricsAppInterface
          expectTotalCount:1
              forHistogram:@"Signin.AccountCapabilities.FetchLatency"],
      @"Failed to record fetch latency histogram.");
  // Verify that History Sync buttons metrics are recorded.
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonsType::
                                              kHistorySyncEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Shown"],
      @"Failed to record History Sync button type histogram.");
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonClicked::
                                              kHistorySyncOptInEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Clicked"],
      @"Failed to record History Sync buttons clicked histogram.");
}

// Tests that the History Sync Opt-In screen will not have equally weighted
// button for users without minor mode restrictions.
- (void)testHistorySyncShownWithoutMinorModeRestrictions {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Set up capabilities.
  [SigninEarlGrey
      setCanShowHistorySyncOptInsWithoutMinorModeRestrictions:YES
                                                  forIdentity:fakeIdentity];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Verify that the History Sync Opt-In screen is shown.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Verify that buttons have the expected colors.
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(chrome_test_util::ButtonWithForegroundColor(
                         kSolidButtonTextColor),
                     chrome_test_util::ButtonWithBackgroundColor(kBlueColor),
                     chrome_test_util::SigninScreenPromoPrimaryButtonMatcher(),
                     nil)] assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(
              chrome_test_util::ButtonWithForegroundColor(kBlueColor),
              chrome_test_util::SigninScreenPromoSecondaryButtonMatcher(), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Decline History Sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoSecondaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that History Sync buttons metrics are recorded.
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonsType::
                                              kHistorySyncNotEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Shown"],
      @"Failed to record History Sync button type histogram.");
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:
                                static_cast<int>(
                                    signin_metrics::SyncButtonClicked::
                                        kHistorySyncCancelNotEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Clicked"],
      @"Failed to record History Sync buttons clicked histogram.");
}

// Tests that the History Sync Opt-In screen will have equally weighted button
// for users with unknown minor mode restrictions status.
- (void)
    testHistorySyncShownWithEquallyWeightedButtonsOnCapabilitiesFetchTimeout {
  // Add identity without setting capabilities.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity withUnknownCapabilities:YES];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Wait for the History Sync Opt-In screen.
  [ChromeEarlGrey
      waitForSufficientlyVisibleElementWithMatcher:
          grey_accessibilityID(kHistorySyncViewAccessibilityIdentifier)];
  // Verify that the title and subtitle are present.
  [[EarlGrey selectElementWithMatcher:grey_text(l10n_util::GetNSString(
                                          IDS_IOS_HISTORY_SYNC_TITLE))]
      assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey selectElementWithMatcher:grey_text(l10n_util::GetNSString(
                                          IDS_IOS_HISTORY_SYNC_SUBTITLE))]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Verify that the primary and secondary buttons have the same foreground and
  // background colors.
  NSString* foregroundColorName = kBlueColor;
  NSString* backgroundColorName = kBlueHaloColor;
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(
              chrome_test_util::ButtonWithForegroundColor(foregroundColorName),
              chrome_test_util::ButtonWithBackgroundColor(backgroundColorName),
              chrome_test_util::SigninScreenPromoPrimaryButtonMatcher(), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(
              chrome_test_util::ButtonWithForegroundColor(foregroundColorName),
              chrome_test_util::ButtonWithBackgroundColor(backgroundColorName),
              chrome_test_util::SigninScreenPromoSecondaryButtonMatcher(), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Accept History Sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that latency metrics are recorded later for when the capability is
  // not immediately available.
  GREYAssertNil([MetricsAppInterface
                    expectUniqueSampleWithCount:1
                                      forBucket:false
                                   forHistogram:@"Signin.AccountCapabilities."
                                                @"ImmediatelyAvailable"],
                @"Incorrect immediate availability histogram");
  GREYAssertNil(
      [MetricsAppInterface
          expectTotalCount:1
              forHistogram:@"Signin.AccountCapabilities.UserVisibleLatency"],
      @"Failed to record user visibile latency histogram");
  GREYAssertNil(
      [MetricsAppInterface
          expectTotalCount:1
              forHistogram:@"Signin.AccountCapabilities.FetchLatency"],
      @"Fetch latency should not be recorded on immediate availability.");
  // Verify that History Sync buttons metrics are recorded.
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonsType::
                                              kHistorySyncEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Shown"],
      @"Failed to record History Sync button type histogram.");
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonClicked::
                                              kHistorySyncOptInEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Clicked"],
      @"Failed to record History Sync buttons clicked histogram.");
}

// Tests that the History Sync Opt-In screen for users with unknown minor mode
// restrictions status, if declined, will have metrics correctly recorded.
- (void)
    testHistorySyncShownWithEquallyWeightedButtonsOnCapabilitiesFetchTimeoutThenDeclined {
  // Add identity without setting capabilities.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity withUnknownCapabilities:YES];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Wait for the History Sync Opt-In screen.
  [ChromeEarlGrey
      waitForSufficientlyVisibleElementWithMatcher:
          grey_accessibilityID(kHistorySyncViewAccessibilityIdentifier)];
  // Decline History Sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoSecondaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that latency metrics are recorded later for when the capability is
  // not immediately available.
  GREYAssertNil([MetricsAppInterface
                    expectUniqueSampleWithCount:1
                                      forBucket:false
                                   forHistogram:@"Signin.AccountCapabilities."
                                                @"ImmediatelyAvailable"],
                @"Incorrect immediate availability histogram");
  GREYAssertNil(
      [MetricsAppInterface
          expectTotalCount:1
              forHistogram:@"Signin.AccountCapabilities.UserVisibleLatency"],
      @"Failed to record user visibile latency histogram");
  GREYAssertNil(
      [MetricsAppInterface
          expectTotalCount:1
              forHistogram:@"Signin.AccountCapabilities.FetchLatency"],
      @"Fetch latency should not be recorded on immediate availability.");
  // Verify that History Sync buttons metrics are recorded.
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonsType::
                                              kHistorySyncEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Shown"],
      @"Failed to record History Sync button type histogram.");
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonClicked::
                                              kHistorySyncCancelEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Clicked"],
      @"Failed to record History Sync buttons clicked histogram.");
}

#pragma mark - Sync UI Disabled

// Tests sign-in with FRE when there's no account on the device.
// See https://crbug.com/1471972.
- (void)testSignInWithNoAccount {
  // Add account.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentityForSSOAuthAddAccountFlow:fakeIdentity];
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityID(
                                       kFakeAuthAddAccountButtonIdentifier),
                                   grey_sufficientlyVisible(), nil)]
      performAction:grey_tap()];
  // Verify that the primary button text is correct.
  NSString* continueAsText = l10n_util::GetNSStringF(
      IDS_IOS_FIRST_RUN_SIGNIN_CONTINUE_AS,
      base::SysNSStringToUTF16(fakeIdentity.userGivenName));
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(chrome_test_util::SigninScreenPromoPrimaryButtonMatcher(),
                     grey_descendant(grey_text(continueAsText)), nil)]
      assertWithMatcher:grey_notNil()];
  // Sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that the History Sync Opt-In screen is shown.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Check signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
}

// Tests if the user skip the Sign-in step, the History Sync Opt-in screen is
// skipped and the default browser screen is shown.
- (void)testHistorySyncSkipIfNoSignIn {
  // Skip sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifySignedOut];
  // Verify that the History Sync Opt-In screen is hidden.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_nil()];
  // Verify that the default browser screen is shown.
  [self verifyDefaultBrowserIsDisplayed];
}

// Tests if the user signs in with the first screen, the History Sync Opt-In
// screen is shown next.
- (void)testHistorySyncShownAfterSignIn {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Verify that the History Sync Opt-In screen is shown.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Verify that the footer is shown without the user's email.
  NSString* disclaimerText =
      l10n_util::GetNSString(IDS_IOS_HISTORY_SYNC_FOOTER_WITHOUT_EMAIL);
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(disclaimerText),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
  // Verify that buttons have the expected colors.
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(chrome_test_util::ButtonWithForegroundColor(
                         kSolidButtonTextColor),
                     chrome_test_util::ButtonWithBackgroundColor(kBlueColor),
                     chrome_test_util::SigninScreenPromoPrimaryButtonMatcher(),
                     nil)] assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(
              chrome_test_util::ButtonWithForegroundColor(kBlueColor),
              chrome_test_util::SigninScreenPromoSecondaryButtonMatcher(), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Accept History Sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that History Sync buttons metrics are recorded.
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonsType::
                                              kHistorySyncNotEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Shown"],
      @"Failed to record History Sync button type histogram.");
  GREYAssertNil(
      [MetricsAppInterface
          expectUniqueSampleWithCount:1
                            forBucket:static_cast<int>(
                                          signin_metrics::SyncButtonClicked::
                                              kHistorySyncOptInNotEqualWeighted)
                         forHistogram:@"Signin.SyncButtons.Clicked"],
      @"Failed to record History Sync buttons clicked histogram.");
}

// Tests that the correct subtitle is shown in the FRE sign-in screen if the
// History Sync Opt-In feature is enabled.
- (void)testSignInSubtitleIfHistorySyncOptInEnabled {
  // Verify that the first run screen is present.
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     first_run::kFirstRunSignInScreenAccessibilityIdentifier)]
      assertWithMatcher:grey_notNil()];
  // Validate the subtitle text.
  NSString* subtitle =
      l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_BENEFITS_SUBTITLE_SHORT);
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(subtitle),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
}

// Tests that the standard subtitle is shown in the FRE sign-in screen, and that
// History Sync Opt-In screen is skipped, if the sync is disabled by policy, and
// History Sync Opt-In feature is enabled.
- (void)testHistorySyncSkipIfSyncDisabled {
  [self relaunchAppWithPolicyKey:policy::key::kSyncDisabled
                  xmlPolicyValue:"<true/>"];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify that the first run screen is present.
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     first_run::kFirstRunSignInScreenAccessibilityIdentifier)]
      assertWithMatcher:grey_notNil()];
  // Verify the subtitle text is the standard one.
  NSString* subtitle =
      l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_SUBTITLE_SHORT);
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(subtitle),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Verify that the History Sync Opt-In screen is hidden.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_nil()];
  // Verify that the default browser screen is shown.
  [self verifyDefaultBrowserIsDisplayed];
}

// Tests that the standard subtitle is shown in the FRE sign-in screen, and that
// History Sync Opt-In screen is skipped, in case the tabs sync is disabled by
// policy, and History Sync Opt-In feature is enabled.
- (void)testHistorySyncSkipIfTabsSyncDisabled {
  [self relaunchAppWithPolicyKey:policy::key::kSyncTypesListDisabled
                  xmlPolicyValue:"<array><string>tabs</string></array>"];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify that the first run screen is present.
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     first_run::kFirstRunSignInScreenAccessibilityIdentifier)]
      assertWithMatcher:grey_notNil()];
  // Verify the subtitle text is the standard one.
  NSString* subtitle =
      l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_SUBTITLE_SHORT);
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(subtitle),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Verify that the History Sync Opt-In screen is hidden.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_nil()];
  // Verify that the default browser screen is shown.
  [self verifyDefaultBrowserIsDisplayed];
}

// Tests that the standard subtitle is shown in the FRE sign-in screen, and
// that History Sync Opt-In screen is shown, in case only the bookmarks sync is
// disabled by policy, and History Sync Opt-In feature is enabled.
- (void)testHistorySyncShownIfBookmarksSyncDisabled {
  [self relaunchAppWithPolicyKey:policy::key::kSyncTypesListDisabled
                  xmlPolicyValue:"<array><string>bookmarks</string></array>"];
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Verify that the first run screen is present.
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     first_run::kFirstRunSignInScreenAccessibilityIdentifier)]
      assertWithMatcher:grey_notNil()];
  // Verify the subtitle text is the standard one.
  NSString* subtitle =
      l10n_util::GetNSString(IDS_IOS_FIRST_RUN_SIGNIN_SUBTITLE_SHORT);
  [[self elementInteractionWithGreyMatcher:grey_allOf(
                                               grey_text(subtitle),
                                               grey_sufficientlyVisible(), nil)
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      assertWithMatcher:grey_notNil()];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  [SigninEarlGrey verifyPrimaryAccountWithEmail:fakeIdentity.userEmail
                                        consent:signin::ConsentLevel::kSignin];
  // Verify that the History Sync Opt-In screen is shown.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
}

// Tests that accepting History Sync enables the history sync, grants the
// history sync and MSBB consent.
- (void)testHistorySyncConsentGrantedAfterConfirm {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Accept History Sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that the default browser screen is shown.
  [self verifyDefaultBrowserIsDisplayed];
  // Verify that the history sync is enabled.
  GREYAssertTrue(
      [SigninEarlGrey
          isSelectedTypeEnabled:syncer::UserSelectableType::kHistory],
      @"History sync should be enabled.");
  GREYAssertTrue(
      [SigninEarlGrey isSelectedTypeEnabled:syncer::UserSelectableType::kTabs],
      @"Tabs sync should be enabled.");
  // TODO(crbug.com/40068130): Verify that sync consent is granted.
  // Verify that MSBB consent is granted.
  GREYAssertTrue(
      [ChromeEarlGrey
          userBooleanPref:unified_consent::prefs::
                              kUrlKeyedAnonymizedDataCollectionEnabled],
      @"MSBB consent was not granted.");
  // Verify that the identity is signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
}

// Tests that refusing History Sync keep the history syncdisabled, and does not
// grant the history sync and MSBB consent.
- (void)testHistorySyncConsentNotGrantedAfterReject {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Refuse History Sync.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that the default browser screen is shown.
  [self verifyDefaultBrowserIsDisplayed];
  // Verify that the history sync is disabled.
  GREYAssertFalse(
      [SigninEarlGrey
          isSelectedTypeEnabled:syncer::UserSelectableType::kHistory],
      @"History sync should be disabled.");
  GREYAssertFalse(
      [SigninEarlGrey isSelectedTypeEnabled:syncer::UserSelectableType::kTabs],
      @"Tabs sync should be disabled.");
  // TODO(crbug.com/40068130): Verify that sync consent is not granted.
  // Verify that MSBB consent is not granted.
  GREYAssertFalse(
      [ChromeEarlGrey
          userBooleanPref:unified_consent::prefs::
                              kUrlKeyedAnonymizedDataCollectionEnabled],
      @"MSBB consent should not be granted.");
  // Verify that the identity is signed in.
  [SigninEarlGrey verifySignedInWithFakeIdentity:fakeIdentity];
}

// Tests that the History Sync Opt-In screen contains the avatar of the
// signed-in user, and the correct background image for the avatar.
- (void)testHistorySyncLayout {
  // Add identity.
  FakeSystemIdentity* fakeIdentity = [FakeSystemIdentity fakeIdentity1];
  [SigninEarlGrey addFakeIdentity:fakeIdentity];
  // Accept sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::SigninScreenPromoPrimaryButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Verify that the History Sync Opt-In screen is shown.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kHistorySyncViewAccessibilityIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Verify that the user's avatar is shown.
  NSString* avatarLabel =
      [NSString stringWithFormat:@"%@ %@", fakeIdentity.userFullName,
                                 fakeIdentity.userEmail];
  [[EarlGrey selectElementWithMatcher:grey_accessibilityLabel(avatarLabel)]
      assertWithMatcher:grey_sufficientlyVisible()];
  // Verify that the avatar background is shown.
  [[EarlGrey
      selectElementWithMatcher:
          grey_allOf(
              grey_accessibilityID(
                  kPromoStyleHeaderViewBackgroundAccessibilityIdentifier),
              chrome_test_util::ImageViewWithImageNamed(
                  @"history_sync_opt_in_background"),
              grey_sufficientlyVisible(), nil)]
      assertWithMatcher:grey_notNil()];
}

@end

// Tests first run stages with search engine choice
@interface FirstRunWithSearchEngineChoiceTestCase : FirstRunTestCase

@end

@implementation FirstRunWithSearchEngineChoiceTestCase

- (AppLaunchConfiguration)appConfigurationForTestCase {
  AppLaunchConfiguration config = [super appConfigurationForTestCase];
  // Set the country to one that is eligible for the choice screen (in this
  // case, France).
  config.additional_args.push_back(
      "--" + std::string(switches::kSearchEngineChoiceCountry) + "=FR");
  config.features_enabled.push_back(switches::kSearchEngineChoiceTrigger);
  config.additional_args.push_back(
      "--" + std::string(switches::kForceSearchEngineChoiceScreen));
  config.additional_args.push_back("true");
  return config;
}

- (void)setUp {
  [super setUp];
  // Make sure the search engine has been reset, to avoid any issues if it was
  // not by a previous test.
  [SettingsAppInterface resetSearchEngine];
}

- (void)tearDown {
  // Reset the search engine for any other tests.
  [SettingsAppInterface resetSearchEngine];
  [super tearDown];
}

#pragma mark - Tests

// Tests that the Search Engine Choice screen is displayed, that the primary
// button is correctly updated when the user selects a search engine then
// scrolls down and that it correctly sets the default search engine.
// TODO(crbug.com/329210226): Re-enable the test.
- (void)DISABLED_testSearchEngineChoiceScreenSelectThenScroll {
  // Skips sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Checks that the choice screen is shown
  [SearchEngineChoiceEarlGreyUI verifySearchEngineChoiceScreenIsDisplayed];

  // Verifies that the primary button is initially the "More" button.
  id<GREYMatcher> moreButtonMatcher =
      grey_accessibilityID(kSearchEngineMoreButtonIdentifier);
  [[EarlGrey selectElementWithMatcher:moreButtonMatcher]
      assertWithMatcher:grey_allOf(grey_enabled(), grey_notNil(), nil)];

  // Selects a search engine.
  NSString* searchEngineToSelect = @"Bing";
  [SearchEngineChoiceEarlGreyUI
      selectSearchEngineCellWithName:searchEngineToSelect
                     scrollDirection:kGREYDirectionDown
                              amount:50];
  // Taps the primary button. This scrolls the table down to the bottom.
  [[[EarlGrey selectElementWithMatcher:moreButtonMatcher]
      assertWithMatcher:grey_notNil()] performAction:grey_tap()];
  // Verify that the "More" button has been removed.
  [[EarlGrey selectElementWithMatcher:moreButtonMatcher]
      assertWithMatcher:grey_nil()];
  [SearchEngineChoiceEarlGreyUI confirmSearchEngineChoiceScreen];
  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [SearchEngineChoiceEarlGreyUI
      verifyDefaultSearchEngineSetting:searchEngineToSelect];
}

// Tests that the Search Engine Choice screen is displayed, that the
// primary button is correctly updated when the user scrolls down then selects a
// search engine and that it correctly sets the default search engine.
// TODO(crbug.com/329210226): Re-enable the test.
- (void)DISABLED_testSearchEngineChoiceScreenScrollThenSelect {
  // Skips sign-in.
  [[self elementInteractionWithGreyMatcher:
             chrome_test_util::PromoStyleSecondaryActionButtonMatcher()
                      scrollViewIdentifier:
                          kPromoStyleScrollViewAccessibilityIdentifier]
      performAction:grey_tap()];
  // Checks that the choice screen is shown
  [SearchEngineChoiceEarlGreyUI verifySearchEngineChoiceScreenIsDisplayed];

  // Verifies that the primary button is initially the "More" button.
  id<GREYMatcher> moreButtonMatcher =
      grey_accessibilityID(kSearchEngineMoreButtonIdentifier);
  [[EarlGrey selectElementWithMatcher:moreButtonMatcher]
      assertWithMatcher:grey_allOf(grey_enabled(), grey_notNil(), nil)];

  // Taps the primary button. This scrolls the table down to the bottom.
  [[[EarlGrey selectElementWithMatcher:moreButtonMatcher]
      assertWithMatcher:grey_notNil()] performAction:grey_tap()];

  // Verifies that the primary button is now the disabled "Set as Default"
  // button.
  id<GREYMatcher> primaryActionButtonMatcher =
      grey_accessibilityID(kSetAsDefaultSearchEngineIdentifier);
  [[EarlGrey selectElementWithMatcher:primaryActionButtonMatcher]
      assertWithMatcher:grey_allOf(grey_not(grey_enabled()), grey_notNil(),
                                   nil)];

  // Selects a search engine.
  NSString* searchEngineToSelect = @"Bing";
  [SearchEngineChoiceEarlGreyUI
      selectSearchEngineCellWithName:searchEngineToSelect
                     scrollDirection:kGREYDirectionUp
                              amount:300];
  [SearchEngineChoiceEarlGreyUI confirmSearchEngineChoiceScreen];

  DismissDefaultBrowserAndOmniboxPositionSelectionScreens();
  [SearchEngineChoiceEarlGreyUI
      verifyDefaultSearchEngineSetting:searchEngineToSelect];
}
@end
