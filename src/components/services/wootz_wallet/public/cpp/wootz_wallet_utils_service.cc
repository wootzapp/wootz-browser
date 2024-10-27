/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/services/wootz_wallet/public/cpp/wootz_wallet_utils_service.h"

#include <utility>

#include "base/no_destructor.h"
#include "build/build_config.h"

#if BUILDFLAG(IS_IOS)
#include "components/services/wootz_wallet/public/cpp/wootz_wallet_utils_service_in_process_launcher.h"
#else
#include "components/services/wootz_wallet/content/wootz_wallet_utils_service_launcher.h"
#endif

namespace wootz_wallet {

WootzWalletUtilsService::WootzWalletUtilsService() = default;
WootzWalletUtilsService::~WootzWalletUtilsService() = default;

// static
WootzWalletUtilsService* WootzWalletUtilsService::GetInstance() {
  static base::NoDestructor<WootzWalletUtilsService> service;
  return service.get();
}

void WootzWalletUtilsService::CreateZCashDecoder(
    mojo::PendingAssociatedReceiver<zcash::mojom::ZCashDecoder> receiver) {
  MaybeLaunchService();
  wootz_wallet_utils_service_->CreateZCashDecoderService(std::move(receiver));
}

void WootzWalletUtilsService::MaybeLaunchService() {
#if BUILDFLAG(IS_IOS)
  LaunchInProcessWootzWalletUtilsService(
      wootz_wallet_utils_service_.BindNewPipeAndPassReceiver());
  wootz_wallet_utils_service_.reset_on_disconnect();
#else
  LaunchWootzWalletUtilsService(
      wootz_wallet_utils_service_.BindNewPipeAndPassReceiver());
  wootz_wallet_utils_service_.reset_on_disconnect();
  // 10 minutes is a default wallet lock time
  wootz_wallet_utils_service_.reset_on_idle_timeout(base::Minutes(10));
#endif
}

}  // namespace wootz_wallet
