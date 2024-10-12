// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/birch/birch_keyed_service.h"

#include <memory>
#include <optional>

#include "ash/birch/birch_item.h"
#include "ash/birch/birch_model.h"
#include "ash/constants/ash_features.h"
#include "ash/constants/ash_switches.h"
#include "ash/shell.h"
#include "base/files/file_path.h"
#include "base/functional/callback_forward.h"
#include "base/test/scoped_feature_list.h"
#include "base/time/time.h"
#include "base/version_info/version_info.h"
#include "chrome/browser/ash/file_suggest/file_suggest_keyed_service.h"
#include "chrome/browser/ash/file_suggest/file_suggest_keyed_service_factory.h"
#include "chrome/browser/ash/file_suggest/file_suggest_test_util.h"
#include "chrome/browser/ash/file_suggest/mock_file_suggest_keyed_service.h"
#include "chrome/browser/ash/login/users/fake_chrome_user_manager.h"
#include "chrome/browser/ash/release_notes/release_notes_storage.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/browser/sync/session_sync_service_factory.h"
#include "chrome/browser/sync/sync_service_factory.h"
#include "chrome/browser/ui/ash/birch/birch_file_suggest_provider.h"
#include "chrome/browser/ui/ash/birch/birch_keyed_service_factory.h"
#include "chrome/browser/ui/ash/holding_space/scoped_test_mount_point.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "components/sessions/core/serialized_navigation_entry_test_helper.h"
#include "components/signin/public/identity_manager/identity_test_utils.h"
#include "components/sync/test/test_sync_service.h"
#include "components/sync_sessions/open_tabs_ui_delegate.h"
#include "components/sync_sessions/session_sync_service.h"
#include "components/sync_sessions/synced_session.h"
#include "components/user_manager/scoped_user_manager.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ash {

namespace {

constexpr char kSessionName1[] = "test_session_name 1";
constexpr char kSessionName2[] = "test_session_name 2";

constexpr char kSessionTag1[] = "SessionTag1";
constexpr char kSessionTag2[] = "SessionTag2";

constexpr char kExampleURL1[] = "http://www.example.com/1";
constexpr char kExampleURL2[] = "http://www.example.com/2";

constexpr char16_t kTabTitle1[] = u"Tab Title 1";
constexpr char16_t kTabTitle2[] = u"Tab Title 2";

std::unique_ptr<sync_sessions::SyncedSession> CreateNewSession(
    const std::string& session_name,
    const std::string& session_tag,
    syncer::DeviceInfo::FormFactor form_factor) {
  auto session = std::make_unique<sync_sessions::SyncedSession>();
  auto window = std::make_unique<sync_sessions::SyncedSessionWindow>();
  auto tab = std::make_unique<sessions::SessionTab>();

  session->SetSessionName(session_name);
  session->SetDeviceTypeAndFormFactor(sync_pb::SyncEnums::TYPE_UNSET,
                                      form_factor);

  window->wrapped_window.tabs.push_back(std::move(tab));
  session->windows[SessionID::NewUnique()] = std::move(window);
  session->SetSessionTag(session_tag);
  return session;
}

class MockSessionSyncService : public sync_sessions::SessionSyncService {
 public:
  MockSessionSyncService() = default;
  ~MockSessionSyncService() override = default;

  MOCK_METHOD(syncer::GlobalIdMapper*,
              GetGlobalIdMapper,
              (),
              (const, override));
  MOCK_METHOD(sync_sessions::OpenTabsUIDelegate*,
              GetOpenTabsUIDelegate,
              (),
              (override));
  base::CallbackListSubscription SubscribeToForeignSessionsChanged(
      const base::RepeatingClosure& cb) override {
    return subscriber_list_.Add(cb);
  }
  MOCK_METHOD(base::WeakPtr<syncer::ModelTypeControllerDelegate>,
              GetControllerDelegate,
              ());

  void NotifyMockForeignSessionsChanged() { subscriber_list_.Notify(); }

  bool IsSubscribersEmpty() { return subscriber_list_.empty(); }

