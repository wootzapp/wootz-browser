// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/growth/campaigns_manager_session.h"

#include <optional>

#include "ash/constants/ash_features.h"
#include "base/check.h"
#include "base/logging.h"
#include "chrome/browser/apps/app_service/app_service_proxy_ash.h"
#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/ash/ownership/owner_settings_service_ash.h"
#include "chrome/browser/ash/ownership/owner_settings_service_ash_factory.h"
#include "chrome/browser/policy/profile_policy_connector.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/web_applications/app_browser_controller.h"
#include "chromeos/ash/components/growth/action_performer.h"
#include "chromeos/ash/components/growth/campaigns_manager.h"
#include "chromeos/ash/components/growth/campaigns_model.h"
#include "components/app_constants/constants.h"
#include "components/session_manager/session_manager_types.h"

namespace {

CampaignsManagerSession* g_instance = nullptr;

std::optional<growth::ActionType> GetActionTypeBySlot(growth::Slot slot) {
  if (slot == growth::Slot::kNotification) {
    return growth::ActionType::kShowNotification;
  }

  if (slot == growth::Slot::kNudge) {
    return growth::ActionType::kShowNudge;
  }

  return std::nullopt;
}

void MaybeTriggerSlot(growth::Slot slot) {
  const auto action_type = GetActionTypeBySlot(slot);
  if (!action_type) {
    LOG(ERROR) << "Invalid: no supported action type for slot "
               << static_cast<int>(action_type.value());
    return;
  }

  auto* campaigns_manager = growth::CampaignsManager::Get();
  CHECK(campaigns_manager);

  auto* campaign = campaigns_manager->GetCampaignBySlot(slot);
  if (!campaign) {
    // No campaign matched.
    return;
  }

  auto campaign_id = growth::GetCampaignId(campaign);
  if (!campaign_id) {
    LOG(ERROR) << "Invalid: Missing campaign id.";
    return;
  }

  const auto* payload = growth::GetPayloadBySlot(campaign, slot);
  if (!payload) {
    // No payload for the targeted slot. It is valid for counterfactual control.
    return;
  }

  campaigns_manager->PerformAction(campaign_id.value(), action_type.value(),
                                   payload);
}

void MaybeTriggerCampaignsWhenAppOpened() {
  if (!ash::features::IsGrowthCampaignsTriggerByAppOpenEnabled()) {
    return;
  }

  auto* campaigns_manager = growth::CampaignsManager::Get();
  CHECK(campaigns_manager);

  growth::Trigger trigger(growth::TriggerType::kAppOpened);
  campaigns_manager->SetTrigger(std::move(trigger));

  MaybeTriggerSlot(growth::Slot::kNudge);
  MaybeTriggerSlot(growth::Slot::kNotification);
}

void MaybeTriggerCampaignsWhenCampaignsLoaded() {
  if (!ash::features::IsGrowthCampaignsTriggerAtLoadComplete()) {
    return;
  }

  auto* campaigns_manager = growth::CampaignsManager::Get();
  CHECK(campaigns_manager);

  growth::Trigger trigger(growth::TriggerType::kCampaignsLoaded);
  campaigns_manager->SetTrigger(std::move(trigger));

  MaybeTriggerSlot(growth::Slot::kNudge);
  MaybeTriggerSlot(growth::Slot::kNotification);
}

const GURL FindActiveWebAppBrowser(Profile* profile,
                                   const webapps::AppId& app_id) {
  for (Browser* browser : BrowserList::GetInstance()->OrderedByActivation()) {
    if (browser->profile() != profile) {
      continue;
    }

    if (web_app::AppBrowserController::IsForWebApp(browser, app_id)) {
      const auto* tab_strip_model = browser->tab_strip_model();
      if (!tab_strip_model) {
        LOG(ERROR) << "No tab_strip_model.";
        continue;
      }

      auto* active_web_contents = tab_strip_model->GetActiveWebContents();
      if (!active_web_contents) {
        LOG(ERROR) << "No active web contents.";
        continue;
      }

      return active_web_contents->GetURL();
    }
  }

  return GURL::EmptyGURL();
}

bool IsBrowserApp(const std::string& app_id) {
  return app_id == app_constants::kChromeAppId ||
         app_id == app_constants::kAshDebugBrowserAppId ||
         app_id == app_constants::kLacrosAppId;
}

bool IsAppActiveAndVisible(const apps::InstanceUpdate& update) {
  return ((update.State() & apps::InstanceState::kActive) &&
          (update.State() & apps::InstanceState::kVisible));
}

}  // namespace

// static
CampaignsManagerSession* CampaignsManagerSession::Get() {
  return g_instance;
}

CampaignsManagerSession::CampaignsManagerSession() {
  CHECK_EQ(g_instance, nullptr);
  g_instance = this;

  // SessionManager may be unset in unit tests.
  auto* session_manager = session_manager::SessionManager::Get();
  if (session_manager) {
    session_manager_observation_.Observe(session_manager);
    OnSessionStateChanged();
  }
}

CampaignsManagerSession::~CampaignsManagerSession() {
  CHECK_EQ(g_instance, this);
  g_instance = nullptr;
}

