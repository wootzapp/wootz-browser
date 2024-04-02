// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/optimization_guide/core/model_execution/model_execution_features_controller.h"

#include "base/run_loop.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "components/optimization_guide/core/model_execution/model_execution_features.h"
#include "components/optimization_guide/core/model_execution/model_execution_prefs.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/signin/public/base/consent_level.h"
#include "components/signin/public/identity_manager/account_capabilities_test_mutator.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "components/signin/public/identity_manager/identity_test_environment.h"
#include "components/signin/public/identity_manager/test_identity_manager_observer.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "components/variations/scoped_variations_ids_provider.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace optimization_guide {

class ModelExecutionFeaturesControllerTest : public testing::Test {
 public:
  ModelExecutionFeaturesControllerTest() = default;
  ~ModelExecutionFeaturesControllerTest() override = default;

  void SetUp() override {
    pref_service_ = std::make_unique<TestingPrefServiceSimple>();
    prefs::RegisterProfilePrefs(pref_service_->registry());
    model_execution::prefs::RegisterProfilePrefs(pref_service_->registry());
  }

  void CreateModelExecutionFeaturesController() {
    model_execution_features_controller_ =
        std::make_unique<ModelExecutionFeaturesController>(
            pref_service_.get(), identity_test_env_.identity_manager());
  }

  void EnableSignIn() {
    auto account_info = identity_test_env()->MakePrimaryAccountAvailable(
        "test_email", signin::ConsentLevel::kSignin);
    AccountCapabilitiesTestMutator mutator(&account_info.capabilities);
    mutator.set_can_use_model_execution_features(true);
    signin::UpdateAccountInfoForAccount(identity_test_env_.identity_manager(),
                                        account_info);
    RunUntilIdle();
  }

  void EnableSignInWithoutCapability() {
    auto account_info = identity_test_env()->MakePrimaryAccountAvailable(
        "test_email", signin::ConsentLevel::kSignin);
    AccountCapabilitiesTestMutator mutator(&account_info.capabilities);
    mutator.set_can_use_model_execution_features(false);
    signin::UpdateAccountInfoForAccount(identity_test_env_.identity_manager(),
                                        account_info);
    RunUntilIdle();
  }

  prefs::FeatureOptInState GetFeaturePrefValue(
      proto::ModelExecutionFeature feature) {
    return static_cast<prefs::FeatureOptInState>(
        pref_service_->GetInteger(prefs::GetSettingEnabledPrefName(feature)));
  }

  ModelExecutionFeaturesController* model_execution_features_controller() {
    return model_execution_features_controller_.get();
  }

  signin::IdentityTestEnvironment* identity_test_env() {
    return &identity_test_env_;
  }

  void RunUntilIdle() {
    task_environment_.RunUntilIdle();
    base::RunLoop().RunUntilIdle();
  }
  base::HistogramTester* histogram_tester() { return &histogram_tester_; }

  PrefService* pref_service() { return pref_service_.get(); }

 private:
  base::test::TaskEnvironment task_environment_;
  signin::IdentityTestEnvironment identity_test_env_;
  std::unique_ptr<TestingPrefServiceSimple> pref_service_;
  variations::ScopedVariationsIdsProvider scoped_variations_ids_provider_{
      variations::VariationsIdsProvider::Mode::kUseSignedInState};
  std::unique_ptr<ModelExecutionFeaturesController>
      model_execution_features_controller_;
  base::HistogramTester histogram_tester_;
};

TEST_F(ModelExecutionFeaturesControllerTest, OneFeatureSettingVisible) {
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitAndEnableFeature(
      features::internal::kComposeSettingsVisibility);
  CreateModelExecutionFeaturesController();

  EnableSignIn();
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
  histogram_tester()->ExpectUniqueSample(
      "OptimizationGuide.ModelExecution.FeatureEnabledAtStartup.Compose", false,
      1);
  histogram_tester()->ExpectUniqueSample(
      "OptimizationGuide.ModelExecution.FeatureEnabledAtStartup."
      "TabOrganization",
      false, 1);
  histogram_tester()->ExpectUniqueSample(
      "OptimizationGuide.ModelExecution.FeatureEnabledAtStartup."
      "WallpaperSearch",
      false, 1);
}

TEST_F(ModelExecutionFeaturesControllerTest,
       DefaultFeatureSettingForUnsignedUser) {
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeaturesAndParameters(
      {{features::internal::kComposeSettingsVisibility, {}},
       {features::internal::kTabOrganizationSettingsVisibility, {}}},
      {});
  CreateModelExecutionFeaturesController();
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));

  EnableSignIn();
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
}

TEST_F(ModelExecutionFeaturesControllerTest,
       FeatureSettingChangedForUnsignedUser) {
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeaturesAndParameters(
      {{features::internal::kComposeSettingsVisibility,
        {{"allow_unsigned_user", "true"}}},
       {features::internal::kTabOrganizationSettingsVisibility,
        {{"allow_unsigned_user", "true"}}}},
      {});
  CreateModelExecutionFeaturesController();
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));

  EnableSignIn();
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
}

