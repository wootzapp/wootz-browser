// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/auto_reset.h"
#include "base/functional/callback_forward.h"
#include "base/test/gmock_callback_support.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/banners/android/chrome_app_banner_manager_android.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/test/base/android/android_browser_test.h"
#include "chrome/test/base/chrome_test_utils.h"
#include "components/segmentation_platform/public/segment_selection_result.h"
#include "components/segmentation_platform/public/testing/mock_segmentation_platform_service.h"
#include "components/site_engagement/content/site_engagement_score.h"
#include "components/site_engagement/content/site_engagement_service.h"
#include "components/webapps/browser/android/add_to_homescreen_params.h"
#include "components/webapps/browser/android/ambient_badge_manager.h"
#include "components/webapps/browser/android/app_banner_manager_android.h"
#include "components/webapps/browser/android/bottomsheet/pwa_bottom_sheet_controller.h"
#include "components/webapps/browser/banners/app_banner_settings_helper.h"
#include "components/webapps/browser/banners/install_banner_config.h"
#include "components/webapps/browser/features.h"
#include "components/webapps/browser/installable/installable_data.h"
#include "components/webapps/browser/installable/ml_installability_promoter.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "net/base/url_util.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "url/gurl.h"

using ::base::test::RunOnceCallback;
using testing::_;

namespace webapps {

class TestAmbientBadgeManager : public AmbientBadgeManager {
 public:
  explicit TestAmbientBadgeManager(
      content::WebContents* web_contents,
      segmentation_platform::SegmentationPlatformService*
          segmentation_platform_service,
      PrefService* prefs)
      : AmbientBadgeManager(*web_contents,
                            segmentation_platform_service,
                            *prefs) {}

  TestAmbientBadgeManager(const TestAmbientBadgeManager&) = delete;
  TestAmbientBadgeManager& operator=(const TestAmbientBadgeManager&) = delete;

  ~TestAmbientBadgeManager() override = default;

  void WaitForState(State target, base::OnceClosure on_done) {
    target_state_ = target;
    on_done_ = std::move(on_done);
  }

 protected:
  void UpdateState(State state) override {
    AmbientBadgeManager::UpdateState(state);
    if (state == target_state_ && on_done_) {
      base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
          FROM_HERE, std::move(on_done_));
    }
  }

 private:
  State target_state_;
  base::OnceClosure on_done_;
};

class TestAppBannerManager : public AppBannerManagerAndroid {
 public:
  explicit TestAppBannerManager(content::WebContents* web_contents)
      : AppBannerManagerAndroid(
            web_contents,
            std::make_unique<ChromeAppBannerManagerAndroid>(*web_contents)) {}

  explicit TestAppBannerManager(
      content::WebContents* web_contents,
      segmentation_platform::SegmentationPlatformService*
          segmentation_platform_service)
      : AppBannerManagerAndroid(
            web_contents,
            std::make_unique<ChromeAppBannerManagerAndroid>(*web_contents)),
        mock_segmentation_(segmentation_platform_service) {}

  TestAppBannerManager(const TestAppBannerManager&) = delete;
  TestAppBannerManager& operator=(const TestAppBannerManager&) = delete;

  ~TestAppBannerManager() override = default;

  void WaitForAmbientBadgeState(AmbientBadgeManager::State target,
                                base::OnceClosure on_done) {
    target_badge_state_ = target;
    on_badge_done_ = std::move(on_done);
  }

  bool TriggeringDisabledForTesting() const override { return false; }

  TestAmbientBadgeManager* GetBadgeManagerForTest() {
    return ambient_badge_test_.get();
  }

 protected:
  Profile* profile() {
    return Profile::FromBrowserContext(web_contents()->GetBrowserContext());
  }

  void MaybeShowAmbientBadge() override {
    ambient_badge_test_ = std::make_unique<TestAmbientBadgeManager>(
        web_contents(), mock_segmentation_, profile()->GetPrefs());

    ambient_badge_test_->WaitForState(target_badge_state_,
                                      std::move(on_badge_done_));

    InstallBannerConfig install_config = GetCurrentInstallBannerConfig();
    std::unique_ptr<AddToHomescreenParams> a2hs_params =
        AppBannerManagerAndroid::CreateAddToHomescreenParams(
            install_config, native_java_app_data_for_testing(),
            InstallableMetrics::GetInstallSource(
                &GetWebContents(), InstallTrigger::AMBIENT_BADGE));

    ambient_badge_test_->MaybeShow(
        install_config.validated_url, install_config.GetWebOrNativeAppName(),
        install_config.GetWebOrNativeAppIdentifier(), std::move(a2hs_params),
        // TODO(b/323192242): See if these callbacks can be merged.
        base::BindOnce(&AppBannerManagerAndroid::ShowBannerFromBadge,
                       GetAndroidWeakPtr()),
        // Create the params, then pass them to MaybeShow.
        base::BindOnce(&AppBannerManagerAndroid::CreateAddToHomescreenParams,
                       install_config, native_java_app_data_for_testing())
            .Then(base::BindOnce(
                &PwaBottomSheetController::MaybeShow, web_contents(),
                install_config.web_app_data, /*expand_sheet=*/false,
                base::BindRepeating(&TestAppBannerManager::OnInstallEvent,
                                    GetAndroidWeakPtr()))));
  }

