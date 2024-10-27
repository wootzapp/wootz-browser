// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// Types
import { WootzWallet, DAppPermissionDurationOption } from '../constants/types'

export const DAppPermissionDurationOptions: DAppPermissionDurationOption[] = [
  {
    name: 'wootzWalletPermissionUntilClose',
    id: WootzWallet.PermissionLifetimeOption.kPageClosed
  },
  {
    name: 'wootzWalletPermissionOneDay',
    id: WootzWallet.PermissionLifetimeOption.k24Hours
  },
  {
    name: 'wootzWalletPermissionOneWeek',
    id: WootzWallet.PermissionLifetimeOption.k7Days
  },
  {
    name: 'wootzWalletPermissionForever',
    id: WootzWallet.PermissionLifetimeOption.kForever
  }
]
