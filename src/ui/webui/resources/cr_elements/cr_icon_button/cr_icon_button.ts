// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview 'cr-icon-button' is a button which displays an icon with a
 * ripple. It can be interacted with like a normal button using click as well as
 * space and enter to effectively click the button and fire a 'click' event.
 *
 * There are two sources to icons, cr-icons and iron-iconset-svg. The cr-icon's
 * are defined as background images with a reference to a resource file
 * associated with a CSS class name. The iron-icon's are defined as inline SVG's
 * under a key that is stored in a global map that is accessible to the
 * iron-icon element.
 *
 * Example of using a cr-icon:
 * <link rel="import" href="chrome://resources/cr_elements/cr_icons.css.html">
 * <dom-module id="module">
 *   <template>
 *     <style includes="cr-icons"></style>
 *     <cr-icon-button class="icon-class-name"></cr-icon-button>
 *   </template>
 * </dom-module>
 *
 * In general when an icon is specified using a class, the expectation is the
 * class will set an image to the --cr-icon-image variable.
 *
 * Example of using an iron-icon:
 * In the TS file:
 * import 'chrome://resources/cr_elements/icons.html.js';
 *
 * In the HTML template file:
 * <cr-icon-button iron-icon="cr:icon-key"></cr-icon-button>
 *
 * The color of the icon can be overridden using CSS variables. When using
 * iron-icon both the fill and stroke can be overridden the variables:
 * --cr-icon-button-fill-color
 * --cr-icon-button-stroke-color
 *
 * When not using iron-icon (ie. specifying --cr-icon-image), the icons support
 * one color and the 'stroke' variables are ignored.
 *
 * When using iron-icon's, more than one icon can be specified by setting
 * the |ironIcon| property to a comma-delimited list of keys.
 */

import '//resources/polymer/v3_0/iron-icon/iron-icon.js';

import {CrLitElement} from '//resources/lit/v3_0/lit.rollup.js';
import type {PropertyValues} from '//resources/lit/v3_0/lit.rollup.js';

import {CrPaperRippleMixin} from '../cr_paper_ripple_mixin.js';

import {getCss} from './cr_icon_button.css.js';
import {getHtml} from './cr_icon_button.html.js';

export interface CrIconButtonElement {
  $: {
    icon: HTMLElement,
  };
}

const CrIconbuttonElementBase = CrPaperRippleMixin(CrLitElement);

export class CrIconButtonElement extends CrIconbuttonElementBase {
  static get is() {
    return 'cr-icon-button';
  }

  static override get styles() {
    return getCss();
  }

  override render() {
    return getHtml.bind(this)();
  }

  static override get properties() {
    return {
      disabled: {
        type: Boolean,
        reflect: true,
      },

      ironIcon: {
        type: String,
        reflect: true,
      },

      multipleIcons_: {
        type: Boolean,
        reflect: true,
      },
    };
  }

  disabled: boolean = false;
  ironIcon?: string;
  private multipleIcons_: boolean = false;

  /**
   * It is possible to activate a tab when the space key is pressed down. When
   * this element has focus, the keyup event for the space key should not
   * perform a 'click'. |spaceKeyDown_| tracks when a space pressed and
   * handled by this element. Space keyup will only result in a 'click' when
   * |spaceKeyDown_| is true. |spaceKeyDown_| is set to false when element
   * loses focus.
   */
  private spaceKeyDown_: boolean = false;

  constructor() {
    super();

    this.addEventListener('blur', this.onBlur_.bind(this));
    this.addEventListener('click', this.onClick_.bind(this));
    this.addEventListener('keydown', this.onKeyDown_.bind(this));
    this.addEventListener('keyup', this.onKeyUp_.bind(this));

    if (document.documentElement.hasAttribute('chrome-refresh-2023')) {
      this.addEventListener('pointerdown', this.onPointerDown_.bind(this));
    }
  }

  override willUpdate(changedProperties: PropertyValues<this>) {
    super.willUpdate(changedProperties);

    if (changedProperties.has('ironIcon')) {
      const icons = (this.ironIcon || '').split(',');
      this.multipleIcons_ = icons.length > 1;
    }
  }

  override firstUpdated() {
    if (!this.hasAttribute('role')) {
      this.setAttribute('role', 'button');
    }
    if (!this.hasAttribute('tabindex')) {
      this.setAttribute('tabindex', '0');
    }
  }

  override updated(changedProperties: PropertyValues<this>) {
    super.updated(changedProperties);

    if (changedProperties.has('disabled')) {
      this.setAttribute('aria-disabled', this.disabled ? 'true' : 'false');
      this.disabledChanged_(this.disabled, changedProperties.get('disabled'));
    }

    if (changedProperties.has('ironIcon')) {
      this.onIronIconChanged_();
    }
  }

  private disabledChanged_(newValue: boolean, oldValue: boolean|undefined) {
    if (!newValue && oldValue === undefined) {
      return;
    }
    if (this.disabled) {
      this.blur();
    }
    this.setAttribute('tabindex', String(this.disabled ? -1 : 0));
  }

  private onBlur_() {
    this.spaceKeyDown_ = false;
  }

  private onClick_(e: Event) {
    if (this.disabled) {
      e.stopImmediatePropagation();
    }
  }

  private onIronIconChanged_() {
    this.shadowRoot!.querySelectorAll('iron-icon').forEach(el => el.remove());
    if (!this.ironIcon) {
      return;
    }
    const icons = (this.ironIcon || '').split(',');
    icons.forEach(icon => {
      const ironIcon = document.createElement('iron-icon');
      ironIcon.icon = icon;
      this.$.icon.appendChild(ironIcon);
      if (ironIcon.shadowRoot) {
        ironIcon.shadowRoot.querySelectorAll('svg, img')
            .forEach(child => child.setAttribute('role', 'none'));
      }
    });
  }

  private onKeyDown_(e: KeyboardEvent) {
    if (e.key !== ' ' && e.key !== 'Enter') {
      return;
    }

    e.preventDefault();
    e.stopPropagation();
    if (e.repeat) {
      return;
    }

    if (e.key === 'Enter') {
      this.click();
    } else if (e.key === ' ') {
      this.spaceKeyDown_ = true;
    }
  }

  private onKeyUp_(e: KeyboardEvent) {
    if (e.key === ' ' || e.key === 'Enter') {
      e.preventDefault();
      e.stopPropagation();
    }

    if (this.spaceKeyDown_ && e.key === ' ') {
      this.spaceKeyDown_ = false;
      this.click();
    }
  }

  private onPointerDown_() {
    this.ensureRipple();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'cr-icon-button': CrIconButtonElement;
  }
}

customElements.define(CrIconButtonElement.is, CrIconButtonElement);
