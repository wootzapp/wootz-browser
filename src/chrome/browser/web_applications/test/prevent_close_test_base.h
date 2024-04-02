// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WEB_APPLICATIONS_TEST_PREVENT_CLOSE_TEST_BASE_H_
#define CHROME_BROWSER_WEB_APPLICATIONS_TEST_PREVENT_CLOSE_TEST_BASE_H_

#include "base/strings/string_piece.h"
#include "base/test/scoped_feature_list.h"
#include "base/values.h"
#include "chrome/browser/policy/policy_test_utils.h"
#include "components/policy/core/common/mock_configuration_policy_provider.h"
#include "components/webapps/common/web_app_id.h"

#if BUILDFLAG(IS_WIN)
#include "base/base_paths_win.h"
#include "base/test/scoped_path_override.h"
#endif  // BUILDFLAG(IS_WIN)

class Browser;
class GURL;
class Profile;

class PreventCloseTestBase : public policy::PolicyTest {
 public:
  PreventCloseTestBase();
  PreventCloseTestBase(const PreventCloseTestBase&) = delete;
  PreventCloseTestBase& operator=(const PreventCloseTestBase&) = delete;
  ~PreventCloseTestBase() override;

  // policy::PolicyTest:
  void SetUpInProcessBrowserTestFixture() override;
  void TearDownInProcessBrowserTestFixture() override;

  void SetPolicies(base::StringPiece web_app_settings,
                   base::StringPiece web_app_install_force_list);
  void SetPoliciesAndWaitUntilInstalled(
      const webapps::AppId& app_id,
      base::StringPiece web_app_settings,
      base::StringPiece web_app_install_force_list);
  void ClearWebAppSettings();
  void InstallPWA(const GURL& app_url, const webapps::AppId& app_id);
  Browser* LaunchPWA(const webapps::AppId& app_id, bool launch_in_window);
  base::Value ReturnPolicyValueFromJson(base::StringPiece policy);

  Profile* profile();

 private:
#if BUILDFLAG(IS_WIN)
  // This prevents SetRunOnOsLoginMode from leaving shortcuts in the Windows
  // startup directory that cause Chrome to get launched when Windows starts on
  // a bot. It needs to be in the class so that the override lasts until the
  // test object is destroyed, because tasks can keep running after the test
  // method finishes.
  // See https://crbug.com/1239809
  base::ScopedPathOverride override_user_startup_{base::DIR_USER_STARTUP};

  // Similarly, this prevents tests from adding shortcuts to the user's real
  // Windows start menu.
  base::ScopedPathOverride override_start_dir{base::DIR_START_MENU};
#endif  // BUILDFLAG(IS_WIN)

  base::test::ScopedFeatureList scoped_feature_list_;
  testing::NiceMock<policy::MockConfigurationPolicyProvider> provider_;
};

#endif  // CHROME_BROWSER_WEB_APPLICATIONS_TEST_PREVENT_CLOSE_TEST_BASE_H_
