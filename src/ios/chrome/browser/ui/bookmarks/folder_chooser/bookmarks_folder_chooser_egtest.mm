// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>
#import <XCTest/XCTest.h>

#import "base/apple/foundation_util.h"
#import "base/i18n/message_formatter.h"
#import "base/ios/ios_util.h"
#import "base/strings/sys_string_conversions.h"
#import "base/strings/utf_string_conversions.h"
#import "components/strings/grit/components_strings.h"
#import "components/sync/base/features.h"
#import "ios/chrome/browser/bookmarks/model/bookmark_model_type.h"
#import "ios/chrome/browser/signin/model/fake_system_identity.h"
#import "ios/chrome/browser/ui/authentication/signin_earl_grey.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_earl_grey.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_earl_grey_ui.h"
#import "ios/chrome/browser/ui/bookmarks/bookmark_ui_constants.h"
#import "ios/chrome/grit/ios_strings.h"
#import "ios/chrome/test/earl_grey/chrome_actions.h"
#import "ios/chrome/test/earl_grey/chrome_earl_grey.h"
#import "ios/chrome/test/earl_grey/chrome_earl_grey_ui.h"
#import "ios/chrome/test/earl_grey/chrome_matchers.h"
#import "ios/chrome/test/earl_grey/web_http_server_chrome_test_case.h"
#import "ios/testing/earl_grey/earl_grey_test.h"
#import "net/test/embedded_test_server/embedded_test_server.h"
#import "ui/base/l10n/l10n_util.h"

using chrome_test_util::BookmarksDeleteSwipeButton;
using chrome_test_util::BookmarksHomeDoneButton;
using chrome_test_util::BookmarksNavigationBarBackButton;
using chrome_test_util::BookmarksSaveEditDoneButton;
using chrome_test_util::BookmarksSaveEditFolderButton;
using chrome_test_util::ButtonWithAccessibilityLabelId;
using chrome_test_util::ContextBarCenterButtonWithLabel;
using chrome_test_util::ContextBarLeadingButtonWithLabel;
using chrome_test_util::OmniboxText;
using chrome_test_util::ScrollToTop;
using chrome_test_util::TabGridEditButton;
using chrome_test_util::TappableBookmarkNodeWithLabel;

enum class KindOfTest { kSignedOut, kLocal, kAccount };

BookmarkModelType kindOfTestToStorageType(KindOfTest kind) {
  switch (kind) {
    case KindOfTest::kSignedOut:
    case KindOfTest::kLocal:
      return BookmarkModelType::kLocalOrSyncable;
    case KindOfTest::kAccount:
      return BookmarkModelType::kAccount;
  }
}

// Bookmark folders integration tests for Chrome.
@interface BookmarksFolderChooserTestCase : WebHttpServerChromeTestCase
@end

@implementation BookmarksFolderChooserTestCase

- (void)setUp {
  [super setUp];

  [BookmarkEarlGrey waitForBookmarkModelsLoaded];
  [BookmarkEarlGrey clearBookmarks];
}

// Tear down called once per test.
- (void)tearDown {
  [super tearDown];
  [BookmarkEarlGrey clearBookmarks];
  [BookmarkEarlGrey clearBookmarksPositionCache];
}

#pragma mark - utility functions

- (NSString*)getCreateNewFolderCellIdentifier:(KindOfTest)kind {
  switch (kind) {
    case KindOfTest::kSignedOut:
    case KindOfTest::kLocal:
      return kBookmarkCreateNewLocalOrSyncableFolderCellIdentifier;
    case KindOfTest::kAccount:
      return kBookmarkCreateNewAccountFolderCellIdentifier;
  }
}

#pragma mark - BookmarksFolderChooser Tests

// Tests that new folder is created under `Mobile Bookmarks` by default.
// TODO(crbug.com/1442459): Add this test after support is available.
// - (void)testCreateNewAccountFolderDefaultDestination {}

// Tests that new folder is created under `Mobile Bookmarks` by default.
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testCreateNewLocalOrSyncableFolderDefaultDestinationSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCreateNewLocalOrSyncableFolderDefaultDestination:
            KindOfTest::kSignedOut];
}
- (void)testCreateNewLocalOrSyncableFolderDefaultDestinationLocal {
  [self util_testCreateNewLocalOrSyncableFolderDefaultDestination:KindOfTest::
                                                                      kLocal];
}
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testCreateNewLocalOrSyncableFolderDefaultDestinationAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCreateNewLocalOrSyncableFolderDefaultDestination:KindOfTest::
                                                                      kAccount];
}
- (void)util_testCreateNewLocalOrSyncableFolderDefaultDestination:
    (KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Open `Folder 3` nested in `Folder 1->Folder 2`.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_tap()];
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 2")]
      performAction:grey_tap()];

  // Long press on `Folder 3` and try to move it to a new folder.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 3")]
      performAction:grey_longPress()];
  [[EarlGrey selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                          IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)]
      performAction:grey_tap()];
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     [self getCreateNewFolderCellIdentifier:kindOfTest])]
      performAction:grey_tap()];

  // Verify default parent folder is 'Mobile Bookmarks'.
  [[EarlGrey selectElementWithMatcher:grey_allOf(grey_accessibilityID(
                                                     @"Change Folder"),
                                                 grey_accessibilityLabel(

                                                     @"Mobile Bookmarks"),
                                                 nil)]
      assertWithMatcher:grey_sufficientlyVisible()];
}

