/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"

#include <memory>
#include <utility>

#include "base/no_destructor.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_context_utils.h"
#include "components/wootz_wallet/browser/bitcoin/bitcoin_wallet_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_service_delegate.h"
#include "components/wootz_wallet/browser/json_rpc_service.h"
#include "components/wootz_wallet/browser/tx_service.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/storage_partition.h"

namespace wootz_wallet {

// static
WootzWalletServiceFactory* WootzWalletServiceFactory::GetInstance() {
  static base::NoDestructor<WootzWalletServiceFactory> instance;
  return instance.get();
}

// static
WootzWalletService* WootzWalletServiceFactory::GetServiceForContext(
    content::BrowserContext* context) {
  if (!IsAllowedForContext(context)) {
    return nullptr;
  }
  return static_cast<WootzWalletService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

WootzWalletServiceFactory::WootzWalletServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "WootzWalletService",
          BrowserContextDependencyManager::GetInstance()) {}

WootzWalletServiceFactory::~WootzWalletServiceFactory() = default;

KeyedService* WootzWalletServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  auto* default_storage_partition = context->GetDefaultStoragePartition();
  auto shared_url_loader_factory =
      default_storage_partition->GetURLLoaderFactoryForBrowserProcess();

  return new WootzWalletService(
      shared_url_loader_factory, WootzWalletServiceDelegate::Create(context),
      user_prefs::UserPrefs::Get(context), g_browser_process->local_state());
}

content::BrowserContext* WootzWalletServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

bool WootzWalletServiceFactory::ServiceIsNULLWhileTesting() const {
  // KeyringService and WootzWalletP3A expect a valid local state. Without it
  // we'd need to put a lot of unnecessary ifs/checks into those services.
  // Instead, we just won't create the wallet service if the local state isn't
  // available.
  return (g_browser_process->local_state() == nullptr);
}

}  // namespace wootz_wallet
