package org.chromium.chrome.browser.vpn.utils;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.text.TextUtils;
import android.util.Pair;

import androidx.fragment.app.FragmentActivity;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.chromium.base.ContextUtils;
import org.chromium.base.Log;
import org.chromium.chrome.browser.WootzRewardsNativeWorker;
import org.chromium.chrome.browser.vpn.WootzVpnNativeWorker;
import org.chromium.chrome.browser.vpn.activities.VpnAlwaysOnActivity;
import org.chromium.chrome.browser.vpn.activities.VpnServerSelectionActivity;
import org.chromium.chrome.browser.vpn.activities.WootzVpnProfileActivity;
import org.chromium.chrome.browser.vpn.activities.WootzVpnSupportActivity;
import org.chromium.chrome.browser.vpn.fragments.WootzVpnAlwaysOnErrorDialogFragment;
import org.chromium.chrome.browser.vpn.fragments.WootzVpnConfirmDialogFragment;
import org.chromium.chrome.browser.vpn.models.WootzVpnServerRegion;
import org.chromium.chrome.browser.vpn.split_tunnel.SplitTunnelActivity;
import org.chromium.gms.ChromiumPlayServicesAvailability;
import org.chromium.ui.widget.Toast;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class WootzVpnUtils {
    private static final String TAG = "WootzVPN";
    public static final String IAP_ANDROID_PARAM_TEXT = "iap-android";
    public static final String VERIFY_CREDENTIALS_FAILED = "verify_credentials_failed";
    public static final String DESKTOP_CREDENTIAL = "desktop_credential";

    public static boolean mUpdateProfileAfterSplitTunnel;
    public static WootzVpnServerRegion selectedServerRegion;
    private static ProgressDialog sProgressDialog;

    public static String IS_KILL_SWITCH = "is_kill_switch";

    public static void openWootzVpnProfileActivity(Activity activity) {
        if (activity == null) {
            return;
        }
        Intent wootzVpnProfileIntent = new Intent(activity, WootzVpnProfileActivity.class);
        wootzVpnProfileIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        wootzVpnProfileIntent.setAction(Intent.ACTION_VIEW);
        activity.startActivity(wootzVpnProfileIntent);
    }

    public static void openWootzVpnSupportActivity(Activity activity) {
        if (activity == null) {
            return;
        }
        Intent wootzVpnSupportIntent = new Intent(activity, WootzVpnSupportActivity.class);
        wootzVpnSupportIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        wootzVpnSupportIntent.setAction(Intent.ACTION_VIEW);
        activity.startActivity(wootzVpnSupportIntent);
    }

    public static void openSplitTunnelActivity(Activity activity) {
        if (activity == null) {
            return;
        }
        Intent wootzVpnSupportIntent = new Intent(activity, SplitTunnelActivity.class);
        wootzVpnSupportIntent.setAction(Intent.ACTION_VIEW);
        activity.startActivity(wootzVpnSupportIntent);
    }

    public static void openAlwaysOnActivity(Activity activity) {
        if (activity == null) {
            return;
        }
        Intent vpnAlwaysOnActivityIntent = new Intent(activity, VpnAlwaysOnActivity.class);
        activity.startActivity(vpnAlwaysOnActivityIntent);
    }

    public static void openVpnSettings(Activity activity) {
        if (activity == null) {
            return;
        }
        Intent vpnSettingsIntent = new Intent("android.net.vpn.SETTINGS");
        vpnSettingsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        activity.startActivity(vpnSettingsIntent);
    }

    public static void openVpnServerSelectionActivity(Activity activity) {
        if (activity == null) {
            return;
        }
        Intent vpnServerSelectionIntent = new Intent(activity, VpnServerSelectionActivity.class);
        activity.startActivity(vpnServerSelectionIntent);
    }

    public static void showProgressDialog(Activity activity, String message) {
        sProgressDialog = ProgressDialog.show(activity, "", message, true);
    }

    public static void dismissProgressDialog() {
        if (sProgressDialog != null && sProgressDialog.isShowing()) {
            sProgressDialog.dismiss();
        }
    }

    public static WootzVpnServerRegion getServerRegionForTimeZone(
            String jsonTimezones, String currentTimezone) {
        // Add root element to make it real JSON, otherwise getJSONArray cannot parse it
        jsonTimezones = "{\"regions\":" + jsonTimezones + "}";
        try {
            JSONObject result = new JSONObject(jsonTimezones);
            JSONArray regions = result.getJSONArray("regions");
            for (int i = 0; i < regions.length(); i++) {
                JSONObject region = regions.getJSONObject(i);
                JSONArray timezones = region.getJSONArray("timezones");
                for (int j = 0; j < timezones.length(); j++) {
                    if (timezones.getString(j).equals(currentTimezone)) {
                        WootzVpnServerRegion wootzVpnServerRegion =
                                new WootzVpnServerRegion(
                                        region.getString("continent"),
                                                region.getString("country-iso-code"),
                                        region.getString("name"), region.getString("name-pretty"));
                        return wootzVpnServerRegion;
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(TAG, "WootzVpnUtils -> getRegionForTimeZone JSONException error " + e);
        }
        return null;
    }

    public static Pair<String, String> getHostnameForRegion(String jsonHostnames) {
        jsonHostnames = "{\"hostnames\":" + jsonHostnames + "}";
        try {
            JSONObject result = new JSONObject(jsonHostnames);
            JSONArray hostnames = result.getJSONArray("hostnames");
            ArrayList<JSONObject> hosts = new ArrayList<JSONObject>();
            for (int i = 0; i < hostnames.length(); i++) {
                JSONObject hostname = hostnames.getJSONObject(i);
                if (hostname.getInt("capacity-score") == 0
                        || hostname.getInt("capacity-score") == 1) {
                    hosts.add(hostname);
                }
            }

            JSONObject hostname;
            if (hosts.size() < 2) {
                hostname = hostnames.getJSONObject(new Random().nextInt(hostnames.length()));
            } else {
                hostname = hosts.get(new Random().nextInt(hosts.size()));
            }
            return new Pair<>(hostname.getString("hostname"), hostname.getString("display-name"));
        } catch (JSONException e) {
            Log.e(TAG, "WootzVpnUtils -> getHostnameForRegion JSONException error " + e);
        }
        return new Pair<String, String>("", "");
    }

    // public static WootzVpnWireguardProfileCredentials getWireguardProfileCredentials(
    //         String jsonWireguardProfileCredentials) {
    //     try {
    //         JSONObject wireguardProfileCredentials =
    //                 new JSONObject(jsonWireguardProfileCredentials);
    //         WootzVpnWireguardProfileCredentials wootzVpnWireguardProfileCredentials =
    //                 new WootzVpnWireguardProfileCredentials(
    //                         wireguardProfileCredentials.getString("api-auth-token"),
    //                         wireguardProfileCredentials.getString("client-id"),
    //                         wireguardProfileCredentials.getString("mapped-ipv4-address"),
    //                         wireguardProfileCredentials.getString("mapped-ipv6-address"),
    //                         wireguardProfileCredentials.getString("server-public-key"));
    //         return wootzVpnWireguardProfileCredentials;
    //     } catch (JSONException e) {
    //         Log.e(TAG, "WootzVpnUtils -> getWireguardProfileCredentials JSONException error " +
    // e);
    //     }
    //     return null;
    // }

    public static List<WootzVpnServerRegion> getServerLocations(String jsonServerLocations) {
        List<WootzVpnServerRegion> vpnServerRegions = new ArrayList<>();
        if (TextUtils.isEmpty(jsonServerLocations)) {
            return vpnServerRegions;
        }
        jsonServerLocations = "{\"servers\":" + jsonServerLocations + "}";
        try {
            JSONObject result = new JSONObject(jsonServerLocations);
            JSONArray servers = result.getJSONArray("servers");
            for (int i = 0; i < servers.length(); i++) {
                JSONObject server = servers.getJSONObject(i);
                WootzVpnServerRegion vpnServerRegion =
                        new WootzVpnServerRegion(
                                server.getString("continent"), server.getString("country-iso-code"),
                                server.getString("name"), server.getString("name-pretty"));
                vpnServerRegions.add(vpnServerRegion);
            }
        } catch (JSONException e) {
            Log.e(TAG, "WootzVpnUtils -> getServerLocations JSONException error " + e);
        }
        return vpnServerRegions;
    }

    public static void resetProfileConfiguration(Activity activity) {
        if (WootzVpnProfileUtils.getInstance().isWootzVPNConnected(activity)) {
            WootzVpnProfileUtils.getInstance().stopVpn(activity);
        }
        try {
            // WireguardConfigUtils.deleteConfig(activity);
        } catch (Exception ex) {
            Log.e(TAG, "resetProfileConfiguration : " + ex.getMessage());
        }
        WootzVpnPrefUtils.setResetConfiguration(true);
        dismissProgressDialog();
    }

    public static void updateProfileConfiguration(Activity activity) {
        try {
            // Config existingConfig = WireguardConfigUtils.loadConfig(activity);
            // WireguardConfigUtils.deleteConfig(activity);
            // WireguardConfigUtils.createConfig(activity, existingConfig);
        } catch (Exception ex) {
            Log.e(TAG, "updateProfileConfiguration : " + ex.getMessage());
        }
        if (WootzVpnProfileUtils.getInstance().isWootzVPNConnected(activity)) {
            WootzVpnProfileUtils.getInstance().stopVpn(activity);
            WootzVpnProfileUtils.getInstance().startVpn(activity);
        }
        dismissProgressDialog();
    }

    public static void showVpnAlwaysOnErrorDialog(Activity activity) {
        WootzVpnAlwaysOnErrorDialogFragment mWootzVpnAlwaysOnErrorDialogFragment =
                new WootzVpnAlwaysOnErrorDialogFragment();
        mWootzVpnAlwaysOnErrorDialogFragment.show(
                ((FragmentActivity) activity).getSupportFragmentManager(),
                "WootzVpnAlwaysOnErrorDialogFragment");
    }

    public static void showVpnConfirmDialog(Activity activity) {
        WootzVpnConfirmDialogFragment wootzVpnConfirmDialogFragment =
                new WootzVpnConfirmDialogFragment();
        wootzVpnConfirmDialogFragment.show(
                ((FragmentActivity) activity).getSupportFragmentManager(),
                "WootzVpnConfirmDialogFragment");
    }

    public static void reportBackgroundUsageP3A() {
        // Will report previous/current session timestamps...
        WootzVpnNativeWorker.getInstance().reportBackgroundP3A(
                WootzVpnPrefUtils.getSessionStartTimeMs(), WootzVpnPrefUtils.getSessionEndTimeMs());
        // ...and then reset the timestamps so we don't report the same session again.
        WootzVpnPrefUtils.setSessionStartTimeMs(-1);
        WootzVpnPrefUtils.setSessionEndTimeMs(-1);
    }

    public static void showToast(String message) {
        Context context = ContextUtils.getApplicationContext();
        Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
    }

    private static boolean isRegionSupported() {
        WootzRewardsNativeWorker wootzRewardsNativeWorker = WootzRewardsNativeWorker.getInstance();
        return (wootzRewardsNativeWorker != null && wootzRewardsNativeWorker.isSupported());
    }

    public static boolean isVpnFeatureSupported(Context context) {
        return isRegionSupported()
                && ChromiumPlayServicesAvailability.isGooglePlayServicesAvailable(context);
    }

    public static String countryCodeToEmoji(String countryCode) {
        int firstLetter = Character.codePointAt(countryCode, 0) - 0x41 + 0x1F1E6;
        int secondLetter = Character.codePointAt(countryCode, 1) - 0x41 + 0x1F1E6;
        return new String(Character.toChars(firstLetter))
                + new String(Character.toChars(secondLetter));
    }
}