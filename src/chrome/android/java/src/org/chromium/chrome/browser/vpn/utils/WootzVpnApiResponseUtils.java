package org.chromium.chrome.browser.vpn.utils;

import android.app.Activity;
import android.text.TextUtils;
import android.util.Pair;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.util.LiveDataUtil;
import org.chromium.chrome.browser.vpn.WootzVpnNativeWorker;
import org.chromium.chrome.browser.vpn.models.WootzVpnPrefModel;
import org.chromium.chrome.browser.vpn.models.WootzVpnServerRegion;

import java.util.TimeZone;

public class WootzVpnApiResponseUtils {
    public static void handleOnGetTimezonesForRegions(Activity activity,
            WootzVpnPrefModel wootzVpnPrefModel, String jsonTimezones, boolean isSuccess) {
        if (isSuccess) {
            WootzVpnServerRegion wootzVpnServerRegion =
                    WootzVpnUtils.getServerRegionForTimeZone(
                            jsonTimezones, TimeZone.getDefault().getID());
            String region = wootzVpnServerRegion.getName();
            if (TextUtils.isEmpty(region)) {
                WootzVpnUtils.showToast(String.format(
                        activity.getResources().getString(R.string.couldnt_get_matching_timezone),
                        TimeZone.getDefault().getID()));
                return;
            }
            if (WootzVpnUtils.selectedServerRegion != null) {
                if (!WootzVpnUtils.selectedServerRegion
                        .getName()
                        .equals(WootzVpnPrefUtils.PREF_WOOTZ_VPN_AUTOMATIC)) {
                    region = WootzVpnUtils.selectedServerRegion.getName();
                    wootzVpnServerRegion = WootzVpnUtils.selectedServerRegion;
                }
                WootzVpnUtils.selectedServerRegion = null;
            } else {
                String serverRegion = WootzVpnPrefUtils.getServerRegion();
                region = serverRegion.equals(WootzVpnPrefUtils.PREF_WOOTZ_VPN_AUTOMATIC)
                        ? region
                        : serverRegion;
            }

            WootzVpnNativeWorker.getInstance().getHostnamesForRegion(region);
            wootzVpnPrefModel.setServerRegion(wootzVpnServerRegion);
        } else {
            WootzVpnUtils.showToast(
                    activity.getResources().getString(R.string.vpn_profile_creation_failed));
            WootzVpnUtils.dismissProgressDialog();
        }
    }

    public static Pair<String, String> handleOnGetHostnamesForRegion(Activity activity,
            WootzVpnPrefModel wootzVpnPrefModel, String jsonHostNames, boolean isSuccess) {
        Pair<String, String> host = new Pair<String, String>("", "");
        if (isSuccess && wootzVpnPrefModel != null) {
            host = WootzVpnUtils.getHostnameForRegion(jsonHostNames);
            WootzVpnNativeWorker.getInstance().getWireguardProfileCredentials(
                    wootzVpnPrefModel.getClientPublicKey(), host.first);
        } else {
            WootzVpnUtils.showToast(
                    activity.getResources().getString(R.string.vpn_profile_creation_failed));
            WootzVpnUtils.dismissProgressDialog();
        }
        return host;
    }
}