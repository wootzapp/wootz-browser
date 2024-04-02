// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/tabs/model/tabs_closer.h"

#import "base/functional/bind.h"
#import "base/scoped_observation.h"
#import "base/test/test_file_util.h"
#import "components/sessions/core/tab_restore_service.h"
#import "ios/chrome/browser/sessions/fake_tab_restore_service.h"
#import "ios/chrome/browser/sessions/ios_chrome_tab_restore_service_factory.h"
#import "ios/chrome/browser/sessions/session_restoration_service_factory.h"
#import "ios/chrome/browser/sessions/test_session_restoration_service.h"
#import "ios/chrome/browser/shared/coordinator/scene/scene_state.h"
#import "ios/chrome/browser/shared/model/browser/browser.h"
#import "ios/chrome/browser/shared/model/browser_state/test_chrome_browser_state.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list_observer.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_opener.h"
#import "ios/chrome/browser/signin/model/authentication_service_factory.h"
#import "ios/chrome/browser/signin/model/fake_authentication_service_delegate.h"
#import "ios/chrome/test/ios_chrome_scoped_testing_local_state.h"
#import "ios/web/public/test/fakes/fake_navigation_manager.h"
#import "ios/web/public/test/fakes/fake_web_frames_manager.h"
#import "ios/web/public/test/fakes/fake_web_state.h"
#import "ios/web/public/test/web_task_environment.h"
#import "testing/gtest/include/gtest/gtest.h"
#import "testing/platform_test.h"
#import "third_party/ocmock/OCMock/OCMock.h"
#import "ui/base/page_transition_types.h"

namespace {

// Controls whether the WebState is inserted as pinned or regular.
enum class InsertionPolicy {
  kPinned,
  kRegular,
};

// List all ContentWorlds. Necessary because calling SetWebFramesManager(...)
// with a kAllContentWorlds is not enough with FakeWebState.
constexpr web::ContentWorld kContentWorlds[] = {
    web::ContentWorld::kAllContentWorlds,
    web::ContentWorld::kPageContentWorld,
    web::ContentWorld::kIsolatedWorld,
};

// Session name used by the fake SceneState.
const char kSceneSessionID[] = "Identifier";

// WebStateListObserver checking whether a batch operation has been
// performed (i.e. started and then completed).
class ScopedTestWebStateListObserver final : public WebStateListObserver {
 public:
  ScopedTestWebStateListObserver() = default;

  // Start observing `web_state_list`.
  void Observe(WebStateList* web_state_list) {
    scoped_observation_.Observe(web_state_list);
  }

  // Returns whether a batch operation has been performed (i.e. started
  // and then completed).
  bool BatchOperationCompleted() const {
    return batch_operation_started_ && batch_operation_ended_;
  }

  // WebStateListObserver implementation.
  void WillBeginBatchOperation(WebStateList* web_state_list) final {
    batch_operation_started_ = true;
  }

  void BatchOperationEnded(WebStateList* web_state_list) final {
    batch_operation_ended_ = true;
  }

 private:
  // Records whether a batch operation started/ended.
  bool batch_operation_started_ = false;
  bool batch_operation_ended_ = false;

  // Scoped observation used to unregister itself when the object is destroyed.
  base::ScopedObservation<WebStateList, WebStateListObserver>
      scoped_observation_{this};
};

}  // namespace

class TabsCloserTest : public PlatformTest {
 public:
  TabsCloserTest() {
    // Create a TestChromeBrowserState with required services.
    TestChromeBrowserState::Builder builder;
    builder.AddTestingFactory(
        AuthenticationServiceFactory::GetInstance(),
        AuthenticationServiceFactory::GetDefaultFactory());
    builder.AddTestingFactory(
        SessionRestorationServiceFactory::GetInstance(),
        TestSessionRestorationService::GetTestingFactory());
    builder.AddTestingFactory(IOSChromeTabRestoreServiceFactory::GetInstance(),
                              FakeTabRestoreService::GetTestingFactory());
    browser_state_ = builder.Build();

    // Initialize the AuthenticationService.
    AuthenticationServiceFactory::CreateAndInitializeForBrowserState(
        browser_state_.get(),
        std::make_unique<FakeAuthenticationServiceDelegate>());

    scene_state_ = OCMClassMock([SceneState class]);
    OCMStub([scene_state_ sceneSessionID]).andReturn(@(kSceneSessionID));
    browser_ = Browser::Create(browser_state_.get(), scene_state_);
  }

