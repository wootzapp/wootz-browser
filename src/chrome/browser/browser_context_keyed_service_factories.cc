/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/browser_context_keyed_service_factories.h"

#include "base/feature_list.h"

#include "chrome/browser/ephemeral_storage/ephemeral_storage_service_factory.h"
#include "chrome/browser/profiles/wootz_renderer_updater_factory.h"

#include "chrome/browser/wootz_wallet/asset_ratio_service_factory.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_ipfs_service_factory.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "chrome/browser/wootz_wallet/meld_integration_service_factory.h"
#include "chrome/browser/wootz_wallet/simulation_service_factory.h"
#include "chrome/browser/wootz_wallet/swap_service_factory.h"

#include "chrome/browser/permissions/permission_lifetime_manager_factory.h"

#include "components/wootz_wallet/common/common_utils.h"

namespace chrome {

void EnsureBrowserContextKeyedServiceFactoriesBuilt() {
 
  WootzRendererUpdaterFactory::GetInstance();

  wootz_wallet::AssetRatioServiceFactory::GetInstance();
  wootz_wallet::MeldIntegrationServiceFactory::GetInstance();
  wootz_wallet::SwapServiceFactory::GetInstance();
  wootz_wallet::SimulationServiceFactory::GetInstance();

  wootz_wallet::WootzWalletServiceFactory::GetInstance();
  wootz_wallet::WootzWalletIpfsServiceFactory::GetInstance();
  EphemeralStorageServiceFactory::GetInstance();
  PermissionLifetimeManagerFactory::GetInstance();
}

}  // namespace chrome