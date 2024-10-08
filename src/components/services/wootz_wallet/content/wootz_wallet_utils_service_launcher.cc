/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/services/wootz_wallet/content/wootz_wallet_utils_service_launcher.h"

#include <utility>

#include "chrome/grit/wootz_generated_resources.h"
#include "content/public/browser/service_process_host.h"

namespace wootz_wallet {

void LaunchWootzWalletUtilsService(
    mojo::PendingReceiver<mojom::WootzWalletUtilsService> receiver) {
  content::ServiceProcessHost::Launch(
      std::move(receiver),
      content::ServiceProcessHost::Options()
          .WithDisplayName(IDS_UTILITY_PROCESS_WALLET_UTILS_NAME)
          .Pass());
}

}  // namespace wootz_wallet
