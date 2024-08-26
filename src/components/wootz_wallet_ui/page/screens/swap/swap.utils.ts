// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.
import { WootzWallet, SpotPriceRegistry } from '../../../constants/types'
import {
  LiquiditySource,
  Providers,
  QuoteOption,
  RouteTagsType
} from './constants/types'

// Constants
import LPMetadata from './constants/LpMetadata'

// Utils
import Amount from '../../../utils/amount'
import { getTokenPriceAmountFromRegistry } from '../../../utils/pricing-utils'
import { makeNetworkAsset } from '../../../options/asset-options'

function getZeroExNetworkFee({
  quote,
  fromNetwork
}: {
  quote: WootzWallet.ZeroExQuote
  fromNetwork: WootzWallet.NetworkInfo
}): Amount {
  if (!fromNetwork) {
    return Amount.empty()
  }

  return new Amount(quote.gasPrice)
    .times(quote.gas)
    .divideByDecimals(fromNetwork.decimals)
}

export function getZeroExFromAmount({
  quote,
  fromToken
}: {
  quote: WootzWallet.ZeroExQuote
  fromToken: WootzWallet.BlockchainToken
}): Amount {
  return new Amount(quote.sellAmount).divideByDecimals(fromToken.decimals)
}

export function getZeroExToAmount({
  quote,
  toToken
}: {
  quote: WootzWallet.ZeroExQuote
  toToken: WootzWallet.BlockchainToken
}): Amount {
  return new Amount(quote.buyAmount).divideByDecimals(toToken.decimals)
}

export function getZeroExQuoteOptions({
  quote,
  fromNetwork,
  fromToken,
  toToken,
  spotPrices,
  defaultFiatCurrency
}: {
  quote: WootzWallet.ZeroExQuote
  fromNetwork: WootzWallet.NetworkInfo
  fromToken: WootzWallet.BlockchainToken
  toToken: WootzWallet.BlockchainToken
  spotPrices: SpotPriceRegistry
  defaultFiatCurrency: string
}): QuoteOption[] {
  const networkFee = getZeroExNetworkFee({ quote, fromNetwork })

  return [
    {
      fromAmount: new Amount(quote.sellAmount).divideByDecimals(
        fromToken.decimals
      ),
      toAmount: getZeroExToAmount({ quote, toToken }),
      minimumToAmount: undefined,
      fromToken,
      toToken,
      rate: new Amount(quote.buyAmount)
        .divideByDecimals(toToken.decimals)
        .div(new Amount(quote.sellAmount).divideByDecimals(fromToken.decimals)),
      impact: new Amount(quote.estimatedPriceImpact),
      sources: quote.sources
        .map((source) => ({
          name: source.name,
          proportion: new Amount(source.proportion)
        }))
        .filter((source) => source.proportion.gt(0)),
      routing: 'split', // 0x supports split routing only
      networkFee,
      networkFeeFiat: networkFee.isUndefined()
        ? ''
        : networkFee
            .times(
              getTokenPriceAmountFromRegistry(
                spotPrices,
                makeNetworkAsset(fromNetwork)
              )
            )
            .formatAsFiat(defaultFiatCurrency),
      provider: '0x',
      // There is only 1 quote returned for Ox
      // making it the Fastest and Cheapest.
      tags: ['FASTEST', 'CHEAPEST'],
      id: getUniqueRouteId('0x')
    }
  ]
}

function getJupiterNetworkFee({
  quote,
  fromNetwork
}: {
  quote: WootzWallet.JupiterQuote
  fromNetwork: WootzWallet.NetworkInfo
}): Amount {
  if (!fromNetwork) {
    return Amount.empty()
  }

  return new Amount('0.000005')
}

export function getJupiterFromAmount({
  quote,
  fromToken
}: {
  quote: WootzWallet.JupiterQuote
  fromToken: WootzWallet.BlockchainToken
}): Amount {
  return new Amount(quote.inAmount).divideByDecimals(fromToken.decimals)
}

export function getJupiterToAmount({
  quote,
  toToken
}: {
  quote: WootzWallet.JupiterQuote
  toToken: WootzWallet.BlockchainToken
}): Amount {
  return new Amount(quote.outAmount).divideByDecimals(toToken.decimals)
}

export function getLiFiFromAmount(route: WootzWallet.LiFiRoute): Amount {
  return new Amount(route.fromAmount).divideByDecimals(route.fromToken.decimals)
}

export function getLiFiToAmount(route: WootzWallet.LiFiRoute): Amount {
  return new Amount(route.toAmount).divideByDecimals(route.toToken.decimals)
}

