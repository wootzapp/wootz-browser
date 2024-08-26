// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// types
import { WootzWallet } from '../constants/types'

// constants
import { BLOWFISH_URL_WARNING_KINDS } from '../common/constants/blowfish'

// utils
import { getLocale } from '../../common/locale'

interface GroupedEVMStateChanges {
  evmApprovals: WootzWallet.BlowfishEVMStateChange[]
  evmTransfers: WootzWallet.BlowfishEVMStateChange[]
}

interface GroupedSVMStateChanges {
  solStakeAuthorityChanges: WootzWallet.BlowfishSolanaStateChange[]
  splApprovals: WootzWallet.BlowfishSolanaStateChange[]
  svmTransfers: WootzWallet.BlowfishSolanaStateChange[]
  accountOwnerChangeData: WootzWallet.BlowfishSolanaStateChange[]
}

export const groupSimulatedEVMStateChanges = (
  evmStateChanges: WootzWallet.BlowfishEVMStateChange[]
): GroupedEVMStateChanges => {
  const changes: GroupedEVMStateChanges = {
    evmApprovals: [],
    evmTransfers: []
  }

  for (const stateChange of evmStateChanges) {
    const { data } = stateChange.rawInfo

    // approvals & approvals for all
    if (
      data.erc20ApprovalData ||
      data.erc721ApprovalData ||
      data.erc721ApprovalForAllData ||
      data.erc1155ApprovalForAllData
    ) {
      changes.evmApprovals.push(stateChange)
    }

    // transfers
    if (
      data.erc20TransferData ||
      data.erc721TransferData ||
      data.erc1155TransferData ||
      data.nativeAssetTransferData
    ) {
      changes.evmTransfers.push(stateChange)
    }
  }

  return changes
}

export const decodeSimulatedSVMStateChanges = (
  stateChanges: WootzWallet.BlowfishSolanaStateChange[]
): GroupedSVMStateChanges => {
  const changes: GroupedSVMStateChanges = {
    solStakeAuthorityChanges: [],
    splApprovals: [],
    svmTransfers: [],
    accountOwnerChangeData: []
  }

  for (const stateChange of stateChanges) {
    const { data } = stateChange.rawInfo

    // staking auth changes
    if (data.solStakeAuthorityChangeData) {
      changes.solStakeAuthorityChanges.push(stateChange)
    }

    // approvals
    if (data.splApprovalData) {
      changes.splApprovals.push(stateChange)
    }

    // transfers
    if (data.solTransferData || data.splTransferData) {
      changes.svmTransfers.push(stateChange)
    }

    // account ownership changes
    if (data.userAccountOwnerChangeData) {
      changes.accountOwnerChangeData.push(stateChange)
    }
  }

  return changes
}

export const isUrlWarning = (warningKind: WootzWallet.BlowfishWarningKind) => {
  return BLOWFISH_URL_WARNING_KINDS.includes(warningKind)
}

const { BlowfishEVMErrorKind, BlowfishSolanaErrorKind: SolErrorKind } =
  WootzWallet

export const translateSimulationWarning = (
  warning: WootzWallet.BlowfishWarning | undefined
) => {
  if (!warning) {
    return ''
  }

  switch (warning.kind) {
    case WootzWallet.BlowfishWarningKind.kApprovalToEOA:
      return getLocale('wootzWalletSimulationWarningApprovalToEoa')

    case WootzWallet.BlowfishWarningKind.kBulkApprovalsRequest:
      return getLocale('wootzWalletSimulationWarningBulkApprovalsRequest')

    case WootzWallet.BlowfishWarningKind.kCopyCatDomain:
    case WootzWallet.BlowfishWarningKind.kMultiCopyCatDomain:
      return getLocale('wootzWalletSimulationWarningCopyCatDomain')

    case WootzWallet.BlowfishWarningKind.kDanglingApproval:
      return getLocale('wootzWalletSimulationWarningDanglingApproval')

    case WootzWallet.BlowfishWarningKind.kKnownMalicious:
      return getLocale('wootzWalletSimulationWarningKnownMalicious')

    case WootzWallet.BlowfishWarningKind.kNewDomain:
      return getLocale('wootzWalletSimulationWarningNewDomain')

    case WootzWallet.BlowfishWarningKind.kPoisonedAddress:
      return getLocale('wootzWalletSimulationWarningPoisonedAddress')

    case WootzWallet.BlowfishWarningKind.kSetOwnerAuthority:
      return getLocale('wootzWalletSimulationWarningSetOwnerAuthority')

    case WootzWallet.BlowfishWarningKind.kSuspectedMalicious:
      return getLocale('wootzWalletSimulationWarningSuspectedMalicious')

    case WootzWallet.BlowfishWarningKind.kTooManyTransactions:
      return warning.severity === WootzWallet.BlowfishWarningSeverity.kCritical
        ? getLocale('wootzWalletSimulationWarningTooManyTransactionsCritical')
        : getLocale('wootzWalletSimulationWarningTooManyTransactions')

    case WootzWallet.BlowfishWarningKind.kTradeForNothing:
      return getLocale('wootzWalletSimulationWarningTradeForNothing')

    case WootzWallet.BlowfishWarningKind.kTransferringErc20ToOwnContract:
      return getLocale(
        'wootzWalletSimulationWarningTransferringErc20ToOwnContract'
      )

    case WootzWallet.BlowfishWarningKind.kUserAccountOwnerChange:
      return getLocale('wootzWalletSimulationWarningUserAccountOwnerChange')

    default:
      return warning.message
  }
}

export const translateSimulationResultError = (
  error:
    | WootzWallet.BlowfishEVMError
    | WootzWallet.BlowfishSolanaError
    | undefined,
  /** prevents collisions between error enums */
  coinType: WootzWallet.CoinType
) => {
  if (!error) {
    return ''
  }

  // SVM
  if (coinType === WootzWallet.CoinType.SOL) {
    switch (error.kind) {
      case SolErrorKind.kAccountDoesNotHaveEnoughSolToPerformTheOperation: //
      case SolErrorKind.kInsufficientFunds:
        return getLocale('wootzWalletSimulationErrorInsufficientFunds')

      case SolErrorKind.kInsufficientFundsForFee:
        return getLocale('wootzWalletSimulationErrorInsufficientFundsForFee')

      case SolErrorKind.kTooManyTransactions:
        return getLocale(
          'wootzWalletSimulationWarningTooManyTransactionsCritical'
        )

      default:
        return (
          error.humanReadableError ||
          getLocale('wootzWalletSimulationUnexpectedError')
        )
    }
  }

  // EVM
  switch (error.kind) {
    case BlowfishEVMErrorKind.kTransactionReverted:
      return getLocale('wootzWalletSimulationErrorTransactionReverted')

    // Known Unknowns
    case BlowfishEVMErrorKind.kTransactionError:
    case BlowfishEVMErrorKind.kSimulationFailed:
    case BlowfishEVMErrorKind.kUnknownError:
      return getLocale('wootzWalletSimulationUnexpectedError')

    // Unknown error type
    default:
      return (
        error.humanReadableError ||
        getLocale('wootzWalletSimulationUnexpectedError')
      )
  }
}
