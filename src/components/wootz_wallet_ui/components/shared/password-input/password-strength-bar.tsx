// Copyright (c) 2022 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// utils
import { getLocale } from '$web-common/locale'

// style
import {
  ProgressBar,
  BarAndMessageContainer,
  BarMessage
} from './password-strength-bar.styles'

// components

interface Props {
  criteria: boolean[]
}

export const PasswordStrengthBar: React.FC<Props> = ({
  criteria
}) => {
  // memos
  const strongPasswordCrtieriaPercentComplete = React.useMemo(
    () => (criteria.filter((c) => !!c).length / criteria.length) * 100,
    [criteria]
  )

  // render
  return (
    <BarAndMessageContainer>
      <ProgressBar
        criteria={criteria}
        progress={strongPasswordCrtieriaPercentComplete / 100}
      />
      <BarMessage criteria={criteria}>
        {strongPasswordCrtieriaPercentComplete === 100
          ? getLocale('wootzWalletPasswordIsStrong')
          : strongPasswordCrtieriaPercentComplete < 50
          ? getLocale('wootzWalletPasswordIsWeak')
          : getLocale('wootzWalletPasswordIsMediumStrength')}
      </BarMessage>
    </BarAndMessageContainer>
  )
}

export default PasswordStrengthBar
