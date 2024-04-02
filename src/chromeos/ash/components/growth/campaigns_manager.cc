// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/ash/components/growth/campaigns_manager.h"

#include <optional>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/syslog_logging.h"
#include "base/task/thread_pool.h"
#include "base/time/time.h"
#include "chromeos/ash/components/growth/campaigns_matcher.h"
#include "chromeos/ash/components/growth/growth_metrics.h"
#include "components/prefs/pref_service.h"

namespace growth {

namespace {

CampaignsManager* g_instance = nullptr;

inline constexpr char kCampaignFileName[] = "campaigns.json";

std::optional<base::Value::Dict> ReadCampaignsFile(
    const base::FilePath& campaigns_component_path) {
  const auto campaigns_load_start_time = base::TimeTicks::Now();

  std::string campaigns_data;
  if (!base::ReadFileToString(
          campaigns_component_path.Append(kCampaignFileName),
          &campaigns_data)) {
    LOG(ERROR) << "Failed to read campaigns file from disk.";
    RecordCampaignsManagerError(CampaignsManagerError::kCampaignsFileLoadFail);
    RecordCampaignsComponentReadDuration(base::TimeTicks::Now() -
                                         campaigns_load_start_time);
    return std::nullopt;
  }

  std::optional<base::Value> value(base::JSONReader::Read(campaigns_data));
  if (!value || !value->is_dict()) {
    LOG(ERROR) << "Failed to parse campaigns file.";
    RecordCampaignsManagerError(CampaignsManagerError::kCampaignsParsingFail);
    RecordCampaignsComponentReadDuration(base::TimeTicks::Now() -
                                         campaigns_load_start_time);
    return std::nullopt;
  }

  RecordCampaignsComponentReadDuration(base::TimeTicks::Now() -
                                       campaigns_load_start_time);
  return std::move(value->GetDict());
}

void LogCampaignInSystemLog(const Campaign* campaign, Slot slot) {
  if (!campaign) {
    return;
  }

  std::optional<int> id = growth::GetCampaignId(campaign);
  if (!id) {
    // TODO(b/308684443): Add error metrics in a follow up CL.
    LOG(ERROR) << "Growth campaign id not found";
    return;
  }

  SYSLOG(INFO) << "Growth Campaign " << *id
               << " is selected for slot: " << base::NumberToString(int(slot));
}

}  // namespace

// static
CampaignsManager* CampaignsManager::Get() {
  DCHECK(g_instance);
  return g_instance;
}

CampaignsManager::CampaignsManager(CampaignsManagerClient* client,
                                   PrefService* local_state)
    : client_(client), matcher_(client, local_state) {
  CHECK_EQ(g_instance, nullptr);
  g_instance = this;
}

CampaignsManager::~CampaignsManager() {
  CHECK_EQ(g_instance, this);
  g_instance = nullptr;
}

void CampaignsManager::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void CampaignsManager::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void CampaignsManager::SetPrefs(PrefService* prefs) {
  // Pass `prefs` to `CampaignsMatcher` to enable prefs related targettings.
  matcher_.SetPrefs(prefs);
}

void CampaignsManager::LoadCampaigns(base::OnceClosure load_callback,
                                     bool in_oobe) {
  campaigns_download_start_time_ = base::TimeTicks::Now();
  client_->LoadCampaignsComponent(base::BindOnce(
      &CampaignsManager::OnCampaignsComponentLoaded, weak_factory_.GetWeakPtr(),
      std::move(load_callback), in_oobe));
}

const Campaign* CampaignsManager::GetCampaignBySlot(Slot slot) const {
  CHECK(campaigns_loaded_)
      << "Getting campaign before campaigns finish loading";
  const auto match_start = base::TimeTicks::Now();
  auto* match_result = matcher_.GetCampaignBySlot(slot);
  if (match_result) {
    RecordGetCampaignBySlot(slot);
  }

  RecordCampaignMatchDuration(base::TimeTicks::Now() - match_start);
  LogCampaignInSystemLog(match_result, slot);

  RegisterTrialForCampaign(match_result);

  return match_result;
}

void CampaignsManager::OnCampaignsComponentLoaded(
    base::OnceClosure load_callback,
    bool in_oobe,
    const std::optional<const base::FilePath>& path) {
  RecordCampaignsComponentDownloadDuration(
      base::TimeTicks::Now() - campaigns_download_start_time_, in_oobe);
  if (!path.has_value()) {
    LOG(ERROR) << "Failed to load campaign component.";
    RecordCampaignsManagerError(
        CampaignsManagerError::kCampaignsComponentLoadFail);
    OnCampaignsLoaded(std::move(load_callback), /*campaigns=*/std::nullopt);
    return;
  }
  // Read the campaigns file from component mounted path.
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock()}, base::BindOnce(&ReadCampaignsFile, *path),
      base::BindOnce(&CampaignsManager::OnCampaignsLoaded,
                     weak_factory_.GetWeakPtr(), std::move(load_callback)));
}

void CampaignsManager::OnCampaignsLoaded(
    base::OnceClosure load_callback,
    std::optional<base::Value::Dict> campaigns_dict) {
  // Load campaigns into campaigns store.
  if (campaigns_dict.has_value()) {
    // Update campaigns store.
    campaigns_ = std::move(campaigns_dict.value());
  } else {
    LOG(ERROR) << "No campaign is loaded.";
  }

  // Load campaigns into `CampaignMatcher` for selecting campaigns.
  matcher_.SetCampaigns(&campaigns_);
  campaigns_loaded_ = true;

  std::move(load_callback).Run();
  NotifyCampaignsLoaded();
}

void CampaignsManager::NotifyCampaignsLoaded() {
  for (auto& observer : observers_) {
    observer.OnCampaignsLoadCompleted();
  }
}

void CampaignsManager::RegisterTrialForCampaign(
    const Campaign* campaign) const {
  if (!campaign) {
    return;
  }

  std::optional<int> id = growth::GetCampaignId(campaign);
  if (!id) {
    // TODO(b/308684443): Add error metrics in a second CL.
    LOG(ERROR) << "Growth campaign id not found";
    return;
  }

  client_->RegisterSyntheticFieldTrial(
      /*study_id=*/growth::GetStudyId(campaign),
      /*campaign_id=*/*id);
}

}  // namespace growth
