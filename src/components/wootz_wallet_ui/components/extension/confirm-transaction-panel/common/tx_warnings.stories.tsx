// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { TransactionWarnings } from './tx_warnings'
import { WootzWallet } from '../../../../constants/types'
// import { Column } from '../../../shared/style'

const warnings: Array<
  Pick<WootzWallet.BlowfishWarning, 'message' | 'severity'>
> = [
    {
      message: 'An example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Another example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Additional example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Yet another example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Yet another example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Yet another example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Yet another example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Yet another example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    },
    {
      message: 'Yet another example warning',
      severity: WootzWallet.BlowfishWarningSeverity.kWarning
    }
  ]

const criticalWarnings: Array<
  Pick<WootzWallet.BlowfishWarning, 'message' | 'severity'>
> = [
    {
      message: 'An example critical warning',
      severity: WootzWallet.BlowfishWarningSeverity.kCritical
    },
    {
      message: 'Another example critical warning',
      severity: WootzWallet.BlowfishWarningSeverity.kCritical
    }
  ]

export const _TxWarning = {
  render: () => {
    // state
    const [isCollapsed, setIsCollapsed] = React.useState(false)
    return (
      <TransactionWarnings
        classifyAs='issues'
        isWarningCollapsed={isCollapsed}
        setIsWarningCollapsed={setIsCollapsed}
        warnings={[warnings[0]]}
        onDismiss={() => alert('dismiss')}
      />
    )
  }
}

export const _TxWarnings = {
  render: () => {
    // state
    const [isCollapsed, setIsCollapsed] = React.useState(true)
    return (
      <TransactionWarnings
        classifyAs='issues'
        isWarningCollapsed={isCollapsed}
        setIsWarningCollapsed={setIsCollapsed}
        warnings={warnings}
        onDismiss={() => alert('dismiss')}
      />
    )
  }
}

export const _CriticalTxWarning = {
  render: () => {
    // state
    const [isCollapsed, setIsCollapsed] = React.useState(false)
    return (
      <TransactionWarnings
        classifyAs='risks'
        isWarningCollapsed={isCollapsed}
        setIsWarningCollapsed={setIsCollapsed}
        warnings={[criticalWarnings[0]]}
        onDismiss={() => alert('dismiss')}
      />
    )
  }
}

export const _CriticalTxWarnings = {
  render: () => {
    // state
    const [isCollapsed, setIsCollapsed] = React.useState(false)
    return (
      <TransactionWarnings
        classifyAs='risks'
        isWarningCollapsed={isCollapsed}
        setIsWarningCollapsed={setIsCollapsed}
        warnings={criticalWarnings}
        onDismiss={() => alert('dismiss')}
      />
    )
  }
}

export default { component: TransactionWarnings }
