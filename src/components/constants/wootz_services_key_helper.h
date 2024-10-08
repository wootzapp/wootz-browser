/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_CONSTANTS_WOOTZ_SERVICES_KEY_HELPER_H_
#define COMPONENTS_CONSTANTS_WOOTZ_SERVICES_KEY_HELPER_H_

class GURL;

namespace wootz {

bool ShouldAddWootzServicesKeyHeader(const GURL& url);

}  // namespace wootz

#endif  // COMPONENTS_CONSTANTS_WOOTZ_SERVICES_KEY_HELPER_H_
