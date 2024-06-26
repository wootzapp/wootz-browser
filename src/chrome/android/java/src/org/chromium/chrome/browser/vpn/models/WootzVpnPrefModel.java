package org.chromium.chrome.browser.vpn.models;

public class WootzVpnPrefModel {
    private String mProductId;
    private String mClientId;
    private String mApiAuthToken;
    private String mHostname;
    private String mHostnameDisplay;
    private WootzVpnServerRegion mServerRegion;
    private String mClientPrivateKey;
    private String mClientPublicKey;

    public void setProductId(String productId) {
        mProductId = productId;
    }

    public void setClientId(String clientId) {
        mClientId = clientId;
    }

    public void setApiAuthToken(String apiAuthToken) {
        mApiAuthToken = apiAuthToken;
    }

    public void setHostname(String hostname) {
        mHostname = hostname;
    }

    public void setHostnameDisplay(String hostnameDisplay) {
        mHostnameDisplay = hostnameDisplay;
    }

    public void setServerRegion(WootzVpnServerRegion serverRegion) {
        mServerRegion = serverRegion;
    }

    public void setClientPrivateKey(String clientPrivateKey) {
        mClientPrivateKey = clientPrivateKey;
    }

    public void setClientPublicKey(String clientPublicKey) {
        mClientPublicKey = clientPublicKey;
    }

    public String getProductId() {
        return mProductId;
    }

    public String getClientId() {
        return mClientId;
    }

    public String getApiAuthToken() {
        return mApiAuthToken;
    }

    public String getHostname() {
        return mHostname;
    }

    public String getHostnameDisplay() {
        return mHostnameDisplay;
    }

    public WootzVpnServerRegion getServerRegion() {
        return mServerRegion;
    }

    public String getClientPrivateKey() {
        return mClientPrivateKey;
    }

    public String getClientPublicKey() {
        return mClientPublicKey;
    }
}