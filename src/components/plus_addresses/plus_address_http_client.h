// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PLUS_ADDRESSES_PLUS_ADDRESS_HTTP_CLIENT_H_
#define COMPONENTS_PLUS_ADDRESSES_PLUS_ADDRESS_HTTP_CLIENT_H_

#include <string>

#include "components/plus_addresses/plus_address_types.h"

namespace url {
class Origin;
}  // namespace url

namespace plus_addresses {

// An interface for communicating with the remote plus address server.
class PlusAddressHttpClient {
 public:
  virtual ~PlusAddressHttpClient() = default;

  // Initiates a request to get a plus address for use on `origin` and runs
  // `on_completed` when the request is completed.
  virtual void ReservePlusAddress(const url::Origin& origin,
                                  PlusAddressRequestCallback on_completed) = 0;

  // Initiates a request to confirm `plus_address` for use on `origin` and runs
  // `on_completed` when the request is completed.
  virtual void ConfirmPlusAddress(const url::Origin& origin,
                                  const std::string& plus_address,
                                  PlusAddressRequestCallback on_completed) = 0;

  // Initiates a request to get all plus addresses from the remote enterprise-
  // specified server and runs `on_completed` when the request is completed.
  virtual void GetAllPlusAddresses(
      PlusAddressMapRequestCallback on_completed) = 0;

  // Resets state in the client, e.g., by cancelling ongoing network requests.
  virtual void Reset() = 0;
};

}  // namespace plus_addresses

#endif  // COMPONENTS_PLUS_ADDRESSES_PLUS_ADDRESS_HTTP_CLIENT_H_
