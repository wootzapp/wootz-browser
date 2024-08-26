/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_WOOTZ_WALLET_BROWSER_UNSTOPPABLE_DOMAINS_DNS_RESOLVE_H_
#define COMPONENTS_WOOTZ_WALLET_BROWSER_UNSTOPPABLE_DOMAINS_DNS_RESOLVE_H_

#include <string>
#include <vector>

#include "url/gurl.h"

namespace wootz_wallet::unstoppable_domains {

const std::vector<std::string>& GetRecordKeys();
GURL ResolveUrl(const std::vector<std::string>& response);

}  // namespace wootz_wallet::unstoppable_domains

#endif  // COMPONENTS_WOOTZ_WALLET_BROWSER_UNSTOPPABLE_DOMAINS_DNS_RESOLVE_H_
