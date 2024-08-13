/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/notifications/wallet_notification_service_factory.h"

#include <memory>

#include "base/no_destructor.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_context_utils.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "chrome/browser/wootz_wallet/notifications/wallet_notification_service.h"
#include "chrome/browser/notifications/notification_display_service_factory.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

namespace wootz_wallet {

// static
WalletNotificationServiceFactory*
WalletNotificationServiceFactory::GetInstance() {
  static base::NoDestructor<WalletNotificationServiceFactory> instance;
  return instance.get();
}

WalletNotificationServiceFactory::WalletNotificationServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "WalletNotificationService",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(NotificationDisplayServiceFactory::GetInstance());
  DependsOn(wootz_wallet::WootzWalletServiceFactory::GetInstance());
}

WalletNotificationServiceFactory::~WalletNotificationServiceFactory() = default;

KeyedService* WalletNotificationServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new WalletNotificationService(
      WootzWalletServiceFactory::GetServiceForContext(context), context);
}

// static
WalletNotificationService*
WalletNotificationServiceFactory::GetServiceForContext(
    content::BrowserContext* context) {
  if (!IsAllowedForContext(context)) {
    return nullptr;
  }
  return static_cast<WalletNotificationService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

}  // namespace wootz_wallet
