// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/app_restore/full_restore_service.h"

#include "ash/accelerators/accelerator_controller_impl.h"
#include "ash/constants/ash_features.h"
#include "ash/constants/ash_pref_names.h"
#include "ash/constants/ash_switches.h"
#include "ash/shell.h"
#include "ash/test/ash_test_util.h"
#include "ash/wm/desks/desk.h"
#include "ash/wm/desks/desks_controller.h"
#include "ash/wm/overview/overview_grid.h"
#include "ash/wm/overview/overview_grid_test_api.h"
#include "ash/wm/overview/overview_test_util.h"
#include "ash/wm/window_restore/pine_contents_data.h"
#include "ash/wm/window_restore/pine_contents_view.h"
#include "ash/wm/window_restore/pine_controller.h"
#include "ash/wm/window_restore/pine_test_api.h"
#include "ash/wm/window_restore/window_restore_util.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/ash/app_restore/app_restore_test_util.h"
#include "chrome/browser/ash/app_restore/full_restore_app_launch_handler.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/ash/ash_test_util.h"
#include "chrome/browser/ui/ash/system_web_apps/system_web_app_ui_utils.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_tabstrip.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/prefs/pref_service.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/test_navigation_observer.h"

namespace ash::full_restore {

namespace {

const PineContentsView* GetPineContentsView() {
  OverviewGrid* overview_grid =
      GetOverviewGridForRoot(Shell::GetPrimaryRootWindow());
  if (!overview_grid) {
    return nullptr;
  }

  views::Widget* pine_widget = OverviewGridTestApi(overview_grid).pine_widget();
  if (!pine_widget) {
    return nullptr;
  }

  return views::AsViewClass<PineContentsView>(pine_widget->GetContentsView());
}

// Retrieve the "Restore" button from the pine dialog, if we are in a overview
// pine session.
const PillButton* GetPineDialogRestoreButton() {
  const PineContentsView* pine_contents_view = GetPineContentsView();
  return pine_contents_view
             ? PineContentsViewTestApi(pine_contents_view).restore_button()
             : nullptr;
}

const PillButton* GetPineDialogCancelButton() {
  const PineContentsView* pine_contents_view = GetPineContentsView();
  return pine_contents_view
             ? PineContentsViewTestApi(pine_contents_view).cancel_button()
             : nullptr;
}

}  // namespace

// Class used to wait for multiple browser windows to be created.
class BrowsersWaiter : public BrowserListObserver {
 public:
  explicit BrowsersWaiter(int expected_count)
      : expected_count_(expected_count) {
    BrowserList::AddObserver(this);
  }
  BrowsersWaiter(const BrowsersWaiter&) = delete;
  BrowsersWaiter& operator=(const BrowsersWaiter&) = delete;
  ~BrowsersWaiter() override { BrowserList::RemoveObserver(this); }

  void Wait() { run_loop_.Run(); }

  // BrowserListObserver:
  void OnBrowserAdded(Browser* browser) override {
    ++current_count_;
    if (current_count_ == expected_count_) {
      run_loop_.Quit();
    }
  }

 private:
  int current_count_ = 0;
  const int expected_count_;
  base::RunLoop run_loop_;
};

class PineBrowserTest : public InProcessBrowserTest {
 public:
  PineBrowserTest() {
    switches::SetIgnoreForestSecretKeyForTest(true);
    set_launch_browser_for_testing(nullptr);
  }
  PineBrowserTest(const PineBrowserTest&) = delete;
  PineBrowserTest& operator=(const PineBrowserTest&) = delete;
  ~PineBrowserTest() override {
    switches::SetIgnoreForestSecretKeyForTest(false);
  }

  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();

    // Set the restore pref setting as "Ask every time". This will ensure the
    // pine dialog comes up on the next session.
    auto* prefs = ProfileManager::GetActiveUserProfile()->GetPrefs();
    prefs->SetInteger(prefs::kRestoreAppsAndPagesPrefName,
                      static_cast<int>(RestoreOption::kAskEveryTime));
    prefs->SetBoolean(prefs::kShouldShowPineOnboarding, false);
  }

 private:
  base::test::ScopedFeatureList feature_list_{features::kForestFeature};
};

// Creates 2 browser windows that will be restored in the main test.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, PRE_LaunchBrowsers) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  Profile* profile = ProfileManager::GetActiveUserProfile();
  CreateBrowser(profile);
  CreateBrowser(profile);
  EXPECT_EQ(2u, BrowserList::GetInstance()->size());

  // Immediate save to full restore file to bypass the 2.5 second throttle.
  AppLaunchInfoSaveWaiter::Wait();
}

