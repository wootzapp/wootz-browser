// Copyright (c) 2023 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// types
import { WalletApiEndpointBuilderParams } from '../api-base.slice'
import { WootzRewardsInfo } from '../../../constants/types'

// utils
import { handleEndpointError } from '../../../utils/api-utils'

export function wootzRewardsApiEndpoints({
  mutation,
  query
}: WalletApiEndpointBuilderParams) {
  return {
    getRewardsInfo: query<WootzRewardsInfo, void>({
      queryFn: async (arg, { endpoint }, extraOptions, baseQuery) => {
        try {
          const { cache } = baseQuery(undefined)
          return {
            data: cache.rewardsInfo || (await cache.getWootzRewardsInfo())
          }
        } catch (error) {
          return handleEndpointError(
            endpoint,
            'Failed to get Wootz Rewards information',
            error
          )
        }
      },
      providesTags: ['WootzRewards-Info']
    })
  } as const
}
