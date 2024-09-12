class BrowserBridge {
    constructor() {}

    loginWallet(email, password) {
        chrome.send('loginWallet', [email, password]);
    }

    getUserProfile(token) {
        chrome.send('getUserProfile', [token]);
    }

    async loadWallet(token) {
        const encryptionKey = '123123'; // Directly using the encryption key
        try {
            const result = await window.loadWallet(token, encryptionKey);
            console.log('Wallet loaded successfully', result);
            // Update the UI to show success message
            displayMessage('Wallet loaded successfully', 'success');
            displayResponse(result);
        } catch (error) {
            console.error('Error loading wallet:', error);
            // Update the UI to show error message
            displayMessage('Error loading wallet: ' + error.message, 'error');
        }
    }

    static getInstance() {
        if (!this.instance) {
            this.instance = new BrowserBridge();
        }
        return this.instance;
    }
}

function displayMessage(message, type) {
    const messageDiv = document.getElementById('message');
    if (messageDiv) {
        messageDiv.textContent = message;
        messageDiv.className = `message ${type}`; // Add classes like 'success' or 'error' to style appropriately
        messageDiv.style.display = 'block';
    }
}

function displayResponse(response) {
    const responseDiv = document.getElementById('response');
    if (responseDiv) {
        responseDiv.textContent = JSON.stringify(response, null, 2);
        responseDiv.style.display = 'block';
    }
}

function updateLoginStatus() {
    const loginStatus = document.getElementById('loginStatus');
    const connectButton = document.getElementById('connectButton');
    const logoutButton = document.getElementById('logoutButton');
    const getUserProfileButton = document.getElementById('getUserProfileButton');
    const loadWalletButton = document.getElementById('loadWalletButton');
    const idToken = localStorage.getItem('id_token');
    const userEmail = localStorage.getItem('user_email');

    // Clear the previous status
    while (loginStatus && loginStatus.firstChild) {
        loginStatus.removeChild(loginStatus.firstChild);
    }

    if (idToken && loginStatus) {
        const responseCard = document.createElement('div');
        responseCard.className = 'response-card';
        responseCard.style.color = 'green';

        const userEmailPara = document.createElement('p');
        userEmailPara.textContent = `Logged in as: ${userEmail}`;
        responseCard.appendChild(userEmailPara);

        const tokenIdPara = document.createElement('p');
        tokenIdPara.textContent = `Token ID: ${idToken}`;
        responseCard.appendChild(tokenIdPara);

        loginStatus.appendChild(responseCard);

        if (connectButton) connectButton.style.display = 'none';
        if (logoutButton) logoutButton.style.display = 'block';
        if (getUserProfileButton) getUserProfileButton.style.display = 'block';
        if (loadWalletButton) loadWalletButton.style.display = 'block';
    } else {
        if (connectButton) connectButton.style.display = 'block';
        if (logoutButton) logoutButton.style.display = 'none';
        if (getUserProfileButton) getUserProfileButton.style.display = 'none';
        if (loadWalletButton) loadWalletButton.style.display = 'none';
    }
}

function handleResponse(response) {
    try {
        const responseData = JSON.parse(response);
        if (responseData.success) {
            localStorage.setItem('id_token', responseData.data.id_token);
            updateLoginStatus();
            document.getElementById('loginPage').style.display = 'none';
            document.getElementById('rewardsPage').style.display = 'block';
        } else {
            const responseElement = document.createElement('p');
            responseElement.style.color = 'red';
            responseElement.textContent = responseData.error;
            document.querySelector('.modal .container').appendChild(responseElement);
        }
    } catch (error) {
        console.error('Error parsing JSON response:', error);
        displayMessage('Error parsing server response', 'error');
    }
}

