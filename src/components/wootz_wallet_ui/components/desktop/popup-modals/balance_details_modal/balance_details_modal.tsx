// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import Tooltip from '@wootz/leo/react/tooltip'

// Types
import {
  WootzWallet,
  BitcoinBalances //
} from '../../../../constants/types'

// Utils
import { getLocale } from '../../../../../common/locale'
import Amount from '../../../../utils/amount'

// Components
import { PopupModal } from '../index'
import LoadingSkeleton from '../../../shared/loading-skeleton'

// Styled Components
import {
  BalancesColumn,
  SectionText,
  BalanceText,
  InfoIcon,
  TooltipContent
} from './balance_details_modal.style'
import { Column, Row, VerticalDivider } from '../../../shared/style'

interface Props {
  token: WootzWallet.BlockchainToken
  balances?: BitcoinBalances
  isLoadingBalances?: boolean
  onClose: () => void
}

export const BalanceDetailsModal = React.forwardRef<HTMLDivElement, Props>(
  (props: Props, forwardedRef) => {
    const { onClose, token, balances, isLoadingBalances } = props

    return (
      <PopupModal
        title={getLocale('wootzWalletDetails')}
        onClose={onClose}
        width='560px'
        ref={forwardedRef}
        headerPaddingHorizontal={'32px'}
        headerPaddingVertical={'32px'}
      >
        <Column
          fullHeight={true}
          fullWidth={true}
          justifyContent='flex-start'
          padding='0px 32px 32px 32px'
        >
          <BalancesColumn fullWidth={true}>
            <Row justifyContent='space-between'>
              <Row
                width='unset'
                gap='4px'
              >
                <SectionText textSize='14px'>
                  {getLocale('wootzWalletAvailable')}:
                </SectionText>
                <Tooltip
                  mode='default'
                  placement='top'
                >
                  <TooltipContent slot='content'>
                    {getLocale('wootzWalletAvailableBalanceDescription')}
                  </TooltipContent>
                  <InfoIcon />
                </Tooltip>
              </Row>
              {isLoadingBalances ? (
                <Column>
                  <LoadingSkeleton
                    width={100}
                    height={22}
                  />
                </Column>
              ) : (
                <BalanceText textSize='14px'>
                  {new Amount(balances?.availableBalance ?? '')
                    .divideByDecimals(token.decimals)
                    .formatAsAsset(6, token.symbol)}
                </BalanceText>
              )}
            </Row>

            <VerticalDivider />

            <Row justifyContent='space-between'>
              <Row
                width='unset'
                gap='4px'
              >
                <SectionText textSize='14px'>
                  {getLocale('wootzWalletPending')}:
                </SectionText>
                <Tooltip
                  mode='default'
                  placement='top'
                >
                  <TooltipContent slot='content'>
                    {getLocale('wootzWalletPendingBalanceDescription')}
                  </TooltipContent>
                  <InfoIcon />
                </Tooltip>
              </Row>
              {isLoadingBalances ? (
                <Column>
                  <LoadingSkeleton
                    width={100}
                    height={22}
                  />
                </Column>
              ) : (
                <BalanceText textSize='14px'>
                  {new Amount(balances?.pendingBalance ?? '')
                    .divideByDecimals(token.decimals)
                    .formatAsAsset(6, token.symbol)}
                </BalanceText>
              )}
            </Row>

            <VerticalDivider />

            <Row justifyContent='space-between'>
              <Row
                width='unset'
                gap='4px'
              >
                <SectionText
                  textSize='14px'
                  isBold={true}
                >
                  {getLocale('wootzWalletConfirmTransactionTotal')}:
                </SectionText>
                <Tooltip
                  mode='default'
                  placement='top'
                >
                  <TooltipContent slot='content'>
                    {getLocale('wootzWalletTotalBalanceDescription')}
                  </TooltipContent>
                  <InfoIcon />
                </Tooltip>
              </Row>
              {isLoadingBalances ? (
                <Column>
                  <LoadingSkeleton
                    width={100}
                    height={22}
                  />
                </Column>
              ) : (
                <BalanceText
                  textSize='14px'
                  isBold={true}
                >
                  {new Amount(balances?.totalBalance ?? '')
                    .divideByDecimals(token.decimals)
                    .formatAsAsset(6, token.symbol)}
                </BalanceText>
              )}
            </Row>
          </BalancesColumn>
        </Column>
      </PopupModal>
    )
  }
)