  Browser* browser() { return browser_.get(); }

  sessions::TabRestoreService* restore_service() {
    return IOSChromeTabRestoreServiceFactory::GetForBrowserState(
        browser_state_.get());
  }

  // Appends a fake WebState in `browser_` using `policy` and `opener`.
  web::WebState* AppendWebState(InsertionPolicy policy, WebStateOpener opener) {
    const GURL url = GURL("https://example.com");
    auto navigation_manager = std::make_unique<web::FakeNavigationManager>();
    navigation_manager->AddItem(url, ui::PAGE_TRANSITION_LINK);
    navigation_manager->SetLastCommittedItem(
        navigation_manager->GetItemAtIndex(0));

    auto web_state = std::make_unique<web::FakeWebState>();
    web_state->SetIsRealized(true);
    web_state->SetVisibleURL(url);
    web_state->SetBrowserState(browser_->GetBrowserState());
    web_state->SetNavigationManager(std::move(navigation_manager));
    web_state->SetNavigationItemCount(1);

    for (const web::ContentWorld content_world : kContentWorlds) {
      web_state->SetWebFramesManager(
          content_world, std::make_unique<web::FakeWebFramesManager>());
    }

    WebStateList* web_state_list = browser_->GetWebStateList();
    // Force the insertion at the end. Otherwise, the opener will trigger logic
    // to move an inserted WebState close to its opener.
    const WebStateList::InsertionParams params =
        WebStateList::InsertionParams::AtIndex(web_state_list->count())
            .Pinned(policy == InsertionPolicy::kPinned)
            .WithOpener(opener);
    const int insertion_index =
        web_state_list->InsertWebState(std::move(web_state), params);

    return web_state_list->GetWebStateAt(insertion_index);
  }

 private:
  web::WebTaskEnvironment task_environment_;
  IOSChromeScopedTestingLocalState local_state_;
  std::unique_ptr<ChromeBrowserState> browser_state_;
  __strong SceneState* scene_state_;
  std::unique_ptr<Browser> browser_;
};

// Tests how a TabsCloser behaves when presented with a Browser containing
// no tabs.
//
// Variants: ClosePolicy::kAllTabs
TEST_F(TabsCloserTest, EmptyBrowser_ClosePolicyAllTabs) {
  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kAllTabs);
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());
}

// Tests how a TabsCloser behaves when presented with a Browser containing
// no tabs.
//
// Variants: ClosePolicy::kRegularTabs
TEST_F(TabsCloserTest, EmptyBrowser_ClosePolicyRegularTabs) {
  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kRegularTabs);
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());
}

// Tests how a TabsCloser behaves when presented with a Browser containing
// only regular tabs.
//
// Variants: ClosePolicy::kAllTabs
TEST_F(TabsCloserTest, BrowserWithOnlyRegularTabs_ClosePolicyAllTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state2 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state1, 0});

  ASSERT_EQ(web_state_list->count(), 3);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  ASSERT_EQ(web_state_list->GetWebStateAt(2), web_state2);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kAllTabs);

  // Check that some tabs can be closed.
  EXPECT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that calling CloseTabs() close all the tabs registered,
  // allow to undo the operation and leave the WebStateList empty.
  EXPECT_EQ(tabs_closer.CloseTabs(), 3);
  EXPECT_TRUE(tabs_closer.CanUndoCloseTabs());
  EXPECT_TRUE(web_state_list->empty());

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as it has not been confirmed).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling UndoCloseTabs() correctly restored all the
  // closed tabs, in the correct order, and with the opener-opened
  // relationship intact.
  EXPECT_EQ(tabs_closer.UndoCloseTabs(), 3);
  ASSERT_EQ(web_state_list->count(), 3);
  EXPECT_EQ(web_state_list->pinned_tabs_count(), 0);
  EXPECT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  EXPECT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  EXPECT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(2).opener, web_state1);

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as the operation has been cancelled).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling CloseTabs() and ConfirmDeletion() correctly
  // close the tabs and prevents undo.
  ASSERT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_EQ(tabs_closer.CloseTabs(), 3);
  EXPECT_EQ(tabs_closer.ConfirmDeletion(), 3);
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that the TabRestoreService has now been informed of the
  // close operation which has been confirmed.
  EXPECT_EQ(restore_service()->entries().size(), 3u);
}

