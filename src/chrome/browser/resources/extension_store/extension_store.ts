/**
 * Extension information interface
 */
interface ExtensionInfo {
  name: string;
  description: string;
  version: string;
  icon_url: string;
  download_url: string;
  icon_base64?: string;
  id: string;
  author: string;
  github_url: string;
  tags: string[];
  created_at: string;
  updated_at: string;
}

/**
 * UI-formatted extension interface
 */
interface UIExtension {
  id: string;
  name: string;
  version: string;
  description: string;
  icon: string;
  tags: string[];
  download_url: string;
  installed: boolean;
  needsUpdate: boolean;
  installedVersion: string | null;
  author: string;
  realExtensionId: string;
  github_url: string;
}

/**
 * Installed extension interface
 */
interface InstalledExtension {
  id: string;
  name: string;
  version: string;
  description: string;
}

/**
 * Chrome WebUI interface
 */
declare const chrome: any;


interface Window {
  chrome: any;
}


/**
 * Handles communication between the extension store page and the browser.
 */
class BrowserBridge {
  private cachedExtensions_: Record<string, ExtensionInfo> = {};
  private installedExtensions_: Record<string, InstalledExtension> = {};
  private extensionsChangedListener_: ((extensions: ExtensionInfo[]) => void) | null = null;
  private static instance_: BrowserBridge | null = null;

