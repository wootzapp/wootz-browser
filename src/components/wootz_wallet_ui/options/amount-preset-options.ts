// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import { AmountPresetObjectType } from '../constants/types'
import { getLocale } from '../../common/locale'

export const AmountPresetOptions = (): AmountPresetObjectType[] => [
  {
    name: getLocale('wootzWalletPreset25'),
    value: 0.25
  },
  {
    name: getLocale('wootzWalletPreset50'),
    value: 0.5
  },
  {
    name: getLocale('wootzWalletPreset75'),
    value: 0.75
  },
  {
    name: getLocale('wootzWalletPreset100'),
    value: 1
  }
]
