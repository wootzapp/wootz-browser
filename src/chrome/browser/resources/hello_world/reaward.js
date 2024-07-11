import {BrowserBridge} from './browser_bridge.js';

document.addEventListener('DOMContentLoaded', function () {
    const connectButton = document.getElementById('connectButton');
    const modal = document.getElementById('id01');
    const loginForm = document.getElementById('loginForm');
    const userXP = document.getElementById('userXP');
    const walletBalance = document.getElementById('walletBalance');

    this.browserBridge_ = BrowserBridge.getInstance();

    // Show the modal when the button is clicked
    connectButton.addEventListener('click', function () {
      modal.style.display = 'block';
    });

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

      // Login the user
      await this.browserBridge_.loginWallet(email, password);

      // Fetch user profile
      const profile = await this.browserBridge_.getUserProfile();
      userXP.textContent = `User XP: ${profile.xp}`;
      walletBalance.textContent = `Wallet Balance: ${profile.walletBalance}`;

      // Close the modal after submitting the form
      modal.style.display = 'none';
    }.bind(this));
});
