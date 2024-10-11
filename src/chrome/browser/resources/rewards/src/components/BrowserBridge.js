/* global chrome */

class BrowserBridge {
    constructor() {}
  
    loginWallet(email, password) {
      return this.sendMessage('loginWallet', [email, password]);
    }
  
    getUserProfile(token) {
      console.log('getUserProfile called with token:', token);
      return this.sendMessage('getUserProfile', [token]);
    }
  
    sendMessage(method, args) {
        return new Promise((resolve, reject) => {
            const handleResponse = (response) => {
                try {
                    // Check if the response is a valid JSON
                    const responseData = JSON.parse(response);
                    if (responseData.success) {
                        if (method === 'loginWallet') {
                            resolve(responseData.data);
                        } else if (method === 'getUserProfile') {
                            resolve(responseData.profile);
                        } else {
                            resolve(responseData.data);
                        }
                    } else {
                        // Pass through the specific error message from the server
                        reject(new Error(responseData.error || responseData.message || `${method} failed`));
                    }
                } catch (error) {
                    console.error(`Error parsing ${method} response:`, error);
                    // If parsing fails, assume it's an authentication error
                    if (method === 'loginWallet') {
                        reject(new Error('Invalid Email or Password'));
                    } else {
                        reject(new Error('Error parsing server response'));
                    }
                }
            };
        
            // Register the callback
            if (method === 'getUserProfile') {
                window.handleProfileResponse = handleResponse;
            } else {
                window.handleResponse = handleResponse;
            }
        
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