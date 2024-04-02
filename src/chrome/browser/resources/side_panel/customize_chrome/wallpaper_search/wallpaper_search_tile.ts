// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {getTemplate} from './wallpaper_search_tile.html.js';

export class WallpaperSearchTileElement extends PolymerElement {
  static get is() {
    return 'customize-chrome-wallpaper-search-tile';
  }

  static get template() {
    return getTemplate();
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'customize-chrome-wallpaper-search-tile': WallpaperSearchTileElement;
  }
}

customElements.define(
    WallpaperSearchTileElement.is, WallpaperSearchTileElement);