 private:
  base::RepeatingClosureList subscriber_list_;
};

class MockOpenTabsUIDelegate : public sync_sessions::OpenTabsUIDelegate {
 public:
  MockOpenTabsUIDelegate() {
    foreign_sessions_owned_.push_back(CreateNewSession(
        kSessionName1, kSessionTag1, syncer::DeviceInfo::FormFactor::kDesktop));
    foreign_sessions_.push_back(foreign_sessions_owned_.back().get());
    foreign_sessions_owned_.push_back(CreateNewSession(
        kSessionName2, kSessionTag2, syncer::DeviceInfo::FormFactor::kPhone));
    foreign_sessions_.push_back(foreign_sessions_owned_.back().get());

    std::vector<std::unique_ptr<sessions::SessionTab>> session_tabs_one;
    auto tab = std::make_unique<sessions::SessionTab>();
    tab->timestamp = base::Time::Now();
    tab->navigations.push_back(sessions::SerializedNavigationEntryTestHelper::
                                   CreateNavigationForTest());
    tab->navigations[0].set_timestamp(base::Time::Now());
    tab->navigations[0].set_title(kTabTitle1);
    tab->navigations[0].set_virtual_url(GURL(kExampleURL1));
    session_tabs_one.push_back(std::move(tab));

    std::vector<std::unique_ptr<sessions::SessionTab>> session_tabs_two;
    tab = std::make_unique<sessions::SessionTab>();
    tab->timestamp = base::Time::Now();
    tab->navigations.push_back(sessions::SerializedNavigationEntryTestHelper::
                                   CreateNavigationForTest());
    tab->navigations[0].set_timestamp(base::Time::Now() + base::Minutes(5));
    tab->navigations[0].set_title(kTabTitle2);
    tab->navigations[0].set_virtual_url(GURL(kExampleURL2));
    session_tabs_two.push_back(std::move(tab));

    session_tabs_.emplace(kSessionTag1, std::move(session_tabs_one));
    session_tabs_.emplace(kSessionTag2, std::move(session_tabs_two));
  }

  bool GetAllForeignSessions(
      std::vector<raw_ptr<const sync_sessions::SyncedSession,
                          VectorExperimental>>* sessions) override {
    *sessions = foreign_sessions_;
    base::ranges::sort(*sessions, std::greater(),
                       [](const sync_sessions::SyncedSession* session) {
                         return session->GetModifiedTime();
                       });

    return !sessions->empty();
  }

  MOCK_METHOD1(GetLocalSession,
               bool(const sync_sessions::SyncedSession** local_session));

  MOCK_METHOD3(GetForeignTab,
               bool(const std::string& tag,
                    const SessionID tab_id,
                    const sessions::SessionTab** tab));

  MOCK_METHOD1(DeleteForeignSession, void(const std::string& tag));

  MOCK_METHOD1(
      GetForeignSession,
      std::vector<const sessions::SessionWindow*>(const std::string& tag));

  bool GetForeignSessionTabs(
      const std::string& tag,
      std::vector<const sessions::SessionTab*>* tabs) override {
    auto it = session_tabs_.find(tag);
    if (it != session_tabs_.end()) {
      for (auto& tab : it->second) {
        tabs->push_back(tab.get());
      }
    }

    return true;
  }

 private:
  std::vector<std::unique_ptr<sync_sessions::SyncedSession>>
      foreign_sessions_owned_;
  std::vector<raw_ptr<const sync_sessions::SyncedSession, VectorExperimental>>
      foreign_sessions_;
  std::map<std::string, std::vector<std::unique_ptr<sessions::SessionTab>>>
      session_tabs_;
};

std::unique_ptr<KeyedService> BuildMockSessionSyncService(
    content::BrowserContext* context) {
  return std::make_unique<testing::NiceMock<MockSessionSyncService>>();
}

std::unique_ptr<KeyedService> BuildTestSyncService(
    content::BrowserContext* context) {
  return std::make_unique<syncer::TestSyncService>();
}

}  // namespace

// TODO(https://crbug.com/1370774): move `ScopedTestMountPoint` out of holding
// space to remove the dependency on holding space code.
using ash::holding_space::ScopedTestMountPoint;

