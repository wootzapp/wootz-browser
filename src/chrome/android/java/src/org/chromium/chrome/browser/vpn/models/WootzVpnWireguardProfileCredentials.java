package org.chromium.chrome.browser.vpn.models;

public class WootzVpnWireguardProfileCredentials {
    private String mApiAuthToken;
    private String mClientId;
    private String mMappedIpv4Address;
    private String mMappedIpv6Address;
    private String mServerPublicKey;

    public WootzVpnWireguardProfileCredentials(String apiAuthToken, String clientId,
            String mappedIpv4Address, String mappedIpv6Address, String serverPublicKey) {
        this.mApiAuthToken = apiAuthToken;
        this.mClientId = clientId;
        this.mMappedIpv4Address = mappedIpv4Address;
        this.mMappedIpv6Address = mappedIpv6Address;
        this.mServerPublicKey = serverPublicKey;
    }

    public String getApiAuthToken() {
        return mApiAuthToken;
    }

    public String getClientId() {
        return mClientId;
    }

    public String getMappedIpv4Address() {
        return mMappedIpv4Address;
    }

    public String getMappedIpv6Address() {
        return mMappedIpv6Address;
    }

    public String getServerPublicKey() {
        return mServerPublicKey;
    }
}