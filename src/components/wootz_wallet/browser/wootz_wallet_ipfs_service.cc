// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "components/wootz_wallet/browser/wootz_wallet_ipfs_service.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "components/ipfs/ipfs_utils.h"

namespace wootz_wallet {

WootzWalletIpfsService::WootzWalletIpfsService(PrefService* pref_service)
    : pref_service_(pref_service) {}

WootzWalletIpfsService::~WootzWalletIpfsService() = default;

mojo::PendingRemote<mojom::IpfsService> WootzWalletIpfsService::MakeRemote() {
  mojo::PendingRemote<mojom::IpfsService> remote;
  receivers_.Add(this, remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

void WootzWalletIpfsService::Bind(
    mojo::PendingReceiver<mojom::IpfsService> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void WootzWalletIpfsService::TranslateToGatewayURL(
    const std::string& url,
    TranslateToGatewayURLCallback callback) {
  GURL new_url;
  if (ipfs::TranslateIPFSURI(GURL(url), &new_url,
                             false)) {
    std::move(callback).Run(new_url.spec());
  } else {
    std::move(callback).Run(std::nullopt);
  }
}

void WootzWalletIpfsService::ContentHashToCIDv1URL(
    const std::vector<uint8_t>& content_hash,
    ContentHashToCIDv1URLCallback callback) {
  std::move(callback).Run(ipfs::ContentHashToCIDv1URL(content_hash).spec());
}

}  // namespace wootz_wallet