class BirchKeyedServiceTest : public BrowserWithTestWindowTest {
 public:
  BirchKeyedServiceTest()
      : BrowserWithTestWindowTest(
            base::test::TaskEnvironment::TimeSource::MOCK_TIME),
        fake_user_manager_(std::make_unique<FakeChromeUserManager>()) {}

  void SetUp() override {
    switches::SetIgnoreForestSecretKeyForTest(true);

    feature_list_.InitWithFeatures(
        {features::kForestFeature,
         ash::features::kReleaseNotesNotificationAllChannels},
        {});

    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());

    BrowserWithTestWindowTest::SetUp();

    file_suggest_service_ = static_cast<MockFileSuggestKeyedService*>(
        FileSuggestKeyedServiceFactory::GetInstance()->GetService(
            GetProfile()));

    mount_point_ = std::make_unique<ScopedTestMountPoint>(
        "test_mount", storage::kFileSystemTypeLocal,
        file_manager::VOLUME_TYPE_TESTING);
    mount_point_->Mount(GetProfile());

    birch_keyed_service_ =
        BirchKeyedServiceFactory::GetInstance()->GetService(GetProfile());

    session_sync_service_ = static_cast<MockSessionSyncService*>(
        SessionSyncServiceFactory::GetInstance()->GetForProfile(GetProfile()));
    sync_service_ = static_cast<syncer::TestSyncService*>(
        SyncServiceFactory::GetForProfile(profile()));

    SetSessionServiceToReturnOpenTabsDelegate(true);
  }

  void SetSessionServiceToReturnOpenTabsDelegate(bool return_delegate) {
    EXPECT_CALL(*session_sync_service_, GetOpenTabsUIDelegate())
        .WillRepeatedly(
            testing::Return(return_delegate ? &open_tabs_delegate_ : nullptr));
  }

  void TearDown() override {
    mount_point_.reset();
    birch_keyed_service_ = nullptr;
    file_suggest_service_ = nullptr;
    session_sync_service_ = nullptr;
    sync_service_ = nullptr;
    release_notes_storage_ = nullptr;
    fake_user_manager_.Reset();
    BrowserWithTestWindowTest::TearDown();
    switches::SetIgnoreForestSecretKeyForTest(false);
  }

  void LogIn(const std::string& email) override {
    // TODO(crbug.com/40286020): merge into BrowserWithTestWindowTest.
    const AccountId account_id(AccountId::FromUserEmail(email));
    fake_user_manager_->AddUser(account_id);
    fake_user_manager_->LoginUser(account_id);
    GetSessionControllerClient()->AddUserSession(email);
    GetSessionControllerClient()->SwitchActiveUser(account_id);
  }

  TestingProfile* CreateProfile(const std::string& profile_name) override {
    return profile_manager()->CreateTestingProfile(profile_name,
                                                   GetTestingFactories());
  }

  void SetUpReleaseNotesStorage() {
    release_notes_storage_ =
        std::make_unique<ReleaseNotesStorage>(GetProfile());
  }

  void MakePrimaryAccountAvailable() {
    auto* identity_manager =
        IdentityManagerFactory::GetForProfile(GetProfile());
    signin::MakePrimaryAccountAvailable(identity_manager, "user@gmail.com",
                                        signin::ConsentLevel::kSignin);
  }

  void ClearReleaseNotesSurfacesTimesLeftToShowPref() {
    GetProfile()->GetPrefs()->ClearPref(
        prefs::kReleaseNotesSuggestionChipTimesLeftToShow);
  }

  void MarkMilestoneUpToDate() {
    release_notes_storage_->MarkNotificationShown();
  }

  void MarkReleaseNotesSurfacesTimesLeftToShow(int times_left_to_show) {
    GetProfile()->GetPrefs()->SetInteger(
        prefs::kReleaseNotesSuggestionChipTimesLeftToShow, times_left_to_show);
  }

  int GetCurrentMilestone() {
    return version_info::GetVersion().components()[0];
  }

  TestSessionControllerClient* GetSessionControllerClient() {
    return ash_test_helper()->test_session_controller_client();
  }

  MockFileSuggestKeyedService* file_suggest_service() {
    return file_suggest_service_;
  }

  MockSessionSyncService* session_sync_service() {
    return session_sync_service_;
  }

  syncer::TestSyncService* sync_service() { return sync_service_; }

  BirchKeyedService* birch_keyed_service() { return birch_keyed_service_; }

  ScopedTestMountPoint* mount_point() { return mount_point_.get(); }

  TestingProfile::TestingFactories GetTestingFactories() override {
    return {
        {SyncServiceFactory::GetInstance(),
         base::BindRepeating(&BuildTestSyncService)},
        {FileSuggestKeyedServiceFactory::GetInstance(),
         base::BindRepeating(
             &MockFileSuggestKeyedService::BuildMockFileSuggestKeyedService,
             temp_dir_.GetPath())},
        {SessionSyncServiceFactory::GetInstance(),
         base::BindRepeating(&BuildMockSessionSyncService)},
    };
  }

 private:
  user_manager::TypedScopedUserManager<FakeChromeUserManager>
      fake_user_manager_;

  base::ScopedTempDir temp_dir_;

  std::unique_ptr<ScopedTestMountPoint> mount_point_;

  raw_ptr<MockFileSuggestKeyedService> file_suggest_service_ = nullptr;

  raw_ptr<BirchKeyedService> birch_keyed_service_ = nullptr;

  raw_ptr<MockSessionSyncService> session_sync_service_;

  raw_ptr<syncer::TestSyncService> sync_service_;

  MockOpenTabsUIDelegate open_tabs_delegate_;

  std::unique_ptr<ReleaseNotesStorage> release_notes_storage_;

  base::test::ScopedFeatureList feature_list_;
};

