// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// types
import { WootzWallet } from '../../../constants/types'

// hooks
import { useGetVisibleNetworksQuery } from '../../../common/slices/api.slice'

// components
import { SelectNetworkItem } from '../select-network-item/index'

interface Props {
  onSelectCustomNetwork: (network: WootzWallet.NetworkInfo) => void
  selectedNetwork: WootzWallet.NetworkInfo | undefined | null
  customNetwork?: WootzWallet.NetworkInfo
  networkListSubset?: WootzWallet.NetworkInfo[]
}

export function SelectNetwork({
  onSelectCustomNetwork,
  selectedNetwork,
  customNetwork,
  networkListSubset
}: Props) {
  // queries
  const { data: visibleNetworks = [] } = useGetVisibleNetworksQuery(undefined, {
    skip: !!networkListSubset
  })

  // Computed
  const networks = networkListSubset ?? visibleNetworks

  // memos
  const networksList = React.useMemo(() => {
    if (customNetwork) {
      return [customNetwork, ...networks]
    }
    return networks
  }, [networks, customNetwork])

  // render
  return (
    <>
      {networksList.map((network) => (
        <SelectNetworkItem
          selectedNetwork={selectedNetwork}
          key={`${network.chainId}-${network.coin}`}
          network={network}
          onSelectCustomNetwork={onSelectCustomNetwork}
        />
      ))}
    </>
  )
}

export default SelectNetwork
