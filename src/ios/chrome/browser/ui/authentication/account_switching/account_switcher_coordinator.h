// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_AUTHENTICATION_ACCOUNT_SWITCHING_ACCOUNT_SWITCHER_COORDINATOR_H_
#define IOS_CHROME_BROWSER_UI_AUTHENTICATION_ACCOUNT_SWITCHING_ACCOUNT_SWITCHER_COORDINATOR_H_

#import <UIKit/UIKit.h>

#import "ios/chrome/browser/shared/coordinator/chrome_coordinator/chrome_coordinator.h"

// Coordinator to display the fast account switcher view controller.
@interface AccountSwitcherCoordinator : ChromeCoordinator

// Click point to anchor the menu.
@property(nonatomic, assign) CGPoint anchorPoint;

@end

#endif  // IOS_CHROME_BROWSER_UI_AUTHENTICATION_ACCOUNT_SWITCHING_ACCOUNT_SWITCHER_COORDINATOR_H_
