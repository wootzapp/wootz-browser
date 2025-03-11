/**
 * @typedef {{
*   name: string,
*   description: string,
*   version: string,
*   icon_url: string,
*   download_url: string,
*   icon_base64: (string|undefined),
*   id: string,
*   author: string,
*   github_url: string,
*   tags: Array<string>,
*   created_at: string,
*   updated_at: string
* }}
*/
let ExtensionInfo;

/**
* Handles communication between the extension store page and the browser.
*/
class BrowserBridge {
 /** @private */
 constructor() {
   /** @private {Object<string, Object>} */
   this.cachedExtensions_ = {};
   
   /** @private {Object<string, Object>} */
   this.installedExtensions_ = {};
   
   /** @private {?function(!Array<!Object>)} */
   this.extensionsChangedListener_ = null;
   
   this.setupMessageHandlers_();
   
   // Initialize by fetching installed extensions
   this.fetchInstalledExtensions();
   
   // Set up a periodic refresh of installed extensions (every 10 seconds)
   setInterval(() => {
     this.fetchInstalledExtensions();
   }, 10000);
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
       tags: extensionData.tags || [],
       icon_base64: extensionData.icon_base64 || ''
     };
     
     console.log('HANDLER: Processed extension data:', JSON.stringify(extension));
     
     // Check if this is a complete extension or just an icon update
     if (extension.id) {
       // Store in cache, using ID as key for easier updates
       this.cachedExtensions_[extension.id] = extension;
       
       // Convert the object to an array for the listener
       const extensionsArray = Object.values(this.cachedExtensions_);
       
       // Notify listener with the updated extensions array
       if (this.extensionsChangedListener_) {
         console.log('HANDLER: Notifying extensions changed listener with', extensionsArray.length, 'extensions');
         this.extensionsChangedListener_(extensionsArray);
       } else {
         console.warn('HANDLER: No extensions changed listener registered');
       }
       
       // Dispatch custom event for components that need to react to this data
       window.dispatchEvent(new CustomEvent('extension-data-updated', {
         detail: extension
       }));
       console.log('HANDLER: Dispatched extension-data-updated event');
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
 }

 /**
  * Gets the cached extension data.
  * @return {!Array<ExtensionInfo>}
  */
 getCachedExtensions() {
   return Object.values(this.cachedExtensions_);
 }
 
 /**
  * Gets a specific extension by ID.
  * @param {string} id The extension ID.
  * @return {?ExtensionInfo}
  */
 getExtensionById(id) {
   return this.cachedExtensions_[id] || null;
 }

