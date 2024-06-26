package org.chromium.chrome.browser.vpn.settings;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Bundle;
import android.os.Handler;
import android.util.Pair;

import androidx.appcompat.app.AlertDialog;
import androidx.preference.Preference;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.InternetConnection;
import org.chromium.chrome.browser.customtabs.CustomTabActivity;
import org.chromium.chrome.browser.settings.WootzPreferenceFragment;
import org.chromium.chrome.browser.vpn.WootzVpnNativeWorker;
import org.chromium.chrome.browser.vpn.WootzVpnObserver;
import org.chromium.chrome.browser.vpn.models.WootzVpnPrefModel;
import org.chromium.chrome.browser.vpn.timer.TimerDialogFragment;
import org.chromium.chrome.browser.vpn.utils.WootzVpnApiResponseUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnPrefUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnProfileUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnUtils;
import org.chromium.components.browser_ui.settings.ChromeBasePreference;
import org.chromium.components.browser_ui.settings.ChromeSwitchPreference;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.ui.widget.Toast;

public class WootzVpnPreferences extends WootzPreferenceFragment implements WootzVpnObserver {
    private static final String TAG = "WootzVPN";
    public static final String PREF_VPN_SWITCH = "vpn_switch";
    public static final String PREF_SERVER_HOST = "server_host";
    public static final String PREF_SERVER_CHANGE_LOCATION = "server_change_location";
    public static final String PREF_SUPPORT_TECHNICAL = "support_technical";
    public static final String PREF_SUPPORT_VPN = "support_vpn";
    public static final String PREF_SERVER_RESET_CONFIGURATION = "server_reset_configuration";
    private static final String PREF_SPLIT_TUNNELING = "split_tunneling";
    private static final String PREF_ALWAYS_ON = "always_on";

    private static final int INVALIDATE_CREDENTIAL_TIMER_COUNT = 5000;

    private static final String VPN_SUPPORT_PAGE =
            "https://support.wootz.com/hc/en-us/articles/4410838268429";

    private static final String DATE_FORMAT = "dd/MM/yyyy";

    private ChromeSwitchPreference mVpnSwitch;
    private ChromeBasePreference mServerHost;
    private WootzVpnPrefModel mWootzVpnPrefModel;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        getActivity().setTitle(R.string.wootz_firewall_vpn);
        SettingsUtils.addPreferencesFromResource(this, R.xml.wootz_vpn_preferences);

