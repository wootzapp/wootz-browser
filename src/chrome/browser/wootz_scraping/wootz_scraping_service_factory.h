// Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef CHROME_BROWSER_WOOTZ_SCRAPING_WOOTZ_SCRAPING_SERVICE_FACTORY_H_
#define CHROME_BROWSER_WOOTZ_SCRAPING_WOOTZ_SCRAPING_SERVICE_FACTORY_H_

#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/wootz_scraping/common/wootz_scraping.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

class Profile;

namespace base {
template <typename T>
class NoDestructor;
}

namespace wootz_scraping {
class WootzScrapingService;
}

class WootzScrapingServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static wootz_scraping::WootzScrapingService* GetServiceForContext(
      content::BrowserContext* context);
  static WootzScrapingServiceFactory* GetInstance();
  static void BindForContext(
      content::BrowserContext* context,
      mojo::PendingReceiver<wootz_scraping::mojom::WootzScrapingService>
          receiver);

  WootzScrapingServiceFactory(const WootzScrapingServiceFactory&) = delete;
  WootzScrapingServiceFactory& operator=(const WootzScrapingServiceFactory&) =
      delete;

 private:
  friend base::NoDestructor<WootzScrapingServiceFactory>;

  WootzScrapingServiceFactory();
  ~WootzScrapingServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  bool ServiceIsNULLWhileTesting() const override;
};

#endif  // CHROME_BROWSER_WOOTZ_SCRAPING_WOOTZ_SCRAPING_SERVICE_FACTORY_H_
