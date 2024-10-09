// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// Types
import { WalletRoutes } from '../../../constants/types'

// Selectors
import { UISelectors } from '../../../common/selectors'

// Components
import { DefaultPanelHeader } from './default-panel-header'

// Utils
import { getLocale } from '../../../../common/locale'

// Hooks
import { useSafeUISelector } from '../../../common/hooks/use-safe-selector'

// Styled Components
import { SearchBarWrapper } from './activity_page_header.style'
import { HeaderTitle } from './shared-card-headers.style'
import { Row } from '../../shared/style'
import SearchBar from '../../shared/search-bar'

export interface Props {
  searchValue: string
  onSearchValueChange?: React.ChangeEventHandler<HTMLInputElement> | undefined
}

export const ActivityPageHeader = (props: Props) => {
  const { searchValue, onSearchValueChange } = props

  // UI Selectors (safe)
  const isPanel = useSafeUISelector(UISelectors.isPanel)

  return isPanel ? (
    <DefaultPanelHeader
      title={getLocale('wootzWalletActivity')}
      expandRoute={WalletRoutes.Activity}
    />
  ) : (
    <Row
      padding='24px 0px'
      justifyContent='space-between'
    >
      <HeaderTitle>{getLocale('wootzWalletActivity')}</HeaderTitle>
      <SearchBarWrapper alignItems='flex-start'>
        <SearchBar
          placeholder={getLocale('wootzWalletSearchText')}
          action={onSearchValueChange}
          value={searchValue}
          isV2={true}
        />
      </SearchBarWrapper>
    </Row>
  )
}

export default ActivityPageHeader