TEST_F(BirchKeyedServiceTest, HasDataProviders) {
  WaitUntilFileSuggestServiceReady(
      ash::FileSuggestKeyedServiceFactory::GetInstance()->GetService(
          GetProfile()));
  task_environment()->RunUntilIdle();

  EXPECT_TRUE(birch_keyed_service()->GetCalendarProvider());
  EXPECT_TRUE(birch_keyed_service()->GetFileSuggestProvider());
  EXPECT_TRUE(birch_keyed_service()->GetRecentTabsProvider());
}

TEST_F(BirchKeyedServiceTest, BirchFileSuggestProvider) {
  WaitUntilFileSuggestServiceReady(
      ash::FileSuggestKeyedServiceFactory::GetInstance()->GetService(
          GetProfile()));

  EXPECT_EQ(Shell::Get()->birch_model()->GetFileSuggestItemsForTest().size(),
            0u);

  const base::FilePath file_path_1 = mount_point()->CreateArbitraryFile();
  const base::FilePath file_path_2 = mount_point()->CreateArbitraryFile();

  file_suggest_service()->SetSuggestionsForType(
      FileSuggestionType::kDriveFile,
      /*suggestions=*/std::vector<FileSuggestData>{
          {FileSuggestionType::kDriveFile, file_path_1,
           /*new_prediction_reason=*/std::nullopt,
           /*modified_time=*/std::nullopt,
           /*viewed_time=*/std::nullopt,
           /*shared_time=*/std::nullopt,
           /*new_score=*/std::nullopt,
           /*drive_file_id=*/std::nullopt,
           /*icon_url=*/std::nullopt},
          {FileSuggestionType::kDriveFile, file_path_2,
           /*new_prediction_reason=*/std::nullopt,
           /*modified_time=*/std::nullopt,
           /*viewed_time=*/std::nullopt,
           /*shared_time=*/std::nullopt,
           /*new_score=*/std::nullopt,
           /*drive_file_id=*/std::nullopt,
           /*icon_url=*/std::nullopt}});

  birch_keyed_service()
      ->GetFileSuggestProviderForTest()
      ->OnFileSuggestionUpdated(FileSuggestionType::kDriveFile);

  task_environment()->RunUntilIdle();

  // Check that the birch model now has two file suggestions.
  EXPECT_EQ(Shell::Get()->birch_model()->GetFileSuggestItemsForTest().size(),
            2u);
}

