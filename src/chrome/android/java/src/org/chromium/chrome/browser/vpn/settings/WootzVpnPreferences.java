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
                Intent intent = GoBackend.VpnService.prepare(getActivity());
                // if (intent != null || !WireguardConfigUtils.isConfigExist(getActivity())) {
                //     WootzVpnUtils.dismissProgressDialog();
                //     WootzVpnUtils.openWootzVpnProfileActivity(getActivity());
                //     return;
                // }
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
        KeyPair keyPair = new KeyPair();
        mWootzVpnPrefModel.setClientPrivateKey(keyPair.getPrivateKey().toBase64());
        mWootzVpnPrefModel.setClientPublicKey(keyPair.getPublicKey().toBase64());
        Pair<String, String> host =
                WootzVpnApiResponseUtils.handleOnGetHostnamesForRegion(
                        getActivity(), mWootzVpnPrefModel, jsonHostNames, isSuccess);
        mWootzVpnPrefModel.setHostname(host.first);
        mWootzVpnPrefModel.setHostnameDisplay(host.second);
    }

    // @Override
    // public void onGetWireguardProfileCredentials(
    //         String jsonWireguardProfileCredentials, boolean isSuccess) {
    //     if (isSuccess && mWootzVpnPrefModel != null) {
    //         WootzVpnWireguardProfileCredentials wootzVpnWireguardProfileCredentials =
    //
    // WootzVpnUtils.getWireguardProfileCredentials(jsonWireguardProfileCredentials);
    //         // stopStartConnection(wootzVpnWireguardProfileCredentials);
    //     } else {
    //         Toast.makeText(getActivity(), R.string.vpn_profile_creation_failed,
    // Toast.LENGTH_LONG)
    //                 .show();
    //         WootzVpnUtils.dismissProgressDialog();
    //         new Handler().post(() -> updateSummaries());
    //     }
    // }

    // private void stopStartConnection(
    //         WootzVpnWireguardProfileCredentials wootzVpnWireguardProfileCredentials) {
    //     new Thread() {
    //         @Override
    //         public void run() {
    //             try {
    //                 if (WootzVpnProfileUtils.getInstance().isWootzVPNConnected(getActivity())) {
    //                     WootzVpnProfileUtils.getInstance().stopVpn(getActivity());
    //                 }
    //                 // WireguardConfigUtils.deleteConfig(getActivity());
    //                 // if (!WireguardConfigUtils.isConfigExist(getActivity())) {
    //                 //     WireguardConfigUtils.createConfig(getActivity(),
    //                 //             wootzVpnWireguardProfileCredentials.getMappedIpv4Address(),
    //                 //             mWootzVpnPrefModel.getHostname(),
    //                 //             mWootzVpnPrefModel.getClientPrivateKey(),
    //                 //             wootzVpnWireguardProfileCredentials.getServerPublicKey());
    //                 // }
    //                 WootzVpnProfileUtils.getInstance().startVpn(getActivity());
    //             } catch (Exception e) {
    //                 Log.e(TAG, e.getMessage());
    //             }
    //
    // mWootzVpnPrefModel.setClientId(wootzVpnWireguardProfileCredentials.getClientId());
    //             // mWootzVpnPrefModel.setApiAuthToken(
    //             //         wootzVpnWireguardProfileCredentials.getApiAuthToken());
    //             WootzVpnPrefUtils.setPrefModel(mWootzVpnPrefModel);
    //             new Handler(Looper.getMainLooper()).post(() -> updateSummaries());
    //         }
    //     }.start();
    // }

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
