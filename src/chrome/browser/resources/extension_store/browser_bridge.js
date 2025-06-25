// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {addWebUIListener, sendWithPromise} from 'chrome://resources/js/cr.m.js';
import {assert} from 'chrome://resources/js/assert.m.js';

/**
 * @fileoverview BrowserBridge for the extension store that facilitates
 * communication between JavaScript and the browser with response logging.
 */

/**
 * @typedef {{
 *   name: string,
 *   description: string,
 *   version: string,
 *   icon_url: string,
 *   download_url: string,
 *   icon_base64: (string|undefined),
 * }}
 */
export let ExtensionInfo;

/**
 * Handles communication between the extension store page and the browser.
 */
class BrowserBridge {
  /** @private */
  constructor() {
    /** @private {?function(!Array<ExtensionInfo>)} */
    this.extensionsChangedListener_ = null;
    
    /** @private {Array<ExtensionInfo>} */
    this.cachedExtensions_ = [];
    
    // Set up handlers for messages coming from C++
    this.setupMessageHandlers_();
  }

  /**
   * Sets up handlers for messages from C++.
   * @private
   */
  setupMessageHandlers_() {
    // Handler for extension data from C++
    window.handleExtensionData = (extensionData) => {
      console.log('HANDLER: Extension data received from C++:', JSON.stringify(extensionData));
      
      // Ensure extensionData is properly structured
      if (!extensionData) {
        console.error('HANDLER ERROR: Received empty extension data');
        return;
      }
      
      // Extract relevant fields from the extensionData
      //"id": "camp-network",
      // "name": "Camp Network",
      // "version": "v1.0.1",
      // "description": "An extension to scrap twitter(X) in background for tweets to earn passive...",
      // "icon": "https://placeholder.com/60x60",
      // "tags": ["Scrapping", "earn", "reward"],
      // "installed": true
      const extension = {
        name: extensionData.name || '',
        description: extensionData.description || '',
        version: extensionData.version || '',
        icon_url: extensionData.icon_url || '',
        download_url: extensionData.download_url || '',
        author: extensionData.author || '',
        created_at: extensionData.created_at || '',
        updated_at: extensionData.updated_at || '',
        id: extensionData.id || '',
        github_url: extensionData.github_url || '',
        tags: extensionData.tags || []
      };
      
      console.log('HANDLER: Processed extension data:', JSON.stringify(extension));
      
      // Dump full objects to console for inspection
      console.group('Extension Data Details');
      console.log('Raw data:', extensionData);
      console.log('Processed extension:', extension);
      console.log('Object keys in raw data:', Object.keys(extensionData));
      console.groupEnd();
      
      // Cache the extension for later retrieval
      this.cachedExtensions_ = [extension];
      
      // Notify listener with an array containing this extension
      if (this.extensionsChangedListener_) {
        console.log('HANDLER: Notifying extensions changed listener');
        this.extensionsChangedListener_([extension]);
      } else {
        console.warn('HANDLER: No extensions changed listener registered');
      }
      
      // Dispatch custom event for components that need to react to this data
      window.dispatchEvent(new CustomEvent('extension-data-updated', {
        detail: extension
      }));
      console.log('HANDLER: Dispatched extension-data-updated event');
      
      // Pre-fetch the icon if available
      if (extension.icon_url) {
        console.log('HANDLER: Pre-fetching icon from:', extension.icon_url);
        this.fetchIcon(extension.icon_url);
      } else {
        console.warn('HANDLER: No icon URL found for extension');
      }
    };

    // Handler for errors from C++
    window.handleError = (errorMessage) => {
      console.error('HANDLER ERROR: Extension Store Error:', errorMessage);
      // Dispatch custom event for error handling
      window.dispatchEvent(new CustomEvent('extension-store-error', {
        detail: errorMessage
      }));
      console.log('HANDLER: Dispatched extension-store-error event');
    };
    
    // Handler for icon data from C++
    window.handleIconData = (iconUrl, iconBase64) => {
      console.log('HANDLER: Icon data received from C++ for URL:', iconUrl);
      console.log('HANDLER: Icon data length:', iconBase64 ? iconBase64.length : 0);
      
      if (!iconUrl || !iconBase64) {
        console.error('HANDLER ERROR: Missing icon URL or data');
        return;
      }
      
      // Debug the first 100 chars of the base64 data
      const previewLength = Math.min(100, iconBase64.length);
      console.log(`HANDLER: Icon base64 preview: ${iconBase64.substring(0, previewLength)}...`);
      
      // Update cached extension with icon data
      let updated = false;
      this.cachedExtensions_.forEach(extension => {
        if (extension.icon_url === iconUrl) {
          console.log('HANDLER: Updating icon data for extension:', extension.name);
          extension.icon_base64 = iconBase64;
          updated = true;
          
          // Notify listeners about the updated icon
          if (this.extensionsChangedListener_) {
            console.log('HANDLER: Notifying listeners about updated icon');
            this.extensionsChangedListener_(this.cachedExtensions_);
          }
          
          // Dispatch custom event for icon update
          window.dispatchEvent(new CustomEvent('extension-icon-updated', {
            detail: { 
              extensionId: extension.id,
              iconBase64: iconBase64
            }
          }));
          console.log('HANDLER: Dispatched extension-icon-updated event');
        }
      });
      
      if (!updated) {
        console.warn('HANDLER WARNING: No matching extension found for icon URL:', iconUrl);
        console.log('HANDLER: Available extensions:', this.cachedExtensions_.map(e => e.icon_url));
      }
    };
  }