TEST_F(ModelExecutionFeaturesControllerTest,
       FeatureSettingDisabledWhenCapabilityDisabled) {
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeatures(
      {features::internal::kComposeSettingsVisibility}, {});
  CreateModelExecutionFeaturesController();
  EnableSignInWithoutCapability();
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
}

TEST_F(ModelExecutionFeaturesControllerTest,
       FeatureSettingAllowedWhenCapabilityCheckDisabled) {
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeatures(
      {features::internal::kComposeSettingsVisibility,
       features::internal::kModelExecutionCapabilityDisable},
      {});
  CreateModelExecutionFeaturesController();
  EnableSignInWithoutCapability();

  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
}

TEST_F(ModelExecutionFeaturesControllerTest,
       MainToggleEnablesAllVisibleFeatures) {
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeatures(
      {features::internal::kComposeSettingsVisibility,
       features::internal::kTabOrganizationSettingsVisibility},
      {});
  CreateModelExecutionFeaturesController();
  EnableSignIn();
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));

  // Enabling the main toggle enables visible features.
  pref_service()->SetInteger(
      prefs::kModelExecutionMainToggleSettingState,
      static_cast<int>(optimization_guide::prefs::FeatureOptInState::kEnabled));
  EXPECT_TRUE(
      model_execution_features_controller()
          ->ShouldFeatureBeCurrentlyEnabledForUser(
              proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_TRUE(model_execution_features_controller()
                  ->ShouldFeatureBeCurrentlyEnabledForUser(
                      proto::ModelExecutionFeature::
                          MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()
                   ->ShouldFeatureBeCurrentlyEnabledForUser(
                       proto::ModelExecutionFeature::
                           MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
  // Only the visible feature prefs should be enabled.
  EXPECT_EQ(prefs::FeatureOptInState::kEnabled,
            GetFeaturePrefValue(
                proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_EQ(prefs::FeatureOptInState::kEnabled,
            GetFeaturePrefValue(proto::ModelExecutionFeature::
                                    MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_EQ(prefs::FeatureOptInState::kNotInitialized,
            GetFeaturePrefValue(proto::ModelExecutionFeature::
                                    MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));

  // Disabling the main toggle disables all features.
  pref_service()->SetInteger(
      prefs::kModelExecutionMainToggleSettingState,
      static_cast<int>(
          optimization_guide::prefs::FeatureOptInState::kDisabled));
  EXPECT_FALSE(
      model_execution_features_controller()
          ->ShouldFeatureBeCurrentlyEnabledForUser(
              proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_FALSE(model_execution_features_controller()
                   ->ShouldFeatureBeCurrentlyEnabledForUser(
                       proto::ModelExecutionFeature::
                           MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()
                   ->ShouldFeatureBeCurrentlyEnabledForUser(
                       proto::ModelExecutionFeature::
                           MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
  // Only the visible feature prefs should be disabled.
  EXPECT_EQ(prefs::FeatureOptInState::kDisabled,
            GetFeaturePrefValue(
                proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_EQ(prefs::FeatureOptInState::kDisabled,
            GetFeaturePrefValue(proto::ModelExecutionFeature::
                                    MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_EQ(prefs::FeatureOptInState::kNotInitialized,
            GetFeaturePrefValue(proto::ModelExecutionFeature::
                                    MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
}

TEST_F(ModelExecutionFeaturesControllerTest, GraduatedFeatureIsNotVisible) {
  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeatures(
      /*enabled_features=*/
      {features::internal::kComposeGraduatedFromSettings,
       features::internal::kWallpaperSearchSettingsVisibility},
      /*disabled_features=*/
      {features::internal::kComposeSettingsVisibility});
  CreateModelExecutionFeaturesController();

  EnableSignIn();
  // IsSettingVisible
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_FALSE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_TRUE(model_execution_features_controller()->IsSettingVisible(
      proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
  // ShouldFeatureBeCurrentlyEnabledForUser
  EXPECT_TRUE(
      model_execution_features_controller()
          ->ShouldFeatureBeCurrentlyEnabledForUser(
              proto::ModelExecutionFeature::MODEL_EXECUTION_FEATURE_COMPOSE));
  EXPECT_FALSE(model_execution_features_controller()
                   ->ShouldFeatureBeCurrentlyEnabledForUser(
                       proto::ModelExecutionFeature::
                           MODEL_EXECUTION_FEATURE_TAB_ORGANIZATION));
  EXPECT_FALSE(model_execution_features_controller()
                   ->ShouldFeatureBeCurrentlyEnabledForUser(
                       proto::ModelExecutionFeature::
                           MODEL_EXECUTION_FEATURE_WALLPAPER_SEARCH));
  histogram_tester()->ExpectUniqueSample(
      "OptimizationGuide.ModelExecution.FeatureEnabledAtStartup.Compose", false,
      1);
  histogram_tester()->ExpectUniqueSample(
      "OptimizationGuide.ModelExecution.FeatureEnabledAtStartup."
      "TabOrganization",
      false, 1);
  histogram_tester()->ExpectUniqueSample(
      "OptimizationGuide.ModelExecution.FeatureEnabledAtStartup."
      "WallpaperSearch",
      false, 1);
}

}  // namespace optimization_guide
