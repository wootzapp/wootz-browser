// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/login/oobe_apps_service/oobe_apps_discovery_service_factory.h"

#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/ash/login/oobe_apps_service/oobe_apps_discovery_service.h"
#include "chrome/browser/profiles/profile.h"

namespace {
static constexpr const char* kOobeAppsDiscoveryService =
    "OobeAppsDiscoveryService";
}  // namespace

namespace ash {

// static
OobeAppsDiscoveryService* OobeAppsDiscoveryServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<OobeAppsDiscoveryService*>(
      OobeAppsDiscoveryServiceFactory::GetInstance()
          ->GetServiceForBrowserContext(profile, /*create=*/true));
}

// static
OobeAppsDiscoveryServiceFactory*
OobeAppsDiscoveryServiceFactory::GetInstance() {
  static base::NoDestructor<OobeAppsDiscoveryServiceFactory> instance;
  return instance.get();
}

OobeAppsDiscoveryServiceFactory::OobeAppsDiscoveryServiceFactory()
    : ProfileKeyedServiceFactory(
          kOobeAppsDiscoveryService,
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOwnInstance)
              .WithGuest(ProfileSelection::kNone)
              .WithSystem(ProfileSelection::kNone)
              .Build()) {
  DependsOn(apps::AppServiceProxyFactory::GetInstance());
}

OobeAppsDiscoveryServiceFactory::~OobeAppsDiscoveryServiceFactory() = default;

std::unique_ptr<KeyedService>
OobeAppsDiscoveryServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<OobeAppsDiscoveryService>(
      Profile::FromBrowserContext(context));
}

}  // namespace ash
