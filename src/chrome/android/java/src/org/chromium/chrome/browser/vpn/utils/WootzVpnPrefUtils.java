package org.chromium.chrome.browser.vpn.utils;

import org.chromium.base.ContextUtils;
import org.chromium.chrome.browser.preferences.ChromeSharedPreferences;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.chrome.browser.vpn.models.WootzVpnPrefModel;

import java.util.Collections;
import java.util.Set;

public class WootzVpnPrefUtils {
    private static final String PREF_WOOTZ_VPN_CALLOUT = "wootz_vpn_callout";
    private static final String PREF_WOOTZ_VPN_HOSTNAME = "wootz_vpn_hostname";
    private static final String PREF_WOOTZ_VPN_HOSTNAME_DISPLAY = "wootz_vpn_hostname_display";
    private static final String PREF_WOOTZ_VPN_SERVER_REGIONS = "wootz_vpn_server_regions";
    private static final String PREF_WOOTZ_VPN_SERVER_CHANGE_LOCATION = "server_change_location";
    private static final String PREF_WOOTZ_VPN_SERVER_ISO_CODE = "server_iso_code";
    private static final String PREF_WOOTZ_VPN_SERVER_NAME_PRETTY = "server_name_pretty";
    private static final String PREF_WOOTZ_VPN_RESET_CONFIGURATION = "wootz_vpn_reset_configuration";
    private static final String PREF_EXCLUDED_PACKAGES = "excluded_packages";
    public static final String PREF_WOOTZ_VPN_AUTOMATIC = "automatic";
    public static final String PREF_WOOTZ_VPN_START = "wootz_vpn_start";
    public static final String PREF_WOOTZ_VPN_API_AUTH_TOKEN = "wootz_vpn_api_auth_token";
    public static final String PREF_WOOTZ_VPN_CLIENT_ID = "wootz_vpn_client_id";
    public static final String PREF_WOOTZ_VPN_SERVER_PUBLIC_KEY = "wootz_vpn_server_public_key";
    public static final String PREF_WOOTZ_VPN_IP_ADDRESS = "wootz_vpn_ip_address";
    public static final String PREF_WOOTZ_VPN_CLIENT_PRIVATE_KEY = "wootz_vpn_client_private_key";
    public static final String PREF_SESSION_START_TIME = "wootz_vpn_session_start_time";
    public static final String PREF_SESSION_END_TIME = "wootz_vpn_session_end_time";
    private static final String PREF_VPN_ISO_CODE_UPGRADE = "vpn_iso_code_upgrade";

    public static boolean isIsoCodeUpgradeDone() {
        return ChromeSharedPreferences.getInstance().readBoolean(PREF_VPN_ISO_CODE_UPGRADE, false);
    }

    public static void setIsoCodeUpgrade(boolean newValue) {
        ChromeSharedPreferences.getInstance().writeBoolean(PREF_VPN_ISO_CODE_UPGRADE, newValue);
    }

    public static boolean shouldShowCallout() {
        return ChromeSharedPreferences.getInstance().readBoolean(PREF_WOOTZ_VPN_CALLOUT, true);
    }

    public static void setCallout(boolean newValue) {
        ChromeSharedPreferences.getInstance().writeBoolean(PREF_WOOTZ_VPN_CALLOUT, newValue);
    }

    public static boolean isResetConfiguration() {
        return ChromeSharedPreferences.getInstance().readBoolean(PREF_WOOTZ_VPN_RESET_CONFIGURATION, false);
    }

    public static void setResetConfiguration(boolean newValue) {
        ChromeSharedPreferences.getInstance().writeBoolean(PREF_WOOTZ_VPN_RESET_CONFIGURATION, newValue);
    }

    public static void setHostname(String value) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_HOSTNAME, value);
    }

    public static String getHostname() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_HOSTNAME, "");
    }

    public static void setHostnameDisplay(String value) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_HOSTNAME_DISPLAY, value);
    }

    public static String getHostnameDisplay() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_HOSTNAME_DISPLAY, "");
    }

    public static String getServerRegion() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_SERVER_CHANGE_LOCATION, PREF_WOOTZ_VPN_AUTOMATIC);
    }

    private static void setServerRegion(String newValue) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_SERVER_CHANGE_LOCATION, newValue);
    }

    public static String getServerIsoCode() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_SERVER_ISO_CODE, "");
    }

    public static void setServerIsoCode(String newValue) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_SERVER_ISO_CODE, newValue);
    }

    public static String getServerNamePretty() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_SERVER_NAME_PRETTY, "");
    }

    public static void setServerNamePretty(String newValue) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_SERVER_NAME_PRETTY, newValue);
    }

    public static void setApiAuthToken(String value) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_API_AUTH_TOKEN, value);
    }

    public static String getApiAuthToken() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_API_AUTH_TOKEN, "");
    }

    public static void setClientId(String value) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_CLIENT_ID, value);
    }

    public static String getClientId() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_CLIENT_ID, "");
    }

    public static void setIpAddress(String value) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_IP_ADDRESS, value);
    }

    public static String getIpAddress() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_IP_ADDRESS, "");
    }

    public static void setServerPublicKey(String value) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_SERVER_PUBLIC_KEY, value);
    }

    public static String getServerPublicKey() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_SERVER_PUBLIC_KEY, "");
    }

    public static void setClientPrivateKey(String value) {
        ChromeSharedPreferences.getInstance().writeString(PREF_WOOTZ_VPN_CLIENT_PRIVATE_KEY, value);
    }

    public static String getClientPrivateKey() {
        return ChromeSharedPreferences.getInstance().readString(PREF_WOOTZ_VPN_CLIENT_PRIVATE_KEY, "");
    }

    public static void setPrefModel(WootzVpnPrefModel wootzVpnPrefModel) {
        setHostname(wootzVpnPrefModel.getHostname());
        setHostnameDisplay(wootzVpnPrefModel.getHostnameDisplay());
        setServerRegion(wootzVpnPrefModel.getServerRegion().getName());
        setServerIsoCode(wootzVpnPrefModel.getServerRegion().getCountryIsoCode());
        setServerNamePretty(wootzVpnPrefModel.getServerRegion().getNamePretty());
        setClientId(wootzVpnPrefModel.getClientId());
        setApiAuthToken(wootzVpnPrefModel.getApiAuthToken());
        setResetConfiguration(false);
    }

    public static Set<String> getExcludedPackages() {
        return ChromeSharedPreferences.getInstance().readStringSet(PREF_EXCLUDED_PACKAGES, Collections.emptySet());
    }

    public static void setExcludedPackages(Set<String> packages) {
        ChromeSharedPreferences.getInstance().writeStringSet(PREF_EXCLUDED_PACKAGES, packages);
    }

    public static void setSessionEndTimeMs(long timeMs) {
        ChromeSharedPreferences.getInstance().writeLong(PREF_SESSION_END_TIME, timeMs);
    }

    public static long getSessionEndTimeMs() {
        return ChromeSharedPreferences.getInstance().readLong(PREF_SESSION_END_TIME, -1);
    }

    public static void setSessionStartTimeMs(long timeMs) {
        ChromeSharedPreferences.getInstance().writeLong(PREF_SESSION_START_TIME, timeMs);
    }

    public static long getSessionStartTimeMs() {
        return ChromeSharedPreferences.getInstance().readLong(PREF_SESSION_START_TIME, -1);
    }
}