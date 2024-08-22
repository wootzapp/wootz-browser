/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_FACTORY_H_
#define CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_FACTORY_H_

#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/browser_context.h"

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

namespace wootz_wallet {

class WootzWalletService;
class JsonRpcService;

class WootzWalletServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static WootzWalletService* GetServiceForContext(
      content::BrowserContext* context);
  static WootzWalletServiceFactory* GetInstance();

 private:
  friend base::NoDestructor<WootzWalletServiceFactory>;

  WootzWalletServiceFactory();
  ~WootzWalletServiceFactory() override;

  WootzWalletServiceFactory(const WootzWalletServiceFactory&) = delete;
  WootzWalletServiceFactory& operator=(const WootzWalletServiceFactory&) =
      delete;

  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsNULLWhileTesting() const override;
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_SERVICE_FACTORY_H_
