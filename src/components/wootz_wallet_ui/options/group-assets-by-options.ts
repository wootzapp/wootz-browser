// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import { DropdownFilterOption } from '../constants/types'

export const NoneGroupByOption: DropdownFilterOption = {
  id: 'none',
  name: 'wootzWalletNone'
}

export const AccountsGroupByOption: DropdownFilterOption = {
  id: 'accounts',
  name: 'wootzWalletAccounts'
}

export const NetworksGroupByOption: DropdownFilterOption = {
  id: 'networks',
  name: 'wootzWalletNetworks'
}

export const GroupAssetsByOptions: DropdownFilterOption[] = [
  NoneGroupByOption,
  AccountsGroupByOption,
  NetworksGroupByOption
]
