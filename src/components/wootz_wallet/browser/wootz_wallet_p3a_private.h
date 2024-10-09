/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_P3A_PRIVATE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_P3A_PRIVATE_H_

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

namespace wootz_wallet {

class WootzWalletP3APrivate : public mojom::WootzWalletP3A {
 public:
  WootzWalletP3APrivate() = default;

  void ReportJSProvider(mojom::JSProviderType provider_type,
                        mojom::CoinType coin_type,
                        bool allow_provider_overwrite) override;
  void ReportOnboardingAction(
      mojom::OnboardingAction onboarding_action) override;
  void RecordActiveWalletCount(int count, mojom::CoinType coin_type) override;
  void RecordNFTGalleryView(int nft_count) override;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_P3A_PRIVATE_H_
