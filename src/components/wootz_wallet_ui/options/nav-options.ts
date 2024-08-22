// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

// Utils
import { isPersistanceOfPanelProhibited } from '../utils/local-storage-utils'

// Types
import {
  NavOption,
  WalletRoutes,
  AccountPageTabs,
  PanelTypes
} from '../constants/types'

const PANEL_TYPES: PanelTypes[] = [
  'accounts',
  'approveTransaction',
  'assets',
  'buy',
  'connectHardwareWallet',
  'connectWithSite',
  'createAccount',
  'expanded',
  'main',
  'networks',
  'send',
  'settings',
  'sitePermissions',
  'swap',
  'activity', // Transactions
  'transactionStatus'
]

export const isValidPanelNavigationOption = (
  panelName: string
): panelName is PanelTypes => {
  return (
    PANEL_TYPES.includes(panelName as PanelTypes) &&
    !isPersistanceOfPanelProhibited(panelName as PanelTypes)
  )
}

const BridgeOption: NavOption = {
  id: 'bridge',
  name: 'wootzWalletBridge',
  icon: 'web3-bridge',
  route: WalletRoutes.Bridge
}

export const BuySendSwapDepositOptions: NavOption[] = [
  {
    id: 'buy',
    name: 'wootzWalletBuy',
    icon: 'coins-alt1',
    route: WalletRoutes.FundWalletPageStart
  },
  {
    id: 'send',
    name: 'wootzWalletSend',
    icon: 'send',
    route: WalletRoutes.Send
  },
  {
    id: 'swap',
    name: 'wootzWalletSwap',
    icon: 'currency-exchange',
    route: WalletRoutes.Swap
  },
  BridgeOption,
  {
    id: 'deposit',
    name: 'wootzWalletDepositCryptoButton',
    icon: 'money-bag-coins',
    route: WalletRoutes.DepositFundsPageStart
  }
]

const ActivityNavOption: NavOption = {
  id: 'activity',
  name: 'wootzWalletActivity',
  icon: 'activity',
  route: WalletRoutes.Activity
}

const ExploreNavOption: NavOption = {
  id: 'explore',
  name: 'wootzWalletTopNavExplore',
  icon: 'discover',
  route: WalletRoutes.Explore
}

export const PanelNavOptions: NavOption[] = [
  {
    id: 'portfolio',
    name: 'wootzWalletTopNavPortfolio',
    icon: 'coins',
    route: WalletRoutes.Portfolio
  },
  ActivityNavOption,
  {
    id: 'accounts',
    name: 'wootzWalletTopNavAccounts',
    icon: 'user-accounts',
    route: WalletRoutes.Accounts
  },
  ExploreNavOption
]

export const NavOptions: NavOption[] = [
  {
    id: 'portfolio',
    name: 'wootzWalletTopNavPortfolio',
    icon: 'coins',
    route: WalletRoutes.Portfolio
  },
  ActivityNavOption,
  {
    id: 'accounts',
    name: 'wootzWalletTopNavAccounts',
    icon: 'user-accounts',
    route: WalletRoutes.Accounts
  },
  ExploreNavOption
]

export const AllNavOptions: NavOption[] = [
  ...NavOptions,
  ...BuySendSwapDepositOptions,
  ActivityNavOption
]

export const PortfolioNavOptions: NavOption[] = [
  {
    id: 'assets',
    name: 'wootzWalletAccountsAssets',
    icon: 'coins',
    route: WalletRoutes.PortfolioAssets
  },
  {
    id: 'nfts',
    name: 'wootzWalletTopNavNFTS',
    icon: 'grid04',
    route: WalletRoutes.PortfolioNFTs
  }
]

export const ExploreNavOptions: NavOption[] = [
  {
    id: 'market',
    name: 'wootzWalletTopNavMarket',
    icon: 'discover',
    route: WalletRoutes.Market
  },
  {
    id: 'web3',
    name: 'wootzWalletWeb3',
    icon: 'discover',
    route: WalletRoutes.Web3
  }
]

export const PortfolioAssetOptions: NavOption[] = [
  {
    id: 'accounts',
    name: 'wootzWalletTopNavAccounts',
    icon: 'user-accounts',
    route: WalletRoutes.AccountsHash
  },
  {
    id: 'transactions',
    name: 'wootzWalletTransactions',
    icon: 'activity',
    route: WalletRoutes.TransactionsHash
  }
]

export const EditVisibleAssetsOptions: NavOption[] = [
  {
    id: 'my_assets',
    name: 'wootzWalletMyAssets',
    icon: '',
    route: WalletRoutes.MyAssetsHash
  },
  {
    id: 'available_assets',
    name: 'wootzWalletAvailableAssets',
    icon: '',
    route: WalletRoutes.AvailableAssetsHash
  }
]

export const CreateAccountOptions: NavOption[] = [
  {
    id: 'accounts',
    name: 'wootzWalletCreateAccountButton',
    icon: 'plus-add',
    route: WalletRoutes.CreateAccountModalStart
  },
  {
    id: 'accounts',
    name: 'wootzWalletImportAccount',
    icon: 'product-wootz-wallet',
    route: WalletRoutes.ImportAccountModalStart
  },
  {
    id: 'accounts',
    name: 'wootzWalletConnectHardwareWallet',
    icon: 'flashdrive',
    route: WalletRoutes.AddHardwareAccountModalStart
  }
]

export const AccountDetailsOptions: NavOption[] = [
  {
    id: 'assets',
    name: 'wootzWalletAccountsAssets',
    icon: '',
    route: AccountPageTabs.AccountAssetsSub
  },
  {
    id: 'nfts',
    name: 'wootzWalletTopNavNFTS',
    icon: '',
    route: AccountPageTabs.AccountNFTsSub
  },
  {
    id: 'transactions',
    name: 'wootzWalletTransactions',
    icon: '',
    route: AccountPageTabs.AccountTransactionsSub
  }
]
