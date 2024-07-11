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
   * Sends a request to login the user.
   * @param {string} email email.
   * @param {string} password password.
   */
  loginWallet(email, password) {
    console.log(`HELLO  ${email}, ${password}`);
    chrome.send('loginWallet', [email, password]);
  }

  /**
   * Sends a request to get the user profile.
   * @returns {Promise} A promise that resolves with the user profile.
   */
  getUserProfile() {
    return sendWithPromise('getUserProfile');
  }

  /**
   * Sends a request to log the URL.
   * @param {string} token user token.
   * @param {string} url URL to log.
   * @returns {Promise} A promise that resolves when the URL is logged.
   */
  logUrl(token, url) {
    return sendWithPromise('logUrl', [token, url]);
  }

  /**
   * Sends a request to encrypt data.
   * @param {string} token user token.
   * @param {string} secret secret key.
   * @param {any} data data to encrypt.
   * @returns {Promise} A promise that resolves with the encrypted data.
   */
  encrypt(token, secret, data) {
    return sendWithPromise('encrypt', [token, secret, data]);
  }

  static getInstance() {
    return instance || (instance = new BrowserBridge());
  }
}
