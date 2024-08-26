// Copyright (c) 2021 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { WootzWallet } from '../../constants/types'

export type AccountPayloadType = {
  selectedAccounts: WootzWallet.AccountInfo[]
}

export type ShowConnectToSitePayload = {
  accounts: string[]
  originInfo: WootzWallet.OriginInfo
}

export type SignMessageProcessedPayload = {
  approved: boolean
  id: number
  signature?: WootzWallet.ByteArrayStringUnion
  error?: string
}

export type SignAllTransactionsProcessedPayload = {
  approved: boolean
  id: number
  signatures?: WootzWallet.ByteArrayStringUnion[]
  error?: string
}
