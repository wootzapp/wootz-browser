// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import { AccountButtonOptionsObjectType } from '../constants/types'

export const AccountDetailsMenuOptions: AccountButtonOptionsObjectType[] = [
  {
    id: 'edit',
    name: 'wootzWalletAllowSpendEditButton',
    icon: 'edit-pencil'
  },
  {
    id: 'explorer',
    name: 'wootzWalletTransactionExplorer',
    icon: 'web3-blockexplorer'
  },
  {
    id: 'deposit',
    name: 'wootzWalletAccountsDeposit',
    icon: 'money-bag-coins'
  },
  {
    id: 'privateKey',
    name: 'wootzWalletAccountsExport',
    icon: 'key'
  },
  {
    id: 'remove',
    name: 'wootzWalletAccountsRemove',
    icon: 'trash'
  }
]
