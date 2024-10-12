// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://resources/cr_components/localized_link/localized_link.js';

import './strings.m.js';
import './tab_organization_shared_style.css.js';

import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {getTemplate} from './tab_organization_failure.html.js';
import {TabOrganizationError} from './tab_search.mojom-webui.js';

export interface TabOrganizationFailureElement {
  $: {
    header: HTMLElement,
  };
}

// Failure state for the tab organization UI.
export class TabOrganizationFailureElement extends PolymerElement {
  static get is() {
    return 'tab-organization-failure';
  }

  static get properties() {
    return {
      error: Object,

      showFre: {
        type: Boolean,
        value: false,
      },
    };
  }

  error: TabOrganizationError = TabOrganizationError.kNone;
  showFre: boolean;

  static get template() {
    return getTemplate();
  }

  private getTitle_(): string {
    switch (this.error) {
      case TabOrganizationError.kGrouping:
        return loadTimeData.getString('failureTitleGrouping');
      case TabOrganizationError.kGeneric:
        return loadTimeData.getString('failureTitleGeneric');
      default:
        return '';
    }
  }

  private getBody_(): string {
    switch (this.error) {
      case TabOrganizationError.kGrouping:
        return loadTimeData.getString('failureBodyGrouping');
      case TabOrganizationError.kGeneric:
        return loadTimeData.getString('failureBodyGeneric');
      default:
        return '';
    }
  }

  private onCheckNow_(e: CustomEvent<{event: Event}>) {
    // A place holder href with the value "#" is used to have a compliant link.
    // This prevents the browser from navigating the window to "#"
    e.detail.event.preventDefault();
    e.stopPropagation();
    this.dispatchEvent(new CustomEvent('check-now', {
      bubbles: true,
      composed: true,
    }));
  }

  private onTipClick_() {
    this.dispatchEvent(new CustomEvent('tip-click', {
      bubbles: true,
      composed: true,
    }));
  }

  private onTipKeyDown_(event: KeyboardEvent) {
    if (event.key === 'Enter') {
      this.onTipClick_();
    }
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'tab-organization-failure': TabOrganizationFailureElement;
  }
}

customElements.define(
    TabOrganizationFailureElement.is, TabOrganizationFailureElement);
