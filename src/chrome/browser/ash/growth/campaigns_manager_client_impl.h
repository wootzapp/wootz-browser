// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_GROWTH_CAMPAIGNS_MANAGER_CLIENT_IMPL_H_
#define CHROME_BROWSER_ASH_GROWTH_CAMPAIGNS_MANAGER_CLIENT_IMPL_H_

#include <map>
#include <memory>
#include <string>

#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/ash/growth/metrics.h"
#include "chrome/browser/ash/growth/ui_action_performer.h"
#include "chromeos/ash/components/growth/campaigns_configuration_provider.h"
#include "chromeos/ash/components/growth/campaigns_manager_client.h"
#include "components/component_updater/ash/component_manager_ash.h"

namespace base {
class Version;
}

namespace growth {
class CampaignsManager;
}  // namespace growth

class CampaignsManagerClientImpl : public growth::CampaignsManagerClient,
                                   public UiActionPerformer::Observer {
 public:
  CampaignsManagerClientImpl();
  CampaignsManagerClientImpl(const CampaignsManagerClientImpl&) = delete;
  CampaignsManagerClientImpl& operator=(const CampaignsManagerClientImpl&) =
      delete;
  ~CampaignsManagerClientImpl() override;

  // growth::CampaignsManagerClient:
  void LoadCampaignsComponent(
      growth::CampaignComponentLoadedCallback callback) override;
  bool IsDeviceInDemoMode() const override;
  bool IsCloudGamingDevice() const override;
  bool IsFeatureAwareDevice() const override;
  const std::string& GetApplicationLocale() const override;
  const std::string& GetUserLocale() const override;
  const base::Version& GetDemoModeAppVersion() const override;
  growth::ActionMap GetCampaignsActions() override;
  void RegisterSyntheticFieldTrial(const std::optional<int> study_id,
                                   const int campaign_id) const override;
  void ClearConfig(const std::map<std::string, std::string>& params) override;
  void NotifyEvent(const std::string& event_name) override;
  bool WouldTriggerHelpUI(
      const std::map<std::string, std::string>& params) override;
  signin::IdentityManager* GetIdentityManager() const override;

  // UiActionPerformer::Observer:
  void OnReadyToLogImpression(int campaign_id) override;
  void OnDismissed(int campaign_id) override;
  void OnButtonPressed(int campaign_id,
                       CampaignButtonId button_id,
                       bool should_mark_dismissed) override;

 private:
  void OnComponentDownloaded(
      growth::CampaignComponentLoadedCallback loaded_callback,
      component_updater::ComponentManagerAsh::Error error,
      const base::FilePath& path);
  void UpdateConfig(const std::map<std::string, std::string>& params);

  growth::CampaignsConfigurationProvider config_provider_;
  std::unique_ptr<growth::CampaignsManager> campaigns_manager_;

  // Reset before `campaigns_manager_`, because `this` observes one of the
  // performers owned by the manager.
  base::ScopedObservation<UiActionPerformer, UiActionPerformer::Observer>
      show_nudge_performer_observation_{this};

  // Reset before `campaigns_manager_`, because `this` observes one of the
  // performers owned by the manager.
  base::ScopedObservation<UiActionPerformer, UiActionPerformer::Observer>
      show_notification_performer_observation_{this};

  base::WeakPtrFactory<CampaignsManagerClientImpl> weak_ptr_factory_{this};
};

#endif  // CHROME_BROWSER_ASH_GROWTH_CAMPAIGNS_MANAGER_CLIENT_IMPL_H_
