// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/system_web_apps/apps/personalization_app/personalization_app_utils.h"

#include "ash/constants/ash_features.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/ash/login/demo_mode/demo_mode_test_helper.h"
#include "chrome/browser/ash/login/demo_mode/demo_session.h"
#include "chrome/browser/ash/login/users/fake_chrome_user_manager.h"
#include "chrome/browser/ash/settings/scoped_cros_settings_test_helper.h"
#include "chrome/browser/policy/profile_policy_connector.h"
#include "chrome/test/base/testing_browser_process.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "chromeos/ash/components/install_attributes/stub_install_attributes.h"
#include "components/account_id/account_id.h"
#include "components/user_manager/scoped_user_manager.h"
#include "components/user_manager/user_manager.h"
#include "components/user_manager/user_type.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace ash::personalization_app {

namespace {

void AddAndLoginUser(const AccountId& account_id,
                     const user_manager::UserType user_type) {
  ash::FakeChromeUserManager* user_manager =
      static_cast<ash::FakeChromeUserManager*>(
          user_manager::UserManager::Get());
  user_manager::User* user = nullptr;
  switch (user_type) {
    case user_manager::UserType::kRegular:
      user = user_manager->AddUser(account_id);
      break;
    case user_manager::UserType::kGuest:
      ASSERT_TRUE(account_id.empty());
      user = user_manager->AddGuestUser();
      break;
    case user_manager::UserType::kChild:
      user = user_manager->AddChildUser(account_id);
      break;
    case user_manager::UserType::kPublicAccount:
      user = user_manager->AddPublicAccountUser(account_id);
      break;
    default:
      GTEST_FAIL() << "Unsupported user type " << user_type;
  }
  user_manager->LoginUser(user->GetAccountId());
  user_manager->SwitchActiveUser(user->GetAccountId());
}

class PersonalizationAppUtilsTest : public testing::Test {
 public:
  PersonalizationAppUtilsTest()
      : scoped_user_manager_(std::make_unique<ash::FakeChromeUserManager>()),
        profile_manager_(TestingBrowserProcess::GetGlobal()) {
    scoped_feature_list_.InitWithFeatures(
        {features::kSeaPen, features::kSeaPenDemoMode,
         features::kFeatureManagementSeaPen},
        {});
  }

  PersonalizationAppUtilsTest(const PersonalizationAppUtilsTest&) = delete;
  PersonalizationAppUtilsTest& operator=(const PersonalizationAppUtilsTest&) =
      delete;
  ~PersonalizationAppUtilsTest() override = default;

  TestingProfileManager& profile_manager() { return profile_manager_; }

  ash::FakeChromeUserManager* GetFakeUserManager() {
    return static_cast<ash::FakeChromeUserManager*>(
        user_manager::UserManager::Get());
  }

  void SetUp() override {
    testing::Test::SetUp();
    ASSERT_TRUE(profile_manager().SetUp());
  }

  void TearDown() override {
    profile_manager().DeleteAllTestingProfiles();
    testing::Test::TearDown();
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
  content::BrowserTaskEnvironment task_environment_;
  user_manager::ScopedUserManager scoped_user_manager_;
  TestingProfileManager profile_manager_;
};

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_Guest) {
  auto* guest_profile = profile_manager().CreateGuestProfile();
  AddAndLoginUser(EmptyAccountId(), user_manager::UserType::kGuest);
  ASSERT_FALSE(IsEligibleForSeaPen(guest_profile));
}

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_Child) {
  const std::string email = "child@example.com";
  auto* child_profile = profile_manager().CreateTestingProfile(email);
  child_profile->SetIsSupervisedProfile();
  AddAndLoginUser(AccountId::FromUserEmail(email),
                  user_manager::UserType::kChild);
  ASSERT_FALSE(IsEligibleForSeaPen(child_profile));
}

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_NoProfile) {
  ASSERT_FALSE(IsEligibleForSeaPen(nullptr));
}

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_Googler) {
  const std::string email = "user@google.com";
  auto* googler_profile = profile_manager().CreateTestingProfile(email);
  googler_profile->GetProfilePolicyConnector()->OverrideIsManagedForTesting(
      true);
  AddAndLoginUser(AccountId::FromUserEmail(email),
                  user_manager::UserType::kRegular);
  ASSERT_TRUE(IsEligibleForSeaPen(googler_profile));
}

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_Managed) {
  const std::string email = "managed@example.com";
  auto* managed_profile = profile_manager().CreateTestingProfile(email);
  managed_profile->GetProfilePolicyConnector()->OverrideIsManagedForTesting(
      true);
  AddAndLoginUser(AccountId::FromUserEmail(email),
                  user_manager::UserType::kRegular);
  ASSERT_FALSE(IsEligibleForSeaPen(managed_profile));
}

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_Regular) {
  const std::string email = "user@example.com";
  auto* regular_profile = profile_manager().CreateTestingProfile(email);
  AddAndLoginUser(AccountId::FromUserEmail(email),
                  user_manager::UserType::kRegular);
  ASSERT_TRUE(IsEligibleForSeaPen(regular_profile));
}

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_PublicAccount) {
  const std::string email = "public-account@example.com";
  auto* managed_profile = profile_manager().CreateTestingProfile(email);
  managed_profile->GetProfilePolicyConnector()->OverrideIsManagedForTesting(
      true);
  AddAndLoginUser(AccountId::FromUserEmail(email),
                  user_manager::UserType::kPublicAccount);

  ASSERT_FALSE(IsEligibleForSeaPen(managed_profile));
}

TEST_F(PersonalizationAppUtilsTest, IsEligibleForSeaPen_PublicAccountDemoMode) {
  const std::string email = "demo-public-account@example.com";
  auto* managed_profile = profile_manager().CreateTestingProfile(email);
  managed_profile->GetProfilePolicyConnector()->OverrideIsManagedForTesting(
      true);
  AddAndLoginUser(AccountId::FromUserEmail(email),
                  user_manager::UserType::kPublicAccount);

  // Force device into demo mode.
  ASSERT_FALSE(::ash::DemoSession::IsDeviceInDemoMode());
  managed_profile->ScopedCrosSettingsTestHelper()
      ->InstallAttributes()
      ->SetDemoMode();
  ASSERT_TRUE(::ash::DemoSession::IsDeviceInDemoMode());

  // Force demo mode session to start.
  ASSERT_FALSE(::ash::DemoSession::Get());
  auto demo_mode_test_helper = std::make_unique<::ash::DemoModeTestHelper>();
  demo_mode_test_helper->InitializeSession();
  ASSERT_TRUE(::ash::DemoSession::Get());

  ASSERT_TRUE(IsEligibleForSeaPen(managed_profile))
      << "Demo mode should force enable SeaPen for managed profile";
}

}  // namespace

}  // namespace ash::personalization_app
