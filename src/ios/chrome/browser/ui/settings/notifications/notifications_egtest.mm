// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UserNotifications/UserNotifications.h>

#import "base/strings/sys_string_conversions.h"
#import "components/commerce/core/commerce_feature_list.h"
#import "ios/chrome/browser/ui/push_notification/scoped_notification_auth_swizzler.h"
#import "ios/chrome/grit/ios_branded_strings.h"
#import "ios/chrome/grit/ios_strings.h"
#import "ios/chrome/test/earl_grey/chrome_earl_grey.h"
#import "ios/chrome/test/earl_grey/chrome_earl_grey_ui.h"
#import "ios/chrome/test/earl_grey/chrome_matchers.h"
#import "ios/chrome/test/earl_grey/chrome_test_case.h"
#import "ios/testing/earl_grey/earl_grey_test.h"
#import "ui/base/l10n/l10n_util.h"

using chrome_test_util::ButtonWithAccessibilityLabelId;
using chrome_test_util::SettingsDoneButton;
using chrome_test_util::SettingsMenuNotificationsButton;
using chrome_test_util::SettingsNotificationsTableView;

namespace {

// Returns the matcher for the Tips Notifications switch.
id<GREYMatcher> TipsSwitchMatcher() {
  NSString* title = l10n_util::GetNSString(IDS_IOS_SET_UP_LIST_TIPS_TITLE);
  return grey_accessibilityID([NSString stringWithFormat:@"%@, switch", title]);
}

// Taps the context menu item with the given label.
void TapMenuItem(int labelId) {
  id item = chrome_test_util::ContextMenuItemWithAccessibilityLabelId(labelId);
  [[EarlGrey selectElementWithMatcher:item] performAction:grey_tap()];
}

}  // namespace

// Integration tests using the Price Notifications settings screen.
@interface NotificationsTestCase : ChromeTestCase
@end

@implementation NotificationsTestCase

- (AppLaunchConfiguration)appConfigurationForTestCase {
  AppLaunchConfiguration config;
  // Feature parameters follow a key/value format to enable or disable
  // parameters.
  std::string params =
      ":enable_price_tracking/true/enable_price_notification/true";
  std::string priceNotificationsFlag =
      std::string(commerce::kCommercePriceTracking.name) + params;
  std::string shoppingListFlag = std::string("ShoppingList");
  std::string notificationMenuItemFlag =
      std::string("NotificationSettingsMenuItem");

  config.additional_args.push_back(
      "--enable-features=" + priceNotificationsFlag + "," + shoppingListFlag +
      "," + notificationMenuItemFlag);
  config.additional_args.push_back("--enable-features=IOSTipsNotifications");
  return config;
}

// Tests that the settings page is dismissed by swiping down from the top.
- (void)testPriceNotificationsSwipeDown {
  // Opens price notifications setting.
  [ChromeEarlGreyUI openSettingsMenu];
  [ChromeEarlGreyUI tapSettingsMenuButton:SettingsMenuNotificationsButton()];

  // Check that Price Notifications TableView is presented.
  [[EarlGrey selectElementWithMatcher:SettingsNotificationsTableView()]
      assertWithMatcher:grey_notNil()];

  // Swipe TableView down.
  [[EarlGrey selectElementWithMatcher:SettingsNotificationsTableView()]
      performAction:grey_swipeFastInDirection(kGREYDirectionDown)];

  // Check that Settings has been dismissed.
  [[EarlGrey selectElementWithMatcher:SettingsNotificationsTableView()]
      assertWithMatcher:grey_nil()];
}

// Tests that switching on Tips Notifications causes the alert prompt to appear
// when the user has disabled notifications.
- (void)testTipsSwitch {
  // Swizzle in the "denied' auth status for notifications.
  ScopedNotificationAuthSwizzler auth(UNAuthorizationStatusDenied, NO);
  // Opens notifications setting.
  [ChromeEarlGreyUI openSettingsMenu];
  [ChromeEarlGreyUI tapSettingsMenuButton:SettingsMenuNotificationsButton()];

  // Check that the TableView is presented.
  [[EarlGrey selectElementWithMatcher:SettingsNotificationsTableView()]
      assertWithMatcher:grey_notNil()];
  [[EarlGrey selectElementWithMatcher:TipsSwitchMatcher()]
      assertWithMatcher:grey_notNil()];

  // Toggle off the switch.
  [[EarlGrey selectElementWithMatcher:TipsSwitchMatcher()]
      performAction:grey_turnSwitchOn(NO)];

  // Toggle on the switch.
  [[EarlGrey selectElementWithMatcher:TipsSwitchMatcher()]
      performAction:grey_turnSwitchOn(YES)];

  // Tap Go To Settings action.
  TapMenuItem(IDS_IOS_NOTIFICATIONS_ALERT_GO_TO_SETTINGS);

  // Verify that settings has opened, then close it.
  XCUIApplication* settingsApp = [[XCUIApplication alloc]
      initWithBundleIdentifier:@"com.apple.Preferences"];
  GREYAssertTrue([settingsApp waitForState:XCUIApplicationStateRunningForeground
                                   timeout:5],
                 @"The iOS Settings app should have opened.");
  [settingsApp terminate];

  // Reactivate the app.
  [[[XCUIApplication alloc] init] activate];
}

@end