// Tests moving bookmarks into a new folder created in the moving process.
// TODO(crbug.com/326425036): Figure out why this test is flaky with this test.
- (void)DISABLED_testCreateNewFolderWhileMovingBookmarksSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCreateNewFolderWhileMovingBookmarks:KindOfTest::kSignedOut];
}
- (void)testCreateNewFolderWhileMovingBookmarksLocal {
  [self util_testCreateNewFolderWhileMovingBookmarks:KindOfTest::kLocal];
}
- (void)testCreateNewFolderWhileMovingBookmarksAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCreateNewFolderWhileMovingBookmarks:KindOfTest::kAccount];
}
- (void)util_testCreateNewFolderWhileMovingBookmarks:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Change to edit mode
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Select URLs.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Second URL")]
      performAction:grey_tap()];
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"First URL")]
      performAction:grey_tap()];

  // Tap on "More".
  [[EarlGrey
      selectElementWithMatcher:ContextBarCenterButtonWithLabel(
                                   [BookmarkEarlGreyUI contextBarMoreString])]
      performAction:grey_tap()];

  // Tap on Move.
  [[EarlGrey selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                          IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)]
      performAction:grey_tap()];

  // Choose to move the bookmark into a new folder.
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     [self getCreateNewFolderCellIdentifier:kindOfTest])]
      performAction:grey_tap()];

  // Enter custom new folder name.
  [BookmarkEarlGreyUI
      renameBookmarkFolderWithFolderTitle:@"Title For New Folder"];

  // Verify current parent folder (Change Folder) is Bookmarks folder.
  NSString* label = (kindOfTest == KindOfTest::kSignedOut)
                        ? @"Mobile Bookmarks. Only on this device."
                        : @"Mobile Bookmarks";
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityID(@"Change Folder"),
                                   grey_accessibilityLabel(label), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Choose new parent folder (Change Folder).
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Change Folder")]
      performAction:grey_tap()];

  // Verify folder picker UI is displayed.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Verify Folder 2 only has one item.
  [BookmarkEarlGrey verifyChildCount:1
                    inFolderWithName:@"Folder 2"
                           inStorage:kindOfTestToStorageType(kindOfTest)];

  // Select Folder 2 as new Change Folder.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 2")]
      performAction:grey_tap()];

  // Verify folder picker is dismissed and folder creator is now visible.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderCreateViewContainerIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      assertWithMatcher:grey_notVisible()];

  // Verify picked parent folder (Change Folder) is Folder 2.
  label = (kindOfTest == KindOfTest::kSignedOut)
              ? @"Folder 2. Only on this device."
              : @"Folder 2";
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityID(@"Change Folder"),
                                   grey_accessibilityLabel(label), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Tap Done to close bookmark move flow.
  [[EarlGrey selectElementWithMatcher:BookmarksSaveEditFolderButton()]
      performAction:grey_tap()];

  // Verify all folder flow UI is now closed.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderCreateViewContainerIdentifier)]
      assertWithMatcher:grey_notVisible()];
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      assertWithMatcher:grey_notVisible()];
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarkFolderEditViewContainerIdentifier)]
      assertWithMatcher:grey_notVisible()];

  // Verify new folder has been created under Folder 2.
  [BookmarkEarlGrey verifyChildCount:2
                    inFolderWithName:@"Folder 2"
                           inStorage:kindOfTestToStorageType(kindOfTest)];

  // Verify new folder has two bookmarks.
  [BookmarkEarlGrey verifyChildCount:2
                    inFolderWithName:@"Title For New Folder"
                           inStorage:kindOfTestToStorageType(kindOfTest)];
}

- (void)testCantDeleteFolderBeingEditedSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCantDeleteFolderBeingEdited:KindOfTest::kSignedOut];
}
- (void)testCantDeleteFolderBeingEditedLocal {
  [self util_testCantDeleteFolderBeingEdited:KindOfTest::kLocal];
}
- (void)testCantDeleteFolderBeingEditedAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCantDeleteFolderBeingEdited:KindOfTest::kAccount];
}
- (void)util_testCantDeleteFolderBeingEdited:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Create a new folder and type "New Folder 1" without pressing return.
  NSString* newFolderTitle = @"New Folder";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:NO];

  // Swipe action to try to delete the newly created folder while its name its
  // being edited.
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(grey_accessibilityID(@"New Folder"),
                                          grey_minimumVisiblePercent(0.7), nil)]
      performAction:grey_swipeFastInDirection(kGREYDirectionLeft)];

  // Verify the delete confirmation button doesn't show up.
  [[EarlGrey selectElementWithMatcher:BookmarksDeleteSwipeButton()]
      assertWithMatcher:grey_nil()];
}

- (void)testNavigateAwayFromFolderBeingEditedSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testNavigateAwayFromFolderBeingEdited:KindOfTest::kSignedOut];
}
- (void)testNavigateAwayFromFolderBeingEditedLocal {
  [self util_testNavigateAwayFromFolderBeingEdited:KindOfTest::kLocal];
}
- (void)testNavigateAwayFromFolderBeingEditedAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testNavigateAwayFromFolderBeingEdited:KindOfTest::kAccount];
}
- (void)util_testNavigateAwayFromFolderBeingEdited:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupBookmarksWhichExceedsScreenHeightInStorage:kindOfTestToStorageType(
                                                          kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Verify bottom URL is not visible before scrolling to bottom (make sure
  // setupBookmarksWhichExceedsScreenHeight works as expected).
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Bottom URL")]
      assertWithMatcher:grey_notVisible()];

  // Verify the top URL is visible (isn't covered by the navigation bar).
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Top URL")]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Test new folder could be created.  This verifies bookmarks scrolled to
  // bottom successfully for folder name editng.
  NSString* newFolderTitle = @"New Folder";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:NO];

  // Scroll to top to navigate away from the folder being created.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarksHomeTableViewIdentifier)]
      performAction:ScrollToTop()];

  // Scroll back to the Folder being created.
  [BookmarkEarlGreyUI scrollToBottom];

  // Folder should still be in Edit mode, because of this match for Value.
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityValue(@"New Folder"),
                                   grey_sufficientlyVisible(), nil)]
      assertWithMatcher:grey_notNil()];
}

- (void)testDeleteSingleFolderNodeSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testDeleteSingleFolderNode:KindOfTest::kSignedOut];
}
- (void)testDeleteSingleFolderNodeLocal {
  [self util_testDeleteSingleFolderNode:KindOfTest::kLocal];
}
- (void)testDeleteSingleFolderNodeAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testDeleteSingleFolderNode:KindOfTest::kAccount];
}
- (void)util_testDeleteSingleFolderNode:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Change to edit mode
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Select single URL.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_tap()];

  // Delete it.
  [[EarlGrey
      selectElementWithMatcher:ContextBarLeadingButtonWithLabel(
                                   [BookmarkEarlGreyUI contextBarDeleteString])]
      performAction:grey_tap()];

  // Wait until it's gone.
  [BookmarkEarlGreyUI waitForDeletionOfBookmarkWithTitle:@"Folder 1"];

  // Press undo
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Undo")]
      performAction:grey_tap()];

  // Verify it's back.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 1")]
      assertWithMatcher:grey_notNil()];

  // Verify edit mode is closed (context bar back to default state).
  [BookmarkEarlGreyUI verifyContextBarInDefaultStateWithSelectEnabled:YES
                                                     newFolderEnabled:YES];
}

- (void)testSwipeDownToDismissFromEditSignedOut {
  [self util_testSwipeDownToDismissFromEditFolder:KindOfTest::kSignedOut];
}
- (void)testSwipeDownToDismissFromEditFolderSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testSwipeDownToDismissFromEditFolder:KindOfTest::kLocal];
}
- (void)testSwipeDownToDismissFromEditFolderAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testSwipeDownToDismissFromEditFolder:KindOfTest::kAccount];
}
- (void)util_testSwipeDownToDismissFromEditFolder:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Invoke Move through long press.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1.1")]
      performAction:grey_longPress()];

  [[EarlGrey selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                          IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)]
      performAction:grey_tap()];

  // Check that the TableView is presented.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      assertWithMatcher:grey_notNil()];

  // Swipe TableView down.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      performAction:grey_swipeFastInDirection(kGREYDirectionDown)];

  // Check that the TableView has been dismissed.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      assertWithMatcher:grey_nil()];
}

// Test when current navigating folder is deleted in background, empty
// background should be shown with context bar buttons disabled.
- (void)testWhenCurrentFolderDeletedInBackgroundSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testWhenCurrentFolderDeletedInBackground:KindOfTest::kSignedOut];
}
- (void)testWhenCurrentFolderDeletedInBackgroundLocal {
  [self util_testWhenCurrentFolderDeletedInBackground:KindOfTest::kLocal];
}
- (void)testWhenCurrentFolderDeletedInBackgroundAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testWhenCurrentFolderDeletedInBackground:KindOfTest::kAccount];
}
- (void)util_testWhenCurrentFolderDeletedInBackground:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Enter Folder 1
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 1")]
      performAction:grey_tap()];

  // Enter Folder 2
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 2")]
      performAction:grey_tap()];

  // Change to edit mode, using context menu.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Delete the Folder 1 and Folder 2 programmatically in background.
  [BookmarkEarlGrey
      removeBookmarkWithTitle:@"Folder 2"
                    inStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGrey
      removeBookmarkWithTitle:@"Folder 1"
                    inStorage:kindOfTestToStorageType(kindOfTest)];

  // Verify edit mode is close automatically (context bar switched back to
  // default state) and both select and new folder button are disabled.
  [BookmarkEarlGreyUI verifyContextBarInDefaultStateWithSelectEnabled:NO
                                                     newFolderEnabled:NO];

  // Verify the empty background appears.
  [BookmarkEarlGreyUI verifyEmptyBackgroundAppears];

  // Come back to Folder 1 (which is also deleted).
  [[EarlGrey selectElementWithMatcher:BookmarksNavigationBarBackButton()]
      performAction:grey_tap()];

  // Verify both select and new folder button are disabled.
  [BookmarkEarlGreyUI verifyContextBarInDefaultStateWithSelectEnabled:NO
                                                     newFolderEnabled:NO];

  // Verify the empty background appears.
  [BookmarkEarlGreyUI verifyEmptyBackgroundAppears];

  // Come back to Mobile Bookmarks.
  [[EarlGrey selectElementWithMatcher:BookmarksNavigationBarBackButton()]
      performAction:grey_tap()];

  // Ensure Folder 1.1 is seen, that means it successfully comes back to Mobile
  // Bookmarks.
  [BookmarkEarlGreyUI verifyBookmarkFolderIsSeen:@"Folder 1.1"];
}

- (void)testLongPressOnSingleSignedOut {
  [self util_testLongPressOnSingleFolder:KindOfTest::kSignedOut];
}
- (void)testLongPressOnSingleFolderLocal {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testLongPressOnSingleFolder:KindOfTest::kLocal];
}
- (void)testLongPressOnSingleFolderAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testLongPressOnSingleFolder:KindOfTest::kAccount];
}
- (void)util_testLongPressOnSingleFolder:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_longPress()];

  [BookmarkEarlGreyUI verifyContextMenuForSingleFolderWithEditEnabled:YES];

  [BookmarkEarlGreyUI dismissContextMenu];

  [ChromeEarlGrey waitForMatcher:grey_allOf(BookmarksNavigationBarBackButton(),
                                            grey_interactable(), nil)];

  // Come back to the root.
  [[EarlGrey selectElementWithMatcher:BookmarksNavigationBarBackButton()]
      performAction:grey_tap()];

  // Long press on Mobile Bookmarks.
  [[EarlGrey selectElementWithMatcher:TappableBookmarkNodeWithLabel(
                                          @"Mobile Bookmarks")]
      performAction:grey_longPress()];

  // We cannot locate new context menus any way, therefore we'll use the
  // 'Edit' action presence as proxy.
  [[EarlGrey selectElementWithMatcher:chrome_test_util::
                                          BookmarksContextMenuEditButton()]
      assertWithMatcher:grey_nil()];
}

