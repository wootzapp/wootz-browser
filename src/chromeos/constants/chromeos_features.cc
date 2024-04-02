// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/constants/chromeos_features.h"

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"

#if BUILDFLAG(IS_CHROMEOS_LACROS)
#include "chromeos/startup/browser_params_proxy.h"
#endif

namespace chromeos::features {

namespace {
bool g_app_install_service_uri_enabled_for_testing = false;
}

#if BUILDFLAG(IS_CHROMEOS_ASH)
// Enables triggering app installs from a specific URI.
BASE_FEATURE(kAppInstallServiceUri,
             "AppInstallServiceUri",
             base::FEATURE_DISABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

// Enables or disables more filtering out of phones from the Bluetooth UI.
BASE_FEATURE(kBluetoothPhoneFilter,
             "BluetoothPhoneFilter",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enables show captive portal signin in a specially flagged popup window.
BASE_FEATURE(kCaptivePortalPopupWindow,
             "CaptivePortalPopupWindow",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables updated UI for the clipboard history menu and new system behavior
// related to clipboard history.
BASE_FEATURE(kClipboardHistoryRefresh,
             "ClipboardHistoryRefresh",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enables cloud game features. A separate flag "LauncherGameSearch" controls
// launcher-only cloud gaming features, since they can also be enabled on
// non-cloud-gaming devices.
BASE_FEATURE(kCloudGamingDevice,
             "CloudGamingDevice",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables ChromeOS Apps APIs.
BASE_FEATURE(kBlinkExtension,
             "BlinkExtension",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the ChromeOS Diagnostics API.
BASE_FEATURE(kBlinkExtensionDiagnostics,
             "BlinkExtensionDiagnostics",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables ChromeOS Kiosk APIs.
BASE_FEATURE(kBlinkExtensionKiosk,
             "BlinkExtensionKiosk",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables handling of key press event in background.
BASE_FEATURE(kCrosAppsBackgroundEventHandling,
             "CrosAppsBackgroundEventHandling",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the use of cros-component UI elements. Contact:
// cros-jellybean-team@google.com.
BASE_FEATURE(kCrosComponents,
             "CrosComponents",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables an app to discover and install other apps. This flag will be enabled
// with Finch.
BASE_FEATURE(kCrosMall, "CrosMall", base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the behaviour difference between web apps and browser created
// shortcut backed by the web app system on Chrome OS.
BASE_FEATURE(kCrosShortstand,
             "CrosShortstand",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the more detailed, OS-level dialog for web app installs from the
// omnibox.
BASE_FEATURE(kCrosOmniboxInstallDialog,
             "CrosOmniboxInstallDialog",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the more detailed, OS-level dialog for web app installs.
BASE_FEATURE(kCrosWebAppInstallDialog,
             "CrosWebAppInstallDialog",
             base::FEATURE_DISABLED_BY_DEFAULT);

#if BUILDFLAG(IS_CHROMEOS_ASH)
// With this feature enabled, the shortcut app badge is painted in the UI
// instead of being part of the shortcut app icon.

// Enables the new UI for browser created shortcut backed by web app system
// on Chrome OS.
BASE_FEATURE(kCrosWebAppShortcutUiUpdate,
             "CrosWebAppShortcutUiUpdate",
             base::FEATURE_DISABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

// Enables denying file access to dlp protected files in MyFiles.
BASE_FEATURE(kDataControlsFileAccessDefaultDeny,
             "DataControlsFileAccessDefaultDeny",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables data migration.
BASE_FEATURE(kDataMigration,
             "DataMigration",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the desk profiles feature.
BASE_FEATURE(kDeskProfiles, "DeskProfiles", base::FEATURE_DISABLED_BY_DEFAULT);

// Disable idle sockets closing on memory pressure for NetworkContexts that
// belong to Profiles. It only applies to Profiles because the goal is to
// improve perceived performance of web browsing within the ChromeOS user
// session by avoiding re-estabshing TLS connections that require client
// certificates.
BASE_FEATURE(kDisableIdleSocketsCloseOnMemoryPressure,
             "disable_idle_sockets_close_on_memory_pressure",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Disables "Office Editing for Docs, Sheets & Slides" component app so handlers
// won't be registered, making it possible to install another version for
// testing.
BASE_FEATURE(kDisableOfficeEditingComponentApp,
             "DisableOfficeEditingComponentApp",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Disables translation services of the Quick Answers V2.
BASE_FEATURE(kDisableQuickAnswersV2Translation,
             "DisableQuickAnswersV2Translation",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables import of PKCS12 files to software backed Chaps storage together with
// import to NSS DB via the "Import" button in the certificates manager.
// When the feature is disabled, PKCS12 files are imported to NSS DB only.
BASE_FEATURE(kEnablePkcs12ToChapsDualWrite,
             "EnablePkcs12ToChapsDualWrite",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables Essential Search in Omnibox for both launcher and browser.
BASE_FEATURE(kEssentialSearch,
             "EssentialSearch",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enable experimental goldfish web app isolation.
BASE_FEATURE(kExperimentalWebAppStoragePartitionIsolation,
             "ExperimentalWebAppStoragePartitionIsolation",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables Jelly features. go/jelly-flags
BASE_FEATURE(kJelly, "Jelly", base::FEATURE_ENABLED_BY_DEFAULT);

// Enables Jellyroll features. Jellyroll is a feature flag for CrOSNext, which
// controls all system UI updates and new system components. go/jelly-flags
BASE_FEATURE(kJellyroll, "Jellyroll", base::FEATURE_ENABLED_BY_DEFAULT);

#if BUILDFLAG(IS_CHROMEOS_ASH)
// Enables Kiosk Heartbeats to be sent via Encrypted Reporting Pipeline
BASE_FEATURE(kKioskHeartbeatsViaERP,
             "KioskHeartbeatsViaERP",
             base::FEATURE_DISABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

// Controls enabling / disabling the mahi feature.
BASE_FEATURE(kMahi, "Mahi", base::FEATURE_DISABLED_BY_DEFAULT);

// Controls enabling / disabling the orca feature.
BASE_FEATURE(kOrca, "Orca", base::FEATURE_DISABLED_BY_DEFAULT);

// Controls enabling / disabling the orca feature for dogfood population.
BASE_FEATURE(kOrcaDogfood, "OrcaDogfood", base::FEATURE_DISABLED_BY_DEFAULT);

// Controls enabling / disabling the orca feature from the feature management
// module.
BASE_FEATURE(kFeatureManagementOrca,
             "FeatureManagementOrca",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Whether to disable chrome compose.
BASE_FEATURE(kFeatureManagementDisableChromeCompose,
             "FeatureManagementDisableChromeCompose",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Controls whether to enable quick answers V2 settings sub-toggles.
BASE_FEATURE(kQuickAnswersV2SettingsSubToggle,
             "QuickAnswersV2SettingsSubToggle",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Controls whether to enable Quick Answers Rich card.
BASE_FEATURE(kQuickAnswersRichCard,
             "QuickAnswersRichCard",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Controls whether to enable Material Next UI for Quick Answers.
BASE_FEATURE(kQuickAnswersMaterialNextUI,
             "QuickAnswersMaterialNextUI",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the Office files upload workflow to improve Office files support.
BASE_FEATURE(kUploadOfficeToCloud,
             "UploadOfficeToCloud",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables the Office files upload workflow for enterprise users to improve
// Office files support.
BASE_FEATURE(kUploadOfficeToCloudForEnterprise,
             "UploadOfficeToCloudForEnterprise",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Enables the Microsoft OneDrive integration workflow for enterprise users to
// cloud integration support.
BASE_FEATURE(kMicrosoftOneDriveIntegrationForEnterprise,
             "MicrosoftOneDriveIntegrationForEnterprise",
             base::FEATURE_ENABLED_BY_DEFAULT);

BASE_FEATURE(kRoundedWindows,
             "RoundedWindows",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables CloudFileSystem for FileSystemProvider extensions.
BASE_FEATURE(kFileSystemProviderCloudFileSystem,
             "FileSystemProviderCloudFileSystem",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Enables a content cache in CloudFileSystem for FileSystemProvider extensions.
BASE_FEATURE(kFileSystemProviderContentCache,
             "FileSystemProviderContentCache",
             base::FEATURE_DISABLED_BY_DEFAULT);

const char kRoundedWindowsRadius[] = "window_radius";

bool IsAppInstallServiceUriEnabled() {
  if (g_app_install_service_uri_enabled_for_testing) {
    return true;
  }
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()->IsAppInstallServiceUriEnabled();
#else
  return base::FeatureList::IsEnabled(kAppInstallServiceUri);
#endif
}

bool IsCaptivePortalPopupWindowEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()
      ->IsCaptivePortalPopupWindowEnabled();
#else
  return base::FeatureList::IsEnabled(kCaptivePortalPopupWindow);
#endif
}

bool IsClipboardHistoryRefreshEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()->EnableClipboardHistoryRefresh();
#else
  return base::FeatureList::IsEnabled(kClipboardHistoryRefresh) &&
         IsJellyEnabled();
#endif
}

bool IsCloudGamingDeviceEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()->IsCloudGamingDevice();
#else
  return base::FeatureList::IsEnabled(kCloudGamingDevice);
#endif
}

bool IsBlinkExtensionEnabled() {
  return base::FeatureList::IsEnabled(kBlinkExtension);
}

bool IsBlinkExtensionDiagnosticsEnabled() {
  return IsBlinkExtensionEnabled() &&
         base::FeatureList::IsEnabled(kBlinkExtensionDiagnostics);
}

bool IsCrosComponentsEnabled() {
  return base::FeatureList::IsEnabled(kCrosComponents) && IsJellyEnabled();
}

bool IsCrosShortstandEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()->IsCrosShortstandEnabled();
#else
  return base::FeatureList::IsEnabled(kCrosShortstand);
#endif
}

bool IsCrosWebAppInstallDialogEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()
      ->IsCrosWebAppInstallDialogEnabled();
#else
  return base::FeatureList::IsEnabled(kCrosWebAppInstallDialog);
#endif
}

bool IsCrosWebAppShortcutUiUpdateEnabled() {
  if (IsCrosShortstandEnabled()) {
    return true;
  }
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()
      ->IsCrosWebAppShortcutUiUpdateEnabled();
#else
  return base::FeatureList::IsEnabled(kCrosWebAppShortcutUiUpdate);
#endif
}

bool IsDataControlsFileAccessDefaultDenyEnabled() {
  return base::FeatureList::IsEnabled(kDataControlsFileAccessDefaultDeny);
}

bool IsDataMigrationEnabled() {
  return base::FeatureList::IsEnabled(kDataMigration);
}

bool IsDeskProfilesEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()->IsDeskProfilesEnabled();
#else
  return base::FeatureList::IsEnabled(kDeskProfiles);
#endif
}

bool IsEssentialSearchEnabled() {
  return base::FeatureList::IsEnabled(kEssentialSearch);
}

bool IsFileSystemProviderCloudFileSystemEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()
      ->IsFileSystemProviderCloudFileSystemEnabled();
#else
  return base::FeatureList::IsEnabled(kFileSystemProviderCloudFileSystem);
#endif
}

bool IsFileSystemProviderContentCacheEnabled() {
  // The `ContentCache` will be owned by the `CloudFileSystem`. Thus, the
  // `FileSystemProviderCloudFileSystem` flag has to be enabled too.
  return IsFileSystemProviderCloudFileSystemEnabled() &&
         base::FeatureList::IsEnabled(kFileSystemProviderContentCache);
}

bool IsJellyEnabled() {
  return base::FeatureList::IsEnabled(kJelly);
}

bool IsJellyrollEnabled() {
  // Only enable Jellyroll if Jelly is also enabled as this is how tests expect
  // this to behave.
  return IsJellyEnabled() && base::FeatureList::IsEnabled(kJellyroll);
}

bool IsMahiEnabled() {
  return base::FeatureList::IsEnabled(kMahi);
}

bool IsOrcaEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()->IsOrcaEnabled();
#else
  return base::FeatureList::IsEnabled(chromeos::features::kOrcaDogfood) ||
         (base::FeatureList::IsEnabled(chromeos::features::kOrca) &&
          base::FeatureList::IsEnabled(kFeatureManagementOrca));
#endif
}

bool ShouldDisableChromeComposeOnChromeOS() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()
      ->ShouldDisableChromeComposeOnChromeOS();
#else
  return base::FeatureList::IsEnabled(kFeatureManagementDisableChromeCompose) ||
         IsOrcaEnabled();
#endif
}

bool IsQuickAnswersV2TranslationDisabled() {
  return base::FeatureList::IsEnabled(kDisableQuickAnswersV2Translation);
}

bool IsQuickAnswersRichCardEnabled() {
  return base::FeatureList::IsEnabled(kQuickAnswersRichCard);
}

bool IsQuickAnswersV2SettingsSubToggleEnabled() {
  return base::FeatureList::IsEnabled(kQuickAnswersV2SettingsSubToggle);
}

bool IsUploadOfficeToCloudEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  return chromeos::BrowserParamsProxy::Get()->IsUploadOfficeToCloudEnabled();
#else
  return base::FeatureList::IsEnabled(kUploadOfficeToCloud);
#endif
}

bool IsUploadOfficeToCloudForEnterpriseEnabled() {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  // TODO(b/296282654): Implement propagation if necessary.
  return false;
#else
  return base::FeatureList::IsEnabled(kUploadOfficeToCloud) &&
         base::FeatureList::IsEnabled(kUploadOfficeToCloudForEnterprise);
#endif
}

bool IsMicrosoftOneDriveIntegrationForEnterpriseEnabled() {
  return IsUploadOfficeToCloudEnabled() &&
         base::FeatureList::IsEnabled(
             kMicrosoftOneDriveIntegrationForEnterprise);
}

bool IsRoundedWindowsEnabled() {
  // Rounded windows are under the Jelly feature.
  return base::FeatureList::IsEnabled(kRoundedWindows) &&
         base::FeatureList::IsEnabled(kJelly);
}

bool IsPkcs12ToChapsDualWriteEnabled() {
  return base::FeatureList::IsEnabled(kEnablePkcs12ToChapsDualWrite);
}

int RoundedWindowsRadius() {
  if (!IsRoundedWindowsEnabled()) {
    return 0;
  }

  return base::GetFieldTrialParamByFeatureAsInt(
      kRoundedWindows, kRoundedWindowsRadius, /*default_value=*/12);
}

base::AutoReset<bool> SetAppInstallServiceUriEnabledForTesting() {
  return {&g_app_install_service_uri_enabled_for_testing, true};
}

}  // namespace chromeos::features
