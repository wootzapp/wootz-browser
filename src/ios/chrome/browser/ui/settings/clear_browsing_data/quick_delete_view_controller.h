// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_SETTINGS_CLEAR_BROWSING_DATA_QUICK_DELETE_VIEW_CONTROLLER_H_
#define IOS_CHROME_BROWSER_UI_SETTINGS_CLEAR_BROWSING_DATA_QUICK_DELETE_VIEW_CONTROLLER_H_

#import "ios/chrome/browser/shared/ui/bottom_sheet/table_view_bottom_sheet_view_controller.h"
#import "ios/chrome/browser/ui/settings/clear_browsing_data/browsing_data_consumer.h"

@protocol BrowsingDataMutator;
@protocol QuickDeletePresentationCommands;

// View controller for Quick Delete, the new vesion of Clear/Delete Browsing
// Data.
@interface QuickDeleteViewController
    : TableViewBottomSheetViewController <
          BrowsingDataConsumer,
          UIAdaptivePresentationControllerDelegate>

// Local dispatcher for this `QuickDeleteViewController`.
@property(nonatomic, weak) id<QuickDeletePresentationCommands>
    presentationHandler;

@property(nonatomic, weak) id<BrowsingDataMutator> mutator;

- (instancetype)init NS_DESIGNATED_INITIALIZER;

- (instancetype)initWithStyle:(UITableViewStyle)style NS_UNAVAILABLE;
- (instancetype)initWithCoder:(NSCoder*)coder NS_UNAVAILABLE;
- (instancetype)initWithNibName:(NSString*)nibNameOrNil
                         bundle:(NSBundle*)nibBundleOrNil NS_UNAVAILABLE;

@end

#endif  // IOS_CHROME_BROWSER_UI_SETTINGS_CLEAR_BROWSING_DATA_QUICK_DELETE_VIEW_CONTROLLER_H_