  private constructor() {
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
   */
  private setupMessageHandlers_(): void {
    // Handler for extension data from C++
    (window as any).handleExtensionData = (extensionData: any) => {
      console.log('HANDLER: Extension data received from C++:', JSON.stringify(extensionData));
      
      // Ensure extensionData is properly structured
      if (!extensionData) {
        console.error('HANDLER ERROR: Received empty extension data');
        return;
      }
      
      // Extract relevant fields from the extensionData
      const extension: ExtensionInfo = {
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
    (window as any).handleError = (errorMessage: string) => {
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
   */
  getCachedExtensions(): ExtensionInfo[] {
    return Object.values(this.cachedExtensions_);
  }
  
  /**
   * Gets a specific extension by ID.
   */
  getExtensionById(id: string): ExtensionInfo | null {
    return this.cachedExtensions_[id] || null;
  }

  /**
   * Sets up a listener for extensions updates from the browser.
   */
  setExtensionsChangedListener(listener: (extensions: ExtensionInfo[]) => void): void {
    this.extensionsChangedListener_ = listener;
    
    // If we already have extensions, notify the listener immediately
    const extensions = this.getCachedExtensions();
    if (extensions.length > 0) {
      listener(extensions);
    }
  }

  /**
   * Fetches all extensions from the GitHub repository.
   */
  fetchExtensions(): Promise<void> {
    console.log('Requesting extensions from GitHub');
    return this.sendWithLogging_('fetchExtensions', []);
  }

  /**
   * Fetches all installed extensions from the browser.
   */
  fetchInstalledExtensions(): Promise<void> {
    console.log(' Requesting installed extensions from browser');
    
    // Set up handler for installed extensions data if not already set
    if (!(window as any).handleInstalledExtensionsData) {
      (window as any).handleInstalledExtensionsData = (installedExtensionsData: any[]) => {
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
   */
  getInstalledExtensions(): Record<string, InstalledExtension> {
    return this.installedExtensions_ || {};
  }
  
  /**
   * Checks if an extension is installed by ID.
   */
  isExtensionInstalled(extensionId: string): boolean {
    return !!(this.installedExtensions_ && this.installedExtensions_[extensionId]);
  }

  /**
   * Installs an extension from the provided download URL.
   */
  installExtension(downloadUrl: string): Promise<void> {
    console.log('Installing extension from URL:', downloadUrl);
    return this.sendWithLogging_('installExtension', [downloadUrl]);
  }

  /**
   * Fetches an icon for an extension.
   */
  fetchIcon(iconUrl: string, extensionId: string): Promise<void> {
    console.log('Fetching icon from URL:', iconUrl, 'for extension:', extensionId);
    return this.sendWithLogging_('fetchIcon', [iconUrl, extensionId]);
  }

  /**
   * Fetches an extension's download URL.
   */
  fetchDownloadUrl(downloadUrl: string, extensionInfo: ExtensionInfo): Promise<void> {
    console.log('Fetching download URL:', downloadUrl);
    return this.sendWithLogging_('fetchDownloadUrl', [downloadUrl, extensionInfo]);
  }

  /**
   * Sends a message to the browser with the given method name and parameters,
   * and logs the response.
   */
  private async sendWithLogging_(methodName: string, params: any[] = []): Promise<void> {
    try {
      console.log(`Calling ${methodName} with params:`, params);
      // Using chrome.send for WebUI messaging
      window.chrome.send(methodName, params);
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
   */
  static getInstance(): BrowserBridge {
    if (!BrowserBridge.instance_) {
      BrowserBridge.instance_ = new BrowserBridge();
    }
    return BrowserBridge.instance_;
  }

  /**
   * Initializes the Extension Store UI.
   * This should be called when the page loads.
   */
  initialize(): void {
    console.log('INIT: Initializing Extension Store');
    
    // Log the WebUI availability
    if (window.chrome && window.chrome.send) {
      console.log('INIT: chrome.send is available, WebUI communication should work');
    } else {
      console.error('INIT: chrome.send not available! WebUI communication will fail');
    }
    
    // Check if our handlers are properly registered
    if ((window as any).handleExtensionData && (window as any).handleError) {
      console.log('INIT: All message handlers are registered');
    } else {
      console.error('INIT: Message handlers not properly registered!');
      console.log('INIT: handleExtensionData available:', !!(window as any).handleExtensionData);
      console.log('INIT: handleError available:', !!(window as any).handleError);
    }
    
    // Fetch extensions on initialization
    console.log('INIT: Triggering initial extension fetch');
    this.fetchExtensions();
    
    // Fetch installed extensions on initialization
    console.log(' Triggering initial installed extensions fetch');
    this.fetchInstalledExtensions();
  }
}

// Initialize the browser bridge
const browserBridge = BrowserBridge.getInstance();

/**
 * Compare two version strings
 */
function compareVersions(v1: string, v2: string): number {
  const parts1 = v1.split('.').map(Number);
  const parts2 = v2.split('.').map(Number);
  
  // Compare each part of the version
  for (let i = 0; i < Math.max(parts1.length, parts2.length); i++) {
    const part1 = i < parts1.length ? parts1[i] : 0;
    const part2 = i < parts2.length ? parts2[i] : 0;
    
    const a = part1 ?? 0;
    const b = part2 ?? 0;
    if (a > b) return 1;
    if (a < b) return -1;
  }
  
  return 0; // Versions are equal
}

// Function to create extension cards
function createExtensionCard(extension: UIExtension): HTMLDivElement {
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
  
  button.dataset['downloadUrl'] = extension.download_url;
  button.dataset['extensionId'] = extension.id;
  
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
 */
function handleDownload(extension: UIExtension, button: HTMLButtonElement): void {
  console.log('Starting download process for:', extension.name);
  
  // Check if already installed
  if (extension.installed && !extension.needsUpdate) {
    console.log('Extension already installed:', extension.name);
    button.textContent = 'Installed';
    button.disabled = true;
    button.classList.remove('loading');
    button.className = 'install-button installed prevent-card-click';
    return;
  }
  
  // Show loading state
  button.textContent = '';
  button.classList.add('loading');
  button.disabled = true;
  
  try {
    // Initiate download
    console.log('Navigating to download URL:', extension.download_url);
    window.location.href = extension.download_url;
    
    // Check installation status
    const checkInstallation = () => {
      // Refresh installed extensions list
      browserBridge.fetchInstalledExtensions();
      
      // Check if installed after a short delay
      setTimeout(() => {
        const isNowInstalled = browserBridge.isExtensionInstalled(extension.id);
        
        if (isNowInstalled) {
          // Update UI to installed state
          updateInstalledState(extension, button);
        } else {
          // Keep checking
          setTimeout(checkInstallation, 2000);
        }
      }, 1);
    };
    
    // Start checking after initial delay
    setTimeout(checkInstallation, 2000);
    
  } catch (error) {
    console.error('Installation failed:', error);
    // Keep showing loading state since installation will continue anyway
  }
}

/**
 * Updates the UI to reflect that an extension has been installed
 */
function updateInstalledState(extension: UIExtension, button: HTMLButtonElement): void {
  console.log('Updating UI for installed extension:', extension.name, '(ID:', extension.id, ')');
  button.classList.remove('loading');
  button.textContent = 'Installed';
  button.className = 'install-button installed prevent-card-click';
  button.disabled = true;
  
  // Store the extension ID for reference (using data attribute)
  button.dataset['extensionId'] = extension.id;
  
  // Mark as installed in the extension object
  extension.installed = true;
  extension.needsUpdate = false;
  
  // Refresh installed extensions information
  browserBridge.fetchInstalledExtensions();
}

// Function to clear and update the extensions list
function updateExtensionsList(extensions: ExtensionInfo[]): void {
  const extensionsList = document.getElementById('extensionsList')!;
  
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
function updateExtensionCard(extension: ExtensionInfo): void {
  const uiExtension = convertToUIFormat(extension);
  const existingCard = document.querySelector(`.extension-card[data-id="${uiExtension.id}"]`) as HTMLElement;
  
  if (existingCard) {
    // Update the icon if it's now available
    const iconElement = existingCard.querySelector('.extension-icon') as HTMLImageElement;
    if (iconElement && uiExtension.icon) {
      iconElement.src = uiExtension.icon;
    }
    
    // Other fields could be updated here if needed
  }
}

// Function to convert backend extension format to UI format
function convertToUIFormat(extension: ExtensionInfo): UIExtension {
  // Check if this extension is installed by ID
  const isInstalled = browserBridge.isExtensionInstalled(extension.id);
  const installedExtension = isInstalled ? browserBridge.getInstalledExtensions()[extension.id] : null;
  
  // Check if an update is available (only if installed)
  let needsUpdate = false;
  let installedVersion: string | null = null;
  
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
    realExtensionId: extension.id,
    github_url: extension.github_url || ''
  };
}

// Listen for extension data updates to refresh specific cards
window.addEventListener('extension-data-updated', (event: Event) => {
    const customEvent = event as CustomEvent;
    console.log('Extension data updated:', customEvent.detail);
    if (customEvent.detail && customEvent.detail.id) {
        updateExtensionCard(customEvent.detail);
    }
});

window.addEventListener('installed-extensions-updated', () => {
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
  window.addEventListener('extension-data-updated', (event: Event) => {
    const customEvent = event as CustomEvent;
    console.log('Extension data updated:', customEvent.detail);
    // We don't need to update the UI here, as the listener will handle it
  });
  
  // Listen for errors
  window.addEventListener('extension-store-error', (event: Event) => {
    const customEvent = event as CustomEvent;
    console.error('Extension store error:', customEvent.detail);
    // Show error message to user
    const extensionsList = document.getElementById('extensionsList')!;
    extensionsList.innerHTML = `<div class="error-message">Error: ${customEvent.detail}</div>`;
  });
});