 private:
  std::unique_ptr<TestAmbientBadgeManager> ambient_badge_test_;
  raw_ptr<segmentation_platform::SegmentationPlatformService>
      mock_segmentation_;
  AmbientBadgeManager::State target_badge_state_;
  base::OnceClosure on_badge_done_;
};

class AmbientBadgeManagerBrowserTest : public AndroidBrowserTest {
 public:
  AmbientBadgeManagerBrowserTest()
      : disable_banner_trigger_(&test::g_disable_banner_triggering_for_testing,
                                true) {}

  AmbientBadgeManagerBrowserTest(const AmbientBadgeManagerBrowserTest&) =
      delete;
  AmbientBadgeManagerBrowserTest& operator=(
      const AmbientBadgeManagerBrowserTest&) = delete;

  ~AmbientBadgeManagerBrowserTest() override = default;

  void SetUp() override {
    SetUpFeatureList();
    AndroidBrowserTest::SetUp();
  }

  void SetUpOnMainThread() override {
    ASSERT_TRUE(embedded_test_server()->Start());
    site_engagement::SiteEngagementScore::SetParamValuesForTesting();

    AndroidBrowserTest::SetUpOnMainThread();
  }

 protected:
  content::WebContents* web_contents() {
    return chrome_test_utils::GetActiveWebContents(this);
  }

  virtual void SetUpFeatureList() {
    scoped_feature_list_.InitWithFeatures(
        /*enabled_features=*/{},
        /*disabled_features=*/{features::kAmbientBadgeSuppressFirstVisit,
                               features::kInstallPromptSegmentation});
  }

  void ResetEngagementForUrl(const GURL& url, double score) {
    site_engagement::SiteEngagementService* service =
        site_engagement::SiteEngagementService::Get(
            Profile::FromBrowserContext(web_contents()->GetBrowserContext()));
    service->ResetBaseScoreForURL(url, score);
  }

  void RunTest(TestAppBannerManager* app_banner_manager,
               const GURL& url,
               AmbientBadgeManager::State expected_state) {
    ResetEngagementForUrl(url, 10);

    base::RunLoop waiter;

    app_banner_manager->WaitForAmbientBadgeState(expected_state,
                                                 waiter.QuitClosure());
    ASSERT_TRUE(content::NavigateToURL(web_contents(), url));

    waiter.Run();
  }

  base::test::ScopedFeatureList scoped_feature_list_;

 private:
  // Disable the banners in the browser so it won't interfere with the test.
  base::AutoReset<bool> disable_banner_trigger_;
};

IN_PROC_BROWSER_TEST_F(AmbientBadgeManagerBrowserTest, ShowAmbientBadge) {
  auto app_banner_manager =
      std::make_unique<TestAppBannerManager>(web_contents());

  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kShowing);
}

IN_PROC_BROWSER_TEST_F(AmbientBadgeManagerBrowserTest, NoServiceWorker) {
  auto app_banner_manager =
      std::make_unique<TestAppBannerManager>(web_contents());
  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL(
              "/banners/manifest_no_service_worker.html"),
          AmbientBadgeManager::State::kPendingWorker);
}

IN_PROC_BROWSER_TEST_F(AmbientBadgeManagerBrowserTest,
                       BlockedIfDismissRecently) {
  auto app_banner_manager =
      std::make_unique<TestAppBannerManager>(web_contents());

  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kShowing);

  // Explicitly dismiss the badge.
  app_banner_manager->GetBadgeManagerForTest()->BadgeDismissed();  // IN-TEST
  EXPECT_EQ(AmbientBadgeManager::State::kDismissed,
            app_banner_manager->GetBadgeManagerForTest()->state());  // IN-TEST

  // Badge blocked because it was recently dismissed.
  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kBlocked);

  // Badge can show again after 91 days.
  webapps::AppBannerManager::SetTimeDeltaForTesting(91);
  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kShowing);
}