function handleProfileResponse(response) {
    try {
        const responseData = JSON.parse(response);
        const profileDiv = document.getElementById('profileData');
        // Clear previous content
        while (profileDiv.firstChild) {
            profileDiv.removeChild(profileDiv.firstChild);
        }

        // Create profile hero section
        const profileHeader = document.querySelector('.profile-header');
        profileHeader.querySelector('.profile-circle').style.backgroundImage = `url(${responseData.profile.avatar_uri || 'default-avatar.png'})`;
        profileHeader.querySelector('#profileName').textContent = responseData.profile.display_name || '';
        profileHeader.querySelector('#profileUsername').textContent = `@${responseData.profile.username || ''}`;

        // Create profile data list
        const profileList = document.createElement('ul');

        // Helper function to create list items
        const createListItem = (key, value) => {
            if (value !== null && value !== '') {
                const listItem = document.createElement('li');
                listItem.textContent = `${key}: ${value}`;
                profileList.appendChild(listItem);
            }
        };

        createListItem('Email', responseData.profile.email);
        createListItem('Role', responseData.profile.role);
        createListItem('Location', responseData.profile.location);
        createListItem('Biography', responseData.profile.biography);
        createListItem('Created At', responseData.profile.created_at);
        createListItem('Omnikey ID', responseData.profile.omnikey_id);

        // Handle nested objects
        if (responseData.profile.affiliate_referral.length > 0) {
            const affiliateList = document.createElement('ul');
            responseData.profile.affiliate_referral.forEach(referral => {
                const referralItem = document.createElement('li');
                referralItem.textContent = `Reference ID: ${referral.reference_id}, Type: ${referral.reference_type}, Detail: ${JSON.stringify(referral.reference_detail)}`;
                affiliateList.appendChild(referralItem);
            });
            profileList.appendChild(affiliateList);
        }

        profileDiv.appendChild(profileList);
        showProfilePage();
    } catch (error) {
        console.error('Error parsing JSON response:', error);
        displayMessage('Error parsing server response', 'error');
    }
}

// Expose the handleResponse and handleProfileResponse functions to the global scope
Object.assign(window, {
    handleResponse,
    handleProfileResponse
});

document.addEventListener('DOMContentLoaded', function () {
    const connectButton = document.getElementById('connectButton');
    const logoutButton = document.getElementById('logoutButton');
    const getUserProfileButton = document.getElementById('getUserProfileButton');
    const modal = document.getElementById('loginPage');
    const loginForm = document.getElementById('loginForm');
    const browserBridge = BrowserBridge.getInstance();
    const userButton = document.getElementById('userButton');
    const dropdownMenu = document.getElementById('dropdownMenu');
    const backButton = document.getElementById('backButton');

    if (connectButton) {
        connectButton.addEventListener('click', function () {
            // Dummy action for the Connect button
            alert('Connect button clicked');
        });
    }


    if (window) {
        window.addEventListener('click', function (event) {
            if (event.target === modal) {
                if (modal) modal.style.display = 'none';
            }
        });
    }

    if (loginForm) {
        loginForm.addEventListener('submit', function(event) {
            event.preventDefault();
            const submitButton = loginForm.querySelector('button[type="submit"]');
            if (submitButton) {
                submitButton.textContent = 'Submitting...';
            }

            const email = document.querySelector('input[name="uname"]').value;
            const password = document.querySelector('input[name="psw"]').value;

            localStorage.setItem('user_email', email);

            browserBridge.loginWallet(email, password);
        });
    }

    if (logoutButton) {
        logoutButton.addEventListener('click', function() {
            localStorage.removeItem('id_token');
            localStorage.removeItem('user_email');
            updateLoginStatus();
            showLoginPage();
        });
    }

    if (getUserProfileButton) {
        getUserProfileButton.addEventListener('click', function() {
            const idToken = localStorage.getItem('id_token');
            if (idToken) {
                browserBridge.getUserProfile(idToken);
            } else {
                alert('Please log in first.');
            }
        });
    }

    if (backButton) {
        backButton.addEventListener('click', function() {
            showRewardsPage();
        });
    }

    if (localStorage.getItem('id_token')) {
        showRewardsPage();
    } else {
        showLoginPage();
    }
});

function showLoginPage() {
    const loginPage = document.getElementById('loginPage');
    const rewardsPage = document.getElementById('rewardsPage');
    const profilePage = document.getElementById('profilePage');
    if (loginPage) loginPage.style.display = 'flex';
    if (rewardsPage) rewardsPage.style.display = 'none';
    if (profilePage) profilePage.style.display = 'none';
}

function showRewardsPage() {
    const loginPage = document.getElementById('loginPage');
    const rewardsPage = document.getElementById('rewardsPage');
    const profilePage = document.getElementById('profilePage');
    if (loginPage) loginPage.style.display = 'none';
    if (rewardsPage) rewardsPage.style.display = 'block';
    if (profilePage) profilePage.style.display = 'none';
}

function showProfilePage() {
    const loginPage = document.getElementById('loginPage');
    const rewardsPage = document.getElementById('rewardsPage');
    const profilePage = document.getElementById('profilePage');
    if (loginPage) loginPage.style.display = 'none';
    if (rewardsPage) rewardsPage.style.display = 'none';
    if (profilePage) profilePage.style.display = 'block';
}
