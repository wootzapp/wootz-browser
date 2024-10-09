/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_DECENTRALIZED_DNS_CORE_CONSTANTS_H_
#define COMPONENTS_DECENTRALIZED_DNS_CORE_CONSTANTS_H_

namespace decentralized_dns {

// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.chrome.browser.decentralized_dns
enum class ResolveMethodTypes {
  ASK,
  DISABLED,
  DEPRECATED_DNS_OVER_HTTPS,
  ENABLED,
  MAX_VALUE = ENABLED,
};

// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.chrome.browser.decentralized_dns
enum class EnsOffchainResolveMethod { kAsk, kDisabled, kEnabled };

}  // namespace decentralized_dns

#endif  // COMPONENTS_DECENTRALIZED_DNS_CORE_CONSTANTS_H_
