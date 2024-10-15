// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'

import { getLocale } from '$web-common/locale'

import { Panel } from '../../panel/index'
import { NavButton } from '../../buttons/nav-button/index'
import {
  ErrorIcon,
  Title,
  ErrorDetailTitle,
  ErrorDetailContent,
  ErrorDetailContentContainer
} from './failed.style'
import { ButtonRow, TransactionStatusDescription } from '../common/common.style'
import { PopupModal } from '../../popup-modals'

interface Props {
  headerTitle: string
  isPrimaryCTADisabled: boolean
  customDescription: string | undefined
  errorDetailTitle: string
  errorDetailContent?: string | undefined
  onClose: () => void
  onClickPrimaryCTA: () => void
}

export const TransactionFailed = (props: Props) => {
  const {
    headerTitle,
    errorDetailTitle,
    errorDetailContent,
    isPrimaryCTADisabled,
    customDescription,
    onClose,
    onClickPrimaryCTA
  } = props

  const [showErrorCodeModal, setShowErrorCodeModal] =
    React.useState<boolean>(false)

  const description = customDescription ||
      getLocale('wootzWalletTransactionFailedDescription')

  return (
    <Panel
      navAction={onClose}
      title={headerTitle}
      headerStyle='slim'
    >
      <ErrorIcon />
      <Title>{getLocale('wootzWalletTransactionFailedTitle')}</Title>
      <TransactionStatusDescription>
        {description}
      </TransactionStatusDescription>
      <ButtonRow>
        {errorDetailContent && (
          <NavButton
            buttonType='secondary'
            text={getLocale('wootzWalletTransactionFailedViewErrorCTA')}
            onSubmit={() => setShowErrorCodeModal(true)}
          />
        )}
        <NavButton
          buttonType='primary'
          text={getLocale('wootzWalletButtonClose')}
          onSubmit={onClickPrimaryCTA}
          disabled={isPrimaryCTADisabled}
        />
      </ButtonRow>

      {showErrorCodeModal && errorDetailContent && (
        <ErrorDetail
          title={errorDetailTitle}
          content={errorDetailContent}
          onClose={() => setShowErrorCodeModal(false)}
        />
      )}
    </Panel>
  )
}

interface ModalProps {
  title: string
  content: string
  onClose: () => void
}

const ErrorDetail = (props: ModalProps) => {
  const { content, title, onClose } = props

  return (
    <PopupModal
      title={getLocale('wootzWalletTransactionFailedModalTitle')}
      onClose={onClose}
    >
      <ErrorDetailTitle>{title}</ErrorDetailTitle>

      <ErrorDetailContentContainer>
        <ErrorDetailContent>{content}</ErrorDetailContent>
      </ErrorDetailContentContainer>

      <ButtonRow>
        <NavButton
          buttonType='primary'
          text={getLocale('wootzWalletButtonClose')}
          onSubmit={onClose}
        />
      </ButtonRow>
    </PopupModal>
  )
}