// Verify that with two elements in the full restore file, we enter overview on
// login. Then when we click the restore button, we restore two browsers.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, LaunchBrowsers) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  // Verify we have entered overview. The restore button will be null if we
  // failed to enter overview.
  WaitForOverviewEnterAnimation();
  const PillButton* restore_button = GetPineDialogRestoreButton();
  ASSERT_TRUE(restore_button);

  // Click the "Restore" button and verify we have launched 2 browsers.
  BrowsersWaiter waiter(/*expected_count=*/2);
  test::Click(restore_button, /*flag=*/0);
  waiter.Wait();
  EXPECT_EQ(2u, BrowserList::GetInstance()->size());
}

// Creates SWAs that will be restored in the main test.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, PRE_LaunchSWA) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  // Create two SWAs, files and settings.
  Profile* profile = ProfileManager::GetActiveUserProfile();
  test::InstallSystemAppsForTesting(profile);
  test::CreateSystemWebApp(profile, SystemWebAppType::FILE_MANAGER);
  test::CreateSystemWebApp(profile, SystemWebAppType::SETTINGS);
  EXPECT_EQ(2u, BrowserList::GetInstance()->size());

  // Immediate save to full restore file to bypass the 2.5 second throttle.
  AppLaunchInfoSaveWaiter::Wait();
}

// Verify that with two elements in the full restore file, we enter overview on
// login. Then when we click the restore button, we restore SWAs.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, LaunchSWA) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  test::InstallSystemAppsForTesting(ProfileManager::GetActiveUserProfile());

  // Verify we have entered overview. The restore button will be null if we
  // failed to enter overview.
  WaitForOverviewEnterAnimation();
  const PillButton* restore_button = GetPineDialogRestoreButton();
  ASSERT_TRUE(restore_button);

  // Click the "Restore" button.
  BrowsersWaiter waiter(/*expected_count=*/2);
  test::Click(restore_button, /*flag=*/0);
  waiter.Wait();

  // Verify that two browsers are launched and they are the file manager and
  // settings SWAs.
  auto* browser_list = BrowserList::GetInstance();
  EXPECT_EQ(2u, browser_list->size());
  EXPECT_TRUE(base::ranges::any_of(*browser_list, [](Browser* browser) {
    return IsBrowserForSystemWebApp(browser, SystemWebAppType::FILE_MANAGER);
  }));
  EXPECT_TRUE(base::ranges::any_of(*browser_list, [](Browser* browser) {
    return IsBrowserForSystemWebApp(browser, SystemWebAppType::SETTINGS);
  }));
}

// Creates 3 browser windows on 3 different desks that will be restored in the
// main test.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, PRE_LaunchBrowsersToDesks) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  Profile* profile = ProfileManager::GetActiveUserProfile();
  Browser* browser1 = CreateBrowser(profile);
  Browser* browser2 = CreateBrowser(profile);
  Browser* browser3 = CreateBrowser(profile);
  EXPECT_EQ(3u, BrowserList::GetInstance()->size());

  // Add two desks for a total of three. The browsers were all created on the
  // active desk.
  auto* desks_controller = DesksController::Get();
  desks_controller->NewDesk(DesksCreationRemovalSource::kKeyboard);
  desks_controller->NewDesk(DesksCreationRemovalSource::kKeyboard);
  ASSERT_EQ(3u, desks_controller->desks().size());
  for (Browser* browser : {browser1, browser2, browser3}) {
    ASSERT_TRUE(desks_controller->BelongsToActiveDesk(
        browser->window()->GetNativeWindow()));
  }

  // Move some windows so there is one window on each desk.
  aura::Window* primary_root = Shell::GetPrimaryRootWindow();
  desks_controller->MoveWindowFromActiveDeskTo(
      browser2->window()->GetNativeWindow(),
      desks_controller->GetDeskAtIndex(1), primary_root,
      DesksMoveWindowFromActiveDeskSource::kShortcut);
  desks_controller->MoveWindowFromActiveDeskTo(
      browser3->window()->GetNativeWindow(),
      desks_controller->GetDeskAtIndex(2), primary_root,
      DesksMoveWindowFromActiveDeskSource::kShortcut);

  const std::vector<std::unique_ptr<Desk>>& desks = desks_controller->desks();
  ASSERT_EQ(3u, desks.size());
  EXPECT_EQ(1u, desks[0]->windows().size());
  EXPECT_EQ(1u, desks[1]->windows().size());
  EXPECT_EQ(1u, desks[2]->windows().size());

  // Immediate save to full restore file to bypass the 2.5 second throttle.
  AppLaunchInfoSaveWaiter::Wait();
}

