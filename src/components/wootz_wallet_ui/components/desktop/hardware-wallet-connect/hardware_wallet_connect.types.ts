/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import {
  LedgerDerivationPaths,
  TrezorDerivationPaths,
  SolDerivationPaths
} from '../../../common/hardware/types'
import { WootzWallet } from '../../../constants/types'
export { SolDerivationPaths } from '../../../common/hardware/types'

// TODO(apaymyshev): strings below need localization.
export const HardwareWalletDerivationPathLocaleMapping = {
  [LedgerDerivationPaths.LedgerLive]: 'Ledger Live',
  [LedgerDerivationPaths.Legacy]: 'Legacy (MEW/MyCrypto)',
  [LedgerDerivationPaths.Deprecated]: 'Deprecated (Not recommended)',
  [TrezorDerivationPaths.Default]: 'Default'
} as const

export const HardwareWalletDerivationPathsMapping = {
  [WootzWallet.LEDGER_HARDWARE_VENDOR]: LedgerDerivationPaths,
  [WootzWallet.TREZOR_HARDWARE_VENDOR]: TrezorDerivationPaths
} as const

// TODO(apaymyshev): strings below need localization.
export const SolHardwareWalletDerivationPathLocaleMapping = {
  [SolDerivationPaths.Default]: 'Default',
  [SolDerivationPaths.LedgerLive]: 'Ledger Live',
  [SolDerivationPaths.Bip44Root]: 'Bip44 Root'
}

export interface ErrorMessage {
  error: string
  userHint: string
}
