// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "base/test/bind.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/with_feature_override.h"
#include "chrome/browser/accessibility/pdf_ocr_controller.h"
#include "chrome/browser/accessibility/pdf_ocr_controller_factory.h"
#include "chrome/browser/pdf/pdf_extension_test_base.h"
#include "chrome/browser/screen_ai/screen_ai_install_state.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_accessibility_state.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "pdf/pdf_features.h"
#include "ui/accessibility/accessibility_features.h"

#if !BUILDFLAG(IS_CHROMEOS_ASH)
#include <optional>

#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile_test_util.h"
#include "content/public/test/scoped_accessibility_mode_override.h"
#else
#include "chrome/browser/ash/accessibility/accessibility_manager.h"
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

namespace {

class WebContentsLoadWaiter : public content::WebContentsObserver {
 public:
  // Observe `DidFinishLoad` for the specified |web_contents|.
  explicit WebContentsLoadWaiter(content::WebContents* web_contents)
      : content::WebContentsObserver(web_contents) {}

  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override {
    run_loop_.Quit();
  }

  void Wait() { run_loop_.Run(); }

 private:
  base::RunLoop run_loop_{base::RunLoop::Type::kNestableTasksAllowed};
};

class PrefChangeWaiter : public KeyedService {
 public:
  // Observe changes in prefs::kAccessibilityPdfOcrAlwaysActive.
  explicit PrefChangeWaiter(Profile* profile) {
    pref_change_registrar_.Init(profile->GetPrefs());
    pref_change_registrar_.Add(
        prefs::kAccessibilityPdfOcrAlwaysActive,
        base::BindLambdaForTesting([&]() { run_loop_.Quit(); }));
  }

  void Wait() { run_loop_.Run(); }

 private:
  base::RunLoop run_loop_{base::RunLoop::Type::kNestableTasksAllowed};
  PrefChangeRegistrar pref_change_registrar_;
};

#if !BUILDFLAG(IS_CHROMEOS_ASH)
Profile* CreateProfile(const base::FilePath& basename) {
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  base::FilePath profile_path =
      profile_manager->user_data_dir().Append(basename);
  return &profiles::testing::CreateProfileSync(profile_manager, profile_path);
}
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

}  // namespace

class PdfOcrControllerBrowserTest : public base::test::WithFeatureOverride,
                                    public PDFExtensionTestBase {
 public:
  PdfOcrControllerBrowserTest()
      : base::test::WithFeatureOverride(chrome_pdf::features::kPdfOopif) {}
  ~PdfOcrControllerBrowserTest() override = default;

  PdfOcrControllerBrowserTest(const PdfOcrControllerBrowserTest&) = delete;
  PdfOcrControllerBrowserTest& operator=(const PdfOcrControllerBrowserTest&) =
      delete;

  // PDFExtensionTestBase overrides:
  void TearDownOnMainThread() override {
    PDFExtensionTestBase::TearDownOnMainThread();
    EnableScreenReader(false);
  }

  void EnableScreenReader(bool enabled) {
#if BUILDFLAG(IS_CHROMEOS_ASH)
    // Enable Chromevox.
    ash::AccessibilityManager::Get()->EnableSpokenFeedback(enabled);
#else
    if (!enabled) {
      scoped_accessibility_override_.reset();
    } else if (!scoped_accessibility_override_) {
      scoped_accessibility_override_.emplace(ui::AXMode::kScreenReader);
    }
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)
  }

#if BUILDFLAG(IS_CHROMEOS_ASH)
  void EnableSelectToSpeak(bool enabled) {
    ash::AccessibilityManager::Get()->SetSelectToSpeakEnabled(enabled);
  }
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

  bool UseOopif() const override { return GetParam(); }

  std::vector<base::test::FeatureRef> GetEnabledFeatures() const override {
    auto enabled = PDFExtensionTestBase::GetEnabledFeatures();
    enabled.push_back(features::kPdfOcr);
#if BUILDFLAG(IS_CHROMEOS)
    enabled.push_back(features::kAccessibilityPdfOcrForSelectToSpeak);
#endif  // BUILDFLAG(IS_CHROMEOS)
    return enabled;
  }

 private:
