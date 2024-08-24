// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { useLocation, useHistory } from 'react-router-dom'
import Toggle from '@brave/leo/react/toggle'

// Selectors
import { UISelectors } from '../../../common/selectors'
import { useSafeUISelector } from '../../../common/hooks/use-safe-selector'

// Types
import {
  AccountPageTabs,
  WootzWallet,
  WalletRoutes
} from '../../../constants/types'

// Constants
import {
  LOCAL_STORAGE_KEYS //
} from '../../../common/constants/local-storage-keys'

// Options
import { CreateAccountOptions } from '../../../options/nav-options'

// Utils
import { getLocale } from '../../../../common/locale'
import {
  useGetSelectedChainQuery,
  useLockWalletMutation
} from '../../../common/slices/api.slice'
import { openWalletSettings } from '../../../utils/routes-utils'
import { useSyncedLocalStorage } from '../../../common/hooks/use_local_storage'

// Styled Components
import {
  StyledWrapper,
  PopupButton,
  PopupButtonText,
  ButtonIcon,
  ToggleRow,
  SectionLabel
} from './wellet-menus.style'
import { VerticalDivider, Row, Column } from '../../shared/style'

export interface Props {
  onClosePopup?: () => void
  yPosition?: number
}

export const WalletSettingsMenu = (props: Props) => {
  const { onClosePopup, yPosition } = props

  // Selectors
  const isPanel = useSafeUISelector(UISelectors.isPanel)

  // Routing
  const history = useHistory()
  const { pathname: walletLocation } = useLocation()

  // local-storage
  const [hidePortfolioBalances, setHidePortfolioBalances] =
    useSyncedLocalStorage(LOCAL_STORAGE_KEYS.HIDE_PORTFOLIO_BALANCES, false)
  const [hidePortfolioNFTsTab, setHidePortfolioNFTsTab] = useSyncedLocalStorage(
    LOCAL_STORAGE_KEYS.HIDE_PORTFOLIO_NFTS_TAB,
    false
  )
  const [hidePortfolioGraph, setHidePortfolioGraph] = useSyncedLocalStorage(
    LOCAL_STORAGE_KEYS.IS_PORTFOLIO_OVERVIEW_GRAPH_HIDDEN,
    false
  )

  // queries
  const { data: selectedNetwork } = useGetSelectedChainQuery()

  // mutations
  const [lockWallet] = useLockWalletMutation()

  // methods
  const onClickConnectedSites = React.useCallback(() => {
    if (!selectedNetwork) {
      return
    }

    const route =
      selectedNetwork.coin === WootzWallet.CoinType.ETH ? 'ethereum' : 'solana'

    chrome.tabs.create({ url: `wootz://settings/content/${route}` }, () => {
      if (chrome.runtime.lastError) {
        console.error('tabs.create failed: ' + chrome.runtime.lastError.message)
      }
    })
    if (onClosePopup) {
      onClosePopup()
    }
  }, [selectedNetwork, onClosePopup])

  const onClickHelpCenter = React.useCallback(() => {
    chrome.tabs.create(
      {
        url: 'https://support.wootz.com/hc/en-us/categories/360001059151-Wootz-Wallet'
      },
      () => {
        if (chrome.runtime.lastError) {
          console.error(
            'tabs.create failed: ' + chrome.runtime.lastError.message
          )
        }
      }
    )
    if (onClosePopup) {
      onClosePopup()
    }
  }, [onClosePopup])

  const onClickSettings = React.useCallback(() => {
    openWalletSettings()
    if (onClosePopup) {
      onClosePopup()
    }
  }, [onClosePopup])

  // Methods
  const onToggleHideGraph = React.useCallback(() => {
    setHidePortfolioGraph((prev) => !prev)
  }, [setHidePortfolioGraph])

  const onToggleHideBalances = React.useCallback(() => {
    setHidePortfolioBalances((prev) => !prev)
  }, [setHidePortfolioBalances])

  const onToggleHideNFTsTab = React.useCallback(() => {
    if (walletLocation.includes(WalletRoutes.PortfolioNFTs)) {
      history.push(WalletRoutes.PortfolioAssets)
    }
    setHidePortfolioNFTsTab((prev) => !prev)
  }, [history, setHidePortfolioNFTsTab, walletLocation])

  const onClickRoute = (route: WalletRoutes | AccountPageTabs) => {
    if (route === WalletRoutes.AddHardwareAccountModalStart && isPanel) {
      chrome.tabs.create({ url: `wootzapp://wallet${route}` }, () => {
        if (chrome.runtime.lastError) {
          console.error(
            'tabs.create failed: ' + chrome.runtime.lastError.message
          )
        }
      })
      return
    }
    history.push(route)
  }

  const onClickBackup = () => {
    if (isPanel) {
      chrome.tabs.create(
        {
          url: `wootzapp://wallet${WalletRoutes.Backup}`
        },
        () => {
          if (chrome.runtime.lastError) {
            console.error(
              'tabs.create failed: ' + chrome.runtime.lastError.message
            )
          }
        }
      )
      return
    }
    history.push(WalletRoutes.Backup)
  }

  return (
    <StyledWrapper
      yPosition={yPosition}
      padding='0px'
    >
      <Column
        fullWidth={true}
        padding='8px 8px 0px 8px'
      >
        <PopupButton
          onClick={async () => {
            await lockWallet()
          }}
        >
          <ButtonIcon name='lock' />
          <PopupButtonText>
            {getLocale('wootzWalletWalletPopupLock')}
          </PopupButtonText>
        </PopupButton>

        <PopupButton onClick={onClickBackup}>
          <ButtonIcon name='safe' />
          <PopupButtonText>
            {getLocale('wootzWalletBackupButton')}
          </PopupButtonText>
        </PopupButton>

        {(selectedNetwork?.coin === WootzWallet.CoinType.ETH ||
          selectedNetwork?.coin === WootzWallet.CoinType.SOL) && (
          <PopupButton onClick={onClickConnectedSites}>
            <ButtonIcon name='link-normal' />
            <PopupButtonText>
              {getLocale('wootzWalletWalletPopupConnectedSites')}
            </PopupButtonText>
          </PopupButton>
        )}

        <PopupButton onClick={onClickSettings}>
          <ButtonIcon name='settings' />
          <PopupButtonText>
            {getLocale('wootzWalletWalletPopupSettings')}
          </PopupButtonText>
        </PopupButton>
      </Column>

      {(walletLocation === WalletRoutes.PortfolioNFTs ||
        walletLocation === WalletRoutes.PortfolioAssets) && (
        <>
          <SectionLabel justifyContent='flex-start'>
            {getLocale('wootzWalletPortfolioSettings')}
          </SectionLabel>
          <Column
            fullWidth={true}
            padding='8px 8px 0px 8px'
          >
            <ToggleRow onClick={onToggleHideBalances}>
              <Row>
                <ButtonIcon name='eye-on' />
                <PopupButtonText>
                  {getLocale('wootzWalletWalletPopupHideBalances')}
                </PopupButtonText>
                <Toggle
                  checked={!hidePortfolioBalances}
                  onChange={onToggleHideBalances}
                  size='small'
                />
              </Row>
            </ToggleRow>

            <ToggleRow onClick={onToggleHideGraph}>
              <Row>
                <ButtonIcon name='graph' />
                <PopupButtonText>
                  {getLocale('wootzWalletWalletPopupShowGraph')}
                </PopupButtonText>
              </Row>
              <Toggle
                checked={!hidePortfolioGraph}
                onChange={onToggleHideGraph}
                size='small'
              />
            </ToggleRow>

            <ToggleRow onClick={onToggleHideNFTsTab}>
              <Row>
                <ButtonIcon name='nft' />
                <PopupButtonText>
                  {getLocale('wootzWalletWalletNFTsTab')}
                </PopupButtonText>
              </Row>
              <Toggle
                checked={!hidePortfolioNFTsTab}
                onChange={onToggleHideNFTsTab}
                size='small'
              />
            </ToggleRow>
          </Column>
        </>
      )}

      {walletLocation === WalletRoutes.Accounts && isPanel && (
        <>
          <SectionLabel justifyContent='flex-start'>
            {getLocale('wootzWalletAccountSettings')}
          </SectionLabel>
          <Column
            fullWidth={true}
            padding='8px 8px 0px 8px'
          >
            {CreateAccountOptions.map((option) => (
              <PopupButton
                key={option.name}
                onClick={() => onClickRoute(option.route)}
                minWidth={240}
              >
                <ButtonIcon name={option.icon} />
                <PopupButtonText>{getLocale(option.name)}</PopupButtonText>
              </PopupButton>
            ))}
          </Column>
        </>
      )}

      <VerticalDivider />

      <Column
        fullWidth={true}
        padding='8px 8px 0px 8px'
      >
        <PopupButton onClick={onClickHelpCenter}>
          <ButtonIcon name='help-outline' />
          <PopupButtonText>
            {getLocale('wootzWalletHelpCenter')}
          </PopupButtonText>
        </PopupButton>
      </Column>
    </StyledWrapper>
  )
}