 /**
  * Sets up a listener for extensions updates from the browser.
  * @param {function(!Array<ExtensionInfo>)} listener The listener function.
  */
 setExtensionsChangedListener(listener) {
   this.extensionsChangedListener_ = listener;
   
   // If we already have extensions, notify the listener immediately
   const extensions = this.getCachedExtensions();
   if (extensions.length > 0) {
     listener(extensions);
   }
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
  * Fetches all installed extensions from the browser.
  * @return {!Promise<void>}
  */
 fetchInstalledExtensions() {
   console.log(' Requesting installed extensions from browser');
   
   /** @private {Object<string, Object>} */
   this.installedExtensions_ = this.installedExtensions_ || {};
   
   // Set up handler for installed extensions data if not already set
   if (!window.handleInstalledExtensionsData) {
     window.handleInstalledExtensionsData = (installedExtensionsData) => {
       console.log(' Received installed extensions data from C++:', 
                   installedExtensionsData ? installedExtensionsData.length : 0, 'extensions');
       
       // Log the raw data for debugging
       console.log('Raw installed extensions data:', JSON.stringify(installedExtensionsData));
       
       // Clear existing installed extensions data
       this.installedExtensions_ = {};
       
       // Process each installed extension
       if (installedExtensionsData && installedExtensionsData.length) {
         installedExtensionsData.forEach(extension => {
           if (extension && extension.id) {
             console.log(' Processing installed extension:', extension.id, extension.name, extension.version);
             
             // Store in map with ID as key
             this.installedExtensions_[extension.id] = {
               id: extension.id,
               name: extension.name || '',
               version: extension.version || '',
               description: extension.description || ''
             };
           }
         });
       }
       
       console.log(' Total installed extensions in map:', Object.keys(this.installedExtensions_).length);
       
       // Dispatch event for components that need to react to installed extensions
       window.dispatchEvent(new CustomEvent('installed-extensions-updated', {
         detail: this.getInstalledExtensions()
       }));
     };
   }
   
   return this.sendWithLogging_('fetchInstalledExtensions', []);
 }
 
 /**
  * Gets the map of installed extensions.
  * @return {!Object<string, Object>}
  */
 getInstalledExtensions() {
   return this.installedExtensions_ || {};
 }
 
 /**
  * Checks if an extension is installed by ID.
  * @param {string} extensionId The extension ID to check.
  * @return {boolean}
  */
 isExtensionInstalled(extensionId) {
   return !!(this.installedExtensions_ && this.installedExtensions_[extensionId]);
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
  * @param {string} extensionId The ID of the extension.
  * @return {!Promise<void>}
  */
 fetchIcon(iconUrl, extensionId) {
   console.log('Fetching icon from URL:', iconUrl, 'for extension:', extensionId);
   return this.sendWithLogging_('fetchIcon', [iconUrl, extensionId]);
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
   if (window.handleExtensionData && window.handleError) {
     console.log('INIT: All message handlers are registered');
   } else {
     console.error('INIT: Message handlers not properly registered!');
     console.log('INIT: handleExtensionData available:', !!window.handleExtensionData);
     console.log('INIT: handleError available:', !!window.handleError);
   }
   
   // Fetch extensions on initialization
   console.log('INIT: Triggering initial extension fetch');
   this.fetchExtensions();
   
   // Fetch installed extensions on initialization
   console.log(' Triggering initial installed extensions fetch');
   this.fetchInstalledExtensions();
 }
}

/** @private {BrowserBridge} */
BrowserBridge.instance_ = null;

// Initialize the browser bridge
const browserBridge = BrowserBridge.getInstance();

// Function to convert backend extension format to UI format
function convertToUIFormat(extension) {
   // Check if this extension is installed by ID
   const isInstalled = browserBridge.isExtensionInstalled(extension.id);
   const installedExtension = isInstalled ? browserBridge.installedExtensions_[extension.id] : null;
   
   // Check if an update is available (only if installed)
   let needsUpdate = false;
   let installedVersion = null;
   
   if (isInstalled && extension.version && installedExtension && installedExtension.version) {
     installedVersion = installedExtension.version;
     console.log(' Comparing versions for', extension.name, '- Store:', extension.version, 'Installed:', installedVersion);
     
     // Simple version comparison (assumes semantic versioning x.y.z)
     needsUpdate = compareVersions(extension.version, installedVersion) > 0;
     if (needsUpdate) {
       console.log(' Update available for', extension.name, 'from', installedVersion, 'to', extension.version);
     }
   }
   
   return {
       id: extension.id || '',
       name: extension.name || '',
       version: extension.version || '',
       description: extension.description || '',
       // Use icon_base64 if available, otherwise fall back to icon_url or placeholder
       icon: extension.icon_base64 ? `data:image/png;base64,${extension.icon_base64}` : 
             (extension.icon_url || 'https://placeholder.com/60x60'),
       tags: extension.tags || [],
       download_url: extension.download_url || '',
       installed: isInstalled,
       needsUpdate: needsUpdate,
       installedVersion: installedVersion,
       author: extension.author || '',
       // Store the real extension ID for reference
       realExtensionId: extension.id
   };
}

/**
 * Compare two version strings
 * @param {string} v1 First version
 * @param {string} v2 Second version
 * @return {number} 1 if v1 > v2, -1 if v1 < v2, 0 if equal
 */
function compareVersions(v1, v2) {
  const parts1 = v1.split('.').map(Number);
  const parts2 = v2.split('.').map(Number);
  
  // Compare each part of the version
  for (let i = 0; i < Math.max(parts1.length, parts2.length); i++) {
    const part1 = i < parts1.length ? parts1[i] : 0;
    const part2 = i < parts2.length ? parts2[i] : 0;
    
    if (part1 > part2) return 1;
    if (part1 < part2) return -1;
  }
  
  return 0; // Versions are equal
}

// Function to create extension cards
function createExtensionCard(extension) {
   const card = document.createElement('div');
   card.className = 'extension-card';
   card.setAttribute('data-id', extension.id);

   // Create main content area (always visible)
   const mainContent = document.createElement('div');
   mainContent.className = 'extension-main';

   const icon = document.createElement('img');
   icon.src = extension.icon;
   icon.alt = extension.name;
   icon.className = 'extension-icon';
   mainContent.appendChild(icon);

   const info = document.createElement('div');
   info.className = 'extension-info';

   const header = document.createElement('div');
   header.className = 'extension-header';

   const name = document.createElement('div');
   name.className = 'extension-name';
   name.textContent = extension.name;
   header.appendChild(name);

   const version = document.createElement('div');
   version.className = 'extension-version';
   version.textContent = `v${extension.version}`;
   header.appendChild(version);

   info.appendChild(header);

   // Add tags
   const tagsContainer = document.createElement('div');
   tagsContainer.className = 'extension-tags';
   extension.tags.forEach(tag => {
       const tagElement = document.createElement('span');
       tagElement.className = 'tag';
       tagElement.textContent = tag;
       tagsContainer.appendChild(tagElement);
   });
   info.appendChild(tagsContainer);

   mainContent.appendChild(info);

   // Add install button to the right
   const actionContainer = document.createElement('div');
   actionContainer.className = 'extension-actions';
   
   const button = document.createElement('button');
   
   // Set button state based on installation status and version
   if (extension.installed) {
     if (extension.needsUpdate) {
       // Update available
       button.className = 'install-button update prevent-card-click';
       button.textContent = 'Update';
       button.disabled = false;
     } else {
       // Already installed with latest version
       button.className = 'install-button installed prevent-card-click';
       button.textContent = 'Installed';
       button.disabled = true;
     }
   } else {
     // Not installed
     button.className = 'install-button prevent-card-click';
     button.textContent = 'Install';
     button.disabled = false;
   }
   
   button.dataset.downloadUrl = extension.download_url;
   button.dataset.extensionId = extension.id;
   
   // Prevent button click from triggering card expansion
   button.addEventListener('click', function(event) {
       event.stopPropagation();
       if (!extension.installed || extension.needsUpdate) {
           // Show "Installing..." state immediately
           this.textContent = '';
           this.classList.add('loading');
           this.disabled = true;
           
           console.log(' Clicked ' + (extension.needsUpdate ? 'Update' : 'Install') + ' button for', extension.name);
           handleDownload(extension, this);
       }
   });
   
   actionContainer.appendChild(button);
   mainContent.appendChild(actionContainer);
   
   // Add to card
   card.appendChild(mainContent);
   
   // Create expandable content area (hidden by default)
   const extraContent = document.createElement('div');
   extraContent.className = 'extension-extra';
   
   // Description (only shown when expanded)
   const description = document.createElement('div');
   description.className = 'extension-description';
   description.textContent = extension.description;
   extraContent.appendChild(description);
   
   // GitHub button (only shown when expanded)
   if (extension.github_url) {
       const githubButton = document.createElement('button');
       githubButton.className = 'github-button prevent-card-click';
       githubButton.textContent = 'GitHub Source';
       githubButton.addEventListener('click', function(event) {
           event.stopPropagation();
           window.open(extension.github_url, '_blank');
       });
       extraContent.appendChild(githubButton);
   }
   
   card.appendChild(extraContent);
   
   // Toggle expansion on card click
   card.addEventListener('click', function() {
       this.classList.toggle('extension-expanded');
   });

   return card;
}

/**
 * Handles downloading and installing an extension
 * @param {ExtensionInfo} extension The extension to install
 * @param {HTMLButtonElement} button The button element that was clicked
 */
function handleDownload(extension, button) {
    console.log('Starting download process for:', extension.name);
    
    // Check if the extension is already installed
    if (extension.installed && !extension.needsUpdate) {
        console.log('Extension already installed:', extension.name);
        button.textContent = 'Installed';
        button.disabled = true;
        button.classList.remove('loading');
        button.className = 'install-button installed prevent-card-click';
        return;
    }
    
    // Update button state
    button.textContent = '';
    button.classList.add('loading');
    button.disabled = true;
    
    // Store the extension ID in the button's dataset for reference
    button.dataset.extensionId = extension.id;

    try {
        // Use direct navigation to the download URL
        console.log('Navigating to download URL:', extension.download_url);
        window.location.href = extension.download_url;
        
        // Set up a retry mechanism to check installation status multiple times
        let checkCount = 0;
        const maxChecks = 5;
        const checkInterval = 2000; // 2 seconds between checks
        
        const checkInstallation = () => {
            checkCount++;
            console.log(`Check #${checkCount} for installation of ${extension.name} (ID: ${extension.id})`);
            
            // Refresh the installed extensions list
            browserBridge.fetchInstalledExtensions();
            
            // Wait a bit for the fetch to complete
            setTimeout(() => {
                // Check if the extension is now installed by ID
                const isNowInstalled = browserBridge.isExtensionInstalled(extension.id);
                console.log(`Installation check #${checkCount} result for ${extension.name} (ID: ${extension.id}):`, isNowInstalled);
                
                // If the button still exists and the extension is installed, update it
                if (button) {
                    if (isNowInstalled) {
                        console.log('Extension is now installed:', extension.name);
                        updateInstalledState(extension, button);
                    } else if (checkCount < maxChecks) {
                        // Try again after a delay
                        console.log(`Installation not detected yet, trying again in ${checkInterval/1000} seconds...`);
                        setTimeout(checkInstallation, checkInterval);
                    } else {
                        // If still not installed after all checks, revert the button
                        console.log('Extension installation not detected after', maxChecks, 'attempts:', extension.name);
                        button.textContent = extension.needsUpdate ? 'Update' : 'Install';
                        button.disabled = false;
                        button.className = extension.needsUpdate ? 
                            'install-button update prevent-card-click' : 
                            'install-button prevent-card-click';
                    }
                }
            }, 500); // Short delay after fetching
        };
        
        // Start the first check after a delay
        setTimeout(checkInstallation, 2000);
        
    } catch (error) {
        console.error('Installation failed:', error);
        button.classList.remove('loading');
        button.textContent = extension.needsUpdate ? 'Update' : 'Retry';
        button.disabled = false;
        button.className = extension.needsUpdate ? 
            'install-button update prevent-card-click' : 
            'install-button prevent-card-click';
    }
}

/**
 * Updates the UI to reflect that an extension has been installed
 * @param {ExtensionInfo} extension The extension that was installed
 * @param {HTMLButtonElement} button The button element to update
 */
function updateInstalledState(extension, button) {
    console.log('Updating UI for installed extension:', extension.name, '(ID:', extension.id, ')');
    button.classList.remove('loading');
    button.textContent = 'Installed';
    button.className = 'install-button installed prevent-card-click';
    button.disabled = true;
    
    // Store the extension ID for reference (using data attribute)
    button.dataset.extensionId = extension.id;
    
    // Mark as installed in the extension object
    extension.installed = true;
    extension.needsUpdate = false;
    
    // Refresh installed extensions information
    browserBridge.fetchInstalledExtensions();
}

// Function to clear and update the extensions list
function updateExtensionsList(extensions) {
    const extensionsList = document.getElementById('extensionsList');
   
   // Remove all existing cards
   while (extensionsList.firstChild) {
       extensionsList.removeChild(extensionsList.firstChild);
   }
   
   // Add new cards
   if (extensions && extensions.length > 0) {
       extensions.forEach(extension => {
           const uiExtension = convertToUIFormat(extension);
           const card = createExtensionCard(uiExtension);
           extensionsList.appendChild(card);
       });
   } else {
       // If no extensions, show a message
       const noExtensions = document.createElement('div');
       noExtensions.className = 'no-extensions';
       noExtensions.textContent = 'No extensions available';
       extensionsList.appendChild(noExtensions);
   }
}

// Update extension-specific DOM elements when data changes
function updateExtensionCard(extension) {
   const uiExtension = convertToUIFormat(extension);
   const existingCard = document.querySelector(`.extension-card[data-id="${uiExtension.id}"]`);
   
   if (existingCard) {
       // Update the icon if it's now available
       const iconElement = existingCard.querySelector('.extension-icon');
       if (iconElement && uiExtension.icon) {
           iconElement.src = uiExtension.icon;
       }
       
       // Other fields could be updated here if needed
   }
}

// Listen for extension data updates to refresh specific cards
window.addEventListener('extension-data-updated', (event) => {
    console.log('Extension data updated:', event.detail);
    if (event.detail && event.detail.id) {
        updateExtensionCard(event.detail);
    }
});

// Listen for installed extensions updates
window.addEventListener('installed-extensions-updated', (event) => {
    console.log('Installed extensions updated, refreshing UI');
    
    // Get current extensions and update their cards with new installation status
    const extensions = browserBridge.getCachedExtensions();
    if (extensions && extensions.length > 0) {
        updateExtensionsList(extensions);
    }
});

// Initialize the store
document.addEventListener('DOMContentLoaded', function() {
   // Initialize the browser bridge
   browserBridge.initialize();
   
   // Set up listener for extension data changes
   browserBridge.setExtensionsChangedListener((extensions) => {
       console.log('Extensions changed:', extensions);
       updateExtensionsList(extensions);
   });
   
   // Listen for extension data updates
   window.addEventListener('extension-data-updated', (event) => {
       console.log('Extension data updated:', event.detail);
       // We don't need to update the UI here, as the listener will handle it
   });
   
   // Listen for errors
   window.addEventListener('extension-store-error', (event) => {
       console.error('Extension store error:', event.detail);
       // Show error message to user
       const extensionsList = document.getElementById('extensionsList');
       extensionsList.innerHTML = `<div class="error-message">Error: ${event.detail}</div>`;
   });
});