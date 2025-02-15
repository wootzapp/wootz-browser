class BrowserBridge {
  constructor() {
    this.extensions = [];
  }

  fetchExtensions() {
    return this.sendMessage('fetchExtensions', []);
  }

  // getExtensionInfo() {
  //   return this.sendMessage('getExtensionsInfo', []);
  // }

  sendMessage(method, args) {
    return new Promise((resolve, reject) => {
      const handleResponse = (response) => {
        try {
          const responseData = JSON.parse(response);
          if (responseData) {
            resolve(responseData);
          } else {
            reject(new Error('Invalid response from server'));
          }
        } catch (error) {
          console.error(`Error parsing ${method} response:`, error);
          reject(new Error('Error parsing server response'));
        }
      };

      // Register the callback
      window.handleExtensionData = handleResponse;

      // Call the C++ function
      chrome.send(method, args);
    });
  }

  static getInstance() {
    if (!this.instance) {
      this.instance = new BrowserBridge();
    }
    return this.instance;
  }
}

export default BrowserBridge;