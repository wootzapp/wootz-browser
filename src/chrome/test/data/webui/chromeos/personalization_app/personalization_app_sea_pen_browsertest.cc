// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/webui/personalization_app/test/personalization_app_mojom_banned_mocha_test_base.h"
#include "chrome/browser/ash/system_web_apps/apps/personalization_app/personalization_app_mocha_test_base.h"
#include "content/public/test/browser_test.h"

// TODO(b/312208348) move this test to ash common sea_pen browsertest.

namespace ash::personalization_app {

// Tests state management and logic in SeaPen.
using SeaPenControllerTest = PersonalizationAppMojomBannedMochaTestBase;

IN_PROC_BROWSER_TEST_F(SeaPenControllerTest, All) {
  RunTest("chromeos/personalization_app/sea_pen_controller_test.js",
          "mocha.run()");
}

// Tests the SeaPen UI.
// TODO(b/329149811) Add screenplay id
class PersonalizationAppSeaPenBrowserTest
    : public PersonalizationAppMochaTestBase {
 public:
  PersonalizationAppSeaPenBrowserTest() = default;
  PersonalizationAppSeaPenBrowserTest(
      const PersonalizationAppSeaPenBrowserTest&) = delete;
  PersonalizationAppSeaPenBrowserTest& operator=(
      const PersonalizationAppSeaPenBrowserTest&) = delete;

  ~PersonalizationAppSeaPenBrowserTest() override = default;
};

// Flaky test disabled b/329149811.
IN_PROC_BROWSER_TEST_F(PersonalizationAppSeaPenBrowserTest, DISABLED_SeaPen) {
  RunTestWithoutTestLoader(
      "chromeos/personalization_app/personalization_app_test.js",
      "runMochaSuite('sea pen')");
}

}  // namespace ash::personalization_app
