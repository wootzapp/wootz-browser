// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

// types
import { WootzWallet } from '../constants/types'

// utils
import { getLocale } from '$web-common/locale'

/**
 * see: https://docs.solana.com/developing/runtime-facilities/programs
 * @param programId Solana Program public key
 * @returns Name of program
 */
export const getSolanaProgramIdName = (programId: string): string => {
  switch (programId) {
    case WootzWallet.SOLANA_SYSTEM_PROGRAM_ID:
      return getLocale('wootzWalletSolanaSystemProgram')
    case WootzWallet.SOLANA_CONFIG_PROGRAM_ID:
      return getLocale('wootzWalletSolanaConfigProgram')
    case WootzWallet.SOLANA_STAKE_PROGRAM_ID:
      return getLocale('wootzWalletSolanaStakeProgram')
    case WootzWallet.SOLANA_VOTE_PROGRAM_ID:
      return getLocale('wootzWalletSolanaVoteProgram')
    case WootzWallet.SOLANA_BPF_LOADER_UPGRADEABLE_PROGRAM_ID:
      return getLocale('wootzWalletSolanaBPFLoader')
    case WootzWallet.SOLANA_ED25519_SIG_VERIFY_PROGRAM_ID:
      return getLocale('wootzWalletSolanaEd25519Program')
    case WootzWallet.SOLANA_KECCAK_SECP256K_PROGRAM_ID:
      return getLocale('wootzWalletSolanaSecp256k1Program')
    case WootzWallet.SOLANA_ASSOCIATED_TOKEN_PROGRAM_ID:
      return getLocale('wootzWalletSolanaAssociatedTokenProgram')
    case WootzWallet.SOLANA_METADATA_PROGRAM_ID:
      return getLocale('wootzWalletSolanaMetaDataProgram')
    case WootzWallet.SOLANA_SYSVAR_RENT_PROGRAM_ID:
      return getLocale('wootzWalletSolanaSysvarRentProgram')
    case WootzWallet.SOLANA_TOKEN_PROGRAM_ID:
      return getLocale('wootzWalletSolanaTokenProgram')
    default:
      return programId
  }
}