void CampaignsManagerSession::OnSessionStateChanged() {
  if (session_manager::SessionManager::Get()->session_state() ==
      session_manager::SessionState::LOCKED) {
    if (scoped_observation_.IsObserving()) {
      scoped_observation_.Reset();
    }
    return;
  }

  if (session_manager::SessionManager::Get()->session_state() !=
      session_manager::SessionState::ACTIVE) {
    // Loads campaigns at session active only.
    return;
  }

  if (!IsEligible()) {
    return;
  }

  ash::OwnerSettingsServiceAsh* service =
      ash::OwnerSettingsServiceAshFactory::GetForBrowserContext(GetProfile());
  if (service) {
    service->IsOwnerAsync(
        base::BindOnce(&CampaignsManagerSession::OnOwnershipDetermined,
                       weak_ptr_factory_.GetWeakPtr()));
  } else {
    // TODO: b/338085893 - Add metric to track the case that settings service
    // is not available at this point.
    LOG(ERROR) << "Owner settings service unavailable for the profile.";
  }
}

void CampaignsManagerSession::OnInstanceUpdate(
    const apps::InstanceUpdate& update) {
  // No state changes. Ignore the update.
  if (!update.StateChanged()) {
    return;
  }

  if (update.IsDestruction()) {
    HandleAppInstanceDestruction(update);
    return;
  }

  auto app_id = update.AppId();
  // For browser app, the user can open a new tab or switch to an existing tab.
  // The campaigns will be triggered when navigating to the target url.
  if (IsBrowserApp(app_id)) {
    if (ash::features::IsGrowthCampaignsTriggerByBrowserEnabled() &&
        IsAppActiveAndVisible(update)) {
      auto* campaigns_manager = growth::CampaignsManager::Get();
      CHECK(campaigns_manager);

      // TODO: b/339706247 - Set the app id and window on PrimaryPageChanged.
      campaigns_manager->SetOpenedApp(app_id);
      opened_window_ = update.Window();
    }

    return;
  }

  if (update.IsCreation()) {
    HandleAppInstanceCreation(update);
  }
}

void CampaignsManagerSession::OnInstanceRegistryWillBeDestroyed(
    apps::InstanceRegistry* cache) {
  if (scoped_observation_.GetSource() == cache) {
    scoped_observation_.Reset();
  }
}

void CampaignsManagerSession::PrimaryPageChanged(const GURL& url) {
  if (!ash::features::IsGrowthCampaignsTriggerByBrowserEnabled()) {
    return;
  }

  auto* campaigns_manager = growth::CampaignsManager::Get();
  CHECK(campaigns_manager);

  if (!IsBrowserApp(campaigns_manager->GetOpenedAppId())) {
    return;
  }
  campaigns_manager->SetActiveUrl(url);
  MaybeTriggerCampaignsWhenAppOpened();
}

void CampaignsManagerSession::SetProfileForTesting(Profile* profile) {
  profile_for_testing_ = profile;
}

Profile* CampaignsManagerSession::GetProfile() {
  if (profile_for_testing_) {
    return profile_for_testing_;
  }

  return ProfileManager::GetActiveUserProfile();
}

bool CampaignsManagerSession::IsEligible() {
  Profile* profile = GetProfile();
  CHECK(profile);
  // TODO(b/320789239): Enable for unicorn users.
  if (profile->GetProfilePolicyConnector()->IsManaged()) {
    // Only enabled for consumer session for now.
    // Demo Mode session is handled separately at `DemoSession`.
    return false;
  }

  return true;
}

void CampaignsManagerSession::SetupWindowObserver() {
  // Tests might not go through LOCKED state and `scoped_observation_` might
  // be observing.
  if (scoped_observation_.IsObserving()) {
    return;
  }

  auto* profile = GetProfile();
  // Some test profiles will not have AppServiceProxy.
  if (!apps::AppServiceProxyFactory::IsAppServiceAvailableForProfile(profile)) {
    return;
  }
  auto* proxy = apps::AppServiceProxyFactory::GetForProfile(profile);
  CHECK(proxy);
  scoped_observation_.Observe(&proxy->InstanceRegistry());
}

void CampaignsManagerSession::OnOwnershipDetermined(bool is_user_owner) {
  auto* campaigns_manager = growth::CampaignsManager::Get();
  CHECK(campaigns_manager);

  campaigns_manager->SetIsUserOwner(is_user_owner);

  campaigns_manager->LoadCampaigns(
      base::BindOnce(&CampaignsManagerSession::OnLoadCampaignsCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
}

void CampaignsManagerSession::OnLoadCampaignsCompleted() {
  if (ash::features::IsGrowthCampaignsTriggerByAppOpenEnabled()) {
    SetupWindowObserver();
  }

  MaybeTriggerCampaignsWhenCampaignsLoaded();
}

void CampaignsManagerSession::HandleAppInstanceCreation(
    const apps::InstanceUpdate& update) {
  auto* campaigns_manager = growth::CampaignsManager::Get();
  CHECK(campaigns_manager);

  auto app_id = update.AppId();

  campaigns_manager->SetOpenedApp(app_id);
  campaigns_manager->SetActiveUrl(
      FindActiveWebAppBrowser(GetProfile(), app_id));
  opened_window_ = update.Window();

  MaybeTriggerCampaignsWhenAppOpened();
}

void CampaignsManagerSession::HandleAppInstanceDestruction(
    const apps::InstanceUpdate& update) {
  // TODO: b/330409492 - Maybe trigger a campaign when app is about to be
  // destroyed.
  auto* campaigns_manager = growth::CampaignsManager::Get();
  CHECK(campaigns_manager);

  if (update.AppId() != campaigns_manager->GetOpenedAppId()) {
    return;
  }

  campaigns_manager->SetOpenedApp(std::string());
  opened_window_ = nullptr;
  active_url_ = GURL::EmptyGURL();
}
