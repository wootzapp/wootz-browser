package org.chromium.chrome.browser.vpn;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import java.util.ArrayList;
import java.util.List;

@JNINamespace("chrome::android")
public class WootzAppVpnNativeWorker {
    private long mNativeWootzAppVpnNativeWorker;
    private static final Object sLock = new Object();
    private static WootzAppVpnNativeWorker sInstance;

    private List<WootzAppVpnObserver> mObservers;

    public static WootzAppVpnNativeWorker getInstance() {
        synchronized (sLock) {
            if (sInstance == null) {
                sInstance = new WootzAppVpnNativeWorker();
                sInstance.init();
            }
        }
        return sInstance;
    }

    private WootzAppVpnNativeWorker() {
        mObservers = new ArrayList<WootzAppVpnObserver>();
    }

    private void init() {
        if (mNativeWootzAppVpnNativeWorker == 0) {
            WootzAppVpnNativeWorkerJni.get().init(this);
        }
    }

    @Override
    protected void finalize() {
        destroy();
    }

    private void destroy() {
        if (mNativeWootzAppVpnNativeWorker != 0) {
            WootzAppVpnNativeWorkerJni.get().destroy(mNativeWootzAppVpnNativeWorker, this);
            mNativeWootzAppVpnNativeWorker = 0;
        }
    }

    public void addObserver(WootzAppVpnObserver observer) {
        synchronized (sLock) {
            mObservers.add(observer);
        }
    }

    public void removeObserver(WootzAppVpnObserver observer) {
        synchronized (sLock) {
            mObservers.remove(observer);
        }
    }

    @CalledByNative
    private void setNativePtr(long nativePtr) {
        assert mNativeWootzAppVpnNativeWorker == 0;
        mNativeWootzAppVpnNativeWorker = nativePtr;
    }

