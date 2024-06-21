package org.chromium.chrome.browser.vpn.activities;

import android.content.Intent;
import android.util.Pair;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;

import org.chromium.base.supplier.OneshotSupplier;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.init.ActivityProfileProvider;
import org.chromium.chrome.browser.init.AsyncInitializationActivity;
import org.chromium.chrome.browser.profiles.ProfileProvider;
import org.chromium.chrome.browser.vpn.WootzVpnObserver;
import org.chromium.chrome.browser.vpn.models.WootzVpnPrefModel;
import org.chromium.chrome.browser.vpn.utils.WootzVpnApiResponseUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnProfileUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnUtils;

public abstract class WootzVpnParentActivity extends AsyncInitializationActivity
        implements WootzVpnObserver {
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

    @Override
    public void onGetTimezonesForRegions(String jsonTimezones, boolean isSuccess) {
        WootzVpnApiResponseUtils.handleOnGetTimezonesForRegions(
                WootzVpnParentActivity.this, mWootzVpnPrefModel, jsonTimezones, isSuccess);
    }

    @Override
    protected OneshotSupplier<ProfileProvider> createProfileProvider() {
        return new ActivityProfileProvider(getLifecycleDispatcher());
    }
}