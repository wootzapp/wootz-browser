// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import { mapLimit } from 'async'
import { EntityId } from '@reduxjs/toolkit'

// types
import type { BaseQueryCache } from '../../async/base-query-cache'
import { WootzWallet, WalletState } from '../../../constants/types'
import { WalletApiEndpointBuilderParams } from '../api-base.slice'
import {
  ImportFromExternalWalletPayloadType,
  ShowRecoveryPhrasePayload
} from '../../../page/constants/action_types'

// actions
import { WalletPageActions } from '../../../page/actions'

// utils
import { handleEndpointError } from '../../../utils/api-utils'
import { getLocale } from '../../../../common/locale'
import { keyringIdForNewAccount } from '../../../utils/account-utils'
import { suggestNewAccountName } from '../../../utils/address-utils'
import { getEntitiesListFromEntityState } from '../../../utils/entities.utils'
import { networkEntityAdapter } from '../entities/network.entity'

type ImportWalletResults = {
  errorMessage?: string
}

export const walletEndpoints = ({
  mutation,
  query
}: WalletApiEndpointBuilderParams) => {
  return {
    getIsWalletBackedUp: query<boolean, void>({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { walletInfo } = await api.walletHandler.getWalletInfo()
          return {
            data: walletInfo.isWalletBackedUp
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to check if wallet is backed up',
            error
          )
        }
      },
      providesTags: ['IsWalletBackedUp']
    }),

    getDefaultEthereumWallet: query<number, void>({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { wootzWalletService } = api
          const { defaultWallet } =
            await wootzWalletService.getDefaultEthereumWallet()
          return {
            data: defaultWallet
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Failed to fetch default Ethereum wallet',
            error
          )
        }
      },
      providesTags: ['DefaultEthWallet']
    }),

    getDefaultSolanaWallet: query<number, void>({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { wootzWalletService } = api
          const { defaultWallet } =
            await wootzWalletService.getDefaultSolanaWallet()
          return {
            data: defaultWallet
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Failed to fetch default Solana wallet',
            error
          )
        }
      },
      providesTags: ['DefaultSolWallet']
    }),

    getIsMetaMaskInstalled: query<boolean, void>({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { wootzWalletService } = api
          const { installed } =
            await wootzWalletService.isExternalWalletInstalled(
              WootzWallet.ExternalWalletType.MetaMask
            )
          return {
            data: installed
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Failed to check if MetaMask is installed',
            error
          )
        }
      },
      providesTags: ['IsMetaMaskInstalled']
    }),

    getActiveOrigin: query<WootzWallet.OriginInfo, void>({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { wootzWalletService } = api

          const { originInfo } = await wootzWalletService.getActiveOrigin()
          return {
            data: originInfo
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'unable to get active origin',
            error
          )
        }
      },
      providesTags: ['ActiveOrigin']
    }),

    createWallet: mutation<true, { password: string }>({
      queryFn: async (
        arg,
        { endpoint, dispatch, getState },
        extraOptions,
        baseQuery
      ) => {
        try {
          const { data: api, cache } = baseQuery(undefined)
          const { keyringService } = api

          const result = await keyringService.createWallet(arg.password)
          if (!result.mnemonic) {
            throw new Error('Unable to create wallet')
          }

          dispatch(
            WalletPageActions.walletCreated({ mnemonic: result.mnemonic })
          )

          const { allowedNewWalletAccountTypeNetworkIds } = (
            getState() as { wallet: WalletState }
          ).wallet

          await createDefaultAccounts({
            allowedNewWalletAccountTypeNetworkIds,
            keyringService,
            cache
          })

          return {
            data: true
          }
        } catch (error) {
          return handleEndpointError(endpoint, 'Unable to create wallet', error)
        }
      }
    }),

    restoreWallet: mutation<
      { success: boolean; invalidMnemonic: boolean },
      {
        mnemonic: string
        password: string
        isLegacy: boolean
        completeWalletSetup?: boolean
      }
    >({
      queryFn: async (
        arg,
        { endpoint, dispatch, getState },
        extraOptions,
        baseQuery
      ) => {
        try {
          const { data: api, cache } = baseQuery(undefined)
          const { keyringService } = api

          const result = await keyringService.restoreWallet(
            arg.mnemonic,
            arg.password,
            arg.isLegacy
          )

          if (!result.isValidMnemonic) {
            return {
              data: {
                success: false,
                invalidMnemonic: true
              }
            }
          }

          cache.clearWalletInfo()
          keyringService.notifyWalletBackupComplete()

          if (arg?.completeWalletSetup) {
            dispatch(
              WalletPageActions.walletSetupComplete(arg.completeWalletSetup)
            )
          }

          const { allowedNewWalletAccountTypeNetworkIds } = (
            getState() as { wallet: WalletState }
          ).wallet

          await createDefaultAccounts({
            allowedNewWalletAccountTypeNetworkIds,
            keyringService,
            cache
          })

          return {
            data: {
              invalidMnemonic: false,
              success: true
            }
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to restore wallet from seed phrase',
            error
          )
        }
      },
      invalidatesTags: [
        'AccountInfos',
        'IsWalletBackedUp',
        'TokenBalances',
        'TokenBalancesForChainId',
        'AccountTokenCurrentBalance'
      ]
    }),

    showRecoveryPhrase: mutation<boolean, ShowRecoveryPhrasePayload>({
      queryFn: async (arg, { endpoint, dispatch }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { keyringService } = api
          const { password } = arg

          if (password) {
            const { mnemonic } = await keyringService.getWalletMnemonic(
              password
            )
            dispatch(
              WalletPageActions.recoveryWordsAvailable({
                mnemonic: mnemonic ?? ''
              })
            )
            return {
              data: true
            }
          }

          dispatch(WalletPageActions.recoveryWordsAvailable({ mnemonic: '' }))

          return {
            data: true
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to fetch wallet recovery phrase',
            error
          )
        }
      }
    }),

    getWalletsToImport: query<
      {
        isMetaMaskInitialized: boolean
        isLegacyCryptoWalletsInitialized: boolean
      },
      void
    >({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { wootzWalletService } = api

          const cwResult = await wootzWalletService.isExternalWalletInitialized(
            WootzWallet.ExternalWalletType.CryptoWallets
          )
          const mmResult = await wootzWalletService.isExternalWalletInitialized(
            WootzWallet.ExternalWalletType.MetaMask
          )

          return {
            data: {
              isLegacyCryptoWalletsInitialized: cwResult.initialized,
              isMetaMaskInitialized: mmResult.initialized
            }
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to fetch wallet extensions for import',
            error
          )
        }
      }
    }),

    checkExternalWalletPassword: mutation<
      { success: boolean; errorMessage?: string },
      { walletType: WootzWallet.ExternalWalletType; password: string }
    >({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { wootzWalletService } = api

          const { errorMessage } =
            await wootzWalletService.importFromExternalWallet(
              arg.walletType,
              arg.password,
              ''
            )

          // was the provided import password correct?
          const checkExistingPasswordError =
            errorMessage === getLocale('wootzWalletImportPasswordError')
              ? errorMessage || undefined
              : undefined

          return {
            data: {
              errorMessage: checkExistingPasswordError,
              success: !checkExistingPasswordError
            }
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to check external wallet password',
            error
          )
        }
      },
      invalidatesTags: ['AccountInfos', 'IsWalletBackedUp']
    }),

    importFromCryptoWallets: mutation<
      { success: boolean; errorMessage?: string },
      ImportFromExternalWalletPayloadType
    >({
      queryFn: async (arg, { endpoint, getState }, extraOptions, baseQuery) => {
        try {
          if (!arg.newPassword) {
            throw new Error('A new password is required')
          }

          const { data: api, cache } = baseQuery(undefined)
          const { keyringService, wootzWalletService } = api

          const results: ImportWalletResults = await importFromExternalWallet(
            WootzWallet.ExternalWalletType.CryptoWallets,
            arg,
            { wootzWalletService, keyringService }
          )

          if (results.errorMessage) {
            return {
              data: {
                success: false,
                errorMessage: results.errorMessage
              }
            }
          }

          const { allowedNewWalletAccountTypeNetworkIds } = (
            getState() as { wallet: WalletState }
          ).wallet

          await createDefaultAccounts({
            allowedNewWalletAccountTypeNetworkIds,
            keyringService,
            cache
          })

          cache.clearWalletInfo()

          return {
            data: {
              success: true
            }
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to restore wallet from legacy crypto wallets extension',
            error
          )
        }
      },
      invalidatesTags: [
        'AccountInfos',
        'IsWalletBackedUp',
        'TokenBalances',
        'TokenBalancesForChainId',
        'AccountTokenCurrentBalance'
      ]
    }),

    importFromMetaMask: mutation<
      { success: boolean; errorMessage?: string },
      ImportFromExternalWalletPayloadType
    >({
      queryFn: async (arg, { endpoint, getState }, extraOptions, baseQuery) => {
        try {
          if (!arg.newPassword) {
            throw new Error('A new password is required')
          }

          const { data: api, cache } = baseQuery(undefined)
          const { keyringService, wootzWalletService } = api

          const { errorMessage }: ImportWalletResults =
            await importFromExternalWallet(
              WootzWallet.ExternalWalletType.MetaMask,
              arg,
              { wootzWalletService, keyringService }
            )

          if (errorMessage) {
            return {
              data: {
                errorMessage,
                success: false
              }
            }
          }

          const { allowedNewWalletAccountTypeNetworkIds } = (
            getState() as { wallet: WalletState }
          ).wallet

          await createDefaultAccounts({
            allowedNewWalletAccountTypeNetworkIds,
            keyringService,
            cache
          })

          cache.clearWalletInfo()

          return {
            data: { success: true }
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to restore wallet from MetaMask extension',
            error
          )
        }
      },
      invalidatesTags: [
        'AccountInfos',
        'IsWalletBackedUp',
        'TokenBalances',
        'TokenBalancesForChainId',
        'AccountTokenCurrentBalance'
      ]
    }),

    completeWalletBackup: mutation<boolean, void>({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const { keyringService } = api

          keyringService.notifyWalletBackupComplete()

          return {
            data: true
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Unable to complete wallet backup',
            error
          )
        }
      },
      invalidatesTags: ['IsWalletBackedUp']
    }),

    lockWallet: mutation<
      true, // success
      void
    >({
      queryFn: async (
        password,
        { endpoint, dispatch },
        extraOptions,
        baseQuery
      ) => {
        try {
          const { data: api } = baseQuery(undefined)
          api.keyringService.lock()

          return {
            data: true
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'An error occurred while attempting to lock the wallet',
            error
          )
        }
      }
    }),

    unlockWallet: mutation<
      boolean, // success
      string // password
    >({
      queryFn: async (
        password,
        { endpoint, dispatch },
        extraOptions,
        baseQuery
      ) => {
        try {
          const { data: api } = baseQuery(undefined)
          const result = await api.keyringService.unlock(password)
          return {
            data: result.success
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'An error occurred while attempting to unlock the wallet',
            error
          )
        }
      }
    }),

    setAutoLockMinutes: mutation<boolean, number>({
      queryFn: async (minutes, { endpoint }, _extraOptions, baseQuery) => {
        try {
          const { data: api } = baseQuery(undefined)
          const result = await api.keyringService.setAutoLockMinutes(minutes)
          return {
            data: result.success
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'An error occurred while attempting to set the auto lock minutes',
            error
          )
        }
      }
    })
  }
}

