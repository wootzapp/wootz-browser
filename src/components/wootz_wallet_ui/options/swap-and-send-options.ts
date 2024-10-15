// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// Types
import { SwapAndSend } from '../constants/types'

export const SwapAndSendOptions: SwapAndSend[] = [
  {
    label: 'wootzSwapToAccount',
    name: 'to-account'
  },
  {
    label: 'wootzSwapToAddress',
    name: 'to-address'
  }
]