class AmbientBadgeManagerSecondVisitTest
    : public AmbientBadgeManagerBrowserTest {
 public:
  AmbientBadgeManagerSecondVisitTest() = default;

  AmbientBadgeManagerSecondVisitTest(
      const AmbientBadgeManagerSecondVisitTest&) = delete;
  AmbientBadgeManagerSecondVisitTest& operator=(
      const AmbientBadgeManagerSecondVisitTest&) = delete;

  ~AmbientBadgeManagerSecondVisitTest() override = default;

  void SetUpFeatureList() override {
    scoped_feature_list_.InitWithFeatures(
        /*enabled_features=*/{features::kAmbientBadgeSuppressFirstVisit},
        /*disabled_features=*/{features::kInstallPromptSegmentation});
  }
};

IN_PROC_BROWSER_TEST_F(AmbientBadgeManagerSecondVisitTest,
                       SuppressedOnFirstVisit) {
  auto app_banner_manager =
      std::make_unique<TestAppBannerManager>(web_contents());

  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kPendingEngagement);

  // Load again, can show.
  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kShowing);
}

class AmbientBadgeManagerSmartTest : public AmbientBadgeManagerBrowserTest {
 public:
  AmbientBadgeManagerSmartTest() = default;

  AmbientBadgeManagerSmartTest(const AmbientBadgeManagerSmartTest&) = delete;
  AmbientBadgeManagerSmartTest& operator=(const AmbientBadgeManagerSmartTest&) =
      delete;

  ~AmbientBadgeManagerSmartTest() override = default;

  void SetUpFeatureList() override {
    scoped_feature_list_.InitWithFeatures(
        /*enabled_features=*/{features::kInstallPromptSegmentation},
        /*disabled_features=*/{features::kAmbientBadgeSuppressFirstVisit});
  }

  segmentation_platform::ClassificationResult GetClassificationResult(
      std::string label) {
    segmentation_platform::ClassificationResult result(
        segmentation_platform::PredictionStatus::kSucceeded);
    result.ordered_labels.emplace_back(label);
    return result;
  }

  segmentation_platform::MockSegmentationPlatformService
      mock_segmentation_service_;
};

IN_PROC_BROWSER_TEST_F(AmbientBadgeManagerSmartTest, ShowInstallMessages) {
  EXPECT_CALL(mock_segmentation_service_, GetClassificationResult(_, _, _, _))
      .WillOnce(RunOnceCallback<3>(GetClassificationResult(
          MLInstallabilityPromoter::kShowInstallPromptLabel)));

  auto app_banner_manager = std::make_unique<TestAppBannerManager>(
      web_contents(), &mock_segmentation_service_);

  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kShowing);
}

IN_PROC_BROWSER_TEST_F(AmbientBadgeManagerSmartTest,
                       BlockedBySegmentationResult) {
  EXPECT_CALL(mock_segmentation_service_, GetClassificationResult(_, _, _, _))
      .WillOnce(RunOnceCallback<3>(GetClassificationResult("DontShow")));

  auto app_banner_manager = std::make_unique<TestAppBannerManager>(
      web_contents(), &mock_segmentation_service_);

  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kSegmentation);
}

IN_PROC_BROWSER_TEST_F(AmbientBadgeManagerSmartTest, BlockedByGuardrail) {
  EXPECT_CALL(mock_segmentation_service_, GetClassificationResult(_, _, _, _))
      .Times(testing::Exactly(2))
      .WillRepeatedly(
          base::test::RunOnceCallbackRepeatedly<3>(GetClassificationResult(
              MLInstallabilityPromoter::kShowInstallPromptLabel)));

  auto app_banner_manager = std::make_unique<TestAppBannerManager>(
      web_contents(), &mock_segmentation_service_);

  // Showing OK for the first visit
  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kShowing);

  // Explicitly dismiss the badge.
  app_banner_manager->GetBadgeManagerForTest()->BadgeDismissed();  // IN-TEST
  EXPECT_EQ(AmbientBadgeManager::State::kDismissed,
            app_banner_manager->GetBadgeManagerForTest()->state());  // IN-TEST

  // Badge blocked because it was recently dismissed.
  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kBlocked);

  // Badge can show again after 91 days.
  webapps::AppBannerManager::SetTimeDeltaForTesting(91);
  RunTest(app_banner_manager.get(),
          embedded_test_server()->GetURL("/banners/manifest_test_page.html"),
          AmbientBadgeManager::State::kShowing);
}

}  // namespace webapps
