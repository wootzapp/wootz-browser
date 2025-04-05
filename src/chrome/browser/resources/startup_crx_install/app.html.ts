// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import type {StartupCrxInstallAppElement} from './app.ts'; 

export function getHtml(this: StartupCrxInstallAppElement) {
  return `<html>
  <body>
<h1>Startup Crx Install</h1>
<div id="example-div">${this.message_}</div>
<body>
</html>`;
}