// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "base/feature_list.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/about_flags.h"
#include "chrome/browser/compose/chrome_compose_client.h"
#include "chrome/browser/compose/compose_enabling.h"
#include "chrome/browser/optimization_guide/browser_test_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/compose/core/browser/compose_features.h"
#include "components/optimization_guide/core/model_execution/model_execution_features.h"
#include "components/optimization_guide/core/optimization_guide_features.h"
#include "components/optimization_guide/core/optimization_guide_prefs.h"
#include "components/prefs/pref_service.h"
#include "components/signin/public/identity_manager/identity_test_utils.h"
#include "components/unified_consent/pref_names.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/fenced_frame_test_util.h"
#include "content/public/test/test_host_resolver.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/default_handlers.h"
#include "testing/gtest/include/gtest/gtest.h"
#if BUILDFLAG(IS_CHROMEOS)
#include "chromeos/constants/chromeos_features.h"
#endif  // BUILDFLAG(IS_CHROMEOS)
#if BUILDFLAG(IS_CHROMEOS_LACROS)
#include "chromeos/startup/browser_init_params.h"
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)

#if BUILDFLAG(IS_CHROMEOS_LACROS)
void SetLacrosInitParams(bool disable_compose) {
  crosapi::mojom::BrowserInitParamsPtr init_params =
      chromeos::BrowserInitParams::GetForTests()->Clone();
  init_params->should_disable_chrome_compose_on_chromeos = disable_compose;
  chromeos::BrowserInitParams::SetInitParamsForTests(std::move(init_params));
}
#endif

class ComposeEnablingBrowserTestBase : public InProcessBrowserTest {
 public:
  ComposeEnablingBrowserTestBase() = default;

  void SetUp() override { InProcessBrowserTest::SetUp(); }

  void EnableComposePrefs() {
    optimization_guide::EnableSigninAndModelExecutionCapability(
        browser()->profile());

    // Turn on MSBB.
    PrefService* prefs = browser()->profile()->GetPrefs();
    prefs->SetBoolean(
        unified_consent::prefs::kUrlKeyedAnonymizedDataCollectionEnabled, true);

    // Confirm that the required feature flags are enabled by default.
    EXPECT_TRUE(
        base::FeatureList::IsEnabled(compose::features::kEnableCompose));

    // Enable Compose via the Optimization Guide's pref.
    browser()->profile()->GetPrefs()->SetInteger(
        optimization_guide::prefs::GetSettingEnabledPrefName(
            optimization_guide::UserVisibleFeatureKey::kCompose),
        static_cast<int>(
            optimization_guide::prefs::FeatureOptInState::kEnabled));
  }

  ComposeEnabling& GetComposeEnabling() {
    content::WebContents* web_contents =
        browser()->tab_strip_model()->GetActiveWebContents();
    ChromeComposeClient* compose_client =
        ChromeComposeClient::FromWebContents(web_contents);
    return compose_client->GetComposeEnabling();
  }

  OptimizationGuideKeyedService* GetOptimizationGuide() {
    return OptimizationGuideKeyedServiceFactory::GetForProfile(
        browser()->profile());
  }

 protected:
  base::test::ScopedFeatureList scoped_feature_list_;
};

class ComposeEnablingBrowserTest : public ComposeEnablingBrowserTestBase {
 public:
  ComposeEnablingBrowserTest() {
    scoped_feature_list_.InitWithFeatures(
        /*enabled_features=*/
        {
            optimization_guide::features::kOptimizationGuideModelExecution,
            optimization_guide::features::internal::kComposeSettingsVisibility,
        },
        /*disabled_features=*/
        {
#if BUILDFLAG(IS_CHROMEOS_ASH)
            // All of these flags must be disabled for Compose to be enabled on
            // ChromeOS Ash.
            chromeos::features::kFeatureManagementDisableChromeCompose,
            chromeos::features::kOrca,
            chromeos::features::kOrcaDogfood,
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)
        });

#if BUILDFLAG(IS_CHROMEOS_LACROS)
    SetLacrosInitParams(/*disable_compose=*/false);
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)
  }
};

// PRE_ step simulates a browser restart.
IN_PROC_BROWSER_TEST_F(ComposeEnablingBrowserTest,
                       PRE_EnableComposeViaSettings) {
  EnableComposePrefs();

  // Checks that Compose is immediately enabled.
  EXPECT_EQ(base::ok(), GetComposeEnabling().IsEnabled());
  EXPECT_TRUE(GetOptimizationGuide()->ShouldFeatureBeCurrentlyEnabledForUser(
      optimization_guide::UserVisibleFeatureKey::kCompose));
}