// Internals
async function importFromExternalWallet(
  walletType: WootzWallet.ExternalWalletType,
  payload: ImportFromExternalWalletPayloadType,
  services: {
    wootzWalletService: WootzWallet.WootzWalletServiceRemote
    keyringService: WootzWallet.KeyringServiceRemote
  }
): Promise<ImportWalletResults> {
  // need new password to continue
  if (!payload.newPassword) {
    return { errorMessage: undefined }
  }

  const { wootzWalletService, keyringService } = services

  const { errorMessage } = await wootzWalletService.importFromExternalWallet(
    walletType,
    payload.password,
    payload.newPassword
  )

  // complete backup
  if (!errorMessage) {
    keyringService.notifyWalletBackupComplete()
  }

  return {
    errorMessage: errorMessage || undefined
  }
}

async function createDefaultAccounts({
  allowedNewWalletAccountTypeNetworkIds,
  keyringService,
  cache
}: {
  cache: BaseQueryCache
  keyringService: WootzWallet.KeyringServiceRemote
  allowedNewWalletAccountTypeNetworkIds: EntityId[]
}) {
  const networksRegistry = await cache.getNetworksRegistry()
  const accountsRegistry = await cache.getAccountsRegistry()

  const visibleNetworks = getEntitiesListFromEntityState(
    networksRegistry,
    networksRegistry.visibleIds
  )

  const networkKeyrings: number[] = []
  const networksWithUniqueKeyrings = []

  for (const net of visibleNetworks) {
    const keyringId = keyringIdForNewAccount(net.coin, net.chainId)
    if (!networkKeyrings.includes(keyringId)) {
      networkKeyrings.push(keyringId)
      networksWithUniqueKeyrings.push(net)
    }
  }

  const accounts = getEntitiesListFromEntityState(accountsRegistry)

  // create accounts for visible network coin types if needed
  await mapLimit(
    networksWithUniqueKeyrings,
    3,
    async function (net: WootzWallet.NetworkInfo) {
      // TODO: remove these checks when we can hide "default" networks
      if (
        !allowedNewWalletAccountTypeNetworkIds.includes(
          networkEntityAdapter.selectId(net)
        )
      ) {
        return
      }

      switch (net.coin) {
        case WootzWallet.CoinType.BTC: {
          await keyringService.addAccount(
            net.coin,
            keyringIdForNewAccount(net.coin, net.chainId),
            suggestNewAccountName(accounts, net)
          )
          return
        }
        case WootzWallet.CoinType.FIL: {
          await keyringService.addAccount(
            net.coin,
            keyringIdForNewAccount(net.coin, net.chainId),
            suggestNewAccountName(accounts, net)
          )
        }
      }
    }
  )
}