// Verify Edit functionality for single folder selection.
- (void)testEditFunctionalityOnSingleSignedOut {
  [self util_testEditFunctionalityOnSingleFolder:KindOfTest::kSignedOut];
}
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testEditFunctionalityOnSingleFolderLocal {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testEditFunctionalityOnSingleFolder:KindOfTest::kLocal];
}
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testEditFunctionalityOnSingleFolderAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testEditFunctionalityOnSingleFolder:KindOfTest::kAccount];
}
- (void)util_testEditFunctionalityOnSingleFolder:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // 1. Edit the folder title at edit page.

  // Invoke Edit through long press.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_longPress()];

  id<GREYMatcher> editFolderMatcher =
      chrome_test_util::BookmarksContextMenuEditButton();
  [[EarlGrey selectElementWithMatcher:editFolderMatcher]
      performAction:grey_tap()];

  // Verify that the editor is present.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarkFolderEditViewContainerIdentifier)]
      assertWithMatcher:grey_notNil()];
  NSString* existingFolderTitle = @"Folder 1";
  NSString* newFolderTitle = @"New Folder Title";
  [BookmarkEarlGreyUI renameBookmarkFolderWithFolderTitle:newFolderTitle];

  [[EarlGrey selectElementWithMatcher:BookmarksSaveEditFolderButton()]
      performAction:grey_tap()];

  // Verify that the change has been made.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(existingFolderTitle)]
      assertWithMatcher:grey_nil()];
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(newFolderTitle)]
      assertWithMatcher:grey_notNil()];

  // Verify edit mode is closed (context bar back to default state).
  [BookmarkEarlGreyUI verifyContextBarInDefaultStateWithSelectEnabled:YES
                                                     newFolderEnabled:YES];

  // 2. Move a single folder at edit page.

  // Change to edit mode
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Select single folder.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(newFolderTitle)]
      performAction:grey_tap()];

  // Move the "New Folder Title" to "Folder 1.1".
  [BookmarkEarlGreyUI
      tapOnContextMenuButton:IDS_IOS_BOOKMARK_CONTEXT_MENU_EDIT_FOLDER
                  openEditor:kBookmarkFolderEditViewContainerIdentifier
           setParentFolderTo:@"Folder 1.1"
                        from:@"Mobile Bookmarks"
            onlyOnThisDevice:kindOfTest == KindOfTest::kLocal];

  // Verify edit mode remains.
  [BookmarkEarlGreyUI verifyContextBarInEditMode];

  // Close edit mode.
  [BookmarkEarlGreyUI closeContextBarEditMode];

  // Navigate to "Folder 1.1" and verify "New Folder Title" is under it.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1.1")]
      performAction:grey_tap()];
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(newFolderTitle)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // 3. Test the cancel button at edit page.

  // Change to edit mode
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Select single folder.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(newFolderTitle)]
      performAction:grey_tap()];

  // Tap cancel after modifying the title.
  [BookmarkEarlGreyUI
      tapOnContextMenuButton:IDS_IOS_BOOKMARK_CONTEXT_MENU_EDIT_FOLDER
                  openEditor:kBookmarkFolderEditViewContainerIdentifier
             modifyTextField:@"Title_textField"
                          to:@"Dummy"
                 dismissWith:@"Cancel"];

  // Verify that the bookmark was not updated.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(newFolderTitle)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Verify edit mode is stayed.
  [BookmarkEarlGreyUI verifyContextBarInEditMode];

  // 4. Test the delete button at edit page.

  // Tap context menu.
  [[EarlGrey
      selectElementWithMatcher:ContextBarCenterButtonWithLabel(
                                   [BookmarkEarlGreyUI contextBarMoreString])]
      performAction:grey_tap()];

  [[EarlGrey
      selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                   IDS_IOS_BOOKMARK_CONTEXT_MENU_EDIT_FOLDER)]
      performAction:grey_tap()];

  // Verify that the editor is present.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarkFolderEditViewContainerIdentifier)]
      assertWithMatcher:grey_notNil()];

  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarkFolderEditorDeleteButtonIdentifier)]
      performAction:grey_tap()];

  [BookmarkEarlGreyUI closeUndoSnackbarAndWait];

  // Verify that the folder is deleted.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(newFolderTitle)]
      assertWithMatcher:grey_notVisible()];

  // 5. Verify that when adding a new folder, edit mode will not mistakenly come
  // back (crbug.com/781783).

  // Create a new folder.
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:YES];

  // Tap on the new folder.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(newFolderTitle)]
      performAction:grey_tap()];

  // Verify we enter the new folder. (instead of selecting it in edit mode).
  [BookmarkEarlGreyUI verifyEmptyBackgroundAppears];
}

