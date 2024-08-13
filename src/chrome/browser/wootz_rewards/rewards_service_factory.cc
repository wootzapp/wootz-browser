/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_rewards/rewards_service_factory.h"

#include <memory>
#include <utility>

#include "base/no_destructor.h"
#include "chrome/browser/wootz_rewards/rewards_util.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "chrome/components/wootz_rewards/browser/rewards_notification_service_observer.h"
#include "chrome/components/wootz_rewards/browser/rewards_service.h"
#include "chrome/components/wootz_rewards/browser/rewards_service_impl.h"
#include "chrome/components/wootz_rewards/browser/rewards_service_observer.h"
// #include "wootz/components/greaselion/browser/buildflags/buildflags.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "extensions/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
// #include "wootz/browser/wootz_rewards/extension_rewards_notification_service_observer.h"
// #include "wootz/browser/wootz_rewards/extension_rewards_service_observer.h"
#include "extensions/browser/event_router_factory.h"
#endif

#if BUILDFLAG(ENABLE_GREASELION)
#include "chrome/browser/greaselion/greaselion_service_factory.h"
#include "chrome/components/greaselion/browser/greaselion_service.h"
#endif

namespace wootz_rewards {

RewardsService* testing_service_ = nullptr;

// static
RewardsService* RewardsServiceFactory::GetForProfile(Profile* profile) {
  if (testing_service_) {
    return testing_service_;
  }

  if (!IsSupportedForProfile(profile)) {
    return nullptr;
  }

  return static_cast<RewardsService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
RewardsServiceFactory* RewardsServiceFactory::GetInstance() {
  static base::NoDestructor<RewardsServiceFactory> instance;
  return instance.get();
}

RewardsServiceFactory::RewardsServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "RewardsService",
          BrowserContextDependencyManager::GetInstance()) {
#if BUILDFLAG(ENABLE_EXTENSIONS)
  DependsOn(extensions::EventRouterFactory::GetInstance());
#endif
#if BUILDFLAG(ENABLE_GREASELION)
  DependsOn(greaselion::GreaselionServiceFactory::GetInstance());
#endif
  DependsOn(wootz_wallet::WootzWalletServiceFactory::GetInstance());
}

KeyedService* RewardsServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  std::unique_ptr<RewardsServiceObserver> extension_observer = nullptr;
  std::unique_ptr<RewardsNotificationServiceObserver> notification_observer =
      nullptr;
#if BUILDFLAG(ENABLE_EXTENSIONS)
  extension_observer = std::make_unique<ExtensionRewardsServiceObserver>(
      Profile::FromBrowserContext(context));
  notification_observer =
      std::make_unique<ExtensionRewardsNotificationServiceObserver>(
          Profile::FromBrowserContext(context));
#endif
  auto* wootz_wallet_service =
      wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(context);
#if BUILDFLAG(ENABLE_GREASELION)
  greaselion::GreaselionService* greaselion_service =
      greaselion::GreaselionServiceFactory::GetForBrowserContext(context);
  std::unique_ptr<RewardsServiceImpl> rewards_service(
      new RewardsServiceImpl(Profile::FromBrowserContext(context),
                             greaselion_service, wootz_wallet_service));
#else
  std::unique_ptr<RewardsServiceImpl> rewards_service(new RewardsServiceImpl(
      Profile::FromBrowserContext(context), wootz_wallet_service));
#endif
  rewards_service->Init(std::move(extension_observer),
                        std::move(notification_observer));
  return rewards_service.release();
}

// static
void RewardsServiceFactory::SetServiceForTesting(RewardsService* service) {
  testing_service_ = service;
}

bool RewardsServiceFactory::ServiceIsNULLWhileTesting() const {
  return false;
}

}  // namespace wootz_rewards