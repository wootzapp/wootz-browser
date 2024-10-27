// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { useHistory } from 'react-router'
import { useDispatch, useSelector } from 'react-redux'

// redux
import { WalletPageActions } from '../../../actions'
import { PageSelectors } from '../../../selectors'

// utils
import { getLocale } from '../../../../../common/locale'
import {
  useReportOnboardingActionMutation //
} from '../../../../common/slices/api.slice'

// components
import { WelcomeAction } from '../components/welcome_action/welcome_action'

// routes
import { WootzWallet, WalletRoutes } from '../../../../constants/types'

// styles
import * as leo from '@brave/leo/tokens/css/variables'
import { Row, VerticalSpace } from '../../../../components/shared/style'
import {
  WootzIcon,
  Content,
  WelcomePageBackground,
  WelcomePageWrapper,
  Title,
  Heading,
  ActionsContainer,
  Footer,
  SubHeading
} from './onboarding-welcome.style'

const walletIcons = [
  'wootz-icon-release-color',
  'phantom-color',
  'metamask-color',
  'coinbase-color',
  'trezor-color',
  'wallet-ledger'
]

export const OnboardingWelcome = () => {
  // routing
  const history = useHistory()

  // redux
  const dispatch = useDispatch()
  const setupStillInProgress = useSelector(PageSelectors.setupStillInProgress)

  // mutations
  const [report] = useReportOnboardingActionMutation()

  // effects
  React.useEffect(() => {
    // start wallet setup
    if (!setupStillInProgress) {
      report(WootzWallet.OnboardingAction.Shown)
      dispatch(WalletPageActions.walletSetupComplete(false))
    }
  }, [setupStillInProgress, report, dispatch])

  return (
    <>
      <WelcomePageBackground />
      <WelcomePageWrapper>
        <Content>
          <Row
            gap='10px'
            justifyContent='flex-start'
            marginBottom={leo.spacing['5Xl']}
          >
            <WootzIcon />
            <Title>{getLocale('wootzWalletTitle')}</Title>
          </Row>
          <Heading>{getLocale('wootzWalletWelcomeTitle')}</Heading>
          <SubHeading>{getLocale('wootzWalletWelcomeDescription')}</SubHeading>

          <ActionsContainer>
            <WelcomeAction
              title={getLocale('wootzWalletWelcomeNewWalletTitle')}
              description={getLocale('wootzWalletWelcomeNewWalletDescription')}
              iconName='plus-add'
              onSelect={() =>
                history.push(WalletRoutes.OnboardingNewWalletTerms)
              }
            />

            <WelcomeAction
              title={getLocale('wootzWalletWelcomeImportWalletTitle')}
              description={getLocale(
                'wootzWalletWelcomeImportWalletDescription'
              )}
              iconName='import-arrow'
              walletIcons={walletIcons}
              onSelect={() =>
                history.push(WalletRoutes.OnboardingImportSelectWalletType)
              }
            />
          </ActionsContainer>
          <VerticalSpace space='96px' />
          <Footer>{getLocale('wootzWalletCopyright')}</Footer>
        </Content>
      </WelcomePageWrapper>
    </>
  )
}
