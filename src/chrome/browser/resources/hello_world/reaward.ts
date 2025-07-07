// import {$} from 'chrome://resources/js/util.js';
// import {BrowserBridge} from './browser_bridge.js';
// import {addNode, addNodeWithText} from './util.js';
// import {DivView} from './view.js';

interface LoginRequest {
  email: string;
  password: string;
}

interface LoginResponse {
  // Add specific properties based on your API response
  token?: string;
  user?: any;
  message?: string;
}

document.addEventListener('DOMContentLoaded', async function (): Promise<void> {
    const connectButton = document.getElementById('connectButton') as HTMLButtonElement;
    const modal = document.getElementById('id01') as HTMLElement;
    const loginForm = document.getElementById('loginForm') as HTMLFormElement;

    // Show the modal when the button is clicked
    connectButton?.addEventListener('click', function (): void {
      // modal.style.display = 'block';
      login();
    });

    async function login(): Promise<void> {
      try {
        const requestBody: LoginRequest = {
          email: "email@gmail.com",
          password: "password"
        };

        const response = await fetch('https://api-staging-0.gotartifact.com/v2/users/authentication/signin', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(requestBody)
        });

        const result: LoginResponse = await response.json();

        if (response.ok) {
          console.log("Login Successful ", result);
          // Handle successful login, e.g., save token, redirect, etc.
        } else {
          console.log("Login failed ", result);
        }
      } catch (error) {
        console.error('Error:', error);
      }
    }

    // Close the modal when clicking outside of it
    window.addEventListener('click', function (event: MouseEvent): void {
      if (event.target === modal) {
        modal.style.display = 'none';
      }
    });

    // Handle form submission
    loginForm?.addEventListener('submit', async function(event: SubmitEvent): Promise<void> {
      event.preventDefault();

      const emailInput = document.querySelector('input[name="uname"]') as HTMLInputElement;
      const passwordInput = document.querySelector('input[name="psw"]') as HTMLInputElement;
      
      const email: string = emailInput?.value || '';
      const password: string = passwordInput?.value || '';

      // Use the form data to login
      try {
        const requestBody: LoginRequest = {
          email: email,
          password: password
        };

        const response = await fetch('https://api-staging-0.gotartifact.com/v2/users/authentication/signin', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(requestBody)
        });

        const result: LoginResponse = await response.json();

        if (response.ok) {
          console.log("Login Successful ", result);
          // Handle successful login, e.g., save token, redirect, etc.
        } else {
          console.log("Login failed ", result);
        }
      } catch (error) {
        console.error('Error:', error);
      }
    });
});