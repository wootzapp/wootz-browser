/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/browser/wootz_wallet_p3a_private.h"

namespace wootz_wallet {

void WootzWalletP3APrivate::ReportJSProvider(
    mojom::JSProviderType provider_type,
    mojom::CoinType coin_type,
    bool allow_provider_overwrite) {}

void WootzWalletP3APrivate::ReportOnboardingAction(
    mojom::OnboardingAction onboarding_action) {}

void WootzWalletP3APrivate::RecordActiveWalletCount(int count,
                                                    mojom::CoinType coin_type) {
}

void WootzWalletP3APrivate::RecordNFTGalleryView(int nft_count) {}

}  // namespace wootz_wallet
