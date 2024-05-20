// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://resources/cr_elements/cr_icons.css.js';
import 'chrome://resources/cr_elements/cr_shared_style.css.js';
import 'chrome://resources/cr_elements/cr_icon_button/cr_icon_button.js';

import {PolymerElement} from '//resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {skColorToRgba} from 'chrome://resources/js/color_utils.js';

import type {Theme} from './search_bubble.mojom-webui.js';
import {getTemplate} from './search_bubble_app.html.js';
import {SearchBubbleProxyImpl} from './search_bubble_proxy.js';
import type {SearchBubbleProxy} from './search_bubble_proxy.js';

export class SearchBubbleAppElement extends PolymerElement {
  singleColored: boolean;
  private setThemeListenerId_: number|null = null;
  private logoColor_: string;
  private theme_: Theme;

  static get is() {
    return 'search-bubble-app';
  }

  static get template() {
    return getTemplate();
  }

  static get properties() {
    return {
      logoColor_: {
        computed: 'computeLogoColor_(theme_)',
        type: String,
      },
      singleColored: {
        reflectToAttribute: true,
        type: Boolean,
        value: false,
      },
      theme_: {
        observer: 'onThemeChange_',
        type: Object,
      },
    };
  }

  private browserProxy_: SearchBubbleProxy =
      SearchBubbleProxyImpl.getInstance();

  override connectedCallback() {
    super.connectedCallback();
    this.browserProxy_.handler.showUI();
    this.setThemeListenerId_ =
        this.browserProxy_.callbackRouter.setTheme.addListener(
            (theme: Theme) => {
              this.theme_ = theme;
            });
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    this.browserProxy_.callbackRouter.removeListener(this.setThemeListenerId_!);
  }

  private onClose_() {
    this.browserProxy_.handler.closeUI();
  }

  private onThemeChange_() {
    this.singleColored =
        this.theme_ && (!!this.theme_.logoColor || this.theme_.isDark);
    document.body.style.backgroundColor =
        skColorToRgba(this.theme_.backgroundColor);
  }

  private computeLogoColor_() {
    return this.theme_.logoColor ? skColorToRgba(this.theme_.logoColor) :
        this.theme_.isDark       ? '#ffffff' :
                                   'var(--google-blue-600)';
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'search-bubble-app': SearchBubbleAppElement;
  }
}

customElements.define(SearchBubbleAppElement.is, SearchBubbleAppElement);
