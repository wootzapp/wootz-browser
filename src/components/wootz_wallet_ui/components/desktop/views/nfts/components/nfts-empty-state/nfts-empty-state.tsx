// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'

// utils
import { getLocale } from '../../../../../../../common/locale'

// styles
import {
  DisclaimerText,
  EmptyStateImage,
  Heading,
  StyledWrapper,
  SubHeading,
  ImportButton
} from './nfts-empty-state.style'
import EmptyStateGraphicLight from '../../../../../../assets/png-icons/nft-empty-state-light.png'
import EmptyStateGraphicDark from '../../../../../../assets/png-icons/nft-empty-state-dark.png'

interface Props {
  onImportNft: () => void
}

export const NftsEmptyState = ({ onImportNft }: Props) => (
  <StyledWrapper>
    <EmptyStateImage
      src={
        window.matchMedia('(prefers-color-scheme: dark)').matches
          ? EmptyStateGraphicDark
          : EmptyStateGraphicLight
      }
    />
    <Heading>{getLocale('wootzNftsTabEmptyStateHeading')}</Heading>
    <SubHeading>{getLocale('wootzNftsTabEmptyStateSubHeading')}</SubHeading>
    <ImportButton onClick={onImportNft}>
      {getLocale('wootzNftsTabImportNft')}
    </ImportButton>
    <DisclaimerText>
      {getLocale('wootzNftsTabEmptyStateDisclaimer')}
    </DisclaimerText>
  </StyledWrapper>
)
