// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'chrome://resources/ash/common/cr_elements/cr_dialog/cr_dialog.js';
import 'chrome://resources/polymer/v3_0/iron-location/iron-location.js';
import 'chrome://resources/polymer/v3_0/iron-location/iron-query-params.js';
import './sea_pen_images_element.js';
import './sea_pen_input_query_element.js';
import './sea_pen_recent_wallpapers_element.js';
import './sea_pen_template_query_element.js';
import './sea_pen_templates_element.js';
import './sea_pen_terms_of_service_dialog_element.js';

import {assert} from 'chrome://resources/js/assert.js';

import {Query} from './constants.js';
import {isSeaPenEnabled, isSeaPenTextInputEnabled} from './load_time_booleans.js';
import {setThumbnailResponseStatusCodeAction} from './sea_pen_actions.js';
import {acceptSeaPenTermsOfService, getShouldShowSeaPenTermsOfServiceDialog} from './sea_pen_controller.js';
import {SeaPenTemplateId} from './sea_pen_generated.mojom-webui.js';
import {getSeaPenProvider} from './sea_pen_interface_provider.js';
import {getTemplate} from './sea_pen_router_element.html.js';
import {WithSeaPenStore} from './sea_pen_store.js';
import {maybeDoPageTransition} from './transition.js';

export enum SeaPenPaths {
  ROOT = '',
  RESULTS = '/results',
}

export interface SeaPenQueryParams {
  seaPenTemplateId?: string;
}

let instance: SeaPenRouterElement|null = null;

export class SeaPenRouterElement extends WithSeaPenStore {
  static get is() {
    return 'sea-pen-router';
  }
  static get template() {
    return getTemplate();
  }
  static get properties() {
    return {
      basePath: String,

      path_: String,

      query_: String,

      queryParams_: Object,

      relativePath_: {
        type: String,
        computed: 'computeRelativePath_(path_, basePath)',
        observer: 'onRelativePathChanged_',
      },

      showSeaPenTermsOfServiceDialog_: Boolean,
    };
  }

  static instance(): SeaPenRouterElement {
    assert(instance, 'sea pen router does not exist');
    return instance;
  }

  basePath: string;
  private path_: string;
  private query_: string;
  private queryParams_: SeaPenQueryParams;
  private relativePath_: string|null;
  private showSeaPenTermsOfServiceDialog_: boolean;

  override connectedCallback() {
    assert(isSeaPenEnabled(), 'sea pen must be enabled');
    super.connectedCallback();
    instance = this;
    this.watch<SeaPenRouterElement['showSeaPenTermsOfServiceDialog_']>(
        'showSeaPenTermsOfServiceDialog_',
        state => state.shouldShowSeaPenTermsOfServiceDialog);
    this.updateFromStore();
    this.fetchTermsOfServiceDialogStatus();
  }

  override disconnectedCallback() {
    super.disconnectedCallback();
    instance = null;
  }

  selectSeaPenTemplate(templateId: SeaPenTemplateId|Query) {
    // resets the Sea Pen thumbnail response status code when switching
    // template; otherwise, error state will remain in sea-pen-images element if
    // it happens in the last query search.
    this.dispatch(setThumbnailResponseStatusCodeAction(null));
    this.goToRoute(
        SeaPenPaths.RESULTS, {seaPenTemplateId: templateId.toString()});
  }

  async goToRoute(
      relativePath: SeaPenPaths, queryParams: SeaPenQueryParams = {}) {
    assert(typeof this.basePath === 'string', 'basePath must be set');
    return maybeDoPageTransition(
        () => this.setProperties(
            {path_: this.basePath + relativePath, queryParams_: queryParams}));
  }

  /**
   * Compute the relative path compared to the SeaPen base path.
   * @param path the absolute path of the current route
   * @param basePath the absolute path of the base seapen route
   * @returns path relative to basePath, or null if path is not relative to
   *     basePath
   * @example
   * computeRelativePath_('/wallpaper/sea_pen', '/wallpaper/sea_pen') => ''
   * computeRelativePath_('/wallpaper/sea_pen/results', '/wallpaper/sea_pen') =>
   *   '/results'
   * computeRelativePath_('/wallpaper', '/wallpaper/sea_pen') => null
   */
  private computeRelativePath_(path: string|null, basePath: string|null): string
      |null {
    if (typeof path !== 'string' || typeof basePath !== 'string') {
      return null;
    }
    if (!path.startsWith(basePath)) {
      return null;
    }
    const relativePath = path.substring(basePath.length);
    // Normalize single slash to empty string.
    // This keeps path consistent between chrome://vc-background/ and
    // chrome://personalization/wallpaper/sea-pen.
    return relativePath === '/' ? '' : relativePath;
  }

  private onRelativePathChanged_(relativePath: string|null) {
    if (typeof relativePath !== 'string') {
      // `relativePath` will be null when using Personalization breadcrumbs to
      // navigate back to home or wallpaper. Don't reset the path, as
      // `SeaPenRouter` may be imminently torn down.
      return;
    }
    if (!Object.values(SeaPenPaths).includes(relativePath as SeaPenPaths)) {
      // If arriving at an unknown path, go back to the root path.
      console.warn('SeaPenRouter unknown path', relativePath);
      this.goToRoute(SeaPenPaths.ROOT);
    }
  }

  private shouldShowTextInputQuery_(
      relativePath: string|null, templateId: string|null): boolean {
    return isSeaPenTextInputEnabled() && relativePath === SeaPenPaths.RESULTS &&
        templateId === 'Query';
  }

  private shouldShowTemplateQuery_(
      relativePath: string|null, templateId: string|null): boolean {
    return relativePath === SeaPenPaths.RESULTS &&
        (!!templateId && templateId !== 'Query');
  }

  private shouldShowSeaPenRoot_(relativePath: string|null): boolean {
    if (typeof relativePath !== 'string') {
      return false;
    }
    return relativePath === SeaPenPaths.ROOT;
  }

  private shouldShowSeaPenImages_(relativePath: string|null): boolean {
    if (typeof relativePath !== 'string') {
      return false;
    }
    return relativePath === SeaPenPaths.RESULTS;
  }

  private getTemplateIdFromQueryParams_(templateId: string): SeaPenTemplateId
      |Query {
    if (templateId === 'Query') {
      return 'Query';
    }
    return parseInt(templateId) as SeaPenTemplateId;
  }

  private async fetchTermsOfServiceDialogStatus() {
    await getShouldShowSeaPenTermsOfServiceDialog(
        getSeaPenProvider(), this.getStore());
  }

  private async onAcceptSeaPenTerms_() {
    await acceptSeaPenTermsOfService(getSeaPenProvider(), this.getStore());
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'sea-pen-router': SeaPenRouterElement;
  }
}

customElements.define(SeaPenRouterElement.is, SeaPenRouterElement);