export function getJupiterQuoteOptions({
  quote,
  fromNetwork,
  fromToken,
  toToken,
  spotPrices,
  defaultFiatCurrency
}: {
  quote: WootzWallet.JupiterQuote
  fromNetwork: WootzWallet.NetworkInfo
  fromToken: WootzWallet.BlockchainToken
  toToken: WootzWallet.BlockchainToken
  spotPrices: SpotPriceRegistry
  defaultFiatCurrency: string
}): QuoteOption[] {
  const networkFee = getJupiterNetworkFee({ quote, fromNetwork })

  return [
    {
      fromAmount: new Amount(quote.inAmount).divideByDecimals(
        fromToken.decimals
      ),
      toAmount: getJupiterToAmount({ quote, toToken }),
      // TODO: minimumToAmount is applicable only for ExactIn swapMode.
      // Create a maximumFromAmount field for ExactOut swapMode if needed.
      minimumToAmount: new Amount(quote.otherAmountThreshold).divideByDecimals(
        toToken.decimals
      ),
      fromToken,
      toToken,
      rate: new Amount(quote.outAmount)
        .divideByDecimals(toToken.decimals)
        .div(new Amount(quote.inAmount).divideByDecimals(fromToken.decimals)),
      impact: new Amount(quote.priceImpactPct),
      sources: quote.routePlan.map((step) => ({
        name: step.swapInfo.label,
        proportion: new Amount(step.percent).times(0.01)
      })),
      routing: 'flow',
      networkFee,
      networkFeeFiat: networkFee.isUndefined()
        ? ''
        : networkFee
            .times(
              getTokenPriceAmountFromRegistry(
                spotPrices,
                makeNetworkAsset(fromNetwork)
              )
            )
            .formatAsFiat(defaultFiatCurrency),
      provider: 'Jupiter',
      // There is only 1 quote returned for Jupiter
      // making it the Fastest and Cheapest.
      tags: ['FASTEST', 'CHEAPEST'],
      id: getUniqueRouteId('Jupiter')
    }
  ]
}

// LiFi

export function getLiFiQuoteOptions({
  quote,
  fromNetwork,
  fromToken,
  toToken,
  spotPrices,
  defaultFiatCurrency
}: {
  quote: WootzWallet.LiFiQuote
  fromNetwork: WootzWallet.NetworkInfo
  spotPrices: SpotPriceRegistry
  defaultFiatCurrency: string
  fromToken: WootzWallet.BlockchainToken
  toToken: WootzWallet.BlockchainToken
}): QuoteOption[] {
  return quote.routes.map((route) => {
    const networkFee = route.steps[0].estimate.gasCosts
      .reduce((total, cost) => {
        return total.plus(cost.amount)
      }, new Amount('0'))
      .divideByDecimals(fromNetwork.decimals)

    const fromAmount = new Amount(route.fromAmount).divideByDecimals(
      fromToken.decimals
    )

    const toAmount = new Amount(route.toAmount).divideByDecimals(
      route.toToken.decimals
    )

    const fromAmountFiat = fromAmount.times(
      getTokenPriceAmountFromRegistry(spotPrices, fromToken)
    )

    const toAmountFiat = toAmount.times(
      getTokenPriceAmountFromRegistry(spotPrices, toToken)
    )

    const fiatDiff = toAmountFiat.minus(fromAmountFiat)
    const fiatDiffRatio = fiatDiff.div(fromAmountFiat)
    const impact = fiatDiffRatio.times(100).toAbsoluteValue()

    return {
      fromAmount: fromAmount,
      fromToken: fromToken,
      impact,
      minimumToAmount: new Amount(route.toAmountMin).divideByDecimals(
        toToken.decimals
      ),
      networkFee,
      networkFeeFiat: networkFee.isUndefined()
        ? ''
        : networkFee
            .times(
              getTokenPriceAmountFromRegistry(
                spotPrices,
                makeNetworkAsset(fromNetwork)
              )
            )
            .formatAsFiat(defaultFiatCurrency),
      rate: toAmount.div(fromAmount),
      routing: 'flow',
      sources: route.steps.map((step) => ({
        name: step.toolDetails.name,
        proportion: new Amount(1),
        logo: step.toolDetails.logo,
        tool: step.tool,
        includedSteps: step.includedSteps
      })),
      toAmount: toAmount,
      toToken: toToken,
      executionDuration: route.steps
        .map((step) => Number(step.estimate.executionDuration))
        .reduce((a, b) => a + b, 0)
        .toString(),
      provider: 'Li.Fi',
      // We need to filter out RECOMMENDED since it is now
      // being depreciated by Li.Fi.
      tags: route.tags.filter(
        (tag) => tag !== 'RECOMMENDED'
      ) as RouteTagsType[],
      id: getUniqueRouteId('Li.Fi', route.steps)
    }
  })
}

export const getLPIcon = (source: Pick<LiquiditySource, 'name' | 'logo'>) => {
  const iconFromMetadata = LPMetadata[source.name]
  if (iconFromMetadata) {
    return iconFromMetadata
  }
  if (source.logo) {
    return `chrome://image?${source.logo}`
  }
  return ''
}

export const getUniqueRouteId = (
  provider: Providers,
  sources?: Array<Pick<LiquiditySource, 'tool'>>
) => {
  if (provider === '0x' || provider === 'Jupiter') {
    return provider
  }
  return sources?.map((source) => source.tool).join('-') ?? ''
}
