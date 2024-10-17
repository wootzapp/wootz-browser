// import {$} from 'chrome://resources/js/util.js';
// import {BrowserBridge} from './browser_bridge.js';
// import {addNode, addNodeWithText} from './util.js';
// import {DivView} from './view.js';

document.addEventListener('DOMContentLoaded', async function () {
    const connectButton = document.getElementById('connectButton');
    const modal = document.getElementById('id01');
    const loginForm = document.getElementById('loginForm');

    // Show the modal when the button is clicked
    connectButton.addEventListener('click', function () {
      // modal.style.display = 'block';
      login()
    });

    async function login()
    {
      try {
        const response = await fetch('https://api-staging-0.gotartifact.com/v2/users/authentication/signin', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify({
            email: "email@gmail.com",
            password: "password"
          })
        });

        const result = await response.json();

        if (response.ok) {
          console.log("Login Successfull ",result);
          // Handle successful login, e.g., save token, redirect, etc.
        } else {
          console.log("Login failed ",result);

        }
      } catch (error) {
        console.error('Error:', error);
      }
    }

    // Close the modal when clicking outside of it
    window.addEventListener('click', function (event) {
      if (event.target === modal) {
        modal.style.display = 'none';
      }
    });

    // Handle form submission
    loginForm.addEventListener('submit', async function(event) {
      event.preventDefault();

      const email = document.querySelector('input[name="uname"]').value;
      const password = document.querySelector('input[name="psw"]').value;

    
    });
  });