// Checks that after the browser restarts required features are enabled.
IN_PROC_BROWSER_TEST_F(ComposeEnablingBrowserTest, EnableComposeViaSettings) {
  EXPECT_EQ(base::ok(), GetComposeEnabling().IsEnabled());
  EXPECT_TRUE(GetOptimizationGuide()->ShouldFeatureBeCurrentlyEnabledForUser(
      optimization_guide::UserVisibleFeatureKey::kCompose));
}

#if BUILDFLAG(IS_CHROMEOS)

// For testing that the feature is disabled by the appropriate feature
// management flag on CrOS.
class ComposeOnChromeOS : public ComposeEnablingBrowserTestBase {
 public:
  ComposeOnChromeOS() {
    scoped_feature_list_.InitWithFeatures(
        /*enabled_features=*/
        {
            optimization_guide::features::kOptimizationGuideModelExecution,
            optimization_guide::features::internal::kComposeSettingsVisibility,
#if BUILDFLAG(IS_CHROMEOS_ASH)
            chromeos::features::kFeatureManagementDisableChromeCompose,
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)
        },
        /*disabled_features=*/{
            chromeos::features::kOrca,
            chromeos::features::kOrcaDogfood,
        });

#if BUILDFLAG(IS_CHROMEOS_LACROS)
    SetLacrosInitParams(/*disable_compose=*/true);
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)
  }
};

// Similar to above, PRE_ step for checking that Compose is disabled on
// non-eligible CrOS devices.
IN_PROC_BROWSER_TEST_F(ComposeOnChromeOS,
                       PRE_ComposeDisabledOnNonEligibleCrOSDevices) {
  EnableComposePrefs();

  // Checks that Compose is still disabled.
  EXPECT_EQ(base::unexpected(compose::ComposeShowStatus::kDisabledOnChromeOS),
            GetComposeEnabling().IsEnabled());
}

// Checks that Compose is disabled on non-eligible CrOS devices.
IN_PROC_BROWSER_TEST_F(ComposeOnChromeOS,
                       ComposeDisabledOnNonEligibleCrOSDevices) {
  EXPECT_EQ(base::unexpected(compose::ComposeShowStatus::kDisabledOnChromeOS),
            GetComposeEnabling().IsEnabled());
}

#endif  // BUILDFLAG(IS_CHROMEOS)

class ComposeEnablingWithFencedFramesBrowserTest
    : public ComposeEnablingBrowserTest {
 public:
  void SetUpOnMainThread() override {
    host_resolver()->AddRule("*", "127.0.0.1");

    // Add content/test/data for cross_site_iframe_factory.html
    https_server()->ServeFilesFromSourceDirectory("content/test/data");

    https_server()->SetSSLConfig(net::EmbeddedTestServer::CERT_TEST_NAMES);
    content::SetupCrossSiteRedirector(https_server());
    net::test_server::RegisterDefaultHandlers(https_server());

    ASSERT_TRUE(https_server()->Start());
  }

 protected:
  net::EmbeddedTestServer* https_server() { return &https_server_; }
  content::test::FencedFrameTestHelper& fenced_frame_test_helper() {
    return fenced_frame_test_helper_;
  }

 private:
  content::test::FencedFrameTestHelper fenced_frame_test_helper_;
  net::EmbeddedTestServer https_server_{net::EmbeddedTestServer::TYPE_HTTPS};
};

IN_PROC_BROWSER_TEST_F(ComposeEnablingWithFencedFramesBrowserTest,
                       DisabledInFencedFrames) {
  // Only checking the cross-fence functionality, can ignore other enablement
  // requirements.
  auto scoped_compose_enabled =
      ComposeEnabling::ScopedEnableComposeForTesting();

  GURL main_url(https_server()->GetURL(
      "a.test", "/cross_site_iframe_factory.html?a.test(a.test{fenced})"));
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), main_url));
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();

  content::RenderFrameHost* main_frame = web_contents->GetPrimaryMainFrame();
  std::vector<content::RenderFrameHost*> child_frames =
      fenced_frame_test_helper().GetChildFencedFrameHosts(main_frame);
  ASSERT_EQ(child_frames.size(), 1u);
  content::RenderFrameHost* fenced_child1 = child_frames[0];

  auto* client = ChromeComposeClient::FromWebContents(web_contents);
  content::ContextMenuParams params;
  params.is_content_editable_for_autofill = true;
  params.frame_origin = fenced_child1->GetLastCommittedOrigin();
  EXPECT_FALSE(client->GetComposeEnabling().ShouldTriggerContextMenu(
      browser()->profile(), nullptr, fenced_child1, params));
}
