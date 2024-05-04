// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { BrowserBridge } from './browser_bridge.js';
import { ThrottleView } from './throttle_view.js';
import { hasTouchScreen } from './util.js';
import { WindowView } from './view.js';

/** @type {?MainView} */
let instance = null;

/**
 * This class is the root view object of the page. It owns all other views,
 * and manages switching between them. It is also responsible for initializing
 * the views and the BrowserBridge.
 */
export class MainView extends WindowView {
  constructor() {
    if (hasTouchScreen()) {
      document.body.classList.add('touch');
    }

    // Initializes the throttle view which controls the network throttling.
    const throttleView = new ThrottleView();
    super(throttleView);
    this.throttleView_ = throttleView;

    // Initialize components if there are any future expansions.
    this.initViews_();

    // Trigger initial layout and potentially read from URL.
    this.resetGeometry();

    window.onhashchange = this.onUrlHashChange_.bind(this);

    // React to URL changes.
    window.onhashchange();
  }

  initViews_() {
    // This method can be expanded to initialize more views or components.
    // Currently, we only initialize the throttle view.
    this.throttleView_.initialize();
  }

  /**
   * Handle URL hash changes to maintain stateful navigation if applicable.
   */
  onUrlHashChange_() {
    const parsed = parseUrlHash_(window.location.hash);
    // If there's specific handling based on URL, it can be managed here.
    // Example: Setting a specific tab or view based on URL parameters.
  }

  static getInstance() {
    return instance || (instance = new MainView());
  }
}

/**
 * Parses the current hash in the form of "#tab&param1=value1&param2=value2&..."
 * into a dictionary. Parameters and values are decoded with decodeURIComponent().
 */
function parseUrlHash_(hash) {
  const parameters = hash.split('&');

  let tabHash = parameters[0];
  if (tabHash === '' || tabHash === '#') {
    tabHash = undefined;
  }

  // Split each string except the first around the '='.
  let paramDict = {};
  for (let i = 1; i < parameters.length; i++) {
    const paramParts = parameters[i].split('=');
    if (paramParts.length === 2) {
      paramDict[decodeURIComponent(paramParts[0])] = decodeURIComponent(paramParts[1]);
    }
  }

  return { tabHash: tabHash, parameters: paramDict };
}

export default MainView;
