// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_TRACKING_SERVICE_FACTORY_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_TRACKING_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"

class Profile;

namespace activity_tracking {

class ActivityTrackingService;

// Factory to create ActivityTrackingService instances per profile.
// This follows the standard Chromium KeyedService pattern.
class ActivityTrackingServiceFactory : public ProfileKeyedServiceFactory {
 public:
  static ActivityTrackingService* GetForProfile(Profile* profile);
  static ActivityTrackingServiceFactory* GetInstance();

  ActivityTrackingServiceFactory(const ActivityTrackingServiceFactory&) =
      delete;
  ActivityTrackingServiceFactory& operator=(
      const ActivityTrackingServiceFactory&) = delete;

 private:
  friend base::NoDestructor<ActivityTrackingServiceFactory>;

  ActivityTrackingServiceFactory();
  ~ActivityTrackingServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_ACTIVITY_TRACKING_SERVICE_FACTORY_H_

