// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import { WootzWallet } from '../../constants/types'
import { getGetMojoEnumValues } from '../../utils/enum-utils'

export const BLOWFISH_URL_WARNING_KINDS = [
  WootzWallet.BlowfishWarningKind.kBlocklistedDomainCrossOrigin,
  WootzWallet.BlowfishWarningKind.kCopyCatDomain,
  WootzWallet.BlowfishWarningKind.kCopyCatImageUnresponsiveDomain,
  WootzWallet.BlowfishWarningKind.kMultiCopyCatDomain,
  WootzWallet.BlowfishWarningKind.kNewDomain,
  WootzWallet.BlowfishWarningKind.kSemiTrustedBlocklistDomain,
  WootzWallet.BlowfishWarningKind.kTrustedBlocklistDomain
] as const

export const BLOWFISH_WARNING_KINDS = getGetMojoEnumValues(
  WootzWallet.BlowfishWarningKind
)
