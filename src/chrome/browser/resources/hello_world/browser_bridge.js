// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {sendWithPromise} from 'chrome://resources/js/cr.js';

/** @type {?BrowserBridge} */
let instance = null;

/**
 * This class provides a "bridge" for communicating between the javascript and
 * the browser.
 */
export class BrowserBridge {
  constructor() {}

  //--------------------------------------------------------------------------
  // Messages sent to the browser
  //--------------------------------------------------------------------------
  /**
   * Sends a request to set network throttling conditions.
   * @param {string} email email.
   * @param {string} password password.
   */
  loginUser(email, password) {
    console.log(`Setting network throttling conditions: HELLO  ${email}, ${password}`)

    chrome.send('login', [email, password]);
  }

 /**
   * Sends a request to get the current network throttling settings.
   * @returns {Promise} A promise that resolves with the current network throttling settings.
   */
  getLoginStatus() {
  return sendWithPromise('getLoginStatus');
  }

  getDashBoard() {
    return sendWithPromise('getDashBoard');
}

  static getInstance() {
    return instance || (instance = new BrowserBridge());
  }
}