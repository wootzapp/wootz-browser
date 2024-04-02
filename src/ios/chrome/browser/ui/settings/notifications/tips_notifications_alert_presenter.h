// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_SETTINGS_NOTIFICATIONS_TIPS_NOTIFICATIONS_ALERT_PRESENTER_H_
#define IOS_CHROME_BROWSER_UI_SETTINGS_NOTIFICATIONS_TIPS_NOTIFICATIONS_ALERT_PRESENTER_H_

// Protocol for displaying Tips Notifications related UIAlerts.
@protocol TipsNotificationsAlertPresenter <NSObject>

// Displays the UIAlert that directs the user to the OS permission settings to
// enable push notification permissions when the user toggles Chrome-level push
// notification permissions for tips.
- (void)presentTipsNotificationPermissionAlert;

@end

#endif  // IOS_CHROME_BROWSER_UI_SETTINGS_NOTIFICATIONS_TIPS_NOTIFICATIONS_ALERT_PRESENTER_H_
