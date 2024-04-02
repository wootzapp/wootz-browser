// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_PUSH_NOTIFICATION_MODEL_CONSTANTS_H_
#define IOS_CHROME_BROWSER_PUSH_NOTIFICATION_MODEL_CONSTANTS_H_

#import <Foundation/Foundation.h>
#import <string>

// Key of commerce notification used in pref
// kFeaturePushNotificationPermissions.
extern const char kCommerceNotificationKey[];

// Key of content notification used in pref kFeaturePushNotificationPermissions.
extern const char kContentNotificationKey[];

// Key of sports notification used in pref kFeaturePushNotificationPermissions.
extern const char kSportsNotificationKey[];

// Key of tips notification used in pref kFeaturePushNotificationPermissions.
extern const char kTipsNotificationKey[];

// Action identifier for the Content Notifications Feedback action.
extern NSString* const kContentNotificationFeedbackActionIdentifier;

// Category identifier for the Content Notifications category that contains a
// Feedback action.
extern NSString* const kContentNotificationFeedbackCategoryIdentifier;

#endif  // IOS_CHROME_BROWSER_PUSH_NOTIFICATION_MODEL_CONSTANTS_H_
