// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_VIEW_CONTROLLER_AUDIENCE_H_
#define IOS_CHROME_BROWSER_UI_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_VIEW_CONTROLLER_AUDIENCE_H_

enum class ContentSuggestionsModuleType;
enum class SafetyCheckItemType;
@class SetUpListItemView;

// Audience for the ContentSuggestions, getting information from it.
@protocol ContentSuggestionsViewControllerAudience

// Notifies the audience of the UIKit viewWillDisappear: callback.
- (void)viewWillDisappear;

// Notifies the audience to present the Set Up List Show More Menu.
- (void)showSetUpListShowMoreMenu;

// Notifies the audience that the module of `type` should be never shown
// anymore.
- (void)neverShowModuleType:(ContentSuggestionsModuleType)type;

// Indicates that the user has tapped the context menu item to enable
// notifications for a module.
- (void)enableNotifications:(ContentSuggestionsModuleType)type;

// Indicates that the user has tapped the context menu item to disable
// notifications.
- (void)disableNotifications:(ContentSuggestionsModuleType)type;

// Notifies the audience that the Magic Stack edit button was tapped.
- (void)didTapMagicStackEditButton;

// Notifies the audience to show the list of parcels.
- (void)showMagicStackParcelList;

// Called when a Safety Check item is selected by the user.
- (void)didSelectSafetyCheckItem:(SafetyCheckItemType)type;

// Indicates that the user has tapped the given `view`.
- (void)didTapSetUpListItemView:(SetUpListItemView*)view;

@end

#endif  // IOS_CHROME_BROWSER_UI_CONTENT_SUGGESTIONS_CONTENT_SUGGESTIONS_VIEW_CONTROLLER_AUDIENCE_H_
