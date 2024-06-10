package org.chromium.chrome.browser.vpn.settings;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Pair;

import androidx.appcompat.app.AlertDialog;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;

import com.wireguard.android.backend.GoBackend;
import com.wireguard.crypto.KeyPair;

import org.chromium.base.BraveFeatureList;
import org.chromium.base.Log;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.InternetConnection;
import org.chromium.chrome.browser.customtabs.CustomTabActivity;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.settings.BravePreferenceFragment;
import org.chromium.chrome.browser.util.LiveDataUtil;
import org.chromium.chrome.browser.util.TabUtils;
import org.chromium.chrome.browser.vpn.BraveVpnNativeWorker;
import org.chromium.chrome.browser.vpn.BraveVpnObserver;
import org.chromium.chrome.browser.vpn.models.BraveVpnPrefModel;
import org.chromium.chrome.browser.vpn.models.BraveVpnWireguardProfileCredentials;
import org.chromium.chrome.browser.vpn.timer.TimerDialogFragment;
import org.chromium.chrome.browser.vpn.utils.BraveVpnApiResponseUtils;
import org.chromium.chrome.browser.vpn.utils.BraveVpnPrefUtils;
import org.chromium.chrome.browser.vpn.utils.BraveVpnProfileUtils;
import org.chromium.chrome.browser.vpn.utils.BraveVpnUtils;
import org.chromium.chrome.browser.vpn.wireguard.WireguardConfigUtils;
import org.chromium.components.browser_ui.settings.ChromeBasePreference;
import org.chromium.components.browser_ui.settings.ChromeSwitchPreference;
import org.chromium.components.browser_ui.settings.SettingsUtils;
import org.chromium.ui.widget.Toast;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class BraveVpnPreferences extends BravePreferenceFragment implements BraveVpnObserver {
    private static final String TAG = "BraveVPN";
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
            "https://support.brave.com/hc/en-us/articles/4410838268429";

    private static final String DATE_FORMAT = "dd/MM/yyyy";

    private ChromeSwitchPreference mVpnSwitch;
    private ChromeBasePreference mServerHost;
    private BraveVpnPrefModel mBraveVpnPrefModel;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        getActivity().setTitle(R.string.brave_firewall_vpn);
        SettingsUtils.addPreferencesFromResource(this, R.xml.brave_vpn_preferences);

        mVpnSwitch = (ChromeSwitchPreference) findPreference(PREF_VPN_SWITCH);
        mVpnSwitch.setChecked(
                BraveVpnProfileUtils.getInstance().isBraveVPNConnected(getActivity()));
        mVpnSwitch.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                if (mVpnSwitch != null) {
                    mVpnSwitch.setChecked(
                            BraveVpnProfileUtils.getInstance().isBraveVPNConnected(getActivity()));
                }
                if (BraveVpnProfileUtils.getInstance().isBraveVPNConnected(getActivity())) {
                    TimerDialogFragment timerDialogFragment = new TimerDialogFragment();
                    timerDialogFragment.show(
                            getActivity().getSupportFragmentManager(), TimerDialogFragment.TAG);
                } else {
                        if (WireguardConfigUtils.isConfigExist(getActivity())) {
                            BraveVpnProfileUtils.getInstance().startVpn(getActivity());
                        } else {
                            BraveVpnUtils.openBraveVpnProfileActivity(getActivity());
                        }
                    
                }
                return false;
            }
        });

        mServerHost = (ChromeBasePreference) findPreference(PREF_SERVER_HOST);

        findPreference(PREF_SUPPORT_TECHNICAL)
                .setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        BraveVpnUtils.openBraveVpnSupportActivity(getActivity());
                        return true;
                    }
                });

        findPreference(PREF_SUPPORT_VPN)
                .setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
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
                                BraveVpnUtils.openSplitTunnelActivity(getActivity());
                                return true;
                            }
                        });
        findPreference(PREF_ALWAYS_ON)
                .setOnPreferenceClickListener(
                        new Preference.OnPreferenceClickListener() {
                            @Override
                            public boolean onPreferenceClick(Preference preference) {
                                BraveVpnUtils.openAlwaysOnActivity(getActivity());
                                return true;
                            }
                        });
        findPreference(PREF_SERVER_CHANGE_LOCATION)
                .setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                        BraveVpnUtils.openVpnServerSelectionActivity(getActivity());
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
        if (BraveVpnUtils.mUpdateProfileAfterSplitTunnel) {
            BraveVpnUtils.mUpdateProfileAfterSplitTunnel = false;
            BraveVpnUtils.showProgressDialog(
                    getActivity(), getResources().getString(R.string.updating_vpn_profile));
            BraveVpnUtils.updateProfileConfiguration(getActivity());
        } else {
            BraveVpnUtils.dismissProgressDialog();
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
        updateSummary(PREF_SERVER_HOST, BraveVpnPrefUtils.getHostnameDisplay());
        updateSummary(PREF_SERVER_CHANGE_LOCATION, BraveVpnPrefUtils.getServerNamePretty());

        if (mVpnSwitch != null) {
            mVpnSwitch.setChecked(
                    BraveVpnProfileUtils.getInstance().isBraveVPNConnected(getActivity()));
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
                                            findPreference(PREF_SPLIT_TUNNELING)
                                                    .setEnabled(true);
                                            findPreference(PREF_ALWAYS_ON)
                                                    .setEnabled(true);
                                            findPreference(PREF_SUPPORT_TECHNICAL)
                                                    .setEnabled(true);
                                        }
                                    });
                }
            }
        }.start();
        BraveVpnUtils.dismissProgressDialog();
    }

    private final ConnectivityManager.NetworkCallback mNetworkCallback =
            new ConnectivityManager.NetworkCallback() {
                @Override
                public void onAvailable(Network network) {
                    BraveVpnUtils.dismissProgressDialog();
                    if (getActivity() != null) {
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                new Handler().post(() -> updateSummaries());
                            }
                        });
                    }
                }

                @Override
                public void onLost(Network network) {
                    BraveVpnUtils.dismissProgressDialog();
                    if (getActivity() != null) {
                        getActivity().runOnUiThread(new Runnable() {
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
                if (intent != null || !WireguardConfigUtils.isConfigExist(getActivity())) {
                    BraveVpnUtils.dismissProgressDialog();
                    BraveVpnUtils.openBraveVpnProfileActivity(getActivity());
                    return;
                }
                BraveVpnProfileUtils.getInstance().startVpn(getActivity());
            }
        }.start();
    }

    @Override
    public void onGetTimezonesForRegions(String jsonTimezones, boolean isSuccess) {
        BraveVpnApiResponseUtils.handleOnGetTimezonesForRegions(
                getActivity(), mBraveVpnPrefModel, jsonTimezones, isSuccess);
    }

    @Override
    public void onGetHostnamesForRegion(String jsonHostNames, boolean isSuccess) {
        KeyPair keyPair = new KeyPair();
        mBraveVpnPrefModel.setClientPrivateKey(keyPair.getPrivateKey().toBase64());
        mBraveVpnPrefModel.setClientPublicKey(keyPair.getPublicKey().toBase64());
        Pair<String, String> host = BraveVpnApiResponseUtils.handleOnGetHostnamesForRegion(
                getActivity(), mBraveVpnPrefModel, jsonHostNames, isSuccess);
        mBraveVpnPrefModel.setHostname(host.first);
        mBraveVpnPrefModel.setHostnameDisplay(host.second);
    }

    @Override
    public void onGetWireguardProfileCredentials(
            String jsonWireguardProfileCredentials, boolean isSuccess) {
        if (isSuccess && mBraveVpnPrefModel != null) {
            BraveVpnWireguardProfileCredentials braveVpnWireguardProfileCredentials =
                    BraveVpnUtils.getWireguardProfileCredentials(jsonWireguardProfileCredentials);
            stopStartConnection(braveVpnWireguardProfileCredentials);
        } else {
            Toast.makeText(getActivity(), R.string.vpn_profile_creation_failed, Toast.LENGTH_LONG)
                    .show();
            BraveVpnUtils.dismissProgressDialog();
            new Handler().post(() -> updateSummaries());
        }
    }

    private void stopStartConnection(
            BraveVpnWireguardProfileCredentials braveVpnWireguardProfileCredentials) {
        new Thread() {
            @Override
            public void run() {
                try {
                    if (BraveVpnProfileUtils.getInstance().isBraveVPNConnected(getActivity())) {
                        BraveVpnProfileUtils.getInstance().stopVpn(getActivity());
                    }
                    WireguardConfigUtils.deleteConfig(getActivity());
                    if (!WireguardConfigUtils.isConfigExist(getActivity())) {
                        WireguardConfigUtils.createConfig(getActivity(),
                                braveVpnWireguardProfileCredentials.getMappedIpv4Address(),
                                mBraveVpnPrefModel.getHostname(),
                                mBraveVpnPrefModel.getClientPrivateKey(),
                                braveVpnWireguardProfileCredentials.getServerPublicKey());
                    }
                    BraveVpnProfileUtils.getInstance().startVpn(getActivity());
                } catch (Exception e) {
                    Log.e(TAG, e.getMessage());
                }
                mBraveVpnPrefModel.setClientId(braveVpnWireguardProfileCredentials.getClientId());
                mBraveVpnPrefModel.setApiAuthToken(
                        braveVpnWireguardProfileCredentials.getApiAuthToken());
                BraveVpnPrefUtils.setPrefModel(mBraveVpnPrefModel);
                new Handler(Looper.getMainLooper()).post(() -> updateSummaries());
            }
        }.start();
    }

    @Override
    public void onStart() {
        super.onStart();
        BraveVpnNativeWorker.getInstance().addObserver(this);
    }

    @Override
    public void onStop() {
        BraveVpnNativeWorker.getInstance().removeObserver(this);
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
        BraveVpnNativeWorker.getInstance().invalidateCredentials(BraveVpnPrefUtils.getHostname(),
                BraveVpnPrefUtils.getClientId(),
                BraveVpnPrefUtils.getApiAuthToken());
        BraveVpnUtils.showProgressDialog(
                getActivity(), getResources().getString(R.string.resetting_config));
        new Handler().postDelayed(() -> {
            if (isResumed()) {
                BraveVpnUtils.resetProfileConfiguration(getActivity());
                new Handler().post(() -> updateSummaries());
            }
        }, INVALIDATE_CREDENTIAL_TIMER_COUNT);
    }

    @Override
    public void onDestroy() {
        BraveVpnUtils.dismissProgressDialog();
        super.onDestroy();
    }
}