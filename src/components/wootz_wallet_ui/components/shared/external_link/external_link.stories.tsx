// Copyright (c) 2024 The Wootz Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

import { ExternalLink } from './external_link'

export const _ExternalLink = {
  render: () => {
    return (
      <ExternalLink
        href='https://wootz.com'
        text='Wootz'
      />
    )
  }
}

export default {
  component: ExternalLink
}
