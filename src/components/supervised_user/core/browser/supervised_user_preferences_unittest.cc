// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/supervised_user/core/browser/supervised_user_preferences.h"

#include <memory>

#include "base/test/scoped_feature_list.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/testing_pref_service.h"
#include "components/supervised_user/core/browser/supervised_user_utils.h"
#include "components/supervised_user/core/common/features.h"
#include "components/supervised_user/core/common/pref_names.h"
#include "components/supervised_user/core/common/supervised_user_constants.h"
#include "components/supervised_user/test_support/kids_chrome_management_test_utils.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace supervised_user {
namespace {

using ::testing::IsEmpty;
using ::testing::Not;

class SupervisedUserPreferencesTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto* registry = pref_service_.registry();
    supervised_user::RegisterProfilePrefs(registry);
  }

 protected:
  TestingPrefServiceSimple pref_service_;
};

TEST_F(SupervisedUserPreferencesTest, RegisterProfilePrefs) {
  // Checks the preference registration from the Setup.
  EXPECT_EQ(
      pref_service_.GetInteger(prefs::kDefaultSupervisedUserFilteringBehavior),
      static_cast<int>(supervised_user::FilteringBehavior::kAllow));
  EXPECT_EQ(pref_service_.GetBoolean(prefs::kSupervisedUserSafeSites), true);
  EXPECT_FALSE(supervised_user::IsSubjectToParentalControls(pref_service_));
  // TODO(b/306376651): When we migrate more preference reading methods in this
  // library, add more test cases for their correct default values.
}

TEST_F(SupervisedUserPreferencesTest, ToggleParentalControls) {
  supervised_user::EnableParentalControls(pref_service_);
  EXPECT_EQ(pref_service_.GetString(prefs::kSupervisedUserId),
            supervised_user::kChildAccountSUID);
  EXPECT_TRUE(supervised_user::IsChildAccountStatusKnown(pref_service_));

  supervised_user::DisableParentalControls(pref_service_);
  EXPECT_EQ(pref_service_.GetString(prefs::kSupervisedUserId), std::string());
  EXPECT_TRUE(supervised_user::IsChildAccountStatusKnown(pref_service_));
}

TEST_F(SupervisedUserPreferencesTest, StartFetchingFamilyInfo) {
  kidsmanagement::ListMembersResponse list_family_members_response;
  supervised_user::SetFamilyMemberAttributesForTesting(
      list_family_members_response.add_members(),
      kidsmanagement::HEAD_OF_HOUSEHOLD, "username_hoh");
  supervised_user::SetFamilyMemberAttributesForTesting(
      list_family_members_response.add_members(), kidsmanagement::PARENT,
      "username_parent");
  supervised_user::SetFamilyMemberAttributesForTesting(
      list_family_members_response.add_members(), kidsmanagement::CHILD,
      "username_child");
  supervised_user::SetFamilyMemberAttributesForTesting(
      list_family_members_response.add_members(), kidsmanagement::MEMBER,
      "username_member");

  supervised_user::RegisterFamilyPrefs(pref_service_,
                                       list_family_members_response);

  EXPECT_EQ("username_hoh",
            pref_service_.GetString(prefs::kSupervisedUserCustodianName));
  EXPECT_EQ("username_parent",
            pref_service_.GetString(prefs::kSupervisedUserSecondCustodianName));
}

TEST_F(SupervisedUserPreferencesTest, FieldsAreClearedForNonChildAccounts) {
  {
    kidsmanagement::ListMembersResponse list_family_members_response;
    supervised_user::SetFamilyMemberAttributesForTesting(
        list_family_members_response.add_members(),
        kidsmanagement::HEAD_OF_HOUSEHOLD, "username_hoh");
    supervised_user::SetFamilyMemberAttributesForTesting(
        list_family_members_response.add_members(), kidsmanagement::PARENT,
        "username_parent");

    supervised_user::RegisterFamilyPrefs(pref_service_,
                                         list_family_members_response);

    for (const char* property : supervised_user::kCustodianInfoPrefs) {
      EXPECT_THAT(pref_service_.GetString(property), Not(IsEmpty()));
    }
  }

  {
    kidsmanagement::ListMembersResponse list_family_members_response;
    supervised_user::RegisterFamilyPrefs(pref_service_,
                                         list_family_members_response);
    for (const char* property : supervised_user::kCustodianInfoPrefs) {
      EXPECT_THAT(pref_service_.GetString(property), IsEmpty());
    }
  }
}