// Tests that the three browser windows are restored to their old desks.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, LaunchBrowsersToDesks) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  // Verify we have entered overview. The restore button will be null if we
  // failed to enter overview.
  WaitForOverviewEnterAnimation();
  const PillButton* restore_button = GetPineDialogRestoreButton();
  ASSERT_TRUE(restore_button);

  // Click the "Restore" button and verify we have launched 3 browsers.
  BrowsersWaiter waiter(/*expected_count=*/3);
  test::Click(restore_button, /*flag=*/0);
  waiter.Wait();

  // Ensure overview animation is finished as overview UI windows go into the
  // active desk container.
  WaitForOverviewExitAnimation();

  // Verify that each desk has one window.
  auto* desks_controller = DesksController::Get();
  const std::vector<std::unique_ptr<Desk>>& desks = desks_controller->desks();
  ASSERT_EQ(3u, desks.size());
  EXPECT_EQ(1u, desks[0]->windows().size());
  EXPECT_EQ(1u, desks[1]->windows().size());
  EXPECT_EQ(1u, desks[2]->windows().size());
}

IN_PROC_BROWSER_TEST_F(PineBrowserTest, PRE_WindowStates) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  Profile* profile = ProfileManager::GetActiveUserProfile();
  Browser* browser_maximized = CreateBrowser(profile);
  Browser* browser_minimized = CreateBrowser(profile);
  Browser* browser_fullscreened = CreateBrowser(profile);
  Browser* browser_floated = CreateBrowser(profile);
  Browser* browser_snapped = CreateBrowser(profile);
  EXPECT_EQ(5u, BrowserList::GetInstance()->size());

  WindowState::Get(browser_maximized->window()->GetNativeWindow())->Maximize();

  // Also maximize `browser_minimized` before minimizing so we can test the
  // pre-minimized state as well.
  WindowState::Get(browser_minimized->window()->GetNativeWindow())->Maximize();
  WindowState::Get(browser_minimized->window()->GetNativeWindow())->Minimize();

  // Fullscreen a window. This should not be restored as full restore does not
  // support restoring fullscreen state.
  const WMEvent fullscreen_event(WM_EVENT_FULLSCREEN);
  WindowState::Get(browser_fullscreened->window()->GetNativeWindow())
      ->OnWMEvent(&fullscreen_event);

  const WMEvent float_event(WM_EVENT_FLOAT);
  WindowState::Get(browser_floated->window()->GetNativeWindow())
      ->OnWMEvent(&float_event);

  const WindowSnapWMEvent snap_event(WM_EVENT_SNAP_PRIMARY);
  WindowState::Get(browser_snapped->window()->GetNativeWindow())
      ->OnWMEvent(&snap_event);

  // Immediate save to full restore file to bypass the 2.5 second throttle.
  AppLaunchInfoSaveWaiter::Wait();
}

// Tests that the browser windows are restored to their old window states.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, WindowStates) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  // Verify we have entered overview. The restore button will be null if we
  // failed to enter overview.
  WaitForOverviewEnterAnimation();
  const PillButton* restore_button = GetPineDialogRestoreButton();
  ASSERT_TRUE(restore_button);

  // Click the "Restore" button and verify we have launched 5 browsers.
  BrowsersWaiter waiter(/*expected_count=*/5);
  test::Click(restore_button, /*flag=*/0);
  waiter.Wait();

  auto* browser_list = BrowserList::GetInstance();
  EXPECT_EQ(5u, browser_list->size());

  // Test that there is a maximized, floated and snapped window.
  EXPECT_TRUE(base::ranges::any_of(*browser_list, [](Browser* browser) {
    return WindowState::Get(browser->window()->GetNativeWindow())
        ->IsMaximized();
  }));
  EXPECT_TRUE(base::ranges::any_of(*browser_list, [](Browser* browser) {
    return WindowState::Get(browser->window()->GetNativeWindow())->IsFloated();
  }));
  EXPECT_TRUE(base::ranges::any_of(*browser_list, [](Browser* browser) {
    return WindowState::Get(browser->window()->GetNativeWindow())->IsSnapped();
  }));

  // Test that there is no fullscreen window as full restore does not restore
  // fullscreen state.
  EXPECT_TRUE(base::ranges::none_of(*browser_list, [](Browser* browser) {
    return WindowState::Get(browser->window()->GetNativeWindow())
        ->IsFullscreen();
  }));

  // Test the pre-minimized state of the minimized browser window. When we
  // unminimize it, it should be maximized state.
  auto it = base::ranges::find_if(*browser_list, [](Browser* browser) {
    return WindowState::Get(browser->window()->GetNativeWindow())
        ->IsMinimized();
  });
  ASSERT_NE(it, browser_list->end());
  auto* window_state = WindowState::Get((*it)->window()->GetNativeWindow());
  window_state->Unminimize();
  EXPECT_TRUE(window_state->IsMaximized());
}

