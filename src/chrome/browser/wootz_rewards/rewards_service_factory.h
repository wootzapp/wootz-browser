/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_REWARDS_REWARDS_SERVICE_FACTORY_H_
#define CHROME_BROWSER_WOOTZ_REWARDS_REWARDS_SERVICE_FACTORY_H_

#include "chrome/components/wootz_rewards/browser/rewards_service.h"
#include "chrome/browser/profiles/profile_manager_observer.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

namespace wootz_rewards {
class RewardsService;

// Singleton that owns all RewardsService and associates them with Profiles.
class RewardsServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  RewardsServiceFactory(const RewardsServiceFactory&) = delete;
  RewardsServiceFactory& operator=(const RewardsServiceFactory&) = delete;

  static wootz_rewards::RewardsService* GetForProfile(Profile* profile);

  static RewardsServiceFactory* GetInstance();

  static void SetServiceForTesting(RewardsService* service);

 private:
  friend base::NoDestructor<RewardsServiceFactory>;

  RewardsServiceFactory();
  ~RewardsServiceFactory() override = default;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  bool ServiceIsNULLWhileTesting() const override;
};

}  // namespace wootz_rewards

#endif  // CHROME_BROWSER_WOOTZ_REWARDS_REWARDS_SERVICE_FACTORY_H_