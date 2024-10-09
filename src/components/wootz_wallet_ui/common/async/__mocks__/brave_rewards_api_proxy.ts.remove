// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import { WootzRewardsProxyOverrides } from '../../../constants/testing_types'
import type { WootzRewardsProxyInstance } from '../wootz_rewards_api_proxy'

export class MockWootzRewardsProxy {
  overrides: WootzRewardsProxyOverrides = {
    rewardsEnabled: true,
    balance: 100.5,
    externalWallet: null
  }

  applyOverrides = (overrides?: WootzRewardsProxyOverrides) => {
    if (!overrides) {
      return
    }

    this.overrides = { ...this.overrides, ...overrides }
  }

  getRewardsEnabled = async () => {
    return this.overrides.rewardsEnabled
  }

  fetchBalance = async () => {
    return this.overrides.balance
  }

  getExternalWallet = async () => {
    return this.overrides.externalWallet
  }
}

export type MockWootzRewardsProxyInstance = InstanceType<
  typeof MockWootzRewardsProxy
>

let wootzRewardsProxyInstance: MockWootzRewardsProxyInstance

export const getWootzRewardsProxy = () => {
  if (!wootzRewardsProxyInstance) {
    wootzRewardsProxyInstance = new MockWootzRewardsProxy()
  }

  return wootzRewardsProxyInstance as unknown as WootzRewardsProxyInstance &
    MockWootzRewardsProxy
}

export function resetRewardsProxy(
  overrides?: WootzRewardsProxyOverrides | undefined
) {
  wootzRewardsProxyInstance = new MockWootzRewardsProxy()
  wootzRewardsProxyInstance.applyOverrides(overrides)
}