// Tests how a TabsCloser behaves when presented with a Browser containing
// only regular tabs.
//
// Variants: ClosePolicy::kRegularTabs
TEST_F(TabsCloserTest, BrowserWithOnlyRegularTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state2 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state1, 0});

  ASSERT_EQ(web_state_list->count(), 3);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  ASSERT_EQ(web_state_list->GetWebStateAt(2), web_state2);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kRegularTabs);

  // Check that some tabs can be closed.
  EXPECT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that calling CloseTabs() close all the tabs registered,
  // allow to undo the operation and leave the WebStateList empty.
  EXPECT_EQ(tabs_closer.CloseTabs(), 3);
  EXPECT_TRUE(tabs_closer.CanUndoCloseTabs());
  EXPECT_TRUE(web_state_list->empty());

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as it has not been confirmed).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling UndoCloseTabs() correctly restored all the
  // closed tabs, in the correct order, and with the opener-opened
  // relationship intact.
  EXPECT_EQ(tabs_closer.UndoCloseTabs(), 3);
  ASSERT_EQ(web_state_list->count(), 3);
  EXPECT_EQ(web_state_list->pinned_tabs_count(), 0);
  EXPECT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  EXPECT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  EXPECT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(2).opener, web_state1);

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as the operation has been cancelled).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling CloseTabs() and ConfirmDeletion() correctly
  // close the tabs and prevents undo.
  ASSERT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_EQ(tabs_closer.CloseTabs(), 3);
  EXPECT_EQ(tabs_closer.ConfirmDeletion(), 3);
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that the TabRestoreService has now been informed of the
  // close operation which has been confirmed.
  EXPECT_EQ(restore_service()->entries().size(), 3u);
}

// Tests how a TabsCloser behaves when presented with a Browser containing
// only pinned tabs.
//
// Variants: ClosePolicy::kAllTabs
TEST_F(TabsCloserTest, BrowserWithOnlyPinnedTabs_ClosePolicyAllTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{web_state0, 0});

  ASSERT_EQ(web_state_list->count(), 2);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kAllTabs);

  // Check that some tabs can be closed.
  EXPECT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that calling CloseTabs() close all the tabs registered,
  // allow to undo the operation and leave the WebStateList empty.
  EXPECT_EQ(tabs_closer.CloseTabs(), 2);
  EXPECT_TRUE(tabs_closer.CanUndoCloseTabs());
  EXPECT_TRUE(web_state_list->empty());

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as it has not been confirmed).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling UndoCloseTabs() correctly restored all the
  // closed tabs, in the correct order, and with the opener-opened
  // relationship intact.
  EXPECT_EQ(tabs_closer.UndoCloseTabs(), 2);
  ASSERT_EQ(web_state_list->count(), 2);
  EXPECT_EQ(web_state_list->pinned_tabs_count(), 2);
  EXPECT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  EXPECT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(1).opener, web_state0);

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as the operation has been cancelled).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling CloseTabs() and ConfirmDeletion() correctly
  // close the tabs and prevents undo.
  ASSERT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_EQ(tabs_closer.CloseTabs(), 2);
  EXPECT_EQ(tabs_closer.ConfirmDeletion(), 2);
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that the TabRestoreService has now been informed of the
  // close operation which has been confirmed.
  EXPECT_EQ(restore_service()->entries().size(), 2u);
}

// Tests how a TabsCloser behaves when presented with a Browser containing
// only pinned tabs.
//
// Variants: ClosePolicy::kRegularTabs
TEST_F(TabsCloserTest, BrowserWithOnlyPinnedTabs_ClosePolicyRegularTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{web_state0, 0});

  ASSERT_EQ(web_state_list->count(), 2);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kRegularTabs);

  // Check that nothing can be closed since there are only pinned
  // tabs in the WebStateList.
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());
}

