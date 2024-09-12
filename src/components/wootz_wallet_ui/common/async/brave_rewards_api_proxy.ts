// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import {
  UserType,
  userTypeFromString
} from '../../../wootz_rewards/resources/shared/lib/user_type'

import {
  externalWalletFromExtensionData //
} from '../../../wootz_rewards/resources/shared/lib/external_wallet'
import { RewardsExternalWallet, WalletStatus } from '../../constants/types'
import { WootzRewardsProxyOverrides } from '../../constants/testing_types'

export class WootzRewardsProxy {
  getRewardsEnabled = () => {
    return new Promise<boolean>((resolve) =>
      chrome.wootzRewards.getRewardsEnabled((enabled) => {
        resolve(enabled)
      })
    )
  }

  fetchBalance = () => {
    return new Promise<number | undefined>((resolve) =>
      chrome.wootzRewards.fetchBalance((balance) => {
        resolve(balance)
      })
    )
  }

  getBalanceReport = (month: number, year: number) => {
    return new Promise<RewardsExtension.BalanceReport>((resolve) =>
      chrome.wootzRewards.getBalanceReport(month, year, (report) => {
        resolve(report)
      })
    )
  }

  getUserType = () => {
    return new Promise<UserType>((resolve) => {
      chrome.wootzRewards.getUserType((userType) => {
        resolve(userTypeFromString(userType))
      })
    })
  }

  getWalletExists = () => {
    return new Promise<boolean>((resolve) => {
      chrome.wootzRewards.getWalletExists((exists) => {
        resolve(exists)
      })
    })
  }

  getExternalWallet = () => {
    return new Promise<RewardsExternalWallet | null>((resolve) => {
      chrome.wootzRewards.getExternalWallet((data) => {
        const externalWallet = externalWalletFromExtensionData(data)
        const rewardsWallet: RewardsExternalWallet | null = externalWallet
          ? {
              ...externalWallet,
              status: externalWallet.authenticated
                ? WalletStatus.kConnected
                : WalletStatus.kLoggedOut
            }
          : null
        resolve(rewardsWallet)
      })
    })
  }

  isInitialized = () => {
    return new Promise<boolean>((resolve) => {
      chrome.wootzRewards.isInitialized((initialized) => {
        resolve(initialized)
      })
    })
  }

  isSupported = () => {
    return new Promise<boolean>((resolve) => {
      chrome.wootzRewards.isSupported((isSupported) => {
        resolve(isSupported)
      })
    })
  }

  onCompleteReset = chrome.wootzRewards.onCompleteReset.addListener

  onExternalWalletConnected =
    chrome.wootzRewards.onExternalWalletConnected.addListener

  onExternalWalletLoggedOut =
    chrome.wootzRewards.onExternalWalletLoggedOut.addListener

  onPublisherData = chrome.wootzRewards.onPublisherData.addListener

  onPublisherListNormalized =
    chrome.wootzRewards.onPublisherListNormalized.addListener

  onReconcileComplete = chrome.wootzRewards.onReconcileComplete.addListener

  onRewardsWalletCreated =
    chrome.wootzRewards.onRewardsWalletCreated.addListener

  openRewardsPanel = chrome.wootzRewards.openRewardsPanel

  onInitialized = (callback: () => any) =>
    chrome.wootzRewards.initialized.addListener((error) => {
      if (error === RewardsExtension.Result.OK) {
        callback()
      } else {
        console.error(`rewards onInitialized error: ${error}`)
      }
    })

  getAvailableCountries = () => {
    return new Promise<string[]>((resolve) =>
      chrome.wootzRewards.getAvailableCountries((countries) => {
        resolve(countries)
      })
    )
  }

  getRewardsParameters = () => {
    return new Promise<RewardsExtension.RewardsParameters>((resolve) =>
      chrome.wootzRewards.getRewardsParameters((params) => {
        resolve(params)
      })
    )
  }

  getAllNotifications = () => {
    return new Promise<RewardsExtension.Notification[]>((resolve) => {
      chrome.wootzRewards.getAllNotifications((notifications) => {
        resolve(notifications)
      })
    })
  }
}

export type WootzRewardsProxyInstance = InstanceType<typeof WootzRewardsProxy>

let wootzRewardsProxyInstance: WootzRewardsProxyInstance

export const getWootzRewardsProxy = () => {
  if (!wootzRewardsProxyInstance) {
    wootzRewardsProxyInstance = new WootzRewardsProxy()
  }

  return wootzRewardsProxyInstance
}

/** For testing */
export function resetRewardsProxy(
  overrides?: WootzRewardsProxyOverrides | undefined
) {
  // no-op in production
}
