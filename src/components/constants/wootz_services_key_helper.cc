/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/constants/wootz_services_key_helper.h"

#include "components/constants/network_constants.h"
#include "extensions/common/url_pattern.h"
#include "url/gurl.h"

namespace wootz {

bool ShouldAddWootzServicesKeyHeader(const GURL& url) {
  static URLPattern wootz_proxy_pattern(URLPattern::SCHEME_HTTPS,
                                        kWootzProxyPattern);
  static URLPattern wootzsoftware_proxy_pattern(URLPattern::SCHEME_HTTPS,
                                                kWootzSoftwareProxyPattern);
  return wootz_proxy_pattern.MatchesURL(url) ||
         wootzsoftware_proxy_pattern.MatchesURL(url);
}

}  // namespace wootz
