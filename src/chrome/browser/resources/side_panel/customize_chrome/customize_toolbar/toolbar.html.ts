// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {html} from '//resources/lit/v3_0/lit.rollup.js';

import type {ToolbarElement} from './toolbar.js';

export function getHtml(this: ToolbarElement) {
  return html`<!--_html_template_start_-->
<div class="sp-card">
  <sp-heading id="heading" @back-button-click="${this.onBackClick_}"
      back-button-aria-label="$i18n{backButton}"
      back-button-title="$i18n{backButton}">
    <h2 slot="heading">$i18n{toolbarHeader}</h2>
  </sp-heading>
</div>
<hr class="sp-cards-separator">
<div class="sp-card">
  ${this.actions_.map((action) => html`
    <div class="toggle-container choose-icons-row">
      <h4 class="toggle-title">${action.displayName}</h4>
      <cr-toggle @change="${this.getActionToggleHandler_(action.id)}"
          ?checked="${action.pinned}"></cr-toggle>
    </div>
  `)}
</div>
<!--_html_template_end_-->`;
}
