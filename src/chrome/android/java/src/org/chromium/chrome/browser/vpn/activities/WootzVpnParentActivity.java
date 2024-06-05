package org.chromium.chrome.browser.vpn.activities;

import android.content.Intent;
import android.os.Handler;
import android.util.Pair;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import com.wireguard.android.backend.GoBackend;
import com.wireguard.crypto.KeyPair;

import org.chromium.base.Log;
import org.chromium.base.supplier.OneshotSupplier;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.billing.InAppPurchaseWrapper;
import org.chromium.chrome.browser.billing.PurchaseModel;
import org.chromium.chrome.browser.init.ActivityProfileProvider;
import org.chromium.chrome.browser.init.AsyncInitializationActivity;
import org.chromium.chrome.browser.profiles.ProfileProvider;
import org.chromium.chrome.browser.util.LiveDataUtil;
import org.chromium.chrome.browser.vpn.WootzVpnNativeWorker;
import org.chromium.chrome.browser.vpn.WootzVpnObserver;
import org.chromium.chrome.browser.vpn.models.WootzVpnPrefModel;
import org.chromium.chrome.browser.vpn.models.WootzVpnWireguardProfileCredentials;
import org.chromium.chrome.browser.vpn.utils.WootzVpnApiResponseUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnPrefUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnProfileUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnUtils;
import org.chromium.chrome.browser.vpn.wireguard.WireguardConfigUtils;