TEST_F(BirchKeyedServiceTest, BirchFileSuggestProvider_NoFilesAvailable) {
  WaitUntilFileSuggestServiceReady(
      ash::FileSuggestKeyedServiceFactory::GetInstance()->GetService(
          GetProfile()));
  task_environment()->RunUntilIdle();

  BirchModel* model = Shell::Get()->birch_model();
  model->SetCalendarItems({});
  model->SetRecentTabItems({});
  model->SetSelfShareItems({});
  model->SetWeatherItems({});
  model->SetReleaseNotesItems({});
  model->SetAttachmentItems({});

  // Trigger a file update, with no available files.
  birch_keyed_service()
      ->GetFileSuggestProviderForTest()
      ->OnFileSuggestionUpdated(FileSuggestionType::kDriveFile);
  task_environment()->RunUntilIdle();

  // Check that model data is not fresh, because no file items have yet
  // been provided.
  EXPECT_EQ(model->GetFileSuggestItemsForTest().size(), 0u);
  EXPECT_FALSE(model->IsDataFresh());

  const base::FilePath file_path_1 = mount_point()->CreateArbitraryFile();

  // Once file suggest data has been set and updated, the data should be marked
  // fresh after the file provider has notified of the change.
  file_suggest_service()->SetSuggestionsForType(
      FileSuggestionType::kDriveFile,
      /*suggestions=*/std::vector<FileSuggestData>{
          {FileSuggestionType::kDriveFile, file_path_1,
           /*new_prediction_reason=*/std::nullopt,
           /*modified_time=*/std::nullopt,
           /*viewed_time=*/std::nullopt,
           /*shared_time=*/std::nullopt,
           /*new_score=*/std::nullopt,
           /*drive_file_id=*/std::nullopt,
           /*icon_url=*/std::nullopt}});
  birch_keyed_service()
      ->GetFileSuggestProviderForTest()
      ->OnFileSuggestionUpdated(FileSuggestionType::kDriveFile);
  task_environment()->RunUntilIdle();
  EXPECT_EQ(Shell::Get()->birch_model()->GetFileSuggestItemsForTest().size(),
            1u);
  EXPECT_TRUE(model->IsDataFresh());
}

TEST_F(BirchKeyedServiceTest, BirchRecentTabProvider) {
  WaitUntilFileSuggestServiceReady(
      ash::FileSuggestKeyedServiceFactory::GetInstance()->GetService(
          GetProfile()));
  task_environment()->RunUntilIdle();

  // No tabs data exists before a data fetch has occurred.
  EXPECT_EQ(Shell::Get()->birch_model()->GetTabsForTest().size(), 0u);

  // Request birch data fetch, then verify that tabs data is correct.
  birch_keyed_service()->GetRecentTabsProvider()->RequestBirchDataFetch();

  auto& tabs = Shell::Get()->birch_model()->GetTabsForTest();
  ASSERT_EQ(tabs.size(), 2u);

  EXPECT_EQ(tabs[0].title(), kTabTitle1);
  EXPECT_EQ(tabs[0].url(), GURL(kExampleURL1));
  EXPECT_EQ(tabs[0].session_name(), kSessionName1);
  EXPECT_EQ(tabs[0].form_factor(), BirchTabItem::DeviceFormFactor::kDesktop);

  EXPECT_EQ(tabs[1].title(), kTabTitle2);
  EXPECT_EQ(tabs[1].url(), GURL(kExampleURL2));
  EXPECT_EQ(tabs[1].session_name(), kSessionName2);
  EXPECT_EQ(tabs[1].form_factor(), BirchTabItem::DeviceFormFactor::kPhone);

  // Disable tab sync, then try fetching again and expect an empty list of tabs.
  sync_service()->GetUserSettings()->SetSelectedTypes(
      /*sync_everything = */ false, /*types=*/{});
  birch_keyed_service()->GetRecentTabsProvider()->RequestBirchDataFetch();
  EXPECT_EQ(Shell::Get()->birch_model()->GetTabsForTest().size(), 0u);
  EXPECT_TRUE(session_sync_service()->IsSubscribersEmpty());
}

