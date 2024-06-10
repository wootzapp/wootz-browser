package org.chromium.chrome.browser.vpn.models;

public class WootzVpnServerRegion {
    private String mContinent;
    private String mCountryIsoCode;
    private String mName;
    private String mNamePretty;

    public WootzVpnServerRegion(
            String continent, String countryIsoCode, String name, String namePretty) {
        this.mContinent = continent;
        this.mCountryIsoCode = countryIsoCode;
        this.mName = name;
        this.mNamePretty = namePretty;
    }

    public String getContinent() {
        return mContinent;
    }

    public String getCountryIsoCode() {
        return mCountryIsoCode;
    }

    public String getName() {
        return mName;
    }

    public String getNamePretty() {
        return mNamePretty;
    }
}