public abstract class WootzVpnParentActivity
        extends AsyncInitializationActivity implements WootzVpnObserver {
    private static final String TAG = "WootzVPN";
    public boolean mIsVerification;
    protected WootzVpnPrefModel mWootzVpnPrefModel;
    protected boolean mIsServerLocationChanged;

    private static final int INVALIDATE_CREDENTIAL_TIMER_COUNT = 5000;

    abstract void showRestoreMenu(boolean shouldShowRestore);
    abstract void updateProfileView();

    ActivityResultLauncher<Intent> mIntentActivityResultLauncher = registerForActivityResult(
            new ActivityResultContracts.StartActivityForResult(), getActivityResultRegistry(),
            result -> {
                WootzVpnUtils.dismissProgressDialog();
                if (result.getResultCode() == RESULT_OK) {
                    WootzVpnProfileUtils.getInstance().startVpn(WootzVpnParentActivity.this);
                    WootzVpnUtils.showVpnConfirmDialog(this);
                } else if (result.getResultCode() == RESULT_CANCELED) {
                    if (WootzVpnProfileUtils.getInstance().isVPNRunning(this)) {
                        WootzVpnUtils.showVpnAlwaysOnErrorDialog(this);
                    } else {
                        updateProfileView();
                    }
                    WootzVpnUtils.showToast(
                            getResources().getString(R.string.permission_was_cancelled));
                }
            });

    @Override
    public void finishNativeInitialization() {
        super.finishNativeInitialization();
    }

    @Override
    protected void onDestroy() {
        WootzVpnUtils.dismissProgressDialog();
        super.onDestroy();
    }

    protected void verifySubscription() {
        mWootzVpnPrefModel = new WootzVpnPrefModel();
        MutableLiveData<PurchaseModel> _activePurchases = new MutableLiveData();
        LiveData<PurchaseModel> activePurchases = _activePurchases;
        InAppPurchaseWrapper.getInstance()
                .queryPurchases(_activePurchases, InAppPurchaseWrapper.SubscriptionProduct.VPN);
        LiveDataUtil.observeOnce(
                activePurchases,
                activePurchaseModel -> {
                    if (activePurchaseModel != null) {
                        mWootzVpnPrefModel.setPurchaseToken(activePurchaseModel.getPurchaseToken());
                        mWootzVpnPrefModel.setProductId(activePurchaseModel.getProductId());
                        WootzVpnNativeWorker.getInstance()
                                .verifyPurchaseToken(
                                        mWootzVpnPrefModel.getPurchaseToken(),
                                        mWootzVpnPrefModel.getProductId(),
                                        WootzVpnUtils.SUBSCRIPTION_PARAM_TEXT,
                                        getPackageName());
                    } else {
                        if (!mIsVerification) {
                            WootzVpnApiResponseUtils.queryPurchaseFailed(
                                    WootzVpnParentActivity.this);
                            mIsServerLocationChanged = false;
                        } else {
                            showRestoreMenu(false);
                        }
                        WootzVpnUtils.dismissProgressDialog();
                    }
                });
    }

    @Override
    public void onVerifyPurchaseToken(
            String jsonResponse, String purchaseToken, String productId, boolean isSuccess) {
        if (isSuccess && mWootzVpnPrefModel != null) {
            Long purchaseExpiry = WootzVpnUtils.getPurchaseExpiryDate(jsonResponse);
            int paymentState = WootzVpnUtils.getPaymentState(jsonResponse);
            if (purchaseExpiry > 0 && purchaseExpiry >= System.currentTimeMillis()) {
                WootzVpnPrefUtils.setPurchaseToken(purchaseToken);
                WootzVpnPrefUtils.setProductId(productId);
                WootzVpnPrefUtils.setPurchaseExpiry(purchaseExpiry);
                WootzVpnPrefUtils.setSubscriptionPurchase(true);
                WootzVpnPrefUtils.setPaymentState(paymentState);
                if (!mIsVerification || WootzVpnPrefUtils.isResetConfiguration()) {
                    WootzVpnNativeWorker.getInstance().getSubscriberCredential(
                            WootzVpnUtils.SUBSCRIPTION_PARAM_TEXT,
                            mWootzVpnPrefModel.getProductId(), WootzVpnUtils.IAP_ANDROID_PARAM_TEXT,
                            mWootzVpnPrefModel.getPurchaseToken(), getPackageName());
                } else {
                    mIsVerification = false;
                    showRestoreMenu(true);
                    WootzVpnUtils.showToast(getResources().getString(R.string.already_subscribed));
                    WootzVpnUtils.dismissProgressDialog();
                }
            } else {
                WootzVpnApiResponseUtils.queryPurchaseFailed(WootzVpnParentActivity.this);
                mIsServerLocationChanged = false;
                if (mIsVerification) {
                    mIsVerification = false;
                    showRestoreMenu(false);
                    WootzVpnUtils.dismissProgressDialog();
                } else {
                    WootzVpnUtils.openWootzVpnPlansActivity(WootzVpnParentActivity.this);
                }
            }
        } else {
            WootzVpnUtils.dismissProgressDialog();
        }
    };

    @Override
    public void onGetSubscriberCredential(String subscriberCredential, boolean isSuccess) {
        mWootzVpnPrefModel.setSubscriberCredential(subscriberCredential);
        WootzVpnApiResponseUtils.handleOnGetSubscriberCredential(
                WootzVpnParentActivity.this, isSuccess);
    };

    @Override
    public void onGetTimezonesForRegions(String jsonTimezones, boolean isSuccess) {
        WootzVpnApiResponseUtils.handleOnGetTimezonesForRegions(
                WootzVpnParentActivity.this, mWootzVpnPrefModel, jsonTimezones, isSuccess);
    }

    @Override
    public void onGetHostnamesForRegion(String jsonHostNames, boolean isSuccess) {
        KeyPair keyPair = new KeyPair();
        mWootzVpnPrefModel.setClientPrivateKey(keyPair.getPrivateKey().toBase64());
        mWootzVpnPrefModel.setClientPublicKey(keyPair.getPublicKey().toBase64());
        Pair<String, String> host = WootzVpnApiResponseUtils.handleOnGetHostnamesForRegion(
                WootzVpnParentActivity.this, mWootzVpnPrefModel, jsonHostNames, isSuccess);
        mWootzVpnPrefModel.setHostname(host.first);
        mWootzVpnPrefModel.setHostnameDisplay(host.second);
    }

    @Override
    public void onGetWireguardProfileCredentials(
            String jsonWireguardProfileCredentials, boolean isSuccess) {
        if (isSuccess && mWootzVpnPrefModel != null) {
            WootzVpnWireguardProfileCredentials wootzVpnWireguardProfileCredentials =
                    WootzVpnUtils.getWireguardProfileCredentials(jsonWireguardProfileCredentials);

            int timerCount = 0;
            if (mIsServerLocationChanged) {
                timerCount = INVALIDATE_CREDENTIAL_TIMER_COUNT;
                mIsServerLocationChanged = false;
                try {
                    WootzVpnNativeWorker.getInstance()
                            .invalidateCredentials(
                                    WootzVpnPrefUtils.getHostname(),
                                    WootzVpnPrefUtils.getClientId(),
                                    WootzVpnPrefUtils.getSubscriberCredential(),
                                    WootzVpnPrefUtils.getApiAuthToken());
                } catch (Exception ex) {
                    Log.e(TAG, ex.getMessage());
                }
            }

            new Handler()
                    .postDelayed(
                            () -> {
                                checkForVpn(wootzVpnWireguardProfileCredentials);
                            },
                            timerCount);
        } else {
            WootzVpnUtils.showToast(getResources().getString(R.string.vpn_profile_creation_failed));
            WootzVpnUtils.dismissProgressDialog();
        }
    }

    private void checkForVpn(
            WootzVpnWireguardProfileCredentials wootzVpnWireguardProfileCredentials) {
        new Thread() {
            @Override
            public void run() {
                try {
                    if (WootzVpnProfileUtils.getInstance()
                            .isWootzVPNConnected(WootzVpnParentActivity.this)) {
                        WootzVpnProfileUtils.getInstance().stopVpn(WootzVpnParentActivity.this);
                    }
                    if (WireguardConfigUtils.isConfigExist(getApplicationContext())) {
                        WireguardConfigUtils.deleteConfig(getApplicationContext());
                    }
                    WireguardConfigUtils.createConfig(
                            getApplicationContext(),
                            wootzVpnWireguardProfileCredentials.getMappedIpv4Address(),
                            mWootzVpnPrefModel.getHostname(),
                            mWootzVpnPrefModel.getClientPrivateKey(),
                            wootzVpnWireguardProfileCredentials.getServerPublicKey());

                    mWootzVpnPrefModel.setClientId(
                            wootzVpnWireguardProfileCredentials.getClientId());
                    mWootzVpnPrefModel.setApiAuthToken(
                            wootzVpnWireguardProfileCredentials.getApiAuthToken());
                    WootzVpnPrefUtils.setPrefModel(mWootzVpnPrefModel);

                    WootzVpnUtils.dismissProgressDialog();
                    Intent intent = GoBackend.VpnService.prepare(WootzVpnParentActivity.this);
                    if (intent != null) {
                        mIntentActivityResultLauncher.launch(intent);
                        return;
                    }
                    WootzVpnProfileUtils.getInstance().startVpn(WootzVpnParentActivity.this);
                    finish();
                } catch (Exception e) {
                    WootzVpnUtils.dismissProgressDialog();
                    Log.e(TAG, e.getMessage());
                }
            }
        }.start();
    }

    @Override
    protected OneshotSupplier<ProfileProvider> createProfileProvider() {
        return new ActivityProfileProvider(getLifecycleDispatcher());
    }
}