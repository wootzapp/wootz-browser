class BrowserBridge {
    constructor() {}

    loginWallet(email, password) {
        // Bypass actual login
        console.log('loginWallet called with:', email, password); // Placeholder
        handleResponse(JSON.stringify({ success: true, data: { id_token: 'bypass_token' } }));
    }

    getUserProfile(token) {
        // Bypass actual profile fetching
        console.log('getUserProfile called with:', token); // Placeholder
        handleProfileResponse(JSON.stringify({ success: true, data: { profile: 'User Profile Data' } }));
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
    messageDiv.textContent = message;
    messageDiv.className = type; // Add classes like 'success' or 'error' to style appropriately
    messageDiv.style.display = 'block';
}

function displayResponse(response) {
    const responseDiv = document.getElementById('response');
    responseDiv.textContent = JSON.stringify(response, null, 2);
    responseDiv.style.display = 'block';
}

function updateLoginStatus() {
    const loginStatus = document.getElementById('loginStatus');
    const connectButton = document.getElementById('connectButton');
    const logoutButton = document.getElementById('logoutButton');
    const getUserProfileButton = document.getElementById('getUserProfileButton');
    const loadWalletButton = document.getElementById('loadWalletButton');
    // Bypass actual token and email
    const idToken = 'bypass_token';
    const userEmail = 'bypass_email';

    // Clear the previous status
    while (loginStatus.firstChild) {
        loginStatus.removeChild(loginStatus.firstChild);
    }

    if (idToken) {
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

        connectButton.style.display = 'none';
        logoutButton.style.display = 'block';
        getUserProfileButton.style.display = 'block';
        loadWalletButton.style.display = 'block';
    } else {
        connectButton.style.display = 'block';
        logoutButton.style.display = 'none';
        getUserProfileButton.style.display = 'none';
        loadWalletButton.style.display = 'none';
    }
}

function handleResponse(response) {
    try {
        const responseData = JSON.parse(response);
        if (responseData.success) {
            // Bypass saving the actual token
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
        const profileDiv = document.createElement('div');
        profileDiv.className = 'response-card';

        if (responseData.success) {
            profileDiv.style.color = 'green';

            const profileDataPara = document.createElement('p');
            profileDataPara.textContent = 'Profile Data:';
            profileDiv.appendChild(profileDataPara);

            const profileDataPre = document.createElement('pre');
            profileDataPre.textContent = JSON.stringify(responseData.data, null, 2);
            profileDiv.appendChild(profileDataPre);
        } else {
            profileDiv.style.color = 'blue';
            profileDiv.textContent = JSON.stringify(responseData);
        }

        document.body.appendChild(profileDiv);
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
    const loadWalletButton = document.getElementById('loadWalletButton');
    const modal = document.getElementById('loginPage');
    const loginForm = document.getElementById('loginForm');
    const browserBridge = BrowserBridge.getInstance();
    const userButton = document.getElementById('userButton');

    if (connectButton) {
        connectButton.addEventListener('click', function () {
            if (modal) modal.style.display = 'block';
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

            const email = document.querySelector('input[name="uname"]').value;
            const password = document.querySelector('input[name="psw"]').value;

            localStorage.setItem('user_email', email);

            browserBridge.loginWallet(email, password);
        });
    }

    if (logoutButton) {
        logoutButton.addEventListener('click', function() {
            updateLoginStatus();
            showLoginPage();
        });
    }

    if (getUserProfileButton) {
        getUserProfileButton.addEventListener('click', function() {
            const idToken = 'bypass_token'; // Bypass for now
            if (idToken) {
                browserBridge.getUserProfile(idToken);
            } else {
                alert('Please log in first.');
            }
        });
    }

    if (loadWalletButton) {
        loadWalletButton.addEventListener('click', async function() {
            const idToken = 'bypass_token'; // Bypass for now
            if (idToken) {
                await browserBridge.loadWallet(idToken);
            } else {
                alert('Please log in first.');
            }
        });
    }

    if (userButton) {
        userButton.addEventListener('click', function() {
            const existingUserProfileSection = document.querySelector('.user-profile');
            if (!existingUserProfileSection) {
                createUserProfileSection();
            }
        });
    }

    showRewardsPage();
});

function showLoginPage() {
    const loginPage = document.getElementById('loginPage');
    const rewardsPage = document.getElementById('rewardsPage');
    if (loginPage) loginPage.style.display = 'flex';
    if (rewardsPage) rewardsPage.style.display = 'none';
}

function showRewardsPage() {
    const loginPage = document.getElementById('loginPage');
    const rewardsPage = document.getElementById('rewardsPage');
    if (loginPage) loginPage.style.display = 'none';
    if (rewardsPage) rewardsPage.style.display = 'block';
}

function createUserProfileSection() {
    const userProfileSection = document.createElement('div');
    userProfileSection.className = 'user-profile';

    const tokenIdPara = document.createElement('p');
    tokenIdPara.textContent = `Token ID: bypass_token`; // Bypass for now
    userProfileSection.appendChild(tokenIdPara);

    const getUserProfileButton = document.createElement('button');
    getUserProfileButton.id = 'getUserProfileButton';
    getUserProfileButton.textContent = 'Get Profile';
    userProfileSection.appendChild(getUserProfileButton);

    const loadWalletButton = document.createElement('button');
    loadWalletButton.id = 'loadWalletButton';
    loadWalletButton.textContent = 'Load Wallet';
    userProfileSection.appendChild(loadWalletButton);

    const logoutButton = document.createElement('button');
    logoutButton.id = 'logoutButton';
    logoutButton.textContent = 'Logout';
    userProfileSection.appendChild(logoutButton);

    document.body.appendChild(userProfileSection);

    getUserProfileButton.addEventListener('click', function() {
        const idToken = 'bypass_token'; // Bypass for now
        if (idToken) {
            BrowserBridge.getInstance().getUserProfile(idToken);
        } else {
            alert('Please log in first.');
        }
    });

    loadWalletButton.addEventListener('click', async function() {
        const idToken = 'bypass_token'; // Bypass for now
        if (idToken) {
            await BrowserBridge.getInstance().loadWallet(idToken);
        } else {
            alert('Please log in first.');
        }
    });

    logoutButton.addEventListener('click', function() {
        showLoginPage();
    });
}