// Verify Move functionality on single folder through long press.
- (void)testMoveFunctionalityOnSingleSignedOut {
  [self util_testMoveFunctionalityOnSingleFolder:KindOfTest::kSignedOut];
}
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testMoveFunctionalityOnSingleFolderLocal {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testMoveFunctionalityOnSingleFolder:KindOfTest::kLocal];
}
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testMoveFunctionalityOnSingleFolderAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testMoveFunctionalityOnSingleFolder:KindOfTest::kAccount];
}
- (void)util_testMoveFunctionalityOnSingleFolder:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Invoke Move through long press.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1.1")]
      performAction:grey_longPress()];

  [[EarlGrey selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                          IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)]
      performAction:grey_tap()];

  // Choose to move the bookmark folder - "Folder 1" into a new folder.
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     [self getCreateNewFolderCellIdentifier:kindOfTest])]
      performAction:grey_tap()];

  // Enter custom new folder name.
  [BookmarkEarlGreyUI
      renameBookmarkFolderWithFolderTitle:@"Title For New Folder"];

  // Verify current parent folder for "Title For New Folder" folder is "Mobile
  // Bookmarks" folder.
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityID(@"Change Folder"),
                                   grey_accessibilityLabel(@"Mobile Bookmarks"),
                                   nil)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Choose new parent folder for "Title For New Folder" folder.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Change Folder")]
      performAction:grey_tap()];

  // Verify folder picker UI is displayed.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Verify Folder 2 only has one item.
  [BookmarkEarlGrey verifyChildCount:1
                    inFolderWithName:@"Folder 2"
                           inStorage:kindOfTestToStorageType(kindOfTest)];

  // Select Folder 2 as new parent folder for "Title For New Folder".
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 2")]
      performAction:grey_tap()];

  // Verify folder picker is dismissed and folder creator is now visible.
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderCreateViewContainerIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey
      selectElementWithMatcher:
          grey_accessibilityID(kBookmarkFolderPickerViewContainerIdentifier)]
      assertWithMatcher:grey_notVisible()];

  // Verify picked parent folder is Folder 2.
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityID(@"Change Folder"),
                                   grey_accessibilityLabel(@"Folder 2"), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Tap Done to close bookmark move flow.
  [[EarlGrey selectElementWithMatcher:BookmarksSaveEditFolderButton()]
      performAction:grey_tap()];

  // Verify all folder flow UI is now closed.
  [BookmarkEarlGreyUI verifyFolderFlowIsClosed];

  // Verify new folder "Title For New Folder" has been created under Folder 2.
  [BookmarkEarlGrey verifyChildCount:2
                    inFolderWithName:@"Folder 2"
                           inStorage:kindOfTestToStorageType(kindOfTest)];

  // Verify new folder "Title For New Folder" has one bookmark folder.
  [BookmarkEarlGrey verifyChildCount:1
                    inFolderWithName:@"Title For New Folder"
                           inStorage:kindOfTestToStorageType(kindOfTest)];

  // Drill down to where "Folder 1.1" has been moved and assert it's presence.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_tap()];
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 2")]
      performAction:grey_tap()];
  [[EarlGrey selectElementWithMatcher:TappableBookmarkNodeWithLabel(
                                          @"Title For New Folder")]
      performAction:grey_tap()];
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 1.1")]
      assertWithMatcher:grey_sufficientlyVisible()];
}

// Verify Move functionality on multiple folder selection.
- (void)testMoveFunctionalityOnMultipleSignedOut {
  [self util_testMoveFunctionalityOnMultipleFolder:KindOfTest::kSignedOut];
}
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testMoveFunctionalityOnMultipleFolderLocal {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testMoveFunctionalityOnMultipleFolder:KindOfTest::kLocal];
}
// TODO(crbug.com/326425036): Figure out why Chrome crash with this test.
- (void)DISABLED_testMoveFunctionalityOnMultipleFolderAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testMoveFunctionalityOnMultipleFolder:KindOfTest::kAccount];
}
- (void)util_testMoveFunctionalityOnMultipleFolder:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Change to edit mode, using context menu.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Select multiple folders.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1.1")]
      performAction:grey_tap()];
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_tap()];

  // Tap context menu.
  [[EarlGrey
      selectElementWithMatcher:ContextBarCenterButtonWithLabel(
                                   [BookmarkEarlGreyUI contextBarMoreString])]
      performAction:grey_tap()];

  [[EarlGrey selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                          IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)]
      performAction:grey_tap()];

  // Choose to move into a new folder. By tapping on the New Folder Cell.
  [[EarlGrey selectElementWithMatcher:
                 grey_accessibilityID(
                     [self getCreateNewFolderCellIdentifier:kindOfTest])]
      performAction:grey_tap()];

  // Enter custom new folder name.
  [BookmarkEarlGreyUI
      renameBookmarkFolderWithFolderTitle:@"Title For New Folder"];

  // Verify current parent folder for "Title For New Folder" folder is "Mobile
  // Bookmarks" folder.
  NSString* label;
  switch (kindOfTest) {
    case KindOfTest::kLocal:
      label = @"Mobile Bookmarks. Only on this device.";
      break;
    case KindOfTest::kSignedOut:
      label = @"Mobile Bookmarks";
      break;
    case KindOfTest::kAccount:
      label = @"Mobile Bookmarks in your Google Account, foo1@gmail.com";
      break;
  }
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityID(@"Change Folder"),
                                   grey_accessibilityLabel(label), nil)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Tap Done to close bookmark move flow.
  [[EarlGrey selectElementWithMatcher:BookmarksSaveEditFolderButton()]
      performAction:grey_tap()];

  // Verify all folder flow UI is now closed.
  [BookmarkEarlGreyUI verifyFolderFlowIsClosed];

  [BookmarkEarlGreyUI closeUndoSnackbarAndWait];

  // Verify edit mode is closed (context bar back to default state).
  [BookmarkEarlGreyUI verifyContextBarInDefaultStateWithSelectEnabled:YES
                                                     newFolderEnabled:YES];

  // Verify new folder "Title For New Folder" has two bookmark folder.
  [BookmarkEarlGrey verifyChildCount:2
                    inFolderWithName:@"Title For New Folder"
                           inStorage:kindOfTestToStorageType(kindOfTest)];

  // Drill down to where "Folder 1.1" and "Folder 1" have been moved and assert
  // it's presence.
  [[EarlGrey selectElementWithMatcher:TappableBookmarkNodeWithLabel(
                                          @"Title For New Folder")]
      performAction:grey_tap()];
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 1.1")]
      assertWithMatcher:grey_sufficientlyVisible()];
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 1")]
      assertWithMatcher:grey_sufficientlyVisible()];
}

- (void)testContextBarForSingleFolderSelectionSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testContextBarForSingleFolderSelection:KindOfTest::kSignedOut];
}
- (void)testContextBarForSingleFolderSelectionLocal {
  [self util_testContextBarForSingleFolderSelection:KindOfTest::kLocal];
}
- (void)testContextBarForSingleFolderSelectionAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testContextBarForSingleFolderSelection:KindOfTest::kAccount];
}
- (void)util_testContextBarForSingleFolderSelection:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Change to edit mode
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Select Folder.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_tap()];

  // Tap context menu.
  [[EarlGrey
      selectElementWithMatcher:ContextBarCenterButtonWithLabel(
                                   [BookmarkEarlGreyUI contextBarMoreString])]
      performAction:grey_tap()];

  // Tap Edit Folder.
  [[EarlGrey
      selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                   IDS_IOS_BOOKMARK_CONTEXT_MENU_EDIT_FOLDER)]
      performAction:grey_tap()];

  // Verify it shows edit view controller.  Uses notNil() instead of
  // sufficientlyVisible() because the large title in the navigation bar causes
  // less than 75% of the table view to be visible.
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarkFolderEditViewContainerIdentifier)]
      assertWithMatcher:grey_notNil()];
}

- (void)testContextMenuForMultipleFolderSelectionSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testContextMenuForMultipleFolderSelection:KindOfTest::kSignedOut];
}
- (void)testContextMenuForMultipleFolderSelectionLocal {
  [self util_testContextMenuForMultipleFolderSelection:KindOfTest::kLocal];
}
- (void)testContextMenuForMultipleFolderSelectionAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testContextMenuForMultipleFolderSelection:KindOfTest::kAccount];
}
- (void)util_testContextMenuForMultipleFolderSelection:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Change to edit mode
  [[EarlGrey
      selectElementWithMatcher:grey_accessibilityID(
                                   kBookmarksHomeTrailingButtonIdentifier)]
      performAction:grey_tap()];

  // Select Folders.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1")]
      performAction:grey_tap()];
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"Folder 1.1")]
      performAction:grey_tap()];

  // Tap context menu.
  [[EarlGrey
      selectElementWithMatcher:ContextBarCenterButtonWithLabel(
                                   [BookmarkEarlGreyUI contextBarMoreString])]
      performAction:grey_tap()];

  // Verify it shows the context menu.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarksHomeContextMenuIdentifier)]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Verify options on context menu.
  [[EarlGrey selectElementWithMatcher:ButtonWithAccessibilityLabelId(
                                          IDS_IOS_BOOKMARK_CONTEXT_MENU_MOVE)]
      assertWithMatcher:grey_sufficientlyVisible()];
}

// Tests that the default folder bookmarks are saved in is updated to the last
// used folder.
- (void)testStickyDefaultSignedOut {
  [self util_testStickyDefaultFolder:KindOfTest::kSignedOut];
}
- (void)testStickyDefaultFolderLocal {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [BookmarkEarlGrey setLastUsedBookmarkFolderToMobileBookmarksInStorageType:
                        BookmarkModelType::kLocalOrSyncable];
  [self util_testStickyDefaultFolder:KindOfTest::kLocal];
}
- (void)testStickyDefaultFolderAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testStickyDefaultFolder:KindOfTest::kAccount];
}
- (void)util_testStickyDefaultFolder:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Invoke Edit through long press.
  [[EarlGrey
      selectElementWithMatcher:TappableBookmarkNodeWithLabel(@"First URL")]
      performAction:grey_longPress()];
  [[EarlGrey selectElementWithMatcher:chrome_test_util::
                                          BookmarksContextMenuEditButton()]
      performAction:grey_tap()];

  // Tap the Folder button.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Change Folder")]
      performAction:grey_tap()];

  // Create a new folder.
  [BookmarkEarlGreyUI addFolderWithName:@"Sticky Folder"
                                inModel:kindOfTestToStorageType(kindOfTest)];

  // Verify that the editor is present.  Uses notNil() instead of
  // sufficientlyVisible() because the large title in the navigation bar causes
  // less than 75% of the table view to be visible.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarkEditViewContainerIdentifier)]
      assertWithMatcher:grey_notNil()];

  // Tap the Done button.
  [[EarlGrey selectElementWithMatcher:BookmarksSaveEditDoneButton()]
      performAction:grey_tap()];
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarkEditViewContainerIdentifier)]
      assertWithMatcher:grey_notVisible()];

  // Close bookmarks
  [[EarlGrey selectElementWithMatcher:BookmarksHomeDoneButton()]
      performAction:grey_tap()];

  // Second, bookmark a page.

  // Verify that the bookmark that is going to be added is not in the
  // BookmarkModel.
  GREYAssertTrue(self.testServer->Start(), @"Server did not start.");
  const GURL bookmarkedURL = self.testServer->GetURL("/fullscreen.html");
  NSString* bookmarkedTitle = @"Full Screen";  // See fullscreen.html.

  [BookmarkEarlGrey
      verifyBookmarksWithTitle:bookmarkedTitle
                 expectedCount:0
                     inStorage:kindOfTestToStorageType(kindOfTest)];
  // Open the page.
  std::string expectedURLContent = bookmarkedURL.GetContent();
  [ChromeEarlGrey loadURL:bookmarkedURL];
  [[EarlGrey selectElementWithMatcher:OmniboxText(expectedURLContent)]
      assertWithMatcher:grey_notNil()];

  // Verify that the folder has only one element.
  NSString* folderTitle = @"Sticky Folder";
  [BookmarkEarlGrey verifyChildCount:1
                    inFolderWithName:folderTitle
                           inStorage:kindOfTestToStorageType(kindOfTest)];

  // Bookmark the page.
  [BookmarkEarlGreyUI starCurrentTab];

  // Verify the snackbar title.
  std::u16string title = base::SysNSStringToUTF16(folderTitle);
  std::u16string result;
  switch (kindOfTest) {
    case KindOfTest::kSignedOut:
      result = base::i18n::MessageFormatter::FormatWithNamedArgs(
          l10n_util::GetStringUTF16(IDS_IOS_BOOKMARK_PAGE_SAVED_FOLDER),
          "count", 1, "title", title);
      break;
    case KindOfTest::kLocal:
      result = base::i18n::MessageFormatter::FormatWithNamedArgs(
          l10n_util::GetStringUTF16(
              IDS_IOS_BOOKMARK_PAGE_SAVED_FOLDER_TO_DEVICE),
          "count", 1, "title", title);
      break;
    case KindOfTest::kAccount:
      result = base::i18n::MessageFormatter::FormatWithNamedArgs(
          l10n_util::GetStringUTF16(
              IDS_IOS_BOOKMARK_PAGE_SAVED_INTO_ACCOUNT_FOLDER),
          "count", 1, "title", title, "email", "foo1@gmail.com");
      break;
  }
  [[EarlGrey selectElementWithMatcher:grey_accessibilityLabel(
                                          base::SysUTF16ToNSString(result))]
      assertWithMatcher:grey_sufficientlyVisible()];

  // Verify that the newly-created bookmark is in the BookmarkModel.
  [BookmarkEarlGrey
      verifyBookmarksWithTitle:bookmarkedTitle
                 expectedCount:1
                     inStorage:kindOfTestToStorageType(kindOfTest)];

  // Verify that the folder has now two elements.
  [BookmarkEarlGrey verifyChildCount:2
                    inFolderWithName:@"Sticky Folder"
                           inStorage:kindOfTestToStorageType(kindOfTest)];
}