TEST_F(BirchKeyedServiceTest, ReleaseNotesProvider) {
  BirchModel* model = Shell::Get()->birch_model();
  BirchDataProvider* release_notes_provider =
      birch_keyed_service()->GetReleaseNotesProvider();

  SetUpReleaseNotesStorage();
  MakePrimaryAccountAvailable();

  EXPECT_EQ(model->GetReleaseNotesItemsForTest().size(), 0u);

  release_notes_provider->RequestBirchDataFetch();
  model->SetCalendarItems(std::vector<BirchCalendarItem>());
  model->SetRecentTabItems(std::vector<BirchTabItem>());
  model->SetSelfShareItems(std::vector<BirchSelfShareItem>());
  model->SetFileSuggestItems(std::vector<BirchFileItem>());
  task_environment()->RunUntilIdle();
  auto& release_notes_items = model->GetReleaseNotesItemsForTest();

  ASSERT_EQ(release_notes_items.size(), 1u);
  EXPECT_EQ(release_notes_items[0].title(), u"See what's new");
  EXPECT_EQ(release_notes_items[0].subtitle(), u"Explore the latest features");
  EXPECT_EQ(release_notes_items[0].url(), GURL("chrome://help-app/updates"));
  EXPECT_EQ(GetProfile()->GetPrefs()->GetInteger(
                prefs::kReleaseNotesSuggestionChipTimesLeftToShow),
            3);

  MarkMilestoneUpToDate();
  MarkReleaseNotesSurfacesTimesLeftToShow(1);
  task_environment()->FastForwardBy(base::Hours(23));

  release_notes_provider->RequestBirchDataFetch();
  model->SetCalendarItems({});
  model->SetRecentTabItems(std::vector<BirchTabItem>());
  model->SetFileSuggestItems(std::vector<BirchFileItem>());
  model->SetSelfShareItems(std::vector<BirchSelfShareItem>());
  task_environment()->RunUntilIdle();

  EXPECT_EQ(model->GetReleaseNotesItemsForTest().size(), 1u);
  EXPECT_EQ(GetProfile()->GetPrefs()->GetInteger(
                prefs::kHelpAppNotificationLastShownMilestone),
            GetCurrentMilestone());
  EXPECT_EQ(GetProfile()->GetPrefs()->GetInteger(
                prefs::kReleaseNotesSuggestionChipTimesLeftToShow),
            1);

  ClearReleaseNotesSurfacesTimesLeftToShowPref();

  release_notes_provider->RequestBirchDataFetch();
  model->SetCalendarItems(std::vector<BirchCalendarItem>());
  model->SetRecentTabItems(std::vector<BirchTabItem>());
  model->SetFileSuggestItems(std::vector<BirchFileItem>());
  model->SetSelfShareItems(std::vector<BirchSelfShareItem>());
  task_environment()->RunUntilIdle();

  EXPECT_EQ(model->GetReleaseNotesItemsForTest().size(), 0u);
  EXPECT_EQ(GetProfile()->GetPrefs()->GetInteger(
                prefs::kHelpAppNotificationLastShownMilestone),
            GetCurrentMilestone());
  EXPECT_TRUE(
      GetProfile()
          ->GetPrefs()
          ->FindPreference(prefs::kReleaseNotesSuggestionChipTimesLeftToShow)
          ->IsDefaultValue());
}

TEST_F(BirchKeyedServiceTest, BirchRecentTabsWaitForForeignSessionsChange) {
  task_environment()->RunUntilIdle();
  SetSessionServiceToReturnOpenTabsDelegate(false);

  // Request tab data, and check that no tabs are set when no open tabs delegate
  // is available.
  birch_keyed_service()->GetRecentTabsProvider()->RequestBirchDataFetch();
  EXPECT_EQ(Shell::Get()->birch_model()->GetTabsForTest().size(), 0u);
  EXPECT_FALSE(session_sync_service()->IsSubscribersEmpty());

  SetSessionServiceToReturnOpenTabsDelegate(true);

  // Notify session service of foreign session change, and check that tabs have
  // been set by the recent tabs provider.
  session_sync_service()->NotifyMockForeignSessionsChanged();
  EXPECT_EQ(Shell::Get()->birch_model()->GetTabsForTest().size(), 2u);
  EXPECT_TRUE(session_sync_service()->IsSubscribersEmpty());
}

}  // namespace ash
