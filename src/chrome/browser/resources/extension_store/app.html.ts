// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import type {ExtensionStoreAppElement} from './app.js';

export function getHtml(this: ExtensionStoreAppElement) {
  return `<html>
  <body>
<h1>Extension Store</h1>
<div id="example-div">${this.message_}</div>
<body>
</html>`;
}