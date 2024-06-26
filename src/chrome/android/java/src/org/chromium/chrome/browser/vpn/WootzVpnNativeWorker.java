package org.chromium.chrome.browser.vpn;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import java.util.ArrayList;
import java.util.List;

@JNINamespace("chrome::android")
public class WootzVpnNativeWorker {
    private long mNativeWootzVpnNativeWorker;
    private static final Object sLock = new Object();
    private static WootzVpnNativeWorker sInstance;

    private List<WootzVpnObserver> mObservers;

    public static WootzVpnNativeWorker getInstance() {
        synchronized (sLock) {
            if (sInstance == null) {
                sInstance = new WootzVpnNativeWorker();
                sInstance.init();
            }
        }
        return sInstance;
    }

    private WootzVpnNativeWorker() {
        mObservers = new ArrayList<WootzVpnObserver>();
    }

    private void init() {
        if (mNativeWootzVpnNativeWorker == 0) {
            WootzVpnNativeWorkerJni.get().init(this);
        }
    }

    @Override
    protected void finalize() {
        destroy();
    }

    private void destroy() {
        if (mNativeWootzVpnNativeWorker != 0) {
            WootzVpnNativeWorkerJni.get().destroy(mNativeWootzVpnNativeWorker, this);
            mNativeWootzVpnNativeWorker = 0;
        }
    }

    public void addObserver(WootzVpnObserver observer) {
        synchronized (sLock) {
            mObservers.add(observer);
        }
    }

    public void removeObserver(WootzVpnObserver observer) {
        synchronized (sLock) {
            mObservers.remove(observer);
        }
    }

    @CalledByNative
    private void setNativePtr(long nativePtr) {
        assert mNativeWootzVpnNativeWorker == 0;
        mNativeWootzVpnNativeWorker = nativePtr;
    }

    @CalledByNative
    public void onGetAllServerRegions(String jsonServerRegions, boolean isSuccess) {
        for (WootzVpnObserver observer : mObservers) {
            observer.onGetAllServerRegions(jsonServerRegions, isSuccess);
        }
    }

    @CalledByNative
    public void onGetTimezonesForRegions(String jsonTimezones, boolean isSuccess) {
        for (WootzVpnObserver observer : mObservers) {
            observer.onGetTimezonesForRegions(jsonTimezones, isSuccess);
        }
    }

    @CalledByNative
    public void onGetHostnamesForRegion(String jsonHostnames, boolean isSuccess) {
        for (WootzVpnObserver observer : mObservers) {
            observer.onGetHostnamesForRegion(jsonHostnames, isSuccess);
        }
    }

    // @CalledByNative
    // public void onGetWireguardProfileCredentials(
    //         String jsonWireguardProfileCredentials, boolean isSuccess) {
    //     for (WootzVpnObserver observer : mObservers) {
    //         observer.onGetWireguardProfileCredentials(jsonWireguardProfileCredentials,
    // isSuccess);
    //     }
    // }

    @CalledByNative
    public void onVerifyCredentials(String jsonVerifyCredentials, boolean isSuccess) {
        for (WootzVpnObserver observer : mObservers) {
            observer.onVerifyCredentials(jsonVerifyCredentials, isSuccess);
        }
    }

    @CalledByNative
    public void onInvalidateCredentials(String jsonInvalidateCredentials, boolean isSuccess) {
        for (WootzVpnObserver observer : mObservers) {
            observer.onInvalidateCredentials(jsonInvalidateCredentials, isSuccess);
        }
    }

    @CalledByNative
    public void onGetSubscriberCredential(String subscriberCredential, boolean isSuccess) {
        for (WootzVpnObserver observer : mObservers) {
            observer.onGetSubscriberCredential(subscriberCredential, isSuccess);
        }
    }

    @CalledByNative
    public void onVerifyPurchaseToken(
            String jsonResponse, String purchaseToken, String productId, boolean isSuccess) {
        for (WootzVpnObserver observer : mObservers) {
            observer.onVerifyPurchaseToken(jsonResponse, purchaseToken, productId, isSuccess);
        }
    }

    public void getAllServerRegions() {
        WootzVpnNativeWorkerJni.get().getAllServerRegions(mNativeWootzVpnNativeWorker);
    }

    public void getTimezonesForRegions() {
        WootzVpnNativeWorkerJni.get().getTimezonesForRegions(mNativeWootzVpnNativeWorker);
    }