#if !BUILDFLAG(IS_CHROMEOS_ASH)
  std::optional<content::ScopedAccessibilityModeOverride>
      scoped_accessibility_override_;
#endif
};

// TODO(crbug.com/40267312): Fix flakiness.
// Enabling PDF OCR should affect the accessibility mode of a new WebContents
// of PDF Viewer Mimehandler.
IN_PROC_BROWSER_TEST_P(PdfOcrControllerBrowserTest,
                       DISABLED_OpenPDFAfterTurningOnPdfOcr) {
  EnableScreenReader(true);
  ui::AXMode ax_mode =
      content::BrowserAccessibilityState::GetInstance()->GetAccessibilityMode();
  EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));

  PrefChangeWaiter pref_waiter(browser()->profile());
  browser()->profile()->GetPrefs()->SetBoolean(
      prefs::kAccessibilityPdfOcrAlwaysActive, true);
  // Wait until the PDF OCR pref changes accordingly.
  pref_waiter.Wait();

  // Load test PDF.
  content::WebContents* active_web_contents = GetActiveWebContents();
  WebContentsLoadWaiter load_waiter(active_web_contents);
  ASSERT_TRUE(ui_test_utils::NavigateToURL(
      browser(), embedded_test_server()->GetURL("/pdf/test.pdf")));
  load_waiter.Wait();

  std::vector<content::WebContents*> html_web_contents_vector =
      screen_ai::PdfOcrController::GetAllPdfWebContentsesForTesting(
          browser()->profile());
  for (auto* web_contents : html_web_contents_vector) {
    ax_mode = web_contents->GetAccessibilityMode();
    EXPECT_TRUE(ax_mode.has_mode(ui::AXMode::kPDFOcr));
  }
}

// TODO(crbug.com/40267312): Fix flakiness.
// Enabling PDF OCR should affect the accessibility mode of an exiting
// WebContents of PDF Viewer Mimehandler.
IN_PROC_BROWSER_TEST_P(PdfOcrControllerBrowserTest,
                       DISABLED_OpenPDFBeforeTurningOnPdfOcr) {
  EnableScreenReader(true);
  ui::AXMode ax_mode =
      content::BrowserAccessibilityState::GetInstance()->GetAccessibilityMode();
  EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));

  // Load test PDF.
  content::WebContents* active_web_contents = GetActiveWebContents();
  WebContentsLoadWaiter load_waiter(active_web_contents);
  ASSERT_TRUE(ui_test_utils::NavigateToURL(
      browser(), embedded_test_server()->GetURL("/pdf/test.pdf")));
  load_waiter.Wait();

  std::vector<content::WebContents*> html_web_contents_vector =
      screen_ai::PdfOcrController::GetAllPdfWebContentsesForTesting(
          browser()->profile());
  for (auto* web_contents : html_web_contents_vector) {
    ax_mode = web_contents->GetAccessibilityMode();
    EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));
  }

  PrefChangeWaiter pref_waiter(browser()->profile());
  browser()->profile()->GetPrefs()->SetBoolean(
      prefs::kAccessibilityPdfOcrAlwaysActive, true);
  // Wait until the PDF OCR pref changes accordingly.
  pref_waiter.Wait();

  html_web_contents_vector =
      screen_ai::PdfOcrController::GetAllPdfWebContentsesForTesting(
          browser()->profile());
  for (auto* web_contents : html_web_contents_vector) {
    ax_mode = web_contents->GetAccessibilityMode();
    EXPECT_TRUE(ax_mode.has_mode(ui::AXMode::kPDFOcr));
  }
}

