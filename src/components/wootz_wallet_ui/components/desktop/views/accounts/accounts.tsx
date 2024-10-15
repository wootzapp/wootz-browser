// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { useHistory } from 'react-router'
import { skipToken } from '@reduxjs/toolkit/query/react'

// constants
import { WootzWallet, AccountPageTabs } from '../../../../constants/types'
import {
  querySubscriptionOptions60s //
} from '../../../../common/slices/constants'
// import { emptyRewardsInfo } from '../../../../common/async/base-query-cache'

// utils
import { getLocale } from '../../../../../common/locale'
import {
  groupAccountsById,
  sortAccountsByName
} from '../../../../utils/account-utils'
import { makeAccountRoute } from '../../../../utils/routes-utils'
import { getPriceIdForToken } from '../../../../utils/pricing-utils'

// Styled Components
import { SectionTitle, AccountsListWrapper } from './style'

import { Column, Row } from '../../../shared/style'

// Components
import AccountListItem from '../../account-list-item'
import {
  WalletPageWrapper //
} from '../../wallet-page-wrapper/wallet-page-wrapper'
import { AccountsHeader } from '../../card-headers/accounts-header'

// Hooks
import {
  useBalancesFetcher //
} from '../../../../common/hooks/use-balances-fetcher'
import {
  useGetDefaultFiatCurrencyQuery,
  useGetVisibleNetworksQuery,
  useGetTokenSpotPricesQuery,
  useGetUserTokensRegistryQuery
} from '../../../../common/slices/api.slice'
import { useAccountsQuery } from '../../../../common/slices/api.slice.extra'

