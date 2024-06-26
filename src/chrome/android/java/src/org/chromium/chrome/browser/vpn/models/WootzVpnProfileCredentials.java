package org.chromium.chrome.browser.vpn.models;

public class WootzVpnProfileCredentials {
    private String mApiAuthToken;
    private String mUsername;
    private String mPassword;

    public WootzVpnProfileCredentials(String apiAuthToken, String username, String password) {
        this.mApiAuthToken = apiAuthToken;
        this.mUsername = username;
        this.mPassword = password;
    }

    public String getApiAuthToken() {
        return mApiAuthToken;
    }

    public String getUsername() {
        return mUsername;
    }

    public String getPassword() {
        return mPassword;
    }
}