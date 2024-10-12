// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "chrome/browser/signin/signin_metrics_service_factory.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "components/signin/core/browser/signin_metrics_service.h"

SigninMetricsServiceFactory::SigninMetricsServiceFactory()
    : ProfileKeyedServiceFactory("SigninMetricsHelper",
                                 ProfileSelections::Builder()
                                     .WithAshInternals(ProfileSelection::kNone)
                                     .Build()) {
  DependsOn(IdentityManagerFactory::GetInstance());
}

SigninMetricsServiceFactory::~SigninMetricsServiceFactory() = default;

// static
SigninMetricsService* SigninMetricsServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<SigninMetricsService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
SigninMetricsServiceFactory* SigninMetricsServiceFactory::GetInstance() {
  static base::NoDestructor<SigninMetricsServiceFactory> instance;
  return instance.get();
}

KeyedService* SigninMetricsServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  Profile* profile = Profile::FromBrowserContext(context);
  return new SigninMetricsService(
      *IdentityManagerFactory::GetForProfile(profile), *profile->GetPrefs());
}

bool SigninMetricsServiceFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}
