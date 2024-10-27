/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_IPFS_SERVICE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_IPFS_SERVICE_H_

#include <string>
#include <vector>

#include "components/wootz_wallet/common/wootz_wallet.mojom.h"

#include "base/memory/raw_ptr.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_service.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver_set.h"

namespace wootz_wallet {

class WootzWalletIpfsService : public KeyedService, public mojom::IpfsService {
 public:
  explicit WootzWalletIpfsService(PrefService* pref_service);
  ~WootzWalletIpfsService() override;

  WootzWalletIpfsService(const WootzWalletIpfsService&) = delete;
  WootzWalletIpfsService& operator=(const WootzWalletIpfsService&) = delete;

  mojo::PendingRemote<mojom::IpfsService> MakeRemote();
  void Bind(mojo::PendingReceiver<mojom::IpfsService> receiver);

  void TranslateToGatewayURL(const std::string& url,
                             TranslateToGatewayURLCallback callback) override;

  void ContentHashToCIDv1URL(const std::vector<uint8_t>& content_hash,
                             ContentHashToCIDv1URLCallback callback) override;

 private:
  mojo::ReceiverSet<mojom::IpfsService> receivers_;

  raw_ptr<PrefService> pref_service_;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_WOOTZ_WALLET_IPFS_SERVICE_H_
