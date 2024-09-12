// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

// Types
import { AccountButtonOptionsObjectType } from '../constants/types'

export const BuyButtonOption: AccountButtonOptionsObjectType = {
  id: 'buy',
  name: 'wootzWalletBuy',
  icon: 'coins-alt1'
}

export const DepositButtonOption: AccountButtonOptionsObjectType = {
  id: 'deposit',
  name: 'wootzWalletAccountsDeposit',
  icon: 'money-bag-coins'
}

export const AccountButtonOptions: AccountButtonOptionsObjectType[] = [
  {
    id: 'details',
    name: 'wootzWalletAllowSpendDetailsButton',
    icon: 'eye-on'
  },
  {
    id: 'edit',
    name: 'wootzWalletAllowSpendEditButton',
    icon: 'edit-pencil'
  },
  {
    id: 'privateKey',
    name: 'wootzWalletAccountsExport',
    icon: 'key'
  },
  DepositButtonOption,
  {
    id: 'remove',
    name: 'wootzWalletAccountsRemove',
    icon: 'trash'
  }
]
