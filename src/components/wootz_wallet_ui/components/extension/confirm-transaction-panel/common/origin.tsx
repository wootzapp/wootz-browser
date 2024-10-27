// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// Images
import WootzIcon from '../../../../assets/svg-icons/wootz-icon.svg'

// Components
import { SiteOrigin } from '../../../shared/create-site-origin'
import {
  ChainInfo,
  InlineViewOnBlockExplorerIconButton
} from './view_on_explorer_button'
import { CopyTooltip } from '../../../shared/copy-tooltip/copy-tooltip'

// Styled Components
import { Column, IconsWrapper, Row } from '../../../shared/style'
import { FavIcon } from './style'
import { URLText } from '../../shared-panel-styles'
import {
  ContractOriginColumn,
  InlineContractRow,
  OriginIndicatorIconWrapper,
  OriginURLText,
  OriginWarningIndicator
} from './origin.style'

// Types
import { WootzWallet } from '../../../../constants/types'

// Utils
import { reduceAddress } from '../../../../utils/reduce-address'
import {
  getIsWootzWalletOrigin,
  isComponentInStorybook
} from '../../../../utils/string-utils'
import { getLocale } from '../../../../../common/locale'

interface Props {
  originInfo: WootzWallet.OriginInfo
}

const isStorybook = isComponentInStorybook()

export function Origin(props: Props) {
  const { originInfo } = props
  return (
    <>
      <FavIcon
        src={
          getIsWootzWalletOrigin(originInfo)
            ? WootzIcon
            : isStorybook
            ? `${originInfo.originSpec}/favicon.png`
            : `chrome://favicon/size/64@1x/${originInfo.originSpec}`
        }
      />
      <URLText>
        <SiteOrigin
          originSpec={originInfo.originSpec}
          eTldPlusOne={originInfo.eTldPlusOne}
        />
      </URLText>
    </>
  )
}

export function TransactionOrigin({
  contractAddress,
  originInfo,
  isFlagged,
  network
}: Props & {
  contractAddress?: string
  network: ChainInfo
  isFlagged?: boolean
}) {
  // computed
  const isWootzWalletOrigin = getIsWootzWalletOrigin(originInfo)

  // render
  return (
    <Row
      alignItems='center'
      justifyContent='flex-start'
      padding={'16px 0px'}
    >
      <Column
        width='30px'
        height='30px'
        alignItems='stretch'
        justifyContent='stretch'
        margin={'0px 8px 0px 0px'}
      >
        <IconsWrapper marginRight='0px'>
          <FavIcon
            height='30px'
            src={
              isWootzWalletOrigin
                ? WootzIcon
                : isStorybook
                ? `${originInfo.originSpec}/favicon.png`
                : `chrome://favicon/size/64@1x/${originInfo.originSpec}`
            }
          />
          {!isWootzWalletOrigin && isFlagged && (
            <OriginIndicatorIconWrapper>
              <OriginWarningIndicator />
            </OriginIndicatorIconWrapper>
          )}
        </IconsWrapper>
      </Column>
      <ContractOriginColumn
        alignItems='flex-start'
        justifyContent='flex-start'
        gap={'4px'}
      >
        <OriginURLText>
          <SiteOrigin
            originSpec={originInfo.originSpec}
            eTldPlusOne={originInfo.eTldPlusOne}
          />
        </OriginURLText>
        {contractAddress ? (
          <InlineContractRow>
            <span>{getLocale('wootzWalletContract')}</span>

            <CopyTooltip
              isAddress
              text={contractAddress}
              tooltipText={contractAddress}
            >
              <InlineContractRow>
                <span>{reduceAddress(contractAddress)}</span>
                <InlineViewOnBlockExplorerIconButton
                  address={contractAddress}
                  network={network}
                  urlType='contract'
                />
              </InlineContractRow>
            </CopyTooltip>
          </InlineContractRow>
        ) : null}
      </ContractOriginColumn>
    </Row>
  )
}