TEST_F(SupervisedUserPreferencesTest, IsSafeSitesEnabledSupervisedUser) {
  pref_service_.SetBoolean(prefs::kSupervisedUserSafeSites, true);
  pref_service_.SetString(prefs::kSupervisedUserId,
                            supervised_user::kChildAccountSUID);

  EXPECT_TRUE(supervised_user::IsSafeSitesEnabled(pref_service_));
}

TEST_F(SupervisedUserPreferencesTest, IsSafeSitesEnabledNonSupervisedUser) {
  pref_service_.SetBoolean(prefs::kSupervisedUserSafeSites, true);
  pref_service_.SetString(prefs::kSupervisedUserId, std::string());

  EXPECT_FALSE(supervised_user::IsSafeSitesEnabled(pref_service_));
}

TEST_F(SupervisedUserPreferencesTest, IsSafeSitesDisabled) {
  pref_service_.SetBoolean(prefs::kSupervisedUserSafeSites, false);

  EXPECT_FALSE(supervised_user::IsSafeSitesEnabled(pref_service_));
}

TEST_F(SupervisedUserPreferencesTest,
       IsSubjectToParentalControlsForSupervisedUser) {
  // Set supervised user preference.
  pref_service_.SetString(prefs::kSupervisedUserId,
                          supervised_user::kChildAccountSUID);
  EXPECT_TRUE(supervised_user::IsSubjectToParentalControls(pref_service_));
}

TEST_F(SupervisedUserPreferencesTest,
       IsSubjectToParentalControlsForNonSupervisedUser) {
  // Set non-supervised user preference.
  pref_service_.SetString(prefs::kSupervisedUserId, std::string());
  EXPECT_FALSE(supervised_user::IsSubjectToParentalControls(pref_service_));
}

enum class ExtensionsPermissionStatus { kEnabled, kDisabled };

// Tests for the method AreExtensionsPermissionsEnabled which
// depends on enabling platform-specific feature flags.
class SupervisedUserPreferencesTestWithExtensionsPermissionsFeature
    : public ::testing::Test,
      public testing::WithParamInterface<ExtensionsPermissionStatus> {
 public:
  void SetUp() override {
    auto* registry = pref_service_.registry();
    supervised_user::RegisterProfilePrefs(registry);

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN)
    if (AreExtensionsPermitted()) {
      feature_list_.InitAndEnableFeature(
          supervised_user::
              kEnableExtensionsPermissionsForSupervisedUsersOnDesktop);
    } else {
      feature_list_.InitAndDisableFeature(
          supervised_user::
              kEnableExtensionsPermissionsForSupervisedUsersOnDesktop);
    }
#endif
  }

  bool AreExtensionsPermitted() const {
    return GetParam() == ExtensionsPermissionStatus::kEnabled;
  }

 protected:
  TestingPrefServiceSimple pref_service_;

 private:
  base::test::ScopedFeatureList feature_list_;
};

TEST_P(SupervisedUserPreferencesTestWithExtensionsPermissionsFeature,
       AreExtensionsPermissionsEnabledWithSupervisedUser) {
  pref_service_.SetString(prefs::kSupervisedUserId,
                          supervised_user::kChildAccountSUID);

#if BUILDFLAG(ENABLE_EXTENSIONS)
  EXPECT_EQ(supervised_user::AreExtensionsPermissionsEnabled(pref_service_),
            AreExtensionsPermitted());
#else
  EXPECT_FALSE(supervised_user::AreExtensionsPermissionsEnabled(pref_service_));
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
}

TEST_P(SupervisedUserPreferencesTestWithExtensionsPermissionsFeature,
       AreExtensionsPermissionsEnabledWithNonSupervisedUser) {
  pref_service_.SetString(prefs::kSupervisedUserId, std::string());

  EXPECT_FALSE(supervised_user::AreExtensionsPermissionsEnabled(pref_service_));
}

INSTANTIATE_TEST_SUITE_P(
    All,
    SupervisedUserPreferencesTestWithExtensionsPermissionsFeature,
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN)
    testing::Values(ExtensionsPermissionStatus::kEnabled,
                    ExtensionsPermissionStatus::kDisabled),
#else
    // ChromeOS has supervised user extension permissions on by default.
    testing::Values(ExtensionsPermissionStatus::kEnabled),
#endif
    [](const testing::TestParamInfo<ExtensionsPermissionStatus> info) {
      // Generate the test suffix from boolean param.
      switch (info.param) {
        case ExtensionsPermissionStatus::kEnabled:
          return "with_enabled_extension_permissions";
        case ExtensionsPermissionStatus::kDisabled:
          return "with_disabled_extension_permissions";
      }
    });

}  // namespace
}  // namespace supervised_user