// Tests how a TabsCloser behaves when presented with a Browser containing
// regular and pinned tabs.
//
// Variants: ClosePolicy::kAllTabs
TEST_F(TabsCloserTest, BrowserWithRegularAndPinnedTabs_ClosePolicyAllTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{web_state0, 0});
  web::WebState* web_state2 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state3 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state1, 0});
  web::WebState* web_state4 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state3, 0});

  ASSERT_EQ(web_state_list->count(), 5);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  ASSERT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  ASSERT_EQ(web_state_list->GetWebStateAt(3), web_state3);
  ASSERT_EQ(web_state_list->GetWebStateAt(4), web_state4);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kAllTabs);

  // Check that some tabs can be closed.
  EXPECT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that calling CloseTabs() close all the tabs registered,
  // allow to undo the operation and leave the WebStateList empty.
  EXPECT_EQ(tabs_closer.CloseTabs(), 5);
  EXPECT_TRUE(tabs_closer.CanUndoCloseTabs());
  EXPECT_TRUE(web_state_list->empty());

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as it has not been confirmed).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling UndoCloseTabs() correctly restored all the
  // closed tabs, in the correct order, and with the opener-opened
  // relationship intact.
  EXPECT_EQ(tabs_closer.UndoCloseTabs(), 5);
  ASSERT_EQ(web_state_list->count(), 5);
  EXPECT_EQ(web_state_list->pinned_tabs_count(), 2);
  EXPECT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  EXPECT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  EXPECT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  EXPECT_EQ(web_state_list->GetWebStateAt(3), web_state3);
  EXPECT_EQ(web_state_list->GetWebStateAt(4), web_state4);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(1).opener, web_state0);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(3).opener, web_state1);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(4).opener, web_state3);

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as the operation has been cancelled).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling CloseTabs() and ConfirmDeletion() correctly
  // close the tabs and prevents undo.
  ASSERT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_EQ(tabs_closer.CloseTabs(), 5);
  EXPECT_EQ(tabs_closer.ConfirmDeletion(), 5);
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that the TabRestoreService has now been informed of the
  // close operation which has been confirmed.
  EXPECT_EQ(restore_service()->entries().size(), 5u);
}

// Tests how a TabsCloser behaves when presented with a Browser containing
// regular and pinned tabs.
//
// Variants: ClosePolicy::kRegularTabs
TEST_F(TabsCloserTest, BrowserWithRegularAndPinnedTabs_ClosePolicyRegularTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{web_state0, 0});
  web::WebState* web_state2 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state3 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state1, 0});
  web::WebState* web_state4 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state3, 0});

  ASSERT_EQ(web_state_list->count(), 5);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  ASSERT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  ASSERT_EQ(web_state_list->GetWebStateAt(3), web_state3);
  ASSERT_EQ(web_state_list->GetWebStateAt(4), web_state4);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kRegularTabs);

  // Check that some tabs can be closed.
  EXPECT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that calling CloseTabs() close only the regular tabs,
  // allow to undo the operation and leave the WebStateList with
  // only pinned tabs.
  EXPECT_EQ(tabs_closer.CloseTabs(), 3);
  EXPECT_TRUE(tabs_closer.CanUndoCloseTabs());
  EXPECT_EQ(web_state_list->count(), 2);
  EXPECT_EQ(web_state_list->pinned_tabs_count(), 2);

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as it has not been confirmed).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling UndoCloseTabs() correctly restored all the
  // closed tabs, in the correct order, and with the opener-opened
  // relationship intact.
  EXPECT_EQ(tabs_closer.UndoCloseTabs(), 3);
  ASSERT_EQ(web_state_list->count(), 5);
  EXPECT_EQ(web_state_list->pinned_tabs_count(), 2);
  EXPECT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  EXPECT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  EXPECT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  EXPECT_EQ(web_state_list->GetWebStateAt(3), web_state3);
  EXPECT_EQ(web_state_list->GetWebStateAt(4), web_state4);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(1).opener, web_state0);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(3).opener, web_state1);
  EXPECT_EQ(web_state_list->GetOpenerOfWebStateAt(4).opener, web_state3);

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as the operation has been cancelled).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling CloseTabs() and ConfirmDeletion() correctly
  // close the tabs and prevents undo.
  ASSERT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_EQ(tabs_closer.CloseTabs(), 3);
  EXPECT_EQ(tabs_closer.ConfirmDeletion(), 3);
  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that the TabRestoreService has now been informed of the
  // close operation which has been confirmed.
  EXPECT_EQ(restore_service()->entries().size(), 3u);
}