// Tests the new folder name is committed when name editing is interrupted by
// navigating away.
- (void)testNewFolderNameCommittedOnNavigatingAwaySignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testNewFolderNameCommittedOnNavigatingAway:KindOfTest::kSignedOut];
}
- (void)testNewFolderNameCommittedOnNavigatingAwayLocal {
  [self util_testNewFolderNameCommittedOnNavigatingAway:KindOfTest::kLocal];
}
- (void)testNewFolderNameCommittedOnNavigatingAwayAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testNewFolderNameCommittedOnNavigatingAway:KindOfTest::kAccount];
}
- (void)util_testNewFolderNameCommittedOnNavigatingAway:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Create a new folder and type "New Folder 1" without pressing return.
  NSString* newFolderTitle = @"New Folder 1";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:NO];

  // Interrupt the folder name editing by tapping on back.
  [[EarlGrey selectElementWithMatcher:BookmarksNavigationBarBackButton()]
      performAction:grey_tap()];

  // Come back to Mobile Bookmarks.
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Verify folder name "New Folder 1" was committed.
  [BookmarkEarlGreyUI verifyFolderCreatedWithTitle:newFolderTitle];

  // Create a new folder and type "New Folder 2" without pressing return.
  newFolderTitle = @"New Folder 2";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:NO];

  // Interrupt the folder name editing by tapping on done.
  [[EarlGrey selectElementWithMatcher:BookmarksHomeDoneButton()]
      performAction:grey_tap()];
  // Reopen bookmarks.
  [BookmarkEarlGreyUI openBookmarks];

  // Verify folder name "New Folder 2" was committed.
  [BookmarkEarlGreyUI verifyFolderCreatedWithTitle:newFolderTitle];

  // Create a new folder and type "New Folder 3" without pressing return.
  newFolderTitle = @"New Folder 3";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:NO];

  // Interrupt the folder name editing by entering Folder 1
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarksHomeTableViewIdentifier)]
      performAction:ScrollToTop()];

  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Folder 1")]
      performAction:grey_tap()];
  // Come back to Mobile Bookmarks.
  [[EarlGrey selectElementWithMatcher:BookmarksNavigationBarBackButton()]
      performAction:grey_tap()];

  // Verify folder name "New Folder 3" was committed.
  [BookmarkEarlGreyUI verifyFolderCreatedWithTitle:newFolderTitle];

  // Create a new folder and type "New Folder 4" without pressing return.
  newFolderTitle = @"New Folder 4";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:NO];

  // Interrupt the folder name editing by tapping on First URL.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarksHomeTableViewIdentifier)]
      performAction:ScrollToTop()];

  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"French URL")]
      performAction:grey_tap()];
  // Reopen bookmarks.
  [BookmarkEarlGreyUI openBookmarks];

  // Verify folder name "New Folder 4" was committed.
  [BookmarkEarlGreyUI verifyFolderCreatedWithTitle:newFolderTitle];
}

