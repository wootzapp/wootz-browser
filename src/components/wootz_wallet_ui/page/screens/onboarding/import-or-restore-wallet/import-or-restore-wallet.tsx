// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// utils
import { getLocale } from '../../../../../common/locale'
import {
  useGetWalletsToImportQuery //
} from '../../../../common/slices/api.slice'

// types
import { WalletRoutes } from '../../../../constants/types'

// components
import { CenteredPageLayout } from '../../../../components/desktop/centered-page-layout/centered-page-layout'
import {
  OnboardingStepsNavigation //
} from '../components/onboarding-steps-navigation/onboarding-steps-navigation'

// styles
import { WalletLink } from '../../../../components/shared/style'

import {
  StyledWrapper,
  Title,
  Description,
  MainWrapper
} from '../onboarding.style'

import {
  WootzWalletIcon,
  CardButton,
  CardButtonTextContainer,
  LegacyWalletIcon,
  LinkRow,
  MetaMaskIcon
} from './import-or-restore-wallet.style'

export const OnboardingImportOrRestoreWallet = () => {
  // queries
  const { data: importableWallets } = useGetWalletsToImportQuery()

  // render
  return (
    <CenteredPageLayout>
      <MainWrapper>
        <StyledWrapper>
          <OnboardingStepsNavigation preventSkipAhead />

          <div>
            <Title>{getLocale('wootzWalletImportOrRestoreWalletTitle')}</Title>
            <Description>
              {getLocale('wootzWalletImportOrRestoreDescription')}
            </Description>
          </div>

          <CardButton to={WalletRoutes.OnboardingRestoreWallet}>
            <CardButtonTextContainer>
              <p>{getLocale('wootzWalletRestoreMyWootzWallet')}</p>
              <p>{getLocale('wootzWalletRestoreMyWootzWalletDescription')}</p>
            </CardButtonTextContainer>
            <WootzWalletIcon />
          </CardButton>

          {importableWallets?.isMetaMaskInitialized && (
            <CardButton to={WalletRoutes.OnboardingImportMetaMask}>
              <CardButtonTextContainer>
                <p>{getLocale('wootzWalletImportFromMetaMask')}</p>
                <p>{getLocale('wootzWalletImportFromMetaMaskDescription')}</p>
              </CardButtonTextContainer>
              <MetaMaskIcon />
            </CardButton>
          )}

          {importableWallets?.isLegacyCryptoWalletsInitialized && (
            <CardButton to={WalletRoutes.OnboardingImportLegacy}>
              <CardButtonTextContainer>
                <p>{getLocale('wootzWalletImportFromLegacy')}</p>
              </CardButtonTextContainer>
              <LegacyWalletIcon />
            </CardButton>
          )}

          <LinkRow>
            <WalletLink to={WalletRoutes.OnboardingNewWalletCreatePassword}>
              {getLocale('wootzWalletCreateWalletInsteadLink')}
            </WalletLink>
          </LinkRow>
        </StyledWrapper>
      </MainWrapper>
    </CenteredPageLayout>
  )
}
