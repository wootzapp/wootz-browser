/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_IPFS_SERVICE_FACTORY_H_
#define CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_IPFS_SERVICE_FACTORY_H_

#include "components/wootz_wallet/browser/wootz_wallet_ipfs_service.h"

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/browser_context.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

namespace wootz_wallet {

class WootzWalletIpfsServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  WootzWalletIpfsServiceFactory(const WootzWalletIpfsServiceFactory&) = delete;
  WootzWalletIpfsServiceFactory& operator=(
      const WootzWalletIpfsServiceFactory&) = delete;

  static mojo::PendingRemote<mojom::IpfsService> GetForContext(
      content::BrowserContext* context);
  static WootzWalletIpfsService* GetServiceForContext(
      content::BrowserContext* context);
  static WootzWalletIpfsServiceFactory* GetInstance();
  static void BindForContext(
      content::BrowserContext* context,
      mojo::PendingReceiver<mojom::IpfsService> receiver);

 private:
  friend base::NoDestructor<WootzWalletIpfsServiceFactory>;

  WootzWalletIpfsServiceFactory();
  ~WootzWalletIpfsServiceFactory() override;

  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
};

}  // namespace wootz_wallet

#endif  // CHROME_BROWSER_WOOTZ_WALLET_WOOTZ_WALLET_IPFS_SERVICE_FACTORY_H_