// Tests the creation of new folders by tapping on 'New Folder' button of the
// context bar.
- (void)testCreateNewFolderWithContextBarSignedOut {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCreateNewFolderWithContextBar:KindOfTest::kSignedOut];
}
- (void)testCreateNewFolderWithContextBarLocal {
  [self util_testCreateNewFolderWithContextBar:KindOfTest::kLocal];
}
// TODO(crbug.com/326425036): Figure out why this test is flaky with this test.
- (void)DISABLE_testCreateNewFolderWithContextBarAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testCreateNewFolderWithContextBar:KindOfTest::kAccount];
}
- (void)util_testCreateNewFolderWithContextBar:(KindOfTest)kindOfTest {
  [BookmarkEarlGrey
      setupStandardBookmarksInStorage:kindOfTestToStorageType(kindOfTest)];
  [BookmarkEarlGreyUI openBookmarks];
  [BookmarkEarlGreyUI openMobileBookmarks];

  // Create a new folder and name it "New Folder 1".
  NSString* newFolderTitle = @"New Folder 1";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:YES];

  // Verify "New Folder 1" is created.
  [BookmarkEarlGreyUI verifyFolderCreatedWithTitle:newFolderTitle];

  // Create a new folder and name it "New Folder 2".
  newFolderTitle = @"New Folder 2";
  [BookmarkEarlGreyUI createNewBookmarkFolderWithFolderTitle:newFolderTitle
                                                 pressReturn:YES];

  // Verify "New Folder 2" is created.
  [BookmarkEarlGreyUI verifyFolderCreatedWithTitle:newFolderTitle];

  // Verify context bar does not change after editing folder name.
  [BookmarkEarlGreyUI verifyContextBarInDefaultStateWithSelectEnabled:YES
                                                     newFolderEnabled:YES];
}

// Test the creation of a bookmark and new folder (by tapping on the star).
- (void)testAddBookmarkInNewSignedOut {
  [self util_testAddBookmarkInNewFolder:KindOfTest::kSignedOut];
}
- (void)testAddBookmarkInNewFolderLocal {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [BookmarkEarlGrey setLastUsedBookmarkFolderToMobileBookmarksInStorageType:
                        BookmarkModelType::kLocalOrSyncable];
  [self util_testAddBookmarkInNewFolder:KindOfTest::kLocal];
}
// TODO(crbug.com/326425036): Figure out why this test fails on
// ios-fieldtrial-rel.
- (void)DISABLED_testAddBookmarkInNewFolderAccount {
  [SigninEarlGrey signinWithFakeIdentity:[FakeSystemIdentity fakeIdentity1]];
  [self util_testAddBookmarkInNewFolder:KindOfTest::kAccount];
}
- (void)util_testAddBookmarkInNewFolder:(KindOfTest)kindOfTest {
  GREYAssertTrue(self.testServer->Start(), @"Server did not start.");
  const GURL bookmarkedURL = self.testServer->GetURL("/pony.html");
  const std::string expectedURLContent = bookmarkedURL.GetContent();
  NSString* expectedTitle = @"ponies";  // See pony.html.

  [ChromeEarlGrey loadURL:bookmarkedURL];
  [[EarlGrey selectElementWithMatcher:OmniboxText(expectedURLContent)]
      assertWithMatcher:grey_notNil()];

  [BookmarkEarlGreyUI starCurrentTab];

  std::u16string label;
  switch (kindOfTest) {
    case KindOfTest::kSignedOut:
      label = l10n_util::GetPluralStringFUTF16(IDS_IOS_BOOKMARKS_BULK_SAVED, 1);
      break;
    case KindOfTest::kLocal:
      label = base::i18n::MessageFormatter::FormatWithNamedArgs(
          l10n_util::GetStringUTF16(
              IDS_IOS_BOOKMARK_PAGE_SAVED_FOLDER_TO_DEVICE),
          "count", 1, "title", "Mobile Bookmarks");
      break;
    case KindOfTest::kAccount:
      label = base::i18n::MessageFormatter::FormatWithNamedArgs(
          l10n_util::GetStringUTF16(IDS_IOS_BOOKMARK_PAGE_SAVED_INTO_ACCOUNT),
          "count", 1, "email", "foo1@gmail.com");
      break;
  }

  // Verify the snackbar title.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityLabel(
                                          base::SysUTF16ToNSString(label))]
      assertWithMatcher:grey_notNil()];

  // Tap on the snackbar.
  NSString* snackbarLabel =
      l10n_util::GetNSString(IDS_IOS_BOOKMARK_SNACKBAR_EDIT_BOOKMARK);
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityLabel(snackbarLabel),
                                   grey_userInteractionEnabled(),
                                   grey_not(TabGridEditButton()), nil)]
      performAction:grey_tap()];

  // Verify that the newly-created bookmark is in the BookmarkModel.
  [BookmarkEarlGrey
      verifyBookmarksWithTitle:expectedTitle
                 expectedCount:1
                     inStorage:kindOfTestToStorageType(kindOfTest)];

  // Verify that the editor is present.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarkEditViewContainerIdentifier)]
      assertWithMatcher:grey_notNil()];

  NSString* change_label = @"Mobile Bookmarks";
  [[EarlGrey
      selectElementWithMatcher:grey_allOf(
                                   grey_accessibilityID(@"Change Folder"),
                                   grey_accessibilityLabel(change_label), nil)]
      assertWithMatcher:grey_notNil()];

  // Tap the Folder button.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(@"Change Folder")]
      performAction:grey_tap()];

  // Create a new folder with default name.
  [BookmarkEarlGreyUI addFolderWithName:nil
                                inModel:kindOfTestToStorageType(kindOfTest)];

  // Verify that the editor is present.
  [[EarlGrey selectElementWithMatcher:grey_accessibilityID(
                                          kBookmarkEditViewContainerIdentifier)]
      assertWithMatcher:grey_notNil()];

  [BookmarkEarlGrey
      verifyExistenceOfFolderWithTitle:@"New Folder"
                             inStorage:kindOfTestToStorageType(kindOfTest)];
}

@end