        mVpnSwitch = (ChromeSwitchPreference) findPreference(PREF_VPN_SWITCH);
        mVpnSwitch.setChecked(
                WootzVpnProfileUtils.getInstance().isWootzVPNConnected(getActivity()));
        mVpnSwitch.setOnPreferenceClickListener(
                new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        if (mVpnSwitch != null) {
                            mVpnSwitch.setChecked(
                                    WootzVpnProfileUtils.getInstance()
                                            .isWootzVPNConnected(getActivity()));
                        }
                        if (WootzVpnProfileUtils.getInstance().isWootzVPNConnected(getActivity())) {
                            TimerDialogFragment timerDialogFragment = new TimerDialogFragment();
                            timerDialogFragment.show(
                                    getActivity().getSupportFragmentManager(),
                                    TimerDialogFragment.TAG);
                        } else {
                            // if (WireguardConfigUtils.isConfigExist(getActivity())) {
                            //     WootzVpnProfileUtils.getInstance().startVpn(getActivity());
                            // } else {
                            WootzVpnUtils.openWootzVpnProfileActivity(getActivity());
                            // }

                        }
                        return false;
                    }
                });

        mServerHost = (ChromeBasePreference) findPreference(PREF_SERVER_HOST);

        findPreference(PREF_SUPPORT_TECHNICAL)
                .setOnPreferenceClickListener(
                        new Preference.OnPreferenceClickListener() {
                            @Override
                            public boolean onPreferenceClick(Preference preference) {
                                WootzVpnUtils.openWootzVpnSupportActivity(getActivity());
                                return true;
                            }
                        });

        findPreference(PREF_SUPPORT_VPN)
                .setOnPreferenceClickListener(
                        new Preference.OnPreferenceClickListener() {
                            @Override
                            public boolean onPreferenceClick(Preference preference) {
                                CustomTabActivity.showInfoPage(getActivity(), VPN_SUPPORT_PAGE);
                                return true;
                            }
                        });

        findPreference(PREF_SERVER_RESET_CONFIGURATION)
                .setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        showConfirmDialog();
                        return true;
                    }
                });

        findPreference(PREF_SPLIT_TUNNELING)
                .setOnPreferenceClickListener(
                        new Preference.OnPreferenceClickListener() {
                            @Override
                            public boolean onPreferenceClick(Preference preference) {
                                WootzVpnUtils.openSplitTunnelActivity(getActivity());
                                return true;
                            }
                        });
        findPreference(PREF_ALWAYS_ON)
                .setOnPreferenceClickListener(
                        new Preference.OnPreferenceClickListener() {
                            @Override
                            public boolean onPreferenceClick(Preference preference) {
                                WootzVpnUtils.openAlwaysOnActivity(getActivity());
                                return true;
                            }
                        });
        findPreference(PREF_SERVER_CHANGE_LOCATION)
                .setOnPreferenceClickListener(
                        new Preference.OnPreferenceClickListener() {
                            @Override
                            public boolean onPreferenceClick(Preference preference) {
                                WootzVpnUtils.openVpnServerSelectionActivity(getActivity());
                                return true;
                            }
                        });
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (context != null) {
            ConnectivityManager connectivityManager =
                    (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkRequest networkRequest =
                    new NetworkRequest.Builder()
                            .addTransportType(NetworkCapabilities.TRANSPORT_VPN)
                            .removeCapability(NetworkCapabilities.NET_CAPABILITY_NOT_VPN)
                            .build();
            connectivityManager.registerNetworkCallback(networkRequest, mNetworkCallback);
        }
        if (!InternetConnection.isNetworkAvailable(getActivity())) {
            Toast.makeText(getActivity(), R.string.no_internet, Toast.LENGTH_SHORT).show();
            getActivity().finish();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (WootzVpnUtils.mUpdateProfileAfterSplitTunnel) {
            WootzVpnUtils.mUpdateProfileAfterSplitTunnel = false;
            WootzVpnUtils.showProgressDialog(
                    getActivity(), getResources().getString(R.string.updating_vpn_profile));
            WootzVpnUtils.updateProfileConfiguration(getActivity());
        } else {
            WootzVpnUtils.dismissProgressDialog();
        }
        new Handler().post(() -> updateSummaries());
    }

    private void updateSummary(String preferenceString, String summary) {
        Preference p = findPreference(preferenceString);
        p.setSummary(summary);
    }

    private void updateSummaries() {
        if (getActivity() == null) {
            return;
        }
        updateSummary(PREF_SERVER_HOST, WootzVpnPrefUtils.getHostnameDisplay());
        updateSummary(PREF_SERVER_CHANGE_LOCATION, WootzVpnPrefUtils.getServerNamePretty());

        if (mVpnSwitch != null) {
            mVpnSwitch.setChecked(
                    WootzVpnProfileUtils.getInstance().isWootzVPNConnected(getActivity()));
        }
        new Thread() {
            @Override
            public void run() {
                if (getActivity() != null) {
                    getActivity()
                            .runOnUiThread(
                                    new Runnable() {
                                        @Override
                                        public void run() {
                                            findPreference(PREF_SERVER_CHANGE_LOCATION)
                                                    .setEnabled(true);
                                            findPreference(PREF_SPLIT_TUNNELING).setEnabled(true);
                                            findPreference(PREF_ALWAYS_ON).setEnabled(true);
                                            findPreference(PREF_SUPPORT_TECHNICAL).setEnabled(true);
                                        }
                                    });
                }
            }
        }.start();
        WootzVpnUtils.dismissProgressDialog();
    }

    private final ConnectivityManager.NetworkCallback mNetworkCallback =
            new ConnectivityManager.NetworkCallback() {
                @Override
                public void onAvailable(Network network) {
                    WootzVpnUtils.dismissProgressDialog();
                    if (getActivity() != null) {
                        getActivity()
                                .runOnUiThread(
                                        new Runnable() {
                                            @Override
                                            public void run() {
                                                new Handler().post(() -> updateSummaries());
                                            }
                                        });
                    }
                }

                @Override
                public void onLost(Network network) {
                    WootzVpnUtils.dismissProgressDialog();
                    if (getActivity() != null) {
                        getActivity()
                                .runOnUiThread(
                                        new Runnable() {
                                            @Override
                                            public void run() {
                                                new Handler().post(() -> updateSummaries());
                                            }
                                        });
                    }
                }
            };

    private void checkVpnAfterVerification() {
        new Thread() {
            @Override
            public void run() {
                WootzVpnProfileUtils.getInstance().startVpn(getActivity());
            }
        }.start();
    }

    @Override
    public void onGetTimezonesForRegions(String jsonTimezones, boolean isSuccess) {
        WootzVpnApiResponseUtils.handleOnGetTimezonesForRegions(
                getActivity(), mWootzVpnPrefModel, jsonTimezones, isSuccess);
    }

    @Override
    public void onGetHostnamesForRegion(String jsonHostNames, boolean isSuccess) {
        String hardCodedPrivateKey = "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCAmMwggJfAgEAAoGBALk7B0E6G3O0Qm1JXFLP1vZwoCgEbk2eclz6eZZHQVOTB/Xg9U6+Jsm/IqDtNfx4miHFmRrX8L2AZazQpM8Vs2F2X8gYm1XB2cB+Y6gNEwZkSzsQLi8Rp8Pm8f0LSr/OW+0S6F8v1R7DJEDO9NQQkHj5EzO9oW5u0p9YtIw7j7VVAgMBAAECggEAXZ0AImdHffu4Wn4PiE/MRXCrk9HQ/XZ5x5eK84mOATlD4YO1aMJhHbJAIj63Kq3xt9yzF5PrtsFPOIGW/aEq/V9G73uNNVcVHKMGtZugBoF07FvLQHyQw75MNmOVv9l/8YOY8ZGsbemg4lHdEvEOVF0D1IobTylqu3jRB0FXBm0wJfZyH3UdoqL8YusHvJD3uM7VR5TAcD/R8UXrNfT7E8GzBdLbe/Efnf1EQuBwPQDZT69XsnA16OhWUB0ZGKO5Lds6O7k5/oJq/O5yRXrwFstWs9sewT0mj6dIRnHl1HXJ+DDRe2au+/8Ox4vZBdB4/FTAYJXH8Teh13+CC09FF3sPff/L5YQKBgQDULhrlqNphDJAADeQ5BmENmr1i1h9lfq2ZQGfBS6OtFBlbbjsdKcG0SR/qVxVFr0UmwEzvLy0W20clNPx3Z/C8yT8J7+nqx+mb+24ScZk2GiUEr1GJ5Wldw0W6mv/g9Hb5rISj/WmeIkayELx2CqzEHP3ScY44/8k8m5z2Tk6Y4SpwKBgQDNLb9gXagC2zU+MnxlBbSuvmCfZB1u1Ac6KHimyaeoySBbXw8Zy0l+Gg6nq7LyU6cavfUo1ZdJKgD8D9DxuUVmLppN4nDOHqfFLVpbW+dW+XqqbUHzfOvCm9HGBRIV8VfqUHPznnsD7NT2jLbKOr1w3geplPZSBjt+EB9Uoqh5MQKBgBDTbVJfQGm5M58DP0QyMELm+Pb7pFkHP6fHCL0en2G9nF93FipXEiqh2hfDjAsUnzzwmqNdp8k92dVRDb3l+e2Hr+MgIoKLzRafsf4EYVnb1FnOeQIG/lniGAXD1UzAcb4zLTqljFWvnqY2VVKeFcKk+/my7igkLfNo6+H5ApOFAoGBAKc+91Dz13RxGfQ4zWcbNyyHtwrpuqZVu9OMrxF5gG40ktK2mlhl+zAFOEQCT0mTbV4W+cFHTF6slg4x8VpYq4Ti25XvH3kYrTTJZhb20KNn17sz+35UDbBf9P6RlI4TSNjoKiNLMzvAN8Z0yyiTGX6xGPAYZDDkpOM95y5g8OylAoGAdNo+eQFz1Nsl2CMsCaHv4b8DclcS1Z4uXuZjvPCrP1SngmnYoQxM1RY8KkA5tJazSkZ0AJ34X5QRA5Y2L/j96vAF2pTXzOQFkO7ZDf8kiYkphTGR96v1ZpA3WCM6VRtROp0bJcJgU+VtKft31MzG68hxetRW10QtrMxWwD01mPf2M=";
        String hardCodedPublicKey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuTsHQTobc7RCbUlcUs/W9nCgKARuTZ5yXPp5lkdBU5MH9eD1Tr4myb8ioO01/HiaIcWZGtfwvYBlrNCkzxWzYXZfyBibVcHZwH5jqA0TBmRLOxAuLxGnw+bx/QtKv85b7RLoXy/VHsMkQM702BCQePkTM72hbm7Sn1i0jDuPtVUCAwEAAQ==";

        // Setting the hardcoded keys
        mWootzVpnPrefModel.setClientPrivateKey(hardCodedPrivateKey);
        mWootzVpnPrefModel.setClientPublicKey(hardCodedPublicKey);
        Pair<String, String> host =
                WootzVpnApiResponseUtils.handleOnGetHostnamesForRegion(
                        getActivity(), mWootzVpnPrefModel, jsonHostNames, isSuccess);
        mWootzVpnPrefModel.setHostname(host.first);
        mWootzVpnPrefModel.setHostnameDisplay(host.second);
    }

    @Override
    public void onStart() {
        super.onStart();
        WootzVpnNativeWorker.getInstance().addObserver(this);
    }

    @Override
    public void onStop() {
        WootzVpnNativeWorker.getInstance().removeObserver(this);
        super.onStop();
    }

    private void showConfirmDialog() {
        AlertDialog.Builder confirmDialog = new AlertDialog.Builder(getActivity());
        confirmDialog.setTitle(
                getActivity().getResources().getString(R.string.reset_vpn_config_dialog_title));
        confirmDialog.setMessage(
                getActivity().getResources().getString(R.string.reset_vpn_config_dialog_message));
        confirmDialog.setPositiveButton(
                getActivity().getResources().getString(android.R.string.yes), (dialog, which) -> {
                    resetConfiguration();
                    dialog.dismiss();
                });
        confirmDialog.setNegativeButton(getActivity().getResources().getString(android.R.string.no),
                (dialog, which) -> { dialog.dismiss(); });
        confirmDialog.show();
    }

    private void resetConfiguration() {
        WootzVpnNativeWorker.getInstance()
                .invalidateCredentials(
                        WootzVpnPrefUtils.getHostname(),
                        WootzVpnPrefUtils.getClientId(),
                        "subscriber",
                        WootzVpnPrefUtils.getApiAuthToken());
        WootzVpnUtils.showProgressDialog(
                getActivity(), getResources().getString(R.string.resetting_config));
        new Handler()
                .postDelayed(
                        () -> {
                            if (isResumed()) {
                                WootzVpnUtils.resetProfileConfiguration(getActivity());
                                new Handler().post(() -> updateSummaries());
                            }
                        },
                        INVALIDATE_CREDENTIAL_TIMER_COUNT);
    }

    @Override
    public void onDestroy() {
        WootzVpnUtils.dismissProgressDialog();
        super.onDestroy();
    }
}
