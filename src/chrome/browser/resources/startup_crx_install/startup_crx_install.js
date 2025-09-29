/**
 * Handles communication between the startup CRX install page and the browser.
 */

class BrowserBridge {
  /** @private */
  
  constructor() {
    /** @private {Object<string, Object>} */
    this.installedExtensions_ = {};

    /** @private {Object} */
    this.extensionData_ = null;

    // Set up message handlers first
    this.setupMessageHandlers_();

    this.fetchInstalledExtensions();
    
    // Request extension data from backend
    this.requestExtensionData();

    setInterval(() => {
      this.fetchInstalledExtensions();
    }, 1000);

    // Force redirect to newtab after 1 minute (60 seconds)
    setTimeout(() => {
      console.log('Auto-redirecting to newtab after 1 minute timeout');
      window.location.href = 'wootzapp://newtab';
    }, 60000);
  }

  /**
   * Sets up handlers for messages from C++.
   * @private
   */
  setupMessageHandlers_() {
    console.log('Setting up message handlers');
    
    window.onerror = function(msg,url,line) {
      console.error('JavaScript error:', msg, 'at', url, ':', line);
    };

    // Handler for dynamic extension data from C++
    window.handleExtensionData = (extensionData) => {
      console.log('Received extension data from C++:', extensionData);
      console.log('Extension ID received from C++:', extensionData ? extensionData.id : 'No ID');
      this.extensionData_ = extensionData;
      
      if (this.extensionData_) {
        console.log('Setting up UI with extension data');
        console.log('Extension ID being used for UI setup:', this.extensionData_.id);
        setupUI(this.extensionData_);
      }
    };
      
    // Handler for installed extensions data from C++
    window.handleInstalledExtensionsData = (installedExtensionsData) => {
      console.log('Received installed extensions data from C++:', 
        installedExtensionsData ? installedExtensionsData.length : 0, 'extensions');

      // Clear existing installed extensions data
      this.installedExtensions_ = {};

      // Process each installed extension
      if (installedExtensionsData && installedExtensionsData.length) {
        installedExtensionsData.forEach(extension => {
          if (extension && extension.id) {
            // Store in map with ID as key
            console.log('extension',extension);
            console.log('extension.id',extension.id);
            this.installedExtensions_[extension.id] = extension;
          }
        });
      }
      
      // Log all installed extensions
      console.log('Installed extensions:', this.installedExtensions_);
      
      // Check if the current extension is already installed
      if (this.extensionData_ && this.extensionData_.id) {
        let extensionIdsToCheck = [this.extensionData_.id];
        
        // For coddata extension, also check the hardcoded ID
        if (this.extensionData_.name && this.extensionData_.name.toLowerCase().includes('coddata')) {
          console.log('DEBUG: Detected coddata extension, adding hardcoded ID to check list');
          const hardcodedId = 'fpjibejhpgjibaaakldgdjnkkfmfilih';
          if (!extensionIdsToCheck.includes(hardcodedId)) {
            extensionIdsToCheck.push(hardcodedId);
          }
        }
        
        console.log('Original extension ID from C++:', this.extensionData_.id);
        console.log('Extension IDs to check:', extensionIdsToCheck);
        
        checkAndHandleInstalledExtension(extensionIdsToCheck, this.installedExtensions_);
      }
    };

    // Add error handler
    window.handleError = (errorMessage) => {
      console.error('Error from C++:', errorMessage);
    };
    
    // Add download progress handler
    window.handleDownloadProgress = (progressData) => {
      console.log('Download progress:', progressData);
      if (progressData && progressData.extensionName) {
        console.log('Downloading extension:', progressData.extensionName);
        // Update UI to show download progress
        const progressText = document.getElementById('progress-text');
        if (progressText) {
          progressText.textContent = `Installing ${progressData.extensionName}...`;
        }
      }
    };
    
    // Add UTM source handler
    window.handleUtmSource = (utmSource) => {
      console.log('Received UTM source from C++:', utmSource);
      if (utmSource && utmSource.trim() !== '') {
        console.log('UTM source is available:', utmSource);
      } else {
        console.log('UTM source is empty, will use default extensions');
      }
    };

    // Add handler for default extension installation progress
    window.handleDefaultExtensionProgress = (progressData) => {
      console.log('Default extension installation progress:', progressData);
      const currentIndex = progressData.currentIndex + 1;
      const totalCount = progressData.totalCount;
      const extensionName = progressData.extensionName;
      const state = progressData.state;
      
      // Hide the main progress text since we have progress in the card
      const progressText = document.getElementById('progress-text');
      if (progressText) {
        progressText.style.display = 'none';
      }
      
      // Update the UI to show progress
      const progressElement = document.getElementById('default-extension-progress');
      if (progressElement) {
        const progressFill = progressElement.querySelector('.progress-fill');
        if (progressFill) {
          progressFill.style.width = `${(currentIndex / totalCount) * 100}%`;
        }
        
        const statusText = progressElement.querySelector('p:not(.status)');
        if (statusText) {
          statusText.textContent = `Installing ${currentIndex} of ${totalCount}: ${extensionName}`;
        }
        
        const statusElement = progressElement.querySelector('.status');
        if (statusElement) {
          statusElement.textContent = state;
        }
      }
    };

    // Add handler for when all default extensions are processed
    window.handleDefaultExtensionsComplete = () => {
      console.log('All default extensions have been processed');
      
      // Hide the main progress text since we have progress in the card
      const progressText = document.getElementById('progress-text');
      if (progressText) {
        progressText.style.display = 'none';
      }
      
      // Update the progress UI to show completion
      const progressElement = document.getElementById('default-extension-progress');
      if (progressElement) {
        const progressFill = progressElement.querySelector('.progress-fill');
        if (progressFill) {
          progressFill.style.width = '100%';
        }
        
        const titleElement = progressElement.querySelector('h3');
        if (titleElement) {
          titleElement.textContent = 'Default Extensions Installed Successfully!';
        }
        
        const statusText = progressElement.querySelector('p:not(.status)');
        if (statusText) {
          statusText.textContent = 'All default extensions have been installed.';
        }
        
        const statusElement = progressElement.querySelector('.status');
        if (statusElement) {
          statusElement.textContent = 'Complete';
        }
      }
      
      // Navigate to new tab after a short delay to show completion message
      console.log('Navigating to new tab after default extensions completion');
      setTimeout(() => {
        try {
          // Notify C++ that default extensions are complete
          chrome.send('onDefaultExtensionsComplete', []);
          
          // Try multiple approaches to ensure navigation works
          window.location.href = 'wootzapp://newtab';
          
          // Fallback: try to close the window if navigation doesn't work
          setTimeout(() => {
            console.log('Attempting to close window as fallback');
            window.close();
          }, 1000);
          
        } catch (error) {
          console.error('Error navigating to new tab:', error);
          // Fallback: try to close the window
          window.close();
        }
      }, 2000); // 2 second delay to show completion message
    };
  }