    public void getHostnamesForRegion(String region) {
        WootzVpnNativeWorkerJni.get().getHostnamesForRegion(mNativeWootzVpnNativeWorker, region);
    }

    // public void getWireguardProfileCredentials(
    //         String subscriberCredential, String publicKey, String hostname) {
    //     WootzVpnNativeWorkerJni.get().getWireguardProfileCredentials(
    //             mNativeWootzVpnNativeWorker, subscriberCredential, publicKey, hostname);
    // }

    public void verifyCredentials(
            String hostname, String clientId, String subscriberCredential, String apiAuthToken) {
        WootzVpnNativeWorkerJni.get().verifyCredentials(mNativeWootzVpnNativeWorker, hostname,
                clientId, subscriberCredential, apiAuthToken);
    }

    public void invalidateCredentials(
            String hostname, String clientId, String subscriberCredential, String apiAuthToken) {
        WootzVpnNativeWorkerJni.get().invalidateCredentials(mNativeWootzVpnNativeWorker, hostname,
                clientId, subscriberCredential, apiAuthToken);
    }

    public void getSubscriberCredential(String productType, String productId,
            String validationMethod, String purchaseToken, String packageName) {
        WootzVpnNativeWorkerJni.get().getSubscriberCredential(mNativeWootzVpnNativeWorker,
                productType, productId, validationMethod, purchaseToken, packageName);
    }

    public void verifyPurchaseToken(
            String purchaseToken, String productId, String productType, String packageName) {
        WootzVpnNativeWorkerJni.get().verifyPurchaseToken(
                mNativeWootzVpnNativeWorker, purchaseToken, productId, productType, packageName);
    }

    // Desktop purchase methods
    public void reloadPurchasedState() {
        WootzVpnNativeWorkerJni.get().reloadPurchasedState(mNativeWootzVpnNativeWorker);
    }

    public boolean isPurchasedUser() {
        return WootzVpnNativeWorkerJni.get().isPurchasedUser(mNativeWootzVpnNativeWorker);
    }

    public void getSubscriberCredentialV12() {
        WootzVpnNativeWorkerJni.get().getSubscriberCredentialV12(mNativeWootzVpnNativeWorker);
    }

    public void reportBackgroundP3A(long sessionStartTimeMs, long sessionEndTimeMs) {
        WootzVpnNativeWorkerJni.get().reportBackgroundP3A(
                mNativeWootzVpnNativeWorker, sessionStartTimeMs, sessionEndTimeMs);
    }

    public void reportForegroundP3A() {
        WootzVpnNativeWorkerJni.get().reportForegroundP3A(mNativeWootzVpnNativeWorker);
    }

    @NativeMethods
    interface Natives {
        void init(WootzVpnNativeWorker caller);

        void destroy(long nativeWootzVpnNativeWorker, WootzVpnNativeWorker caller);

        void getAllServerRegions(long nativeWootzVpnNativeWorker);

        void getTimezonesForRegions(long nativeWootzVpnNativeWorker);

        void getHostnamesForRegion(long nativeWootzVpnNativeWorker, String region);

        // void getWireguardProfileCredentials(long nativeWootzVpnNativeWorker,
        //         String subscriberCredential, String publicKey, String hostname);
        void verifyCredentials(
                long nativeWootzVpnNativeWorker,
                String hostname,
                String clientId,
                String subscriberCredential,
                String apiAuthToken);

        void invalidateCredentials(
                long nativeWootzVpnNativeWorker,
                String hostname,
                String clientId,
                String subscriberCredential,
                String apiAuthToken);

        void getSubscriberCredential(
                long nativeWootzVpnNativeWorker,
                String productType,
                String productId,
                String validationMethod,
                String purchaseToken,
                String packageName);

        void verifyPurchaseToken(
                long nativeWootzVpnNativeWorker,
                String purchaseToken,
                String productId,
                String productType,
                String packageName);

        void reloadPurchasedState(long nativeWootzVpnNativeWorker);

        boolean isPurchasedUser(long nativeWootzVpnNativeWorker);

        void getSubscriberCredentialV12(long nativeWootzVpnNativeWorker);

        void reportBackgroundP3A(
                long nativeWootzVpnNativeWorker, long sessionStartTimeMs, long sessionEndTimeMs);

        void reportForegroundP3A(long nativeWootzVpnNativeWorker);
    }
}