    @CalledByNative
    public void onGetAllServerRegions(String jsonServerRegions, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onGetAllServerRegions(jsonServerRegions, isSuccess);
        }
    }

    @CalledByNative
    public void onGetTimezonesForRegions(String jsonTimezones, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onGetTimezonesForRegions(jsonTimezones, isSuccess);
        }
    }

    @CalledByNative
    public void onGetHostnamesForRegion(String jsonHostnames, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onGetHostnamesForRegion(jsonHostnames, isSuccess);
        }
    }

    @CalledByNative
    public void onGetWireguardProfileCredentials(
            String jsonWireguardProfileCredentials, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onGetWireguardProfileCredentials(jsonWireguardProfileCredentials, isSuccess);
        }
    }

    @CalledByNative
    public void onVerifyCredentials(String jsonVerifyCredentials, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onVerifyCredentials(jsonVerifyCredentials, isSuccess);
        }
    }

    @CalledByNative
    public void onInvalidateCredentials(String jsonInvalidateCredentials, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onInvalidateCredentials(jsonInvalidateCredentials, isSuccess);
        }
    }

    @CalledByNative
    public void onGetSubscriberCredential(String subscriberCredential, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onGetSubscriberCredential(subscriberCredential, isSuccess);
        }
    }

    @CalledByNative
    public void onVerifyPurchaseToken(
            String jsonResponse, String purchaseToken, String productId, boolean isSuccess) {
        for (WootzAppVpnObserver observer : mObservers) {
            observer.onVerifyPurchaseToken(jsonResponse, purchaseToken, productId, isSuccess);
        }
    }

    public void getAllServerRegions() {
        WootzAppVpnNativeWorkerJni.get().getAllServerRegions(mNativeWootzAppVpnNativeWorker);
    }

    public void getTimezonesForRegions() {
        WootzAppVpnNativeWorkerJni.get().getTimezonesForRegions(mNativeWootzAppVpnNativeWorker);
    }

    public void getHostnamesForRegion(String region) {
        WootzAppVpnNativeWorkerJni.get().getHostnamesForRegion(mNativeWootzAppVpnNativeWorker, region);
    }

    public void getWireguardProfileCredentials(
            String subscriberCredential, String publicKey, String hostname) {
        WootzAppVpnNativeWorkerJni.get().getWireguardProfileCredentials(
                mNativeWootzAppVpnNativeWorker, subscriberCredential, publicKey, hostname);
    }

    public void verifyCredentials(
            String hostname, String clientId, String subscriberCredential, String apiAuthToken) {
        WootzAppVpnNativeWorkerJni.get().verifyCredentials(mNativeWootzAppVpnNativeWorker, hostname,
                clientId, subscriberCredential, apiAuthToken);
    }

    public void invalidateCredentials(
            String hostname, String clientId, String subscriberCredential, String apiAuthToken) {
        WootzAppVpnNativeWorkerJni.get().invalidateCredentials(mNativeWootzAppVpnNativeWorker, hostname,
                clientId, subscriberCredential, apiAuthToken);
    }

    public void getSubscriberCredential(String productType, String productId,
            String validationMethod, String purchaseToken, String packageName) {
        WootzAppVpnNativeWorkerJni.get().getSubscriberCredential(mNativeWootzAppVpnNativeWorker,
                productType, productId, validationMethod, purchaseToken, packageName);
    }

    public void verifyPurchaseToken(
            String purchaseToken, String productId, String productType, String packageName) {
        WootzAppVpnNativeWorkerJni.get().verifyPurchaseToken(
                mNativeWootzAppVpnNativeWorker, purchaseToken, productId, productType, packageName);
    }

    // Desktop purchase methods
    public void reloadPurchasedState() {
        WootzAppVpnNativeWorkerJni.get().reloadPurchasedState(mNativeWootzAppVpnNativeWorker);
    }

    public boolean isPurchasedUser() {
        return WootzAppVpnNativeWorkerJni.get().isPurchasedUser(mNativeWootzAppVpnNativeWorker);
    }

    public void getSubscriberCredentialV12() {
        WootzAppVpnNativeWorkerJni.get().getSubscriberCredentialV12(mNativeWootzAppVpnNativeWorker);
    }

    public void reportBackgroundP3A(long sessionStartTimeMs, long sessionEndTimeMs) {
        WootzAppVpnNativeWorkerJni.get().reportBackgroundP3A(
                mNativeWootzAppVpnNativeWorker, sessionStartTimeMs, sessionEndTimeMs);
    }

    public void reportForegroundP3A() {
        WootzAppVpnNativeWorkerJni.get().reportForegroundP3A(mNativeWootzAppVpnNativeWorker);
    }

    @NativeMethods
    interface Natives {
        void init(WootzAppVpnNativeWorker caller);
        void destroy(long nativeWootzAppVpnNativeWorker, WootzAppVpnNativeWorker caller);
        void getAllServerRegions(long nativeWootzAppVpnNativeWorker);
        void getTimezonesForRegions(long nativeWootzAppVpnNativeWorker);
        void getHostnamesForRegion(long nativeWootzAppVpnNativeWorker, String region);
        void getWireguardProfileCredentials(long nativeWootzAppVpnNativeWorker,
                String subscriberCredential, String publicKey, String hostname);
        void verifyCredentials(long nativeWootzAppVpnNativeWorker, String hostname, String clientId,
                String subscriberCredential, String apiAuthToken);
        void invalidateCredentials(long nativeWootzAppVpnNativeWorker, String hostname,
                String clientId, String subscriberCredential, String apiAuthToken);
        void getSubscriberCredential(long nativeWootzAppVpnNativeWorker, String productType,
                String productId, String validationMethod, String purchaseToken,
                String packageName);
        void verifyPurchaseToken(long nativeWootzAppVpnNativeWorker, String purchaseToken,
                String productId, String productType, String packageName);
        void reloadPurchasedState(long nativeWootzAppVpnNativeWorker);
        boolean isPurchasedUser(long nativeWootzAppVpnNativeWorker);
        void getSubscriberCredentialV12(long nativeWootzAppVpnNativeWorker);
        void reportBackgroundP3A(
                long nativeWootzAppVpnNativeWorker, long sessionStartTimeMs, long sessionEndTimeMs);
        void reportForegroundP3A(long nativeWootzAppVpnNativeWorker);
    }
}