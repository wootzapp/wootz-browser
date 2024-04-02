// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/ash/glanceables/glanceables_keyed_service.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "ash/constants/ash_features.h"
#include "ash/constants/ash_pref_names.h"
#include "ash/constants/ash_switches.h"
#include "ash/glanceables/glanceables_controller.h"
#include "ash/shell.h"
#include "base/check.h"
#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/metrics/histogram_functions.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/time/default_clock.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/browser/ui/ash/api/tasks/tasks_client_impl.h"
#include "chrome/browser/ui/ash/glanceables/glanceables_classroom_client_impl.h"
#include "chromeos/ash/components/browser_context_helper/browser_context_helper.h"
#include "components/account_id/account_id.h"
#include "components/prefs/pref_service.h"
#include "components/signin/public/base/consent_level.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "components/user_manager/user.h"
#include "google_apis/common/auth_service.h"
#include "google_apis/common/request_sender.h"
#include "net/traffic_annotation/network_traffic_annotation.h"

namespace ash {

namespace {

constexpr net::NetworkTrafficAnnotationTag kTasksTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("glanceables_tasks_integration",
                                        R"(
        semantics {
          sender: "Glanceables keyed service"
          description: "Provide ChromeOS users quick access to their "
                       "task lists without opening the app or website"
          trigger: "User presses the calendar pill in shelf, which triggers "
                   "opening the calendar, classroom (if available) and tasks "
                   "widgets. This specific client implementation "
                   "is responsible for fetching user's tasks data from "
                   "Google Tasks API."
          internal {
            contacts {
              email: "chromeos-launcher@google.com"
            }
          }
          user_data {
            type: ACCESS_TOKEN
          }
          data: "The request is authenticated with an OAuth2 access token "
                "identifying the Google account"
          destination: GOOGLE_OWNED_SERVICE
          last_reviewed: "2023-08-21"
        }
        policy {
          cookies_allowed: NO
          setting: "This feature cannot be disabled in settings"
          chrome_policy {
            GlanceablesEnabled {
              GlanceablesEnabled: false
            }
          }
        }
    )");

}  // namespace

GlanceablesKeyedService::GlanceablesKeyedService(Profile* profile)
    : profile_(profile),
      identity_manager_(IdentityManagerFactory::GetForProfile(profile_)),
      account_id_(BrowserContextHelper::Get()
                      ->GetUserByBrowserContext(profile)
                      ->GetAccountId()) {
  PrefService* const pref_service = profile->GetPrefs();

  CHECK(pref_service);

  // Listen to pref changes.
  pref_change_registrar_.Init(pref_service);
  pref_change_registrar_.Add(
      prefs::kGlanceablesEnabled,
      base::BindRepeating(&GlanceablesKeyedService::UpdateRegistration,
                          base::Unretained(this)));

  UpdateRegistration();
}

GlanceablesKeyedService::~GlanceablesKeyedService() = default;

void GlanceablesKeyedService::Shutdown() {
  ClearClients();
}

GlanceablesKeyedService::GlanceablesStatus
GlanceablesKeyedService::AreGlanceablesEnabled() const {
  if (features::AreAnyGlanceablesTimeManagementViewsEnabled()) {
    // TODO(b/319251265): Finalize policies to control the feature.
    return GlanceablesStatus::kEnabledForFullLaunch;
  }

  PrefService* const prefs = profile_->GetPrefs();
  if (features::AreGlanceablesV2Enabled()) {
    if (prefs->GetBoolean(prefs::kGlanceablesEnabled)) {
      return GlanceablesStatus::kEnabledByV2Flag;
    }
    if (base::CommandLine::ForCurrentProcess()->HasSwitch(
            ash::switches::kAshBypassGlanceablesPref)) {
      return GlanceablesStatus::kEnabledByPrefBypass;
    }
    return GlanceablesStatus::kDisabled;
  }

  if (features::AreGlanceablesV2EnabledForTrustedTesters()) {
    if (prefs->IsManagedPreference(prefs::kGlanceablesEnabled) &&
        prefs->GetBoolean(prefs::kGlanceablesEnabled)) {
      return GlanceablesStatus::kEnabledForTrustedTesters;
    }
  }

  return GlanceablesStatus::kDisabled;
}

std::unique_ptr<google_apis::RequestSender>
GlanceablesKeyedService::CreateRequestSenderForClient(
    const std::vector<std::string>& scopes,
    const net::NetworkTrafficAnnotationTag& traffic_annotation_tag) const {
  const auto url_loader_factory = profile_->GetURLLoaderFactory();
  auto auth_service = std::make_unique<google_apis::AuthService>(
      identity_manager_,
      identity_manager_->GetPrimaryAccountId(signin::ConsentLevel::kSignin),
      url_loader_factory, scopes);
  return std::make_unique<google_apis::RequestSender>(
      std::move(auth_service), url_loader_factory,
      base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(),
           /* `USER_VISIBLE` is because the requested/returned data is visible
              to the user on System UI surfaces. */
           base::TaskPriority::USER_VISIBLE,
           base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN})
          .get(),
      /*custom_user_agent=*/std::string(), traffic_annotation_tag);
}

void GlanceablesKeyedService::RegisterClients() {
  const auto create_request_sender_callback = base::BindRepeating(
      &GlanceablesKeyedService::CreateRequestSenderForClient,
      base::Unretained(this));
  classroom_client_ = std::make_unique<GlanceablesClassroomClientImpl>(
      base::DefaultClock::GetInstance(), create_request_sender_callback);
  tasks_client_ = std::make_unique<api::TasksClientImpl>(
      create_request_sender_callback, kTasksTrafficAnnotation);

  Shell::Get()->glanceables_controller()->UpdateClientsRegistration(
      account_id_, GlanceablesController::ClientsRegistration{
                       .classroom_client = classroom_client_.get(),
                       .tasks_client = tasks_client_.get()});
}

void GlanceablesKeyedService::ClearClients() {
  classroom_client_.reset();
  tasks_client_.reset();
  if (Shell::HasInstance()) {
    Shell::Get()->glanceables_controller()->UpdateClientsRegistration(
        account_id_, GlanceablesController::ClientsRegistration{
                         .classroom_client = nullptr, .tasks_client = nullptr});
  }
}

void GlanceablesKeyedService::UpdateRegistration() {
  if (!Shell::HasInstance()) {
    return;
  }

  DCHECK(Shell::Get()->glanceables_controller());

  PrefService* prefs = profile_->GetPrefs();

  CHECK(prefs);

  GlanceablesStatus status = AreGlanceablesEnabled();
  base::UmaHistogramEnumeration("Ash.Glanceables.TimeManagement.FeatureStatus",
                                status);

  if (status == GlanceablesStatus::kDisabled) {
    Shell::Get()->glanceables_controller()->ClearUserStatePrefs(prefs);
    ClearClients();
    return;
  }

  if (!classroom_client_ || !tasks_client_) {
    RegisterClients();
  }
}

}  // namespace ash