// Tests that TabsCloser mark the original Browser as performing a batch
// operation when confirming the "close all tabs" operation.
//
// Variants: ClosePolicy::kAllTabs
TEST_F(TabsCloserTest,
       BrowserInBatchOperationDuringConfirmation_ClosePolicyAllTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{web_state0, 0});
  web::WebState* web_state2 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state3 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state1, 0});
  web::WebState* web_state4 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state3, 0});

  ASSERT_EQ(web_state_list->count(), 5);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  ASSERT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  ASSERT_EQ(web_state_list->GetWebStateAt(3), web_state3);
  ASSERT_EQ(web_state_list->GetWebStateAt(4), web_state4);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kAllTabs);

  // Check that some tabs can be closed.
  EXPECT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that calling CloseTabs() close all the tabs registered,
  // allow to undo the operation and leave the WebStateList empty.
  EXPECT_EQ(tabs_closer.CloseTabs(), 5);
  EXPECT_TRUE(tabs_closer.CanUndoCloseTabs());
  EXPECT_TRUE(web_state_list->empty());

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as it has not been confirmed).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling ConfirmDeletion() correctly close the tabs
  // and does this while the original Browser is in a batch operation.
  ASSERT_TRUE(tabs_closer.CanUndoCloseTabs());

  ScopedTestWebStateListObserver web_state_list_observer;
  web_state_list_observer.Observe(browser()->GetWebStateList());

  ASSERT_FALSE(web_state_list_observer.BatchOperationCompleted());
  EXPECT_EQ(tabs_closer.ConfirmDeletion(), 5);
  ASSERT_TRUE(web_state_list_observer.BatchOperationCompleted());

  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that the TabRestoreService has now been informed of the
  // close operation which has been confirmed.
  EXPECT_EQ(restore_service()->entries().size(), 5u);
}

// Tests that TabsCloser mark the original Browser as performing a batch
// operation when confirming the "close all tabs" operation.
//
// Variants: ClosePolicy::kRegularTabs
TEST_F(TabsCloserTest,
       BrowserInBatchOperationDuringConfirmation_ClosePolicyRegularTabs) {
  WebStateList* web_state_list = browser()->GetWebStateList();

  web::WebState* web_state0 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{});
  web::WebState* web_state1 =
      AppendWebState(InsertionPolicy::kPinned, WebStateOpener{web_state0, 0});
  web::WebState* web_state2 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{});
  web::WebState* web_state3 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state1, 0});
  web::WebState* web_state4 =
      AppendWebState(InsertionPolicy::kRegular, WebStateOpener{web_state3, 0});

  ASSERT_EQ(web_state_list->count(), 5);
  ASSERT_EQ(web_state_list->GetWebStateAt(0), web_state0);
  ASSERT_EQ(web_state_list->GetWebStateAt(1), web_state1);
  ASSERT_EQ(web_state_list->GetWebStateAt(2), web_state2);
  ASSERT_EQ(web_state_list->GetWebStateAt(3), web_state3);
  ASSERT_EQ(web_state_list->GetWebStateAt(4), web_state4);

  TabsCloser tabs_closer(browser(), TabsCloser::ClosePolicy::kRegularTabs);

  // Check that some tabs can be closed.
  EXPECT_TRUE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that calling CloseTabs() close all the tabs registered,
  // allow to undo the operation and leave the WebStateList empty.
  EXPECT_EQ(tabs_closer.CloseTabs(), 3);
  EXPECT_TRUE(tabs_closer.CanUndoCloseTabs());
  EXPECT_EQ(web_state_list->count(), 2);
  EXPECT_EQ(web_state_list->pinned_tabs_count(), 2);

  // Check that the TabRestoreService has not been informed of the
  // close operation yet (as it has not been confirmed).
  EXPECT_EQ(restore_service()->entries().size(), 0u);

  // Check that calling ConfirmDeletion() correctly close the tabs
  // and does this while the original Browser is in a batch operation.
  ASSERT_TRUE(tabs_closer.CanUndoCloseTabs());

  ScopedTestWebStateListObserver web_state_list_observer;
  web_state_list_observer.Observe(browser()->GetWebStateList());

  ASSERT_FALSE(web_state_list_observer.BatchOperationCompleted());
  EXPECT_EQ(tabs_closer.ConfirmDeletion(), 3);
  ASSERT_TRUE(web_state_list_observer.BatchOperationCompleted());

  EXPECT_FALSE(tabs_closer.CanCloseTabs());
  EXPECT_FALSE(tabs_closer.CanUndoCloseTabs());

  // Check that the TabRestoreService has now been informed of the
  // close operation which has been confirmed.
  EXPECT_EQ(restore_service()->entries().size(), 3u);
}
