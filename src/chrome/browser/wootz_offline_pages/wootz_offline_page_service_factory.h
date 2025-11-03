// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SERVICE_FACTORY_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"

class WootzOfflinePageService;
class Profile;

class WootzOfflinePageServiceFactory : public ProfileKeyedServiceFactory {
 public:
  static WootzOfflinePageService* GetForProfile(Profile* profile);
  static WootzOfflinePageServiceFactory* GetInstance();

 private:
  friend class base::NoDestructor<WootzOfflinePageServiceFactory>;

  WootzOfflinePageServiceFactory();
  ~WootzOfflinePageServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
};

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SERVICE_FACTORY_H_