IN_PROC_BROWSER_TEST_P(PdfOcrControllerBrowserTest,
                       NotEnabledWithoutScreenReader) {
  EnableScreenReader(false);

  screen_ai::PdfOcrControllerFactory::GetForProfile(browser()->profile())
      ->set_ocr_ready_for_testing();
  PrefChangeWaiter pref_waiter(browser()->profile());
  // Turn on PDF OCR.
  browser()->profile()->GetPrefs()->SetBoolean(
      prefs::kAccessibilityPdfOcrAlwaysActive, true);
  // Wait until the PDF OCR pref changes accordingly.
  pref_waiter.Wait();

  ASSERT_TRUE(LoadPdf(embedded_test_server()->GetURL("/pdf/test.pdf")));
  content::WebContents* pdf_contents = GetActiveWebContents();
  ui::AXMode ax_mode = pdf_contents->GetAccessibilityMode();
  EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));
}

#if BUILDFLAG(IS_CHROMEOS_ASH)
IN_PROC_BROWSER_TEST_P(PdfOcrControllerBrowserTest,
                       NotEnabledWithoutSelectToSpeak) {
  EnableSelectToSpeak(false);

  screen_ai::PdfOcrControllerFactory::GetForProfile(browser()->profile())
      ->set_ocr_ready_for_testing();
  PrefChangeWaiter pref_waiter(browser()->profile());
  // Turn on PDF OCR.
  browser()->profile()->GetPrefs()->SetBoolean(
      prefs::kAccessibilityPdfOcrAlwaysActive, true);
  // Wait until the PDF OCR pref changes accordingly.
  pref_waiter.Wait();

  ASSERT_TRUE(LoadPdf(embedded_test_server()->GetURL("/pdf/test.pdf")));
  content::WebContents* pdf_contents = GetActiveWebContents();
  ui::AXMode ax_mode = pdf_contents->GetAccessibilityMode();
  EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));
}
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

// Multi-profile is not supported on Ash.
#if !BUILDFLAG(IS_CHROMEOS_ASH)
// Enabling PDF OCR in one profile should not affect the accessibility mode of
// WebContents in another profile.
IN_PROC_BROWSER_TEST_P(PdfOcrControllerBrowserTest,
                       TurningOnPdfOcrInOneProfileNotAffectingAnotherProfile) {
  EnableScreenReader(true);
  ui::AXMode ax_mode =
      content::BrowserAccessibilityState::GetInstance()->GetAccessibilityMode();
  EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));

  Profile* profile1 = browser()->profile();
  // Load test PDF on profile1.
  ASSERT_TRUE(ui_test_utils::NavigateToURL(
      browser(), embedded_test_server()->GetURL("/pdf/test.pdf")));

  std::vector<content::WebContents*> html_web_contents_vector1 =
      screen_ai::PdfOcrController::GetAllPdfWebContentsesForTesting(profile1);
  for (auto* web_contents : html_web_contents_vector1) {
    ax_mode = web_contents->GetAccessibilityMode();
    EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));
  }

  const base::FilePath kProfileDir2(FILE_PATH_LITERAL("Other"));
  Profile* profile2 = CreateProfile(kProfileDir2);

  // Set the PDF OCR pref for the profile2.
  PrefChangeWaiter pref_waiter(profile2);
  profile2->GetPrefs()->SetBoolean(prefs::kAccessibilityPdfOcrAlwaysActive,
                                   true);
  // Wait until the PDF OCR pref changes accordingly.
  pref_waiter.Wait();

  // Setting the PDF OCR pref for the profile2 should not affect a WebContents
  // of PDF Viewer Mimehandler for the profile1.
  html_web_contents_vector1 =
      screen_ai::PdfOcrController::GetAllPdfWebContentsesForTesting(profile1);
  for (auto* web_contents : html_web_contents_vector1) {
    ax_mode = web_contents->GetAccessibilityMode();
    EXPECT_FALSE(ax_mode.has_mode(ui::AXMode::kPDFOcr));
  }
}
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

// TODO(crbug.com/40268279): Stop testing both modes after OOPIF PDF viewer
// launches.
INSTANTIATE_FEATURE_OVERRIDE_TEST_SUITE(PdfOcrControllerBrowserTest);
