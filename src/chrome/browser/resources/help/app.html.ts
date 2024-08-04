// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { html } from "//resources/lit/v3_0/lit.rollup.js";
import type { HelloWorldAppElement } from "./app.ts";

export function getHtml(this: HelloWorldAppElement) {
  return html` <h1>Hello World</h1>
    <div id="example-div">${this.message_}</div>`;
}
