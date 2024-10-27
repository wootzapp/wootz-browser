/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/wootz_wallet_ipfs_service_factory.h"

#include <utility>

#include "base/no_destructor.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_context_utils.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/storage_partition.h"

namespace wootz_wallet {

// static
WootzWalletIpfsServiceFactory* WootzWalletIpfsServiceFactory::GetInstance() {
  static base::NoDestructor<WootzWalletIpfsServiceFactory> instance;
  return instance.get();
}

// static
mojo::PendingRemote<mojom::IpfsService>
WootzWalletIpfsServiceFactory::GetForContext(content::BrowserContext* context) {
  if (!IsAllowedForContext(context)) {
    return mojo::PendingRemote<mojom::IpfsService>();
  }

  return static_cast<WootzWalletIpfsService*>(
             GetInstance()->GetServiceForBrowserContext(context, true))
      ->MakeRemote();
}

// static
WootzWalletIpfsService* WootzWalletIpfsServiceFactory::GetServiceForContext(
    content::BrowserContext* context) {
  if (!IsAllowedForContext(context)) {
    return nullptr;
  }
  return static_cast<WootzWalletIpfsService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
void WootzWalletIpfsServiceFactory::BindForContext(
    content::BrowserContext* context,
    mojo::PendingReceiver<mojom::IpfsService> receiver) {
  auto* ipfs_service =
      WootzWalletIpfsServiceFactory::GetServiceForContext(context);
  if (ipfs_service) {
    ipfs_service->Bind(std::move(receiver));
  }
}

WootzWalletIpfsServiceFactory::WootzWalletIpfsServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "WootzWalletIpfsService",
          BrowserContextDependencyManager::GetInstance()) {}

WootzWalletIpfsServiceFactory::~WootzWalletIpfsServiceFactory() = default;

KeyedService* WootzWalletIpfsServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new WootzWalletIpfsService(user_prefs::UserPrefs::Get(context));
}

content::BrowserContext* WootzWalletIpfsServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return chrome::GetBrowserContextRedirectedInIncognito(context);
}

}  // namespace wootz_wallet