  /**
   * Gets the cached extension data.
   * @return {!Array<ExtensionInfo>}
   */
  getCachedExtensions() {
    return this.cachedExtensions_;
  }
  
  /**
   * Gets a specific extension by ID.
   * @param {string} id The extension ID.
   * @return {?ExtensionInfo}
   */
  getExtensionById(id) {
    return this.cachedExtensions_.find(ext => ext.id === id) || null;
  }

  /**
   * Sets up a listener for extensions updates from the browser.
   * @param {function(!Array<ExtensionInfo>)} listener The listener function.
   */
  setExtensionsChangedListener(listener) {
    this.extensionsChangedListener_ = listener;
  }

  /**
   * Fetches all extensions from the GitHub repository.
   * @return {!Promise<void>}
   */
  fetchExtensions() {
    console.log('Requesting extensions from GitHub');
    return this.sendWithLogging_('fetchExtensions', []);
  }

  /**
   * Installs an extension from the provided download URL.
   * @param {string} downloadUrl The URL to download the extension from.
   * @return {!Promise<void>}
   */
  installExtension(downloadUrl) {
    console.log('Installing extension from URL:', downloadUrl);
    return this.sendWithLogging_('installExtension', [downloadUrl]);
  }

  /**
   * Fetches an icon for an extension.
   * @param {string} iconUrl The URL of the icon to fetch.
   * @return {!Promise<void>}
   */
  fetchIcon(iconUrl) {
    console.log('Fetching icon from URL:', iconUrl);
    return this.sendWithLogging_('fetchIcon', [iconUrl]);
  }

  /**
   * Fetches an extension's download URL.
   * @param {string} downloadUrl URL to fetch the actual download link from.
   * @param {ExtensionInfo} extensionInfo The extension information.
   * @return {!Promise<void>}
   */
  fetchDownloadUrl(downloadUrl, extensionInfo) {
    console.log('Fetching download URL:', downloadUrl);
    return this.sendWithLogging_('fetchDownloadUrl', [downloadUrl, extensionInfo]);
  }

  /**
   * Sends a message to the browser with the given method name and parameters,
   * and logs the response.
   * @param {string} methodName The name of the method to call.
   * @param {Array=} params The parameters to pass to the method.
   * @return {!Promise<*>}
   * @private
   */
  async sendWithLogging_(methodName, params = []) {
    try {
      console.log(`Calling ${methodName} with params:`, params);
      // Using chrome.send for WebUI messaging
      chrome.send(methodName, params);
      // Note: C++ code uses CallJavascriptFunctionUnsafe for responses,
      // so we don't need to await a direct response here
      return Promise.resolve();
    } catch (error) {
      console.error(`Extension Store Error (${methodName}):`, error);
      throw error;
    }
  }

  /**
   * Gets the singleton instance of BrowserBridge.
   * @return {!BrowserBridge}
   */
  static getInstance() {
    if (!BrowserBridge.instance_) {
      BrowserBridge.instance_ = new BrowserBridge();
    }
    return BrowserBridge.instance_;
  }

  /**
   * Initializes the Extension Store UI.
   * This should be called when the page loads.
   */
  initialize() {
    console.log('INIT: Initializing Extension Store');
    
    // Log the WebUI availability
    if (chrome && chrome.send) {
      console.log('INIT: chrome.send is available, WebUI communication should work');
    } else {
      console.error('INIT: chrome.send not available! WebUI communication will fail');
    }
    
    // Check if our handlers are properly registered
    if (window.handleExtensionData && window.handleError && window.handleIconData) {
      console.log('INIT: All message handlers are registered');
    } else {
      console.error('INIT: Message handlers not properly registered!');
      console.log('INIT: handleExtensionData available:', !!window.handleExtensionData);
      console.log('INIT: handleError available:', !!window.handleError);
      console.log('INIT: handleIconData available:', !!window.handleIconData);
    }
    
    // Fetch extensions on initialization
    console.log('INIT: Triggering initial extension fetch');
    this.fetchExtensions();
  }
}

/** @private {BrowserBridge} */
BrowserBridge.instance_ = null;
