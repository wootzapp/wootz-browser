// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { EntityState } from '@reduxjs/toolkit'

// types
import { WootzWallet, SerializableSolanaTxData } from '../constants/types'

// utils
import { findAccountByAddress } from './account-utils'
import { getGetCleanedMojoEnumKeys } from './enum-utils'
import { lamportsToSol } from './web3-utils'

export const SolanaSystemInstructionKeys = getGetCleanedMojoEnumKeys(
  WootzWallet.SolanaSystemInstruction
)

export const SolanaTokenInstructionKeys = getGetCleanedMojoEnumKeys(
  WootzWallet.SolanaTokenInstruction
)

export type SolanaSystemInstructionType =
  (typeof SolanaSystemInstructionKeys)[number]

export type SolanaTokenInstructionType =
  (typeof SolanaTokenInstructionKeys)[number]

export type TypedSolanaInstructionWithParams = {
  accountMetas: WootzWallet.SolanaAccountMeta[]
  programId: string
  accountParams: WootzWallet.SolanaInstructionParam[]
  type?: SolanaSystemInstructionType | SolanaTokenInstructionType
  params: WootzWallet.SolanaInstructionParam[]
  data: number[]
}

export const getSolanaTransactionInstructionParamsAndType = ({
  programId,
  decodedData,
  accountMetas,
  data
}: WootzWallet.SolanaInstruction): TypedSolanaInstructionWithParams => {
  // the signers are the `accountMetas` from this index to the end of the array
  // its possible to have any number of signers, including 0
  const accountParams: WootzWallet.SolanaInstructionParam[] = (
    decodedData?.accountParams || []
  ).map(({ localizedName, name }, i) => {
    const isSignersParam = name === WootzWallet.SIGNERS

    return {
      name,
      localizedName,
      type: isSignersParam
        ? WootzWallet.SolanaInstructionParamType.kString
        : WootzWallet.SolanaInstructionParamType.kPublicKey,
      // add a comma separated list of signers as a value if param name is
      // "signers"
      value: isSignersParam
        ? accountMetas.slice(i).join(',')
        : accountMetas[i]?.pubkey
    }
  })

  const typedInstruction: TypedSolanaInstructionWithParams = {
    programId,
    accountParams: accountParams,
    params: decodedData?.params || [],
    type: undefined,
    accountMetas,
    data
  }

  if (!decodedData) {
    // return early if nothing to decode
    return typedInstruction
  }

  switch (programId) {
    case WootzWallet.SOLANA_SYSTEM_PROGRAM_ID: {
      typedInstruction.type =
        SolanaSystemInstructionKeys[decodedData.instructionType]
      break
    }
    case WootzWallet.SOLANA_TOKEN_PROGRAM_ID: {
      typedInstruction.type =
        SolanaTokenInstructionKeys[decodedData.instructionType]
      break
    }
  }

  return typedInstruction
}

export const getTypedSolanaTxInstructions = (
  solTxData?: SerializableSolanaTxData | WootzWallet.SolanaTxData
): TypedSolanaInstructionWithParams[] => {
  const instructions: TypedSolanaInstructionWithParams[] = (
    solTxData?.instructions || []
  ).map((instruction) => {
    return getSolanaTransactionInstructionParamsAndType(instruction)
  })
  return instructions || []
}

/**
 * formatted if possible:
 *
 * lamports -> SOL
 *
 * pubkey -> friendly account address name
 */
export const formatSolInstructionParamValue = (
  { name, value, type }: WootzWallet.SolanaInstructionParam,
  accounts: EntityState<WootzWallet.AccountInfo> | undefined
): {
  valueType: 'lamports' | 'address' | 'other'
  formattedValue: string
} => {
  const isAddressParam =
    type === WootzWallet.SolanaInstructionParamType.kOptionalPublicKey ||
    type === WootzWallet.SolanaInstructionParamType.kPublicKey

  const isLamportsParam = name === WootzWallet.LAMPORTS

  const formattedParamValue = (
    isLamportsParam
      ? lamportsToSol(value).formatAsAsset(9, 'SOL')
      : isAddressParam
      ? findAccountByAddress(value, accounts) ?? value
      : value
  ).toString()

  return {
    formattedValue: formattedParamValue,
    valueType: isAddressParam
      ? 'address'
      : isLamportsParam
      ? 'lamports'
      : 'other'
  }
}

export const getSolInstructionAccountParamsObj = (
  accountParams: WootzWallet.SolanaInstructionAccountParam[],
  accountMetas: WootzWallet.SolanaAccountMeta[]
) => {
  let fromAccount: string = ''
  let toAccount: string = ''
  let nonceAccount: string = ''
  let newAccount: string = ''

  accountParams.forEach(({ name }, i) => {
    // Do not show account public key of address table lookup account because
    // it might give the wrong impression to users. For example, users might
    // think they're sending funds to this address table lookup account, but
    // actually they're sending to the account pointed by the index in this
    // address table lookup account.
    const isAddrTableLookupAccount = accountMetas[i]?.addrTableLookupIndex
    const value = isAddrTableLookupAccount ? '' : accountMetas[i]?.pubkey

    switch (name) {
      case WootzWallet.FROM_ACCOUNT: {
        fromAccount = value
        break
      }
      case WootzWallet.TO_ACCOUNT: {
        toAccount = value
        break
      }
      case WootzWallet.NONCE_ACCOUNT: {
        nonceAccount = value
        break
      }
      case WootzWallet.NEW_ACCOUNT: {
        newAccount = value
        break
      }
      default:
        break
    }
  })

  return {
    fromAccount,
    toAccount,
    nonceAccount,
    newAccount
  }
}

export const getSolInstructionParamsObj = (
  params: WootzWallet.SolanaInstructionParam[]
) => {
  let lamports: string = '0'

  params.forEach(({ name, value }, i) => {
    switch (name) {
      case WootzWallet.LAMPORTS: {
        lamports = value ?? '0'
        break
      }
      default:
        break
    }
  })

  return {
    lamports
  }
}
