/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_MELD_INTEGRATION_SERVICE_FACTORY_H_
#define CHROME_BROWSER_WOOTZ_WALLET_MELD_INTEGRATION_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace wootz_wallet {

class MeldIntegrationService;

class MeldIntegrationServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static mojo::PendingRemote<mojom::MeldIntegrationService> GetForContext(
      content::BrowserContext* context);
  static MeldIntegrationService* GetServiceForContext(
      content::BrowserContext* context);
  static MeldIntegrationServiceFactory* GetInstance();
  static void BindForContext(
      content::BrowserContext* context,
      mojo::PendingReceiver<mojom::MeldIntegrationService> receiver);

 private:
  friend base::NoDestructor<MeldIntegrationServiceFactory>;

  MeldIntegrationServiceFactory();
  MeldIntegrationServiceFactory(const MeldIntegrationServiceFactory&) = delete;
  MeldIntegrationServiceFactory& operator=(
      const MeldIntegrationServiceFactory&) = delete;

  ~MeldIntegrationServiceFactory() override;

  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_MELD_INTEGRATION_SERVICE_FACTORY_H_
