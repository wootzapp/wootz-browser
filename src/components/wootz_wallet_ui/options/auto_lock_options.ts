// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import { getLocale } from '../../common/locale'
import { AutoLockOption } from '../constants/types'

export const autoLockOptions: AutoLockOption[] = [
  {
    label: getLocale('wootzWalletAutoLockDurationMinutes').replace('$1', '5'),
    minutes: 5
  },
  {
    label: getLocale('wootzWalletAutoLockDurationMinutes').replace('$1', '10'),
    minutes: 10
  },
  {
    label: getLocale('wootzWalletAutoLockDurationHours').replace('$1', '1'),
    minutes: 60
  },
  {
    label: getLocale('wootzWalletAutoLockDurationHours').replace('$1', '3'),
    minutes: 180
  }
]