  /**
   * Gets the current extension data.
   * @return {Object}
   */
  getExtensionData() {
    console.log('Getting extension data:', this.extensionData_);
    return this.extensionData_;
  }

  /**
   * Requests extension data from the browser.
   */
  requestExtensionData() {
    console.log('Requesting extension data from browser');
    return this.sendWithLogging_('getExtensionData', []);
  }

  /**
   * Fetches all installed extensions from the browser.
   */
  fetchInstalledExtensions() {
    console.log('Requesting installed extensions from browser');
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
   * Sends a message to the browser with the given method name and parameters.
   * @param {string} methodName The name of the method to call.
   * @param {Array=} params The parameters to pass to the method.
   * @return {!Promise<void>}
   * @private
   */
  async sendWithLogging_(methodName, params = []) {
    try {
      console.log(`Calling ${methodName} with params:`, params);
      // Using chrome.send for WebUI messaging
      chrome.send(methodName, params);
      return Promise.resolve();
    } catch (error) {
      console.error(`Error (${methodName}):`, error);
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
   * Initializes the Startup CRX Install UI.
   */
  initialize() {
    console.log('Initializing Startup CRX Install');

    // Log the WebUI availability
    if (chrome && chrome.send) {
      console.log('chrome.send is available, WebUI communication should work');
    } else {
      console.error('chrome.send not available! WebUI communication will fail');
    }

    // Fetch installed extensions
    this.fetchInstalledExtensions();
    
    // Request extension data
    this.requestExtensionData();
  }
}

/** @private {BrowserBridge} */
BrowserBridge.instance_ = null;

// Create a global browserBridge variable
const browserBridge = BrowserBridge.getInstance();

// Initialize when the DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
  const params = new URLSearchParams(window.location.search);
  if (params.get('install_default_extensions') === 'true') {
    console.log('install_default_extensions=true detected, calling installDefaultExtensions');
    
    // Show default UI for default extensions installation
    showDefaultExtensionsUI();
    
    // Set up timeout for no internet connection and no extensions
    let hasProgress = false;
    let timeoutId = null;
    
    // Start timeout after 3 seconds if no progress is made
    timeoutId = setTimeout(() => {
      if (!hasProgress) {
        console.log('No progress detected after 3 seconds, likely no internet connection');
        const progressElement = document.getElementById('default-extension-progress');
        if (progressElement) {
          const statusElement = progressElement.querySelector('.status');
          if (statusElement) {
            statusElement.textContent = 'Cannot able to fetch extensions. Please check your internet connection.';
          }
          const statusText = progressElement.querySelector('p:not(.status)');
          if (statusText) {
            statusText.textContent = 'Closing window...';
          }
        }
        
        // Close window after showing message
        setTimeout(() => {
          window.close();
        }, 1000);
      }
    }, 3000);
    
    // Track progress to cancel timeout
    const originalHandleProgress = window.handleDefaultExtensionProgress;
    window.handleDefaultExtensionProgress = (progressData) => {
      hasProgress = true;
      if (timeoutId) {
        clearTimeout(timeoutId);
        timeoutId = null;
      }
      // Call original handler
      if (originalHandleProgress) {
        originalHandleProgress(progressData);
      }
    };
    
    // Also track completion to cancel timeout
    const originalHandleComplete = window.handleDefaultExtensionsComplete;
    window.handleDefaultExtensionsComplete = () => {
      hasProgress = true;
      if (timeoutId) {
        clearTimeout(timeoutId);
        timeoutId = null;
      }
      // Call original handler
      if (originalHandleComplete) {
        originalHandleComplete();
      }
    };
    
    chrome.send('installDefaultExtensions', []);
  } else {
    console.log('No install_default_extensions parameter, showing default UI');
    // Ensure the default UI is visible
    const defaultExtensionProgress = document.getElementById('default-extension-progress');
    if (defaultExtensionProgress) {
      defaultExtensionProgress.style.display = 'none';
    }
  }
  
  // Get UTM source for debugging
  chrome.send('getUtmSource', []);    
  
  browserBridge.initialize();
});

function showDefaultExtensionsUI() {
  console.log('Showing default extensions UI');
  
  // Update the existing UI elements instead of clearing everything
  const appTitle = document.getElementById('app-title');
  if (appTitle) {
    appTitle.textContent = 'Installing Default Extensions';
  }
  
  const progressText = document.getElementById('progress-text');
  if (progressText) {
    progressText.style.display = 'none';
  }
  
  // Hide the main progress bar since we have one in the progress card
  const mainProgress = document.getElementById('progress');
  if (mainProgress) {
    mainProgress.style.display = 'none';
  }
  
  // Hide the main progress bar container
  const progressBar = document.querySelector('.progress-bar');
  if (progressBar) {
    progressBar.style.display = 'none';
  }
  
  // Hide the continue button since we don't need it for default extensions
  const continueBtn = document.getElementById('continue-btn');
  if (continueBtn) {
    continueBtn.style.display = 'none';
  }
  
  // Show the default extension progress (it's already in the HTML)
  const defaultExtensionProgress = document.getElementById('default-extension-progress');
  if (defaultExtensionProgress) {
    defaultExtensionProgress.style.display = 'block';
    // Ensure it's properly positioned
    defaultExtensionProgress.style.marginTop = '2rem';
  }
  
  // Update the app logo to show Wootzapp logo
  const appLogo = document.getElementById('app-logo');
  if (appLogo && !appLogo.innerHTML.trim()) {
    const wootzappImg = document.createElement('img');
    wootzappImg.src = 'Wootzapp.png';
    wootzappImg.style.width = '100%';
    wootzappImg.style.height = '100%';
    wootzappImg.style.objectFit = 'contain';
    wootzappImg.onerror = function() {
      console.warn('Failed to load Wootzapp logo, using fallback');
      appLogo.innerHTML = '<div style="width: 100%; height: 100%; background: #f0f0f0; border-radius: 12px; display: flex; align-items: center; justify-content: center; font-size: 48px; font-weight: bold; color: #666;">W</div>';
    };
    appLogo.appendChild(wootzappImg);
  }

  // Ensure powered by logo is visible and styled
  const poweredLogo = document.getElementById('powered-logo');
  if (poweredLogo) {
    poweredLogo.src = 'powered_by_wootzapp.png';
    poweredLogo.style.width = '200px';
    poweredLogo.style.height = 'auto';
    poweredLogo.style.maxWidth = '70vw';
    poweredLogo.style.display = 'block';
    poweredLogo.alt = 'Powered by WOOTZAPP';
  }
}

function DownloadExtension(extensionData) {
  console.log('DownloadExtension called with extension data:', extensionData);
  
  if (extensionData && extensionData.download_url) {
    console.log('Downloading extension:', extensionData.name);
    window.location.href = extensionData.download_url;
  } else {
    console.error('No download URL found for extension');
    window.close();
  }
}

function InstallExtensionByUrl(extensionData) {
  console.log('InstallExtensionByUrl called with extension data:', extensionData);
  
  if (extensionData && extensionData.download_url) {
    console.log('Installing extension programmatically:', extensionData.name);
    
    // Use the same download method as Branch - direct window.location.href
    // This is the standard way Chrome handles extension installation
    window.location.href = extensionData.download_url;
    
    // Update progress to show installation is in progress
    const progressText = document.getElementById('progress-text');
    if (progressText) {
      progressText.textContent = `Installing ${extensionData.name}...`;
    }
    
    // Notify completion after a delay to allow installation to process
    setTimeout(() => {
      console.log('Notifying extension installation completion for:', extensionData.name);
      chrome.send('onExtensionInstallComplete', []);
    }, 3000); // Wait 3 seconds for installation to complete,
    // Initiated download of extensions, it will auto download and install, multiple extensions can be download parallely so no need to wait for complete install
    
    console.log('Extension installation initiated for:', extensionData.name);
  } else {
    console.error('No download URL found for extension');
    window.close();
  }
}

/**
 * Checks if any of the extension IDs are already installed and closes the window if found.
 * @param {Array<string>} extensionIds Array of extension IDs to check
 * @param {Object} installedExtensions Map of installed extensions
 */
function checkAndHandleInstalledExtension(extensionIds, installedExtensions) {
  console.log('=== CHECKING EXTENSION INSTALLATION ===');
  console.log('Extension IDs to check:', extensionIds);
  console.log('Expected ID for coddata:', 'fpjibejhpgjibaaakldgdjnkkfmfilih');
  console.log('Available installed extension IDs:', Object.keys(installedExtensions));
  
  // Check if any of the extension IDs are found in installed extensions
  let foundExtension = false;
  let foundId = null;
  
  for (const extensionId of extensionIds) {
    if (extensionId && installedExtensions[extensionId]) {
      foundExtension = true;
      foundId = extensionId;
      break;
    }
  }
  
  if (foundExtension) {
    console.log('installedExtensions', installedExtensions);
    console.log('Extension already installed (matched by ID):', foundId);
    console.log('Extension details:', installedExtensions[foundId]);
    // Close the window after a short delay
    setTimeout(() => {
      window.location.href = 'wootzapp://newtab';
    }, 500);
  } else {
    console.log('Extension not found in installed extensions. Checked IDs:', extensionIds);
    console.log('=== END EXTENSION CHECK ===');
  }
}

async function setupUI(extensionData) {
    console.log('Setting up UI with extension data:', extensionData);
    
    // Create splash container
    const splashContainer = document.createElement('div');
    splashContainer.className = 'splash-container';
    
    // Create logo container
    const logoContainer = document.createElement('div');
    logoContainer.className = 'logo-container';
    // Ensure the logo container is centered and allows proper text alignment
    logoContainer.style.display = 'flex';
    logoContainer.style.flexDirection = 'column';
    logoContainer.style.alignItems = 'center';
    logoContainer.style.textAlign = 'center';

    const appLogo = document.createElement('div');
    appLogo.id = 'app-logo';

    const appTitle = document.createElement('div');
    appTitle.id = 'app-title';    

    // Set logo and title based on extension data
    if (extensionData && extensionData.name) {
        // Use dynamic icon if available (same pattern as extension store)
        if (extensionData.icon_base64) {
            const extensionImg = document.createElement('img');
            extensionImg.src = `data:image/png;base64,${extensionData.icon_base64}`;
            extensionImg.style.width = '100%';
            extensionImg.style.height = '100%';
            extensionImg.style.objectFit = 'contain';
            extensionImg.onerror = function() {
                // Fallback if image fails to load
                console.warn('Failed to load extension icon from base64');
                appLogo.textContent = extensionData.name.charAt(0).toUpperCase();
                appLogo.style.display = 'flex';
                appLogo.style.alignItems = 'center';
                appLogo.style.justifyContent = 'center';
                appLogo.style.fontSize = '48px';
                appLogo.style.fontWeight = 'bold';
                appLogo.style.backgroundColor = '#f0f0f0';
                appLogo.style.borderRadius = '12px';
            };
            appLogo.appendChild(extensionImg);
        } else {
            // Fallback to first letter if no icon
            appLogo.textContent = extensionData.name.charAt(0).toUpperCase();
            appLogo.style.display = 'flex';
            appLogo.style.alignItems = 'center';
            appLogo.style.justifyContent = 'center';
            appLogo.style.fontSize = '48px';
            appLogo.style.fontWeight = 'bold';
            appLogo.style.backgroundColor = '#f0f0f0';
            appLogo.style.borderRadius = '12px';
        }
        
        appTitle.textContent = extensionData.name;
        // Center the extension name under the icon
        appTitle.style.textAlign = 'center';
        appTitle.style.width = '100%';
        appTitle.style.marginTop = '12px';
    } else {
        // Fallback for no extension data
        appTitle.textContent = "Browser Extension";
        // Center the fallback text as well
        appTitle.style.textAlign = 'center';
        appTitle.style.width = '100%';
        appTitle.style.marginTop = '12px';
    }
    
    logoContainer.appendChild(appLogo);
    logoContainer.appendChild(appTitle);

    // Create loading container
    const loadingContainer = document.createElement('div');
    loadingContainer.className = 'loading-container';

    const progressBar = document.createElement('div');
    progressBar.className = 'progress-bar';
    const progress = document.createElement('div');
    progress.className = 'progress';
    progress.id = 'progress';
    progressBar.appendChild(progress);

    const progressText = document.createElement('div');
    progressText.className = 'progress-text';
    progressText.id = 'progress-text';
    progressText.textContent = 'Loading resources...';

    loadingContainer.appendChild(progressBar);
    loadingContainer.appendChild(progressText);
    // Create continue button
    const continueBtn = document.createElement('button');
    continueBtn.className = 'continue-btn';
    continueBtn.id = 'continue-btn';
    continueBtn.textContent = 'Continue';

    // Create powered by section
    const poweredBy = document.createElement('div');
    poweredBy.className = 'powered-by';
    // Position at bottom center
    poweredBy.style.position = 'fixed';
    poweredBy.style.bottom = '20px';
    poweredBy.style.left = '50%';
    poweredBy.style.transform = 'translateX(-50%)';
    poweredBy.style.display = 'flex';
    poweredBy.style.justifyContent = 'center';
    poweredBy.style.alignItems = 'center';
    poweredBy.style.width = '100%';
    poweredBy.style.zIndex = '1000';
    
    const poweredLogo = document.createElement('img');
    poweredLogo.className = 'wootzapp-logo';
    poweredLogo.id = 'powered-logo';
    poweredLogo.src ='powered_by_wootzapp.png';
    poweredLogo.style.width = '200px';
    poweredLogo.style.height = 'auto';
    poweredLogo.style.maxWidth = '70vw';
    poweredBy.appendChild(poweredLogo);

    // Create final animation elements
    const finalAnimation = document.createElement('div');
    finalAnimation.className = 'final-animation';
    finalAnimation.id = 'final-animation';

    const hexagonGrid = document.createElement('div');
    hexagonGrid.className = 'hexagon-grid';
    hexagonGrid.id = 'hexagon-grid';

    const loadingSpinner = document.createElement('div');
    loadingSpinner.className = 'loading-spinner';
    loadingSpinner.id = 'loading-spinner';
    
    for (let i = 0; i < 3; i++) {
        const spinnerRing = document.createElement('div');
        spinnerRing.className = 'spinner-ring';
        loadingSpinner.appendChild(spinnerRing);
    }

    finalAnimation.appendChild(hexagonGrid);
    finalAnimation.appendChild(loadingSpinner);

    // Assemble the UI
    splashContainer.appendChild(logoContainer);
    splashContainer.appendChild(loadingContainer);
    splashContainer.appendChild(continueBtn);
    splashContainer.appendChild(poweredBy);

    // Clear existing body content and add new elements
    while (document.body.firstChild) {
        document.body.removeChild(document.body.firstChild);
    }
    document.body.appendChild(splashContainer);
    document.body.appendChild(finalAnimation);

    // Initialize progress simulation
    let progressValue = 0;
    const progressInterval = setInterval(() => {
        if (progressValue >= 100) {
            clearInterval(progressInterval);
            continueBtn.classList.add('show');
            progressText.textContent = 'Ready to start!';
            return;
        }
        
        const increment = Math.random() * 10;
        progressValue = Math.min(progressValue + increment, 100);
        progress.style.width = `${progressValue}%`;
        progressText.textContent = `Loading resources... ${Math.floor(progressValue)}%`;
    }, 200);

    // Handle continue button click
    continueBtn.addEventListener('click', () => {
        // Remove the loading container
        loadingContainer.remove();
        continueBtn.remove();

        // Create and add the bouncing dots loader
        const loaderContainer = document.createElement('div');
        loaderContainer.className = 'loader';
        
        // Create three dots
        for (let i = 0; i < 3; i++) {
            const dot = document.createElement('div');
            dot.className = 'dot';
            loaderContainer.appendChild(dot);
        }

        // Add custom message text under the bouncing dots
        const downloadMessageText = document.createElement('div');
        downloadMessageText.className = 'custom-message-text';
        downloadMessageText.textContent = `Setting up customized experience for you. \nPlease wait...`;
        downloadMessageText.style.marginTop = '7px';
        downloadMessageText.style.fontSize = '16px';
        downloadMessageText.style.color = '#666';
        downloadMessageText.style.textAlign = 'center';
        downloadMessageText.style.fontWeight = 'bold';
        downloadMessageText.style.whiteSpace = 'pre-line';
        
        // Create a container for the loader and message
        const downloadContainer = document.createElement('div');
        downloadContainer.className = 'download-container';
        downloadContainer.style.display = 'flex';
        downloadContainer.style.flexDirection = 'column';
        downloadContainer.style.alignItems = 'center';
        
        downloadContainer.appendChild(loaderContainer);
        downloadContainer.appendChild(downloadMessageText);

        // Insert the download container after the logo container
        logoContainer.insertAdjacentElement('afterend', downloadContainer);
        
        // Start download after animation
        setTimeout(() => {
            DownloadExtension(extensionData);
        }, 3000);
    });
}

// Handle completion of individual extension installation
function onExtensionInstallComplete() {
    console.log('Extension installation completed, notifying C++ handler');
    chrome.send('onExtensionInstallComplete', []);
}

// Helper function to create hexagon grid
function createHexagonGrid(container) {
    while (container.firstChild) {
        container.removeChild(container.firstChild);
    }
    
    const numHexagons = 12;
    
    for (let i = 0; i < numHexagons; i++) {
        const hexagon = document.createElement('div');
        hexagon.className = 'hexagon';
        
        const randomX = Math.random() * 100;
        const randomY = Math.random() * 100;
        hexagon.style.left = `${randomX}%`;
        hexagon.style.top = `${randomY}%`;
        
        const randomDelay = Math.random() * 2;
        hexagon.style.animation = `hexPulse 2s ${randomDelay}s infinite`;
        
        container.appendChild(hexagon);
    }
}


