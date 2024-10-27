// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// Types
import { WootzWallet } from '../../../constants/types'

// Mocks
import {
  mockTransactionInfo //
} from '../../../stories/mock-data/mock-transaction-info'

// Components
import {
  WalletPanelStory //
} from '../../../stories/wrappers/wallet-panel-story-wrapper'
import { LongWrapper } from '../../../stories/style'
import { PanelWrapper } from '../../../panel/style'
import { ConfirmSwapTransaction } from './swap'

// Utils
import {
  deserializeTransaction //
} from '../../../utils/model-serialization-utils'

export const _ConfirmSwapTransaction = {
  render: () => {
    return (
      <WalletPanelStory
        walletStateOverride={{
          hasInitialized: true,
          isWalletCreated: true
        }}
        uiStateOverride={{
          selectedPendingTransactionId: mockTransactionInfo.id
        }}
        panelStateOverride={{
          hasInitialized: true
        }}
        walletApiDataOverrides={{
          simulationOptInStatus: WootzWallet.BlowfishOptInStatus.kAllowed,
          evmSimulationResponse: {
            error: {
              humanReadableError: 'Simulation failed',
              kind: WootzWallet.BlowfishEVMErrorKind.kSimulationFailed
            },
            expectedStateChanges: [],
            action: WootzWallet.BlowfishSuggestedAction.kWarn,
            warnings: []
          },
          transactionInfos: [
            deserializeTransaction({
              ...mockTransactionInfo,
              txStatus: WootzWallet.TransactionStatus.Unapproved
            })
          ],
          accountInfos: [
            {
              accountId: {
                address: mockTransactionInfo.fromAddress || '',
                coin: WootzWallet.CoinType.ETH,
                keyringId: WootzWallet.KeyringId.kDefault,
                kind: WootzWallet.AccountKind.kDerived,
                uniqueKey: '',
                accountIndex: 0
              },
              address: mockTransactionInfo.fromAddress || '',
              hardware: undefined,
              name: '1'
            }
          ]
        }}
      >
        <PanelWrapper isLonger={true}>
          <LongWrapper>
            <ConfirmSwapTransaction />
          </LongWrapper>
        </PanelWrapper>
      </WalletPanelStory>
    )
  }
}

export default { component: ConfirmSwapTransaction }
