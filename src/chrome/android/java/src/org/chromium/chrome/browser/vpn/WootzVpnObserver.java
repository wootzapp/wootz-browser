package org.chromium.chrome.browser.vpn;

public interface WootzVpnObserver {
    public default void onGetAllServerRegions(String jsonServerRegions, boolean isSuccess) {}
    ;

    public default void onGetTimezonesForRegions(String jsonTimezones, boolean isSuccess) {}
    ;

    public default void onGetHostnamesForRegion(String jsonHostnames, boolean isSuccess) {}
    ;

    public default void onGetWireguardProfileCredentials(
            String jsonWireguardProfileCredentials, boolean isSuccess) {}
    ;

    public default void onVerifyCredentials(String jsonVerifyCredentials, boolean isSuccess) {}
    ;

    public default void onInvalidateCredentials(
            String jsonInvalidateCredentials, boolean isSuccess) {}
    ;

    public default void onGetSubscriberCredential(String subscriberCredential, boolean isSuccess) {}
    ;

    public default void onVerifyPurchaseToken(
            String jsonResponse, String purchaseToken, String productId, boolean isSuccess) {}
    ;

}