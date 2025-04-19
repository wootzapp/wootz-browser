/**
 * Handles communication between the startup CRX install page and the browser.
 */

let utmSource = '';
class BrowserBridge {
  /** @private */
  
  constructor() {
    /** @private {string} */
    this.utmSource_ = '';

    /** @private {Object<string, Object>} */
    this.installedExtensions_ = {};

    // Set up message handlers first before making any calls
    this.getUtmSource();
    this.setupMessageHandlers_();

    this.fetchInstalledExtensions();

    if (this.utmSource_ === '') {
      setTimeout(() => {
        this.getUtmSource();
      }, 1000);
    }

    setInterval(() => {
      this.fetchInstalledExtensions();
    }, 1000);
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

    // Handler for UTM source data from C++
    let isUtmSourceSet = false;
    window.handleUtmSource = (utmSource) => {
      console.log('handleUtmSource',typeof utmSource);
      console.log('Received UTM source from C++:', utmSource);
      this.utmSource_ = utmSource || '';
      console.log('this.utmSource_',this.utmSource_);
      if(this.utmSource_ !== '' && !isUtmSourceSet) {
        console.log('Setting up UI');
        utmSource = this.utmSource_;
        setupUI(utmSource);
        isUtmSourceSet = true;
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
      
      // Check if the extension corresponding to UTM source is already installed
      if (this.utmSource_ !== '') {
        checkAndHandleInstalledExtension(this.utmSource_, this.installedExtensions_);
      }
    };

    // Add error handler
    window.handleError = (errorMessage) => {
      console.error('Error from C++:', errorMessage);
    };
  }

  /**
   * Gets the current UTM source.
   * @return {string}
   */
  getUtmSourceValue() {
    console.log('Getting UTM source value:', this.utmSource_);
    return this.utmSource_;
  }

  /**
   * Fetches the UTM source from the browser.
   */
  getUtmSource() {
    console.log('Requesting UTM source from browser');
    return this.sendWithLogging_('getUtmSource', []);
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
    
    // Fetch UTM source
    this.getUtmSource();
  }
}

/** @private {BrowserBridge} */
BrowserBridge.instance_ = null;

// Create a global browserBridge variable
const browserBridge = BrowserBridge.getInstance();

// Initialize when the DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
  console.log('DOM content loaded, initializing BrowserBridge');
  browserBridge.initialize();
});

function DownloadExtension(utmParam) {
  let downloadUrl;
  console.log('DownloadExtension', utmParam);
  if(utmParam !== '') {
    switch(utmParam.toLowerCase()) {
      case 'artifact':
        downloadUrl = "https://raw.githubusercontent.com/wootzapp/ext-store/main/Artifact/Artifact.crx";
        break;
      case 'eclipse':
        downloadUrl = "https://raw.githubusercontent.com/wootzapp/ext-store/main/Eclipse/tap.eclipse.crx";
        break;
      case 'blockmesh':
        downloadUrl = "https://raw.githubusercontent.com/wootzapp/ext-store/main/Blockmesh%20Network/Blockmesh.crx";
        break;
      case 'camp':
        downloadUrl = "https://raw.githubusercontent.com/wootzapp/ext-store/main/Camp%20Network/CampNetwork.crx";
        break;
      case 'sapien':
        downloadUrl = "https://raw.githubusercontent.com/wootzapp/ext-store/main/Sapien/Sapien.crx";
        break;
      default:
        console.log('Unknown UTM source:', utmParam);
        window.close();
        break;
    }
    if(downloadUrl) {
      window.location.href = downloadUrl;
    }
  } 
}

/**
 * Checks if the extension corresponding to the UTM source is already installed
 * and closes the window if it is.
 * @param {string} utmParam The UTM source parameter
 * @param {Object} installedExtensions Map of installed extensions
 */
