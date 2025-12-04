// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/activity_tracking_service_factory.h"

#include "base/no_destructor.h"
#include "chrome/browser/activity_tracking/activity_tracking_service.h"
#include "chrome/browser/profiles/profile.h"

namespace activity_tracking {

// static
ActivityTrackingService* ActivityTrackingServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<ActivityTrackingService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
ActivityTrackingServiceFactory* ActivityTrackingServiceFactory::GetInstance() {
  static base::NoDestructor<ActivityTrackingServiceFactory> instance;
  return instance.get();
}

ActivityTrackingServiceFactory::ActivityTrackingServiceFactory()
    : ProfileKeyedServiceFactory(
          "ActivityTrackingService",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOriginalOnly)
              .WithGuest(ProfileSelection::kOriginalOnly)
              .Build()) {}

ActivityTrackingServiceFactory::~ActivityTrackingServiceFactory() = default;

std::unique_ptr<KeyedService>
ActivityTrackingServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  Profile* profile = Profile::FromBrowserContext(context);
  return std::make_unique<ActivityTrackingService>(profile);
}

bool ActivityTrackingServiceFactory::ServiceIsCreatedWithBrowserContext()
    const {
  // Create the service immediately when the profile is created
  return true;
}

}  // namespace activity_tracking