export const Accounts = () => {
  // routing
  const history = useHistory()

  // queries
  const { accounts } = useAccountsQuery()

  const { data: userTokensRegistry } = useGetUserTokensRegistryQuery()

  // methods
  const onSelectAccount = React.useCallback(
    (account: WootzWallet.AccountInfo | undefined) => {
      if (account) {
        history.push(
          makeAccountRoute(account, AccountPageTabs.AccountAssetsSub)
        )
      }
    },
    [history]
  )

  // memos && computed
  const derivedAccounts = React.useMemo(() => {
    return accounts.filter(
      (account) => account.accountId.kind === WootzWallet.AccountKind.kDerived
    )
  }, [accounts])

  const importedAccounts = React.useMemo(() => {
    return accounts.filter(
      (account) => account.accountId.kind === WootzWallet.AccountKind.kImported
    )
  }, [accounts])

  const trezorAccounts = React.useMemo(() => {
    const foundTrezorAccounts = accounts.filter((account) => {
      return (
        account.accountId.kind === WootzWallet.AccountKind.kHardware &&
        account.hardware?.vendor === WootzWallet.TREZOR_HARDWARE_VENDOR
      )
    })
    return groupAccountsById(foundTrezorAccounts, 'deviceId')
  }, [accounts])

  const ledgerAccounts = React.useMemo(() => {
    const foundLedgerAccounts = accounts.filter((account) => {
      return (
        account.accountId.kind === WootzWallet.AccountKind.kHardware &&
        account.hardware?.vendor === WootzWallet.LEDGER_HARDWARE_VENDOR
      )
    })
    return groupAccountsById(foundLedgerAccounts, 'deviceId')
  }, [accounts])

  const { data: networks } = useGetVisibleNetworksQuery()
  const { data: defaultFiatCurrency } = useGetDefaultFiatCurrencyQuery()

  const { data: tokenBalancesRegistry, isLoading: isLoadingBalances } =
    useBalancesFetcher({
      accounts,
      networks
    })

  const tokenPriceIds = React.useMemo(() => {
    if (userTokensRegistry) {
      return userTokensRegistry.fungibleVisibleTokenIds.map((id) => {
        return getPriceIdForToken(userTokensRegistry.entities[id]!)
      })
    }
    return []
  }, [userTokensRegistry])

  const { data: spotPriceRegistry, isLoading: isLoadingSpotPrices } =
    useGetTokenSpotPricesQuery(
      tokenPriceIds.length && defaultFiatCurrency
        ? { ids: tokenPriceIds, toCurrency: defaultFiatCurrency }
        : skipToken,
      querySubscriptionOptions60s
    )

  const trezorKeys = React.useMemo(() => {
    return Object.keys(trezorAccounts)
  }, [trezorAccounts])

  const trezorList = React.useMemo(() => {
    return trezorKeys.map((key) => (
      <AccountsListWrapper
        fullWidth={true}
        alignItems='flex-start'
        key={key}
      >
        {sortAccountsByName(trezorAccounts[key]).map(
          (account: WootzWallet.AccountInfo) => (
            <AccountListItem
              key={account.accountId.uniqueKey}
              onClick={onSelectAccount}
              account={account}
              tokenBalancesRegistry={tokenBalancesRegistry}
              isLoadingBalances={isLoadingBalances}
              spotPriceRegistry={spotPriceRegistry}
              isLoadingSpotPrices={isLoadingSpotPrices}
            />
          )
        )}
      </AccountsListWrapper>
    ))
  }, [
    trezorKeys,
    trezorAccounts,
    onSelectAccount,
    tokenBalancesRegistry,
    spotPriceRegistry,
    isLoadingBalances,
    isLoadingSpotPrices
  ])

  const ledgerKeys = React.useMemo(() => {
    return Object.keys(ledgerAccounts)
  }, [ledgerAccounts])

  const ledgerList = React.useMemo(() => {
    return ledgerKeys.map((key) => (
      <AccountsListWrapper
        fullWidth={true}
        alignItems='flex-start'
        key={key}
      >
        {sortAccountsByName(ledgerAccounts[key]).map(
          (account: WootzWallet.AccountInfo) => (
            <AccountListItem
              key={account.accountId.uniqueKey}
              onClick={onSelectAccount}
              account={account}
              tokenBalancesRegistry={tokenBalancesRegistry}
              isLoadingBalances={isLoadingBalances}
              spotPriceRegistry={spotPriceRegistry}
              isLoadingSpotPrices={isLoadingSpotPrices}
            />
          )
        )}
      </AccountsListWrapper>
    ))
  }, [
    ledgerKeys,
    ledgerAccounts,
    onSelectAccount,
    tokenBalancesRegistry,
    spotPriceRegistry,
    isLoadingBalances,
    isLoadingSpotPrices
  ])

  // computed
  const showHardwareWallets = trezorKeys.length !== 0 || ledgerKeys.length !== 0

  // render
  return (
    <WalletPageWrapper
      wrapContentInBox
      cardHeader={<AccountsHeader />}
    >
      <Row
        padding='8px'
        justifyContent='flex-start'
      >
        <SectionTitle>{getLocale('wootzWalletAccounts')}</SectionTitle>
      </Row>
      <AccountsListWrapper
        fullWidth={true}
        alignItems='flex-start'
        margin='0px 0px 24px 0px'
      >
        {derivedAccounts.map((account) => (
          <AccountListItem
            key={account.accountId.uniqueKey}
            onClick={onSelectAccount}
            account={account}
            tokenBalancesRegistry={tokenBalancesRegistry}
            isLoadingBalances={isLoadingBalances}
            spotPriceRegistry={spotPriceRegistry}
            isLoadingSpotPrices={isLoadingSpotPrices}
          />
        ))}
      </AccountsListWrapper>

      {importedAccounts.length !== 0 && (
        <>
          <Row
            padding='8px'
            justifyContent='flex-start'
          >
            <SectionTitle>
              {getLocale('wootzWalletAccountsSecondary')}
            </SectionTitle>
          </Row>
          <AccountsListWrapper
            fullWidth={true}
            alignItems='flex-start'
            margin='0px 0px 24px 0px'
          >
            {importedAccounts.map((account) => (
              <AccountListItem
                key={account.accountId.uniqueKey}
                onClick={onSelectAccount}
                account={account}
                tokenBalancesRegistry={tokenBalancesRegistry}
                isLoadingBalances={isLoadingBalances}
                spotPriceRegistry={spotPriceRegistry}
                isLoadingSpotPrices={isLoadingSpotPrices}
              />
            ))}
          </AccountsListWrapper>
        </>
      )}

      {showHardwareWallets && (
        <>
          <Row
            padding='8px'
            justifyContent='flex-start'
          >
            <SectionTitle>
              {getLocale('wootzWalletConnectedHardwareWallets')}
            </SectionTitle>
          </Row>
          <Column
            fullWidth={true}
            alignItems='flex-start'
            margin='0px 0px 24px 0px'
          >
            {trezorList}
            {ledgerList}
          </Column>
        </>
      )}

      {(
        <>
          <Row
            padding='8px'
            justifyContent='flex-start'
          >
            <SectionTitle>
              {getLocale('wootzWalletConnectedAccounts')}
            </SectionTitle>
          </Row>
          <AccountsListWrapper
            fullWidth={true}
            alignItems='flex-start'
            margin='0px 0px 24px 0px'
          >
          </AccountsListWrapper>
        </>
      )}
    </WalletPageWrapper>
  )
}

export default Accounts
