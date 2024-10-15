// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// types
import type {
  TokenBalancesRegistry //
} from '../common/slices/entities/token-balance.entity'

// constants
import { WootzWallet } from '../constants/types'

// utils
import { loadTimeData } from '../../common/loadTimeData'
import { getActiveWalletCount } from './balance-utils'
import getAPIProxy from '../common/async/bridge'

export async function reportActiveWalletsToP3A(
  accountIds: WootzWallet.AccountId[],
  tokenBalancesRegistry: TokenBalancesRegistry
) {
  const countTestNetworks = loadTimeData.getBoolean(
    WootzWallet.P3A_COUNT_TEST_NETWORKS_LOAD_TIME_KEY
  )

  const activeWalletCount = getActiveWalletCount(
    accountIds,
    tokenBalancesRegistry,
    countTestNetworks
  )

  const { wootzWalletP3A } = getAPIProxy()

  for (const [coin, count] of Object.entries(activeWalletCount)) {
    wootzWalletP3A.recordActiveWalletCount(count, parseInt(coin))
  }
}
