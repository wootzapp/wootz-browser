// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { WootzWallet } from '../constants/types'
import { getLocale } from '../../common/locale'

export const AllAccountsOptionUniqueKey = 'all'

export const AllAccountsOption: WootzWallet.AccountInfo = {
  address: AllAccountsOptionUniqueKey,
  accountId: {
    coin: 0,
    keyringId: WootzWallet.KeyringId.kDefault,
    kind: WootzWallet.AccountKind.kDerived,
    address: AllAccountsOptionUniqueKey,
    accountIndex: 0,
    uniqueKey: AllAccountsOptionUniqueKey
  },
  name: getLocale('wootzWalletAccountFilterAllAccounts'),
  hardware: undefined
}

export const isAllAccountsOptionFilter = (selectedAccountFilter: string) => {
  return selectedAccountFilter === AllAccountsOptionUniqueKey
}
