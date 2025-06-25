// Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "chrome/browser/wootz_scraping/wootz_scraping_service_factory.h"

#include <memory>

#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/wootz_scraping/browser/wootz_scraping_service.h"
#include "components/wootz_scraping/common/wootz_scraping.mojom.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

#if BUILDFLAG(IS_ANDROID)
#include "chrome/browser/wootz_scraping/wootz_scraping_service_delegate_impl_android.h"
#endif

// static
wootz_scraping::WootzScrapingService*
WootzScrapingServiceFactory::GetServiceForContext(
    content::BrowserContext* context) {
  return static_cast<wootz_scraping::WootzScrapingService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
WootzScrapingServiceFactory* WootzScrapingServiceFactory::GetInstance() {
  static base::NoDestructor<WootzScrapingServiceFactory> instance;
  return instance.get();
}

// static
void WootzScrapingServiceFactory::BindForContext(
    content::BrowserContext* context,
    mojo::PendingReceiver<wootz_scraping::mojom::WootzScrapingService>
        receiver) {
  auto* scraping_service = GetServiceForContext(context);
  if (scraping_service) {
    scraping_service->Bind(std::move(receiver));
  }
}

WootzScrapingServiceFactory::WootzScrapingServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "WootzScrapingService",
          BrowserContextDependencyManager::GetInstance()) {}

WootzScrapingServiceFactory::~WootzScrapingServiceFactory() = default;

std::unique_ptr<KeyedService>
WootzScrapingServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  
  auto* profile = Profile::FromBrowserContext(context);
  DCHECK(profile);

  // Get the URL loader factory
  auto url_loader_factory = 
      context->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess();

  // Create the delegate
  std::unique_ptr<WootzScrapingServiceDelegate> delegate;
#if BUILDFLAG(IS_ANDROID)
  delegate = std::make_unique<WootzScrapingServiceDelegateImpl>(context);
#else
  // For other platforms, we'd need platform-specific implementations
  // For now, return nullptr to disable the service on non-Android platforms
  return nullptr;
#endif

  // Create the service with proper constructor
  return std::make_unique<wootz_scraping::WootzScrapingService>(
      url_loader_factory,
      std::move(delegate),
      profile->GetPrefs(),
      g_browser_process->local_state());
}

bool WootzScrapingServiceFactory::ServiceIsNULLWhileTesting() const {
  return true;
}
