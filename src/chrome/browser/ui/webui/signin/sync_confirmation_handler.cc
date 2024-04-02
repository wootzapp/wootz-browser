// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/signin/sync_confirmation_handler.h"

#include <vector>

#include "base/functional/bind.h"
#include "base/location.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "base/metrics/user_metrics.h"
#include "base/notreached.h"
#include "base/time/time.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/consent_auditor/consent_auditor_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_avatar_icon_util.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/browser/sync/sync_service_factory.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/signin/signin_view_controller_delegate.h"
#include "chrome/browser/ui/webui/signin/login_ui_service_factory.h"
#include "chrome/browser/ui/webui/signin/signin_utils.h"
#include "chrome/browser/ui/webui/signin/sync_confirmation_ui.h"
#include "chrome/common/webui_url_constants.h"
#include "components/consent_auditor/consent_auditor.h"
#include "components/signin/public/base/avatar_icon_util.h"
#include "components/signin/public/base/consent_level.h"
#include "components/signin/public/base/signin_switches.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "components/signin/public/identity_manager/tribool.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "url/gurl.h"

#if BUILDFLAG(IS_CHROMEOS_LACROS)
#include "ash/webui/settings/public/constants/routes.mojom.h"
#include "chrome/browser/lacros/lacros_url_handling.h"
#include "chrome/common/webui_url_constants.h"
#include "components/sync/base/features.h"
#endif

using signin::ConsentLevel;

namespace {
const int kProfileImageSize = 128;

// Derives screen mode of sync opt in screen from the
// CanShowHistorySyncOptInsWithoutMinorModeRestrictions capability.
bool UseMinorModeRestrictions() {
#if BUILDFLAG(IS_CHROMEOS_ASH) || BUILDFLAG(IS_CHROMEOS_LACROS)
  // ChromeOS handles minor modes separately.
  return false;
#else
  return base::FeatureList::IsEnabled(
      ::switches::kMinorModeRestrictionsForHistorySyncOptIn);
#endif
}

// After this time delta, user must see a screen. If it was impossible to get
// the CanShowHistorySyncOptInsWithoutMinorModeRestrictions capability before
// the deadline, the screen should be configured in minor-safe way.
base::TimeDelta GetMinorModeRestrictionsDeadline() {
#if BUILDFLAG(IS_CHROMEOS_ASH) || BUILDFLAG(IS_CHROMEOS_LACROS)
  // Not implemented for those platforms.
  NOTREACHED_NORETURN();
#else
  return base::Milliseconds(
      ::switches::kMinorModeRestrictionsFetchDeadlineMs.Get());
#endif
}

inline bool ScreenModeIsPending(const AccountInfo& primary_account_info) {
  return GetScreenMode(primary_account_info.capabilities) ==
         SyncConfirmationScreenMode::kPending;
}

}  // namespace

SyncConfirmationScreenMode GetScreenMode(
    const AccountCapabilities& capabilities) {
  switch (
      capabilities
          .can_show_history_sync_opt_ins_without_minor_mode_restrictions()) {
    case signin::Tribool::kUnknown:
      return SyncConfirmationScreenMode::kPending;
    case signin::Tribool::kFalse:
      return SyncConfirmationScreenMode::kRestricted;
    case signin::Tribool::kTrue:
      return SyncConfirmationScreenMode::kUnrestricted;
  }
}

SyncConfirmationHandler::SyncConfirmationHandler(
    Profile* profile,
    const std::unordered_map<std::string, int>& string_to_grd_id_map,
    Browser* browser)
    : profile_(profile),
      string_to_grd_id_map_(string_to_grd_id_map),
      browser_(browser),
      identity_manager_(IdentityManagerFactory::GetForProfile(profile_)) {
  DCHECK(profile_);
  BrowserList::AddObserver(this);
}

SyncConfirmationHandler::~SyncConfirmationHandler() {
  BrowserList::RemoveObserver(this);

  // Abort signin and prevent sync from starting if none of the actions on the
  // sync confirmation dialog are taken by the user.
  if (!did_user_explicitly_interact_) {
    CloseModalSigninWindow(LoginUIService::UI_CLOSED);
  }
}

void SyncConfirmationHandler::OnBrowserRemoved(Browser* browser) {
  if (browser_ == browser)
    browser_ = nullptr;
}

void SyncConfirmationHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "confirm", base::BindRepeating(&SyncConfirmationHandler::HandleConfirm,
                                     base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "undo", base::BindRepeating(&SyncConfirmationHandler::HandleUndo,
                                  base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "goToSettings",
      base::BindRepeating(&SyncConfirmationHandler::HandleGoToSettings,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "initializedWithSize",
      base::BindRepeating(&SyncConfirmationHandler::HandleInitializedWithSize,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "accountInfoRequest",
      base::BindRepeating(&SyncConfirmationHandler::HandleAccountInfoRequest,
                          base::Unretained(this)));
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  web_ui()->RegisterMessageCallback(
      "openDeviceSyncSettings",
      base::BindRepeating(
          &SyncConfirmationHandler::HandleOpenDeviceSyncSettings,
          base::Unretained(this)));
#endif
}

void SyncConfirmationHandler::HandleConfirm(const base::Value::List& args) {
  did_user_explicitly_interact_ = true;
  RecordConsent(args);
  CloseModalSigninWindow(LoginUIService::SYNC_WITH_DEFAULT_SETTINGS);
}

void SyncConfirmationHandler::HandleGoToSettings(
    const base::Value::List& args) {
  DCHECK(SyncServiceFactory::IsSyncAllowed(profile_));
  did_user_explicitly_interact_ = true;
  RecordConsent(args);
  CloseModalSigninWindow(LoginUIService::CONFIGURE_SYNC_FIRST);
}

void SyncConfirmationHandler::HandleUndo(const base::Value::List& args) {
  did_user_explicitly_interact_ = true;
  CloseModalSigninWindow(LoginUIService::ABORT_SYNC);
}

void SyncConfirmationHandler::HandleAccountInfoRequest(
    const base::Value::List& args) {
  DCHECK(SyncServiceFactory::IsSyncAllowed(profile_));
  AccountInfo primary_account_info = identity_manager_->FindExtendedAccountInfo(
      identity_manager_->GetPrimaryAccountInfo(ConsentLevel::kSignin));

  // Fire the "account-info-changed" and "screen-mode-changed" listeners.
  // Note: If the account info is not available yet in the
  // IdentityManager, i.e. account_info is empty or capabilities are not ready
  // yet, the listener will be fired again through `OnAccountUpdated()`.
  DispatchAccountInfoUpdate(primary_account_info);
}

#if BUILDFLAG(IS_CHROMEOS_LACROS)
void SyncConfirmationHandler::HandleOpenDeviceSyncSettings(
    const base::Value::List& args) {
  std::string os_sync_settings_url = chrome::kChromeUIOSSettingsURL;
  os_sync_settings_url.append(chromeos::settings::mojom::kSyncSubpagePath);
  lacros_url_handling::NavigateInAsh(GURL(os_sync_settings_url));
}
#endif

void SyncConfirmationHandler::RecordConsent(const base::Value::List& args) {
  CHECK_EQ(2U, args.size());
  const base::Value::List& consent_description = args[0].GetList();
  const std::string& consent_confirmation = args[1].GetString();

  // The strings returned by the WebUI are not free-form, they must belong into
  // a pre-determined set of strings (stored in |string_to_grd_id_map_|). As
  // this has privacy and legal implications, CHECK the integrity of the strings
  // received from the renderer process before recording the consent.
  std::vector<int> consent_text_ids;
  for (const base::Value& text : consent_description) {
    auto iter = string_to_grd_id_map_.find(text.GetString());
    CHECK(iter != string_to_grd_id_map_.end()) << "Unexpected string:\n"
                                               << text.GetString();
    consent_text_ids.push_back(iter->second);
  }

  auto iter = string_to_grd_id_map_.find(consent_confirmation);
  CHECK(iter != string_to_grd_id_map_.end()) << "Unexpected string:\n"
                                             << consent_confirmation;
  int consent_confirmation_id = iter->second;

  sync_pb::UserConsentTypes::SyncConsent sync_consent;
  sync_consent.set_confirmation_grd_id(consent_confirmation_id);
  for (int id : consent_text_ids) {
    sync_consent.add_description_grd_ids(id);
  }
  sync_consent.set_status(sync_pb::UserConsentTypes::ConsentStatus::
                              UserConsentTypes_ConsentStatus_GIVEN);

  consent_auditor::ConsentAuditor* consent_auditor =
      ConsentAuditorFactory::GetForProfile(profile_);
  consent_auditor->RecordSyncConsent(
      identity_manager_->GetPrimaryAccountId(ConsentLevel::kSignin),
      sync_consent);
}

void SyncConfirmationHandler::OnAvatarChanged(const AccountInfo& info) {
  DCHECK(info.IsValid());
  avatar_notified_ = true;

  GURL picture_gurl(info.picture_url);
  GURL picture_gurl_with_options = signin::GetAvatarImageURLWithOptions(
      picture_gurl, kProfileImageSize, /*no_silhouette=*/false);

  base::Value::Dict value;
  value.Set("src", picture_gurl_with_options.spec());
  value.Set("showEnterpriseBadge", info.IsManaged());
  FireWebUIListener("account-info-changed", value);
}

void SyncConfirmationHandler::OnScreenModeChanged(
    SyncConfirmationScreenMode mode) {
  DCHECK(mode != SyncConfirmationScreenMode::kPending);
  DCHECK(!screen_mode_notified_) << "Must be called only once";
  screen_mode_notified_ = true;
  screen_mode_deadline_.Stop();

  // Note on timing: this function is executed exactly once
  // because it sets `screen_mode_notified_` to `true`. This means that the
  // latencies are recorded only once, but either immediately from
  // `HandleInitializedWithSize`, or subsequently as a result of OnDeadline or
  // OnExtendedAccountInfoUpdated. For the latter case, the timer was started in
  // `HandleInitializedWithSize` when the capability was requested but not
  // available.

  if (user_visible_latency_.has_value()) {
    // Timer was started so it means it is a subsequent attempt.
    base::TimeDelta elapsed = user_visible_latency_->Elapsed();
    base::UmaHistogramTimes("Signin.AccountCapabilities.UserVisibleLatency",
                            elapsed);
    base::UmaHistogramTimes("Signin.AccountCapabilities.FetchLatency", elapsed);
  } else {
    base::UmaHistogramBoolean("Signin.AccountCapabilities.ImmediatelyAvailable",
                              true);
    base::UmaHistogramTimes("Signin.AccountCapabilities.UserVisibleLatency",
                            base::Seconds(0));
  }

  FireWebUIListener("screen-mode-changed", static_cast<int>(mode));
}

void SyncConfirmationHandler::OnDeadline() {
  if (!screen_mode_notified_) {
    OnScreenModeChanged(SyncConfirmationScreenMode::kRestricted);
  }
}

void SyncConfirmationHandler::DispatchAccountInfoUpdate(
    const AccountInfo& info) {
  if (info.IsEmpty()) {
    // No account is signed in, so there is nothing to be displayed in the sync
    // confirmation dialog.
    return;
  }

  if (!SyncServiceFactory::IsSyncAllowed(profile_)) {
    // The sync disabled confirmation handler does not present the user image.
    // Avoid updating the image URL in this case.
    return;
  }

  if (info.account_id !=
      identity_manager_->GetPrimaryAccountId(ConsentLevel::kSignin)) {
    return;
  }

  AllowJavascript();

  if (info.IsValid() && !avatar_notified_) {
    OnAvatarChanged(info);
  }

  if (screen_mode_notified_) {
    // Screen mode must be changed only once.
    return;
  }

  if (!UseMinorModeRestrictions()) {
    OnScreenModeChanged(SyncConfirmationScreenMode::kUnrestricted);
    return;
  }

  if (ScreenModeIsPending(info)) {
    return;
  }

  OnScreenModeChanged(GetScreenMode(info.capabilities));
}

void SyncConfirmationHandler::OnExtendedAccountInfoUpdated(
    const AccountInfo& info) {
  DispatchAccountInfoUpdate(info);

  if (avatar_notified_ && screen_mode_notified_) {
    // IdentityManager emitted both avatar and screen mode information and its
    // function is done.
    identity_manager_observation_.Reset();
  }
}

void SyncConfirmationHandler::CloseModalSigninWindow(
    LoginUIService::SyncConfirmationUIClosedResult result) {
  switch (result) {
    case LoginUIService::CONFIGURE_SYNC_FIRST:
      base::RecordAction(
          base::UserMetricsAction("Signin_Signin_WithAdvancedSyncSettings"));
      break;
    case LoginUIService::SYNC_WITH_DEFAULT_SETTINGS:
      base::RecordAction(
          base::UserMetricsAction("Signin_Signin_WithDefaultSyncSettings"));
      break;
    case LoginUIService::ABORT_SYNC:
      base::RecordAction(base::UserMetricsAction("Signin_Undo_Signin"));
      break;
    case LoginUIService::UI_CLOSED:
      base::RecordAction(base::UserMetricsAction("Signin_Abort_Signin"));
      break;
  }
  LoginUIServiceFactory::GetForProfile(profile_)->SyncConfirmationUIClosed(
      result);
}

void SyncConfirmationHandler::HandleInitializedWithSize(
    const base::Value::List& args) {
  AccountInfo primary_account_info = identity_manager_->FindExtendedAccountInfo(
      identity_manager_->GetPrimaryAccountInfo(ConsentLevel::kSignin));
  if (primary_account_info.IsEmpty()) {
    // No account is signed in, so there is nothing to be displayed in the sync
    // confirmation dialog.
    return;
  }

  DispatchAccountInfoUpdate(primary_account_info);

  if (UseMinorModeRestrictions() && ScreenModeIsPending(primary_account_info) &&
      !user_visible_latency_.has_value()) {
    CHECK(!screen_mode_notified_);
    // `user_visible_latency_` timer is only ticking when the capabilities
    // were not immediately available, but is only started at the very first
    // attempt to access them.
    user_visible_latency_.emplace();
    base::UmaHistogramBoolean("Signin.AccountCapabilities.ImmediatelyAvailable",
                              false);
  }

  if (!avatar_notified_ ||
      (!screen_mode_notified_ && UseMinorModeRestrictions())) {
    // IdentityManager emits both avatar and screen mode information.
    identity_manager_observation_.Observe(identity_manager_);
  }

  if (!screen_mode_notified_ && UseMinorModeRestrictions()) {
    // Deadline timer for the case when screen mode doesn't arrive in time.
    screen_mode_deadline_.Start(FROM_HERE, GetMinorModeRestrictionsDeadline(),
                                this, &SyncConfirmationHandler::OnDeadline);
  }

  if (browser_)
    signin::SetInitializedModalHeight(browser_, web_ui(), args);
}