function checkAndHandleInstalledExtension(utmParam, installedExtensions) {
  console.log('Checking if extension for UTM source is installed:', utmParam);
  
  // Map UTM sources to extension IDs
  const utmToExtensionId = {
    'artifact': 'allmdcfldidgeghfoioaaiammdlpmnnk',   
    'eclipse': 'gpfellaldmjpgonllcjpjfpodfmgobnk',    
    'blockmesh': 'kpobgdhknoakgagflffeigaojlglkbhn', 
    'camp': 'amjlbcejmaebkjfmeenkcijgpjpieepc',
    'sapien': 'nofldplihhlkcpbejlmccfafcpeejaef'    
  };
  
  const extensionId = utmToExtensionId[utmParam.toLowerCase()];
  console.log('extensionId',extensionId);
  if (extensionId && installedExtensions[extensionId]) {
    console.log('installedExtensions',installedExtensions);
    console.log('Extension already installed, closing window');
    // Close the window after a short delay
    setTimeout(() => {
      window.location.href = "wootzapp://newtab/";
    }, 500);
  }
}

function setupUI(utmSource) {
    // Create splash container
    const splashContainer = document.createElement('div');
    splashContainer.className = 'splash-container';
    
    // Create logo container
    const logoContainer = document.createElement('div');
    logoContainer.className = 'logo-container';

    const appLogo = document.createElement('div');
    appLogo.id = 'app-logo';

    const appTitle = document.createElement('div');
    appTitle.id = 'app-title';

    // Set logo and title based on UTM source
    if (utmSource !== '') {
        switch (utmSource.toLowerCase()) {
            case "artifact":
                const artifactImg = document.createElement('img');
                artifactImg.src = 'ic_launcher_artifact.png';
                artifactImg.style.width = '100%';
                artifactImg.style.height = '100%';
                artifactImg.style.objectFit = 'contain';
                appLogo.appendChild(artifactImg);
                appTitle.textContent = "Artifact";
                break;
            case "eclipse":
                const eclipseImg = document.createElement('img');
                eclipseImg.src = 'ic_launcher_eclipse.png';
                eclipseImg.style.width = '100%';
                eclipseImg.style.height = '100%';
                eclipseImg.style.objectFit = 'contain';
                appLogo.appendChild(eclipseImg);
                appTitle.textContent = "Eclipse";
                break;
            case "blockmesh":
                const blockmeshImg = document.createElement('img');
                blockmeshImg.src = 'ic_launcher_blockmesh.png';
                blockmeshImg.style.width = '100%';
                blockmeshImg.style.height = '100%';
                blockmeshImg.style.objectFit = 'contain';
                appLogo.appendChild(blockmeshImg);
                appTitle.textContent = "Blockmesh";
                break;
            case "camp":
                const campImg = document.createElement('img');
                campImg.src = 'ic_launcher_camp.png';
                campImg.style.width = '100%';
                campImg.style.height = '100%';
                campImg.style.objectFit = 'contain';
                appLogo.appendChild(campImg);
                appTitle.textContent = "Camp Network";
                break;
            case "sapien":
                const sapienImg = document.createElement('img');
                sapienImg.src = 'ic_launcher_sapien.png';
                sapienImg.style.width = '100%';
                sapienImg.style.height = '100%';
                sapienImg.style.objectFit = 'contain';
                appLogo.appendChild(sapienImg);
                appTitle.textContent = "Sapien";
                break;
            default:
                appTitle.textContent = "Browser Extension";
                break;
        }
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
    const poweredLogo = document.createElement('img');
    poweredLogo.className = 'wootzapp-logo';
    poweredLogo.id = 'powered-logo';
    poweredLogo.src ='powered_by_wootzapp.png';
    poweredLogo.style.width = '70vw';
    poweredLogo.style.height = 'auto';
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

        // Insert the loader after the logo container
        logoContainer.insertAdjacentElement('afterend', loaderContainer);
        
        // Start download after animation
        setTimeout(() => {
            DownloadExtension(utmSource);
        }, 3000);
    });
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
