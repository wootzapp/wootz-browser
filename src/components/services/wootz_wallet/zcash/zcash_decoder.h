/* Copyright (c) 2024 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_SERVICES_WOOTZ_WALLET_ZCASH_ZCASH_DECODER_H_
#define COMPONENTS_SERVICES_WOOTZ_WALLET_ZCASH_ZCASH_DECODER_H_

#include "components/services/wootz_wallet/public/mojom/zcash_decoder.mojom.h"

#include <string>
#include <vector>

#include "mojo/public/cpp/bindings/receiver_set.h"

namespace wootz_wallet {

// Parses Zcash protobuf objects and translates them to mojo.
class ZCashDecoder : public zcash::mojom::ZCashDecoder {
 public:
  ZCashDecoder();
  ~ZCashDecoder() override;

  ZCashDecoder(const ZCashDecoder&) = delete;
  ZCashDecoder& operator=(const ZCashDecoder&) = delete;

  void ParseRawTransaction(const std::string& data,
                           ParseRawTransactionCallback callback) override;
  void ParseBlockID(const std::string& data,
                    ParseBlockIDCallback callback) override;
  void ParseGetAddressUtxos(const std::string& data,
                            ParseGetAddressUtxosCallback callback) override;
  void ParseSendResponse(const std::string& data,
                         ParseSendResponseCallback) override;
  void ParseTreeState(const std::string& data,
                      ParseTreeStateCallback callback) override;
};

}  // namespace wootz_wallet

#endif  // COMPONENTS_SERVICES_WOOTZ_WALLET_ZCASH_ZCASH_DECODER_H_
