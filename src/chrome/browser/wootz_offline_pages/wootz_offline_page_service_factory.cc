// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service_factory.h"

#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

// static
WootzOfflinePageService* WootzOfflinePageServiceFactory::GetForProfile(
    Profile* profile) {
  return static_cast<WootzOfflinePageService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
WootzOfflinePageServiceFactory* WootzOfflinePageServiceFactory::GetInstance() {
  static base::NoDestructor<WootzOfflinePageServiceFactory> instance;
  return instance.get();
}

WootzOfflinePageServiceFactory::WootzOfflinePageServiceFactory()
    : ProfileKeyedServiceFactory(
          "WootzOfflinePageService",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOwnInstance)
              .Build()) {}

WootzOfflinePageServiceFactory::~WootzOfflinePageServiceFactory() = default;

KeyedService* WootzOfflinePageServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new WootzOfflinePageService();
}