IN_PROC_BROWSER_TEST_F(PineBrowserTest, PRE_ClickCancelButton) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  Profile* profile = ProfileManager::GetActiveUserProfile();
  CreateBrowser(profile);
  CreateBrowser(profile);
  EXPECT_EQ(2u, BrowserList::GetInstance()->size());

  // Immediate save to full restore file to bypass the 2.5 second throttle.
  AppLaunchInfoSaveWaiter::Wait();
}

// Verify that with two elements in the full restore file, if we click cancel no
// browsers are launched.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, ClickCancelButton) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  // Verify we have entered overview. The cancel button will be null if we
  // failed to enter overview.
  WaitForOverviewEnterAnimation();
  const PillButton* cancel_button = GetPineDialogCancelButton();
  ASSERT_TRUE(cancel_button);

  // Click the cancel button. We spin the run loop because launching browsers is
  // async. Verify that no browsers are launched.
  test::Click(cancel_button, /*flag=*/0);
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(BrowserList::GetInstance()->empty());
}

IN_PROC_BROWSER_TEST_F(PineBrowserTest, PRE_TabInfo) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  Browser* browser = CreateBrowser(ProfileManager::GetActiveUserProfile());
  EXPECT_EQ(1u, BrowserList::GetInstance()->size());

  // Create two more urls in addition to the default "about:blank" tab.
  for (const GURL& url :
       {GURL("https://www.youtube.com/"), GURL("https://www.google.com/")}) {
    content::TestNavigationObserver navigation_observer(url);
    navigation_observer.StartWatchingNewWebContents();
    chrome::AddTabAt(browser, url, /*index=*/-1,
                     /*foreground=*/false);
    navigation_observer.Wait();
  }

  // Immediate save to full restore file to bypass the 2.5 second throttle.
  AppLaunchInfoSaveWaiter::Wait();
}

// Verify that the tab info that is sent to ash shell is as expected.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, TabInfo) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  // The pine dialog is built based on the values in this data structure.
  const PineContentsData* pine_contents_data =
      Shell::Get()->pine_controller()->pine_contents_data();
  ASSERT_TRUE(pine_contents_data);
  const PineContentsData::AppsInfos& apps_infos =
      pine_contents_data->apps_infos;
  ASSERT_EQ(1u, apps_infos.size());
  ASSERT_EQ(3u, apps_infos[0].tab_urls.size());
  EXPECT_EQ(GURL(url::kAboutBlankURL), apps_infos[0].tab_urls[0]);
  EXPECT_EQ(GURL("https://www.youtube.com/"), apps_infos[0].tab_urls[1]);
  EXPECT_EQ(GURL("https://www.google.com/"), apps_infos[0].tab_urls[2]);
}

IN_PROC_BROWSER_TEST_F(PineBrowserTest, PRE_ReenterOverviewPineSession) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());
  CreateBrowser(ProfileManager::GetActiveUserProfile());
  EXPECT_EQ(1u, BrowserList::GetInstance()->size());

  // Immediate save to full restore file to bypass the 2.5 second throttle.
  AppLaunchInfoSaveWaiter::Wait();
}

// TODO(crbug.com/329585661): Test is flaky.
// Test that if we exit overview and reenter without opening a new window, we
// see the pine dialog again.
IN_PROC_BROWSER_TEST_F(PineBrowserTest, DISABLED_ReenterOverviewPineSession) {
  EXPECT_TRUE(BrowserList::GetInstance()->empty());

  // Verify we have entered overview with the pine dialog.
  WaitForOverviewEnterAnimation();
  ASSERT_TRUE(Shell::Get()->overview_controller()->InOverviewSession());
  EXPECT_TRUE(GetPineDialogRestoreButton());
  EXPECT_TRUE(Shell::Get()->pine_controller()->pine_contents_data());

  // Exit overview without clicking restore or cancel.
  ToggleOverview();
  WaitForOverviewExitAnimation();
  EXPECT_TRUE(Shell::Get()->pine_controller()->pine_contents_data());

  // Reenter overview. Test that the dialog is still visible.
  ToggleOverview();
  WaitForOverviewEnterAnimation();
  EXPECT_TRUE(GetPineDialogRestoreButton());

  // Open a new window using the accelerator. This should delete the pine dialog
  // data and the next overview enter will not show the dialog.
  ToggleOverview();
  WaitForOverviewExitAnimation();
  BrowsersWaiter waiter(/*expected_count=*/1);
  ASSERT_TRUE(Shell::Get()->accelerator_controller()->PerformActionIfEnabled(
      AcceleratorAction::kNewWindow, {}));
  waiter.Wait();
  EXPECT_FALSE(Shell::Get()->pine_controller()->pine_contents_data());

  // Reentering overview this time should not show the dialog.
  ToggleOverview();
  WaitForOverviewEnterAnimation();
  EXPECT_FALSE(GetPineDialogRestoreButton());
}

}  // namespace ash::full_restore
