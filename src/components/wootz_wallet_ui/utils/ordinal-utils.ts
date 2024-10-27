// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import { getLocale } from '../../common/locale'
import { unbiasedRandom } from './random-utils'

export const ORDINALS = {
  0: getLocale('wootzWalletOrdinalFirst'),
  1: getLocale('wootzWalletOrdinalSecond'),
  2: getLocale('wootzWalletOrdinalThird'),
  3: getLocale('wootzWalletOrdinalFourth'),
  4: getLocale('wootzWalletOrdinalFifth'),
  5: getLocale('wootzWalletOrdinalSixth'),
  6: getLocale('wootzWalletOrdinalSeventh'),
  7: getLocale('wootzWalletOrdinalEighth'),
  8: getLocale('wootzWalletOrdinalNinth'),
  9: getLocale('wootzWalletOrdinalTenth'),
  10: getLocale('wootzWalletOrdinalEleventh'),
  11: getLocale('wootzWalletOrdinalTwelfth'),
  12: getLocale('wootzWalletOrdinalThirteenth'),
  13: getLocale('wootzWalletOrdinalFourteenth'),
  14: getLocale('wootzWalletOrdinalFifteenth'),
  15: getLocale('wootzWalletOrdinalSixteenth'),
  16: getLocale('wootzWalletOrdinalSeventeenth'),
  17: getLocale('wootzWalletOrdinalEighteenth'),
  18: getLocale('wootzWalletOrdinalNineteenth'),
  19: getLocale('wootzWalletOrdinalTwentieth'),
  20: getLocale('wootzWalletOrdinalTwentyFirst'),
  21: getLocale('wootzWalletOrdinalTwentySecond'),
  22: getLocale('wootzWalletOrdinalTwentyThird'),
  23: getLocale('wootzWalletOrdinalTwentyFourth')
}

const suffixes = new Map([
  ['one', getLocale('wootzWalletOrdinalSuffixOne')],
  ['two', getLocale('wootzWalletOrdinalSuffixTwo')],
  ['few', getLocale('wootzWalletOrdinalSuffixFew')],
  ['other', getLocale('wootzWalletOrdinalSuffixOther')]
])

export const formatOrdinals = (n: number) => {
  const pr = new Intl.PluralRules(navigator.language, { type: 'ordinal' })
  const rule = pr.select(n)
  const suffix = suffixes.get(rule)
  return `${n}${suffix}`
}

export const getWordIndicesToVerify = (_wordsLength: number): number[] => {
  if (_wordsLength < 3) {
    // phrase is not long enough (must be longer than 3 words)
    return [-3, -2, -1]
  }

  // limit randomness to first 24 words
  const wordsLength = _wordsLength > 24 ? 24 : _wordsLength

  // get next random index
  const indicesSet = new Set<number>([])

  while (indicesSet.size < 3) {
    const nextIndex = unbiasedRandom(0, wordsLength - 1)
    indicesSet.add(nextIndex)
  }

  return Array.from(indicesSet).sort((a, b) => a - b) // verify in order
}
