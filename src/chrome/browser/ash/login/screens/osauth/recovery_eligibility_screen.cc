// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/login/screens/osauth/recovery_eligibility_screen.h"

#include <optional>

#include "ash/constants/ash_features.h"
#include "ash/constants/ash_pref_names.h"
#include "base/functional/callback.h"
#include "chrome/browser/ash/login/screens/base_screen.h"
#include "chrome/browser/ash/login/wizard_context.h"
#include "chrome/browser/policy/profile_policy_connector.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/webui/ash/login/recovery_eligibility_screen_handler.h"
#include "chromeos/ash/components/login/auth/public/user_context.h"
#include "chromeos/ash/components/login/auth/recovery/recovery_utils.h"
#include "chromeos/ash/components/osauth/public/auth_session_storage.h"
#include "components/prefs/pref_service.h"

namespace ash {

namespace {

// Returns `true` if the active Profile is enterprise managed.
bool IsUserEnterpriseManaged() {
  Profile* profile = ProfileManager::GetActiveUserProfile();
  return profile->GetProfilePolicyConnector()->IsManaged() &&
         !profile->IsChild();
}

}  // namespace

// static
std::string RecoveryEligibilityScreen::GetResultString(Result result) {
  switch (result) {
    case Result::PROCEED:
      return "Proceed";
    case Result::NOT_APPLICABLE:
      return BaseScreen::kNotApplicable;
  }
}

// static
bool RecoveryEligibilityScreen::ShouldSkipRecoverySetupBecauseOfPolicy() {
  return IsUserEnterpriseManaged() &&
         !GetRecoveryDefaultState(
             IsUserEnterpriseManaged(),
             ProfileManager::GetActiveUserProfile()->GetPrefs());
}

RecoveryEligibilityScreen::RecoveryEligibilityScreen(
    const ScreenExitCallback& exit_callback)
    : BaseOSAuthSetupScreen(RecoveryEligibilityView::kScreenId,
                 OobeScreenPriority::DEFAULT),
      exit_callback_(exit_callback) {}

RecoveryEligibilityScreen::~RecoveryEligibilityScreen() = default;

bool RecoveryEligibilityScreen::MaybeSkip(WizardContext& wizard_context) {
  if (wizard_context.skip_post_login_screens_for_tests) {
    exit_callback_.Run(Result::NOT_APPLICABLE);
    return true;
  }
  return false;
}

void RecoveryEligibilityScreen::ShowImpl() {
  InspectContextAndContinue(
      base::BindOnce(&RecoveryEligibilityScreen::InspectContext,
                     weak_ptr_factory_.GetWeakPtr()),
      base::BindOnce(&RecoveryEligibilityScreen::ProcessOptions,
                     weak_ptr_factory_.GetWeakPtr()));
}

void RecoveryEligibilityScreen::InspectContext(UserContext* user_context) {
  if (!user_context) {
    exit_callback_.Run(Result::NOT_APPLICABLE);
    return;
  }
  auto supported_factors =
      user_context->GetAuthFactorsConfiguration().get_supported_factors();
  recovery_supported_ = supported_factors.Has(cryptohome::AuthFactorType::kRecovery);
}

void RecoveryEligibilityScreen::ProcessOptions() {
  if (recovery_supported_) {
    context()->recovery_setup.is_supported = true;
    // Don't ask about recovery consent for managed users - use the policy value
    // instead.
    context()->recovery_setup.ask_about_recovery_consent =
        !IsUserEnterpriseManaged();
    context()->recovery_setup.recovery_factor_opted_in =
        GetRecoveryDefaultState(
            IsUserEnterpriseManaged(),
            ProfileManager::GetActiveUserProfile()->GetPrefs());
  }
  exit_callback_.Run(Result::PROCEED);
}

}  // namespace ash
