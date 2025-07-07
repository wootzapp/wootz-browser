/* Copyright (c) 2024 The Wootzapp Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/common/mem_utils.h"

#include <algorithm>

namespace wootz_wallet {

void SecureZeroData(base::span<uint8_t> bytes) {
  // 'volatile' is required. Otherwise optimizers can remove this function
  // if cleaning local variables, which are not used after that.
  base::span<volatile uint8_t> data = bytes;
  std::fill(data.begin(), data.end(), 0u);
}

}  // namespace wootz_wallet