// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_ACCOUNT_PICKER_ACCOUNT_PICKER_CONFIGURATION_H_
#define IOS_CHROME_BROWSER_UI_ACCOUNT_PICKER_ACCOUNT_PICKER_CONFIGURATION_H_

#import <Foundation/Foundation.h>

// Configuration for the AccountPickerCoordinator.
@interface AccountPickerConfiguration : NSObject

// Title of the account picker confirmation screen.
@property(nonatomic, copy) NSString* titleText;

// Body of the account picker confirmation screen which explains what the
// account will be used for.
@property(nonatomic, copy) NSString* bodyText;

// Title of the account picker confirmation screen submit button.
@property(nonatomic, copy) NSString* submitButtonTitle;

// The label of the "Ask every time" switch. If left nil, the switch will not be
// shown.
@property(nonatomic, copy) NSString* askEveryTimeSwitchLabelText;

// If yes, view is dismissed when tapping on babground.
@property(nonatomic, assign) BOOL dismissOnBackgroundTap;

// Enable always bounce on the view.
@property(nonatomic, assign) BOOL alwaysBounceVertical;

// Whether to use the default corner radius for the account selection (if `NO`
// set the corner radius to match the UIButton one).
@property(nonatomic, assign) BOOL defaultCornerRadius;

@end

#endif  // IOS_CHROME_BROWSER_UI_ACCOUNT_PICKER_ACCOUNT_PICKER_CONFIGURATION_H_
