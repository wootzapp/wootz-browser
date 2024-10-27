/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/profiles/wootz_renderer_updater_factory.h"

#include "base/no_destructor.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "chrome/browser/profiles/wootz_renderer_updater.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"

WootzRendererUpdaterFactory::WootzRendererUpdaterFactory()
    : ProfileKeyedServiceFactory(
          "WootzRendererUpdater",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOwnInstance)
              .WithGuest(ProfileSelection::kOwnInstance)
              .Build()) {
  DependsOn(wootz_wallet::WootzWalletServiceFactory::GetInstance());
}

WootzRendererUpdaterFactory::~WootzRendererUpdaterFactory() = default;

// static
WootzRendererUpdaterFactory* WootzRendererUpdaterFactory::GetInstance() {
  static base::NoDestructor<WootzRendererUpdaterFactory> instance;
  return instance.get();
}

// static
WootzRendererUpdater* WootzRendererUpdaterFactory::GetForProfile(
    Profile* profile) {
  return static_cast<WootzRendererUpdater*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

std::unique_ptr<KeyedService>
WootzRendererUpdaterFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  auto* wootz_wallet_service =
      wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(context);

  auto* keyring_service =
      wootz_wallet_service ? wootz_wallet_service->keyring_service() : 0;
  return std::make_unique<WootzRendererUpdater>(
      static_cast<Profile*>(context), keyring_service,
      g_browser_process->local_state());
}

bool WootzRendererUpdaterFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}
