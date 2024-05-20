// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/screen_ai/screen_ai_downloader_non_chromeos.h"

#include "base/no_destructor.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/component_updater/screen_ai_component_installer.h"
#include "chrome/browser/screen_ai/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/update_client/crx_update_item.h"
#include "content/public/browser/browser_thread.h"
#include "services/screen_ai/public/cpp/utilities.h"

namespace screen_ai {

// static
std::unique_ptr<ScreenAIInstallState> ScreenAIInstallState::Create() {
  return std::make_unique<ScreenAIDownloaderNonChromeOS>();
}

// static
ScreenAIInstallState* ScreenAIInstallState::CreateForTesting() {
  static base::NoDestructor<ScreenAIDownloaderNonChromeOS> install_state;
  return install_state.get();
}

ScreenAIDownloaderNonChromeOS::ScreenAIDownloaderNonChromeOS() = default;
ScreenAIDownloaderNonChromeOS::~ScreenAIDownloaderNonChromeOS() = default;

void ScreenAIDownloaderNonChromeOS::DownloadComponentInternal() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  component_updater::RegisterScreenAIComponent(
      g_browser_process->component_updater());
  if (!component_updater_observation_.IsObserving()) {
    component_updater_observation_.Observe(
        g_browser_process->component_updater());
  }
}

void ScreenAIDownloaderNonChromeOS::SetLastUsageTime() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  g_browser_process->local_state()->SetTime(
      prefs::kScreenAILastUsedTimePrefName, base::Time::Now());
}

void ScreenAIDownloaderNonChromeOS::OnEvent(
    update_client::UpdateClient::Observer::Events event,
    const std::string& omaha_id) {
  if (omaha_id !=
      component_updater::ScreenAIComponentInstallerPolicy::GetOmahaId()) {
    return;
  }

  switch (event) {
    case Events::COMPONENT_CHECKING_FOR_UPDATES:
    case Events::COMPONENT_UPDATE_FOUND:
    case Events::COMPONENT_WAIT:
    case Events::COMPONENT_UPDATE_READY:
    case Events::COMPONENT_ALREADY_UP_TO_DATE:
    case Events::COMPONENT_UPDATE_UPDATING:
      break;
    case Events::COMPONENT_UPDATED:
      RecordComponentInstallationResult(
          /*install=*/true, /*successful=*/true);
      break;
    case Events::COMPONENT_UPDATE_ERROR:
      RecordComponentInstallationResult(
          /*install=*/true, /*successful=*/false);
      break;

    case Events::COMPONENT_UPDATE_DOWNLOADING:
      update_client::CrxUpdateItem item;
      if (g_browser_process->component_updater()->GetComponentDetails(omaha_id,
                                                                      &item)) {
        SetDownloadProgress(static_cast<double>(item.downloaded_bytes) /
                            item.total_bytes);
      }
  }
}

}  // namespace screen_ai
