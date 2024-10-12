// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_AUTHENTICATION_ACCOUNT_SWITCHING_ACCOUNT_SWITCHER_PRESENTATION_CONTROLLER_H_
#define IOS_CHROME_BROWSER_UI_AUTHENTICATION_ACCOUNT_SWITCHING_ACCOUNT_SWITCHER_PRESENTATION_CONTROLLER_H_

#import <UIKit/UIKit.h>

// Presentation controller for presenting the AccountSwitcher. It is presenting
// it as a Modal.
@interface AccountSwitcherPresentationController : UIPresentationController

@property(nonatomic, assign) CGPoint anchorPoint;

@end

#endif  // IOS_CHROME_BROWSER_UI_AUTHENTICATION_ACCOUNT_SWITCHING_ACCOUNT_SWITCHER_PRESENTATION_CONTROLLER_H_
