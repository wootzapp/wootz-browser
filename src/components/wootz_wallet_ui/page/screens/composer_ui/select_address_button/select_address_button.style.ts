// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import styled from 'styled-components'

// Shared Styles
import { Button } from '../shared_composer.style'

export const SelectButton = styled(Button)<{
  isPlaceholder: boolean
}>`
  justify-content: space-between;
  width: ${(p) => (p.isPlaceholder ? 'unset' : '100%')};
`
