class BrowserBridge {
    constructor() {}

    loginWallet(email, password) {
        chrome.send('loginWallet', [email, password]);
    }

    getUserProfile(token) {
        chrome.send('getUserProfile', [token]);
    }

    static getInstance() {
        if (!this.instance) {
            this.instance = new BrowserBridge();
        }
        return this.instance;
    }
}

// Define updateLoginStatus function
function updateLoginStatus() {
    const loginStatus = document.getElementById('loginStatus');
    const connectButton = document.getElementById('connectButton');
    const logoutButton = document.getElementById('logoutButton');
    const getUserProfileButton = document.getElementById('getUserProfileButton');
    const idToken = localStorage.getItem('id_token');
    const userEmail = localStorage.getItem('user_email');

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
    } else {
        connectButton.style.display = 'block';
        logoutButton.style.display = 'none';
        getUserProfileButton.style.display = 'none';
    }
}

// Define handleResponse function outside the event listener
function handleResponse(response) {
    const responseData = JSON.parse(response);
    if (responseData.success) {
        localStorage.setItem('id_token', responseData.data.id_token);
        localStorage.setItem('user_email', responseData.data.email);
        updateLoginStatus();
        document.getElementById('id01').style.display = 'none';
    } else {
        const responseElement = document.createElement('p');
        responseElement.style.color = 'red';
        responseElement.textContent = responseData.error;
        document.querySelector('.modal .container').appendChild(responseElement);
    }
}

// Define handleProfileResponse function
function handleProfileResponse(response) {
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
    const modal = document.getElementById('id01');
    const loginForm = document.getElementById('loginForm');
    const closeSpan = document.querySelector('.close');
    const browserBridge = BrowserBridge.getInstance();

    connectButton.addEventListener('click', function () {
        modal.style.display = 'block';
    });

    window.addEventListener('click', function (event) {
        if (event.target === modal) {
            modal.style.display = 'none';
        }
    });

    closeSpan.addEventListener('click', function () {
        modal.style.display = 'none';
    });

    loginForm.addEventListener('submit', function(event) {
        event.preventDefault();

        const email = document.querySelector('input[name="uname"]').value;
        const password = document.querySelector('input[name="psw"]').value;

        browserBridge.loginWallet(email, password);
    });

    logoutButton.addEventListener('click', function() {
        localStorage.removeItem('id_token');
        localStorage.removeItem('user_email');
        updateLoginStatus();
    });

    getUserProfileButton.addEventListener('click', function() {
        const idToken = localStorage.getItem('id_token');
        if (idToken) {
            browserBridge.getUserProfile(idToken);
        } else {
            alert('Please log in first.');
        }
    });

    updateLoginStatus